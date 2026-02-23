/*
 *   Copyright (C) 2010,2011,2026 by Jonathan Naylor G4KLX
 *   Copyright (c) 2021,2022 by Geoffrey Merck F4FXL / KC3FRA
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string>
#include <iostream>
#include <vector>
#include <signal.h>
#include <exception>
#include <cassert>
#include <unistd.h>
#ifdef DEBUG_DSTARGW
#include <boost/stacktrace.hpp>
#endif

#include "DStarGatewayDefs.h"
#include "DStarGatewayConfig.h"
#include "DStarGatewayApp.h"
#include "Version.h"
#include "IRCDDBMultiClient.h"
#include "IRCDDBClient.h"
#include "Utils.h"
#include "Version.h"
#include "GitVersion.h"
#include "RepeaterProtocolHandlerFactory.h"
#include "MQTTConnection.h"
#include "Log.h"
#include "APRSFixedIdFrameProvider.h"
#include "Daemon.h"
#include "APRSISHandlerThread.h"
#include "DummyAPRSHandlerThread.h"
#include "HostsFilesManager.h"

// In Log.cpp
extern CMQTTConnection* m_mqtt;

CDStarGatewayApp * CDStarGatewayApp::g_app = nullptr;
const char* BANNER_1 = " Copyright (C) ";
const char* BANNER_2 = "DStarGateway comes with ABSOLUTELY NO WARRANTY; see the LICENSE for details.";
const char* BANNER_3 = "This is free software, and you are welcome to distribute it under certain conditions that are discussed in the LICENSE file.";

#ifdef UNIT_TESTS
int fakemain(int argc, char *argv[])
#else
int main(int argc, char *argv[])
#endif
{
	std::set_terminate(CDStarGatewayApp::terminateHandler);

	signal(SIGSEGV, CDStarGatewayApp::sigHandlerFatal);
	signal(SIGILL, CDStarGatewayApp::sigHandlerFatal);
	signal(SIGFPE, CDStarGatewayApp::sigHandlerFatal);
	signal(SIGABRT, CDStarGatewayApp::sigHandlerFatal);
	signal(SIGTERM, CDStarGatewayApp::sigHandlerExit);
	signal(SIGINT, CDStarGatewayApp::sigHandlerExit);
	signal(SIGUSR1, CDStarGatewayApp::sigHandlerUSR);

	setbuf(stdout, NULL);
	if (2 != argc) {
		printf("usage: %s path_to_config_file\n", argv[0]);
		printf("       %s --version\n", argv[0]);
		return 1;
	}

	std::cout << std::endl << FULL_PRODUCT_NAME << BANNER_1 << VENDOR_NAME << std::endl << BANNER_2 << std::endl << BANNER_3 << std::endl << std::endl;

	if ('-' == argv[1][0]) {
		return 0;
	}

	CDStarGatewayConfig * config = new CDStarGatewayConfig(std::string((argv[1])));
	if(!config->load()) {
		LogFatal("Invalid configuration, aborting");
		return false;
	}

	TDaemon daemon;
	config->getDaemon(daemon);

	if (daemon.daemon) {
		LogInfo("Configured as a daemon, detaching ...");
		auto res = CDaemon::daemonise(daemon.pidFile, daemon.user);

		switch (res)
		{
			case DR_PARENT:
				return 0;
			case DR_CHILD:
				break;
			case DR_PIDFILE_FAILED:
			case DR_FAILURE:
			default:
				LogFatal("Failed to run as daemon");
				delete config;
				LogFinalise();
				return 1;
		}
	}

	// Setup Log
	TLog logConf;
	config->getLog(logConf);

	LogInitialise(logConf.displayLevel, logConf.mqttLevel);

	// Setup MQTT
	TMQTT mqttConf;
	config->getMQTT(mqttConf);

	std::vector<std::pair<std::string, void (*)(const unsigned char*, unsigned int)>> subscriptions;
	// if (m_conf.getRemoteCommandsEnabled())
	//	subscriptions.push_back(std::make_pair("command", CDStarGatewayApp::onCommand));

	m_mqtt = new CMQTTConnection(mqttConf.address, mqttConf.port, mqttConf.name, mqttConf.authenticate, mqttConf.username, mqttConf.password, subscriptions, mqttConf.keepalive);
	bool ret = m_mqtt->open();
	if (!ret)
		return 1;

	//write banner in log file if we are dameon
	if(daemon.daemon) {
		LogInfo(BANNER_1);
		LogInfo(BANNER_2);
		LogInfo(BANNER_3);
	}

	CDStarGatewayApp gateway(config);
	
	if (!gateway.init()) {
		return 1;
	}

	gateway.run();

	if(daemon.daemon) {
		CDaemon::finalise();
	}

	return 0;
}

CDStarGatewayApp::CDStarGatewayApp(CDStarGatewayConfig * config) :
m_config(config),
m_thread(NULL)
{
	assert(config != nullptr);
	g_app = this;
}

CDStarGatewayApp::~CDStarGatewayApp()
{
}

bool CDStarGatewayApp::init()
{
	return createThread();
}

void CDStarGatewayApp::run()
{
	m_thread->Run();
	m_thread->Wait();
	LogInfo("exiting\n");
	LogFinalise();
}

bool CDStarGatewayApp::createThread()
{
	LogDebug("Entering CDStarGatewayApp::createThread - Thread ID %s", THREAD_ID_STR(std::this_thread::get_id()));

	// Log
	TLog log;
	m_config->getLog(log);

	// Paths
	Tpaths paths;
	m_config->getPaths(paths);
	m_thread = new CDStarGatewayThread(paths.dataDir, "");

	// Setup the gateway
	TGateway gatewayConfig;
	m_config->getGateway(gatewayConfig);
	m_thread->setGateway(gatewayConfig.type, gatewayConfig.callsign, gatewayConfig.address);
	m_thread->setLanguage(gatewayConfig.language);
	m_thread->setLocation(gatewayConfig.latitude, gatewayConfig.longitude);

#ifdef USE_GPSD
	// Setup GPSD
	TGPSD gpsdConfig;
	m_config->getGPSD(gpsdConfig);
#endif

	// Setup APRS
	TAPRS aprsConfig;
	m_config->getAPRS(aprsConfig);
	CAPRSHandler * outgoingAprsWriter = nullptr;
	CAPRSHandler * incomingAprsWriter = nullptr;
	if (aprsConfig.enabled) {
		CAPRSISHandlerThread* aprsisthread = new CAPRSISHandlerThread(gatewayConfig.callsign);
		outgoingAprsWriter = new CAPRSHandler((IAPRSHandlerBackend *)aprsisthread);

		incomingAprsWriter = new CAPRSHandler((IAPRSHandlerBackend *)new CDummyAPRSHandlerBackend());

		if(outgoingAprsWriter->open()) {
#ifdef USE_GPSD
			CAPRSIdFrameProvider * idFrameProvider = aprsConfig.m_positionSource == POSSRC_GPSD ? (CAPRSIdFrameProvider *)new CAPRSGPSDIdFrameProvider(gatewayConfig.callsign, gpsdConfig.m_address, gpsdConfig.m_port)
																									: new CAPRSFixedIdFrameProvider(gatewayConfig.callsign);
#else
			CAPRSIdFrameProvider * idFrameProvider = new CAPRSFixedIdFrameProvider(gatewayConfig.callsign);
#endif
			idFrameProvider->start();
			outgoingAprsWriter->setIdFrameProvider(idFrameProvider);
			m_thread->setAPRSWriters(outgoingAprsWriter, incomingAprsWriter);
		}
		else {
			delete outgoingAprsWriter;
			outgoingAprsWriter = NULL;
		}
	}

	// Setup access control
	TAccessControl accessControl;
	m_config->getAccessControl(accessControl);

	CCallsignList * whiteList = new CCallsignList(accessControl.whiteList);
	if(whiteList->load() && whiteList->getCount() > 0U) {
		m_thread->setWhiteList(whiteList);
	}
	else {
		delete whiteList;
	}

	CCallsignList * blackList = new CCallsignList(accessControl.blackList);
	if(blackList->load() && blackList->getCount() > 0U) {
		m_thread->setBlackList(blackList);
	}
	else {
		delete blackList;
	}

	CCallsignList * restrictList = new CCallsignList(accessControl.restrictList);
	if(restrictList->load() && restrictList->getCount() > 0U) {
		m_thread->setRestrictList(restrictList);
	}
	else {
		delete restrictList;
	}

	// Drats
	TDRats drats;
	m_config->getDRats(drats);

	// Setup the repeaters
	bool ddEnabled = false;
	bool atLeastOneRepeater = false;
	CRepeaterProtocolHandlerFactory repeaterProtocolFactory;
	for(unsigned int i = 0U; i < m_config->getRepeaterCount(); i++) {
		LogDebug("Adding repeaters - CDStarGatewayApp::createThread - Rpt Idx %i - Thread ID %s", i, THREAD_ID_STR(std::this_thread::get_id()));
		TRepeater rptrConfig;
		m_config->getRepeater(i, rptrConfig);
		auto  repeaterProtocolHandler = repeaterProtocolFactory.getRepeaterProtocolHandler(rptrConfig.hwType, gatewayConfig, rptrConfig.address, rptrConfig.port);
		if(repeaterProtocolHandler == nullptr)
			continue;
		atLeastOneRepeater = true;
		m_thread->addRepeater(rptrConfig.callsign,
								rptrConfig.band,
								rptrConfig.address,
								rptrConfig.port,
								rptrConfig.hwType,
								rptrConfig.reflector,
								rptrConfig.reflectorAtStartup,
								rptrConfig.reflectorReconnect,
								drats.enabled,
								rptrConfig.frequency,
								rptrConfig.offset,
								rptrConfig.range,
								rptrConfig.latitude,
								rptrConfig.longitude,
								rptrConfig.agl,
								rptrConfig.description1,
								rptrConfig.description2,
								rptrConfig.url,
								repeaterProtocolHandler,
								rptrConfig.band1,
								rptrConfig.band2,
								rptrConfig.band3);

		if(outgoingAprsWriter != nullptr)
			outgoingAprsWriter->setPort(rptrConfig.callsign, rptrConfig.band, rptrConfig.frequency, rptrConfig.offset, rptrConfig.range, rptrConfig.latitude, rptrConfig.longitude, rptrConfig.agl);
		if(incomingAprsWriter != nullptr)
			incomingAprsWriter->setPort(rptrConfig.callsign, rptrConfig.band, rptrConfig.frequency, rptrConfig.offset, rptrConfig.range, rptrConfig.latitude, rptrConfig.longitude, rptrConfig.agl);

		if(!ddEnabled) ddEnabled = rptrConfig.band.length() > 1U;
	}

	LogDebug("Repeaters Added - CDStarGatewayApp::createThread - Thread ID %s", THREAD_ID_STR(std::this_thread::get_id()));

	if(!atLeastOneRepeater) {
		LogError("Error: no repeaters are enabled or opening network communication to repeater failed");
		return false;
	}

	m_thread->setDDModeEnabled(ddEnabled);
	LogInfo("DD Mode enabled: %d", int(ddEnabled));

	// Setup ircddb
	auto ircddbVersionInfo = "linux_" + PRODUCT_NAME + "-" + VERSION;
	std::vector<CIRCDDB *> clients;
	for(unsigned int i=0; i < m_config->getIrcDDBCount(); i++) {
		LogDebug("Adding Ircddb - CDStarGatewayApp::createThread - Ircddb  Idx %i - Thread ID %s", i, THREAD_ID_STR(std::this_thread::get_id()));
		TircDDB ircDDBConfig;
		m_config->getIrcDDB(i, ircDDBConfig);
		LogInfo("ircDDB Network %d set to %s user: %s, Quadnet %d", i + 1,ircDDBConfig.hostname.c_str(), ircDDBConfig.username.c_str(), ircDDBConfig.isQuadNet);
		CIRCDDB * ircDDB = new CIRCDDBClient(ircDDBConfig.hostname, 9007U, ircDDBConfig.username, ircDDBConfig.password, ircddbVersionInfo, gatewayConfig.address, ircDDBConfig.isQuadNet);
		clients.push_back(ircDDB);
	}
	LogDebug("Added Ircddb - CDStarGatewayApp::createThread - Ircddb  Count %i - Thread ID %s", clients.size(), THREAD_ID_STR(std::this_thread::get_id()));
	if(clients.size() > 0U) {
		CIRCDDBMultiClient* multiClient = new CIRCDDBMultiClient(clients);
		bool res = multiClient->open();
		if (!res) {
			LogError("Cannot initialise the ircDDB protocol handler\n");
			return false;
		}
		m_thread->setIRC(multiClient);
	}

	LogDebug("Setting Up Dextra CDStarGatewayApp::createThread - Thread ID %s", THREAD_ID_STR(std::this_thread::get_id()));
	// Setup Dextra
	TDextra dextraConfig;
	m_config->getDExtra(dextraConfig);
	LogInfo("DExtra enabled: %d, max. dongles: %u", int(dextraConfig.enabled), dextraConfig.maxDongles);
	m_thread->setDExtra(dextraConfig.enabled, dextraConfig.maxDongles);

	LogDebug("Setting Up DCS CDStarGatewayApp::createThread - Thread ID %s", THREAD_ID_STR(std::this_thread::get_id()));
	// Setup DCS
	TDCS dcsConfig;
	m_config->getDCS(dcsConfig);
	LogInfo("DCS enabled: %d", int(dcsConfig.enabled));
	m_thread->setDCS(dcsConfig.enabled);

	LogDebug("Setting Up DPlus CDStarGatewayApp::createThread - Thread ID %s", THREAD_ID_STR(std::this_thread::get_id()));
	// Setup DPlus
	TDplus dplusConfig;
	m_config->getDPlus(dplusConfig);
	LogInfo("D-Plus enabled: %d, max. dongles: %u, login: %s", int(dplusConfig.enabled), dplusConfig.maxDongles, dplusConfig.login.c_str());
	m_thread->setDPlus(dplusConfig.enabled, dplusConfig.maxDongles, dplusConfig.login);

	LogDebug("Setting Up XLX CDStarGatewayApp::createThread - Thread ID %s", THREAD_ID_STR(std::this_thread::get_id()));
	// Setup XLX
	TXLX xlxConfig;
	m_config->getXLX(xlxConfig);
	LogInfo("XLX enabled: %d", int(xlxConfig.enabled));
	m_thread->setXLX(xlxConfig.enabled);

	// Setup hostsfiles
	THostsFiles hostsFilesConfig;
	m_config->getHostsFiles(hostsFilesConfig);
	CHostsFilesManager::setHostFilesDirectories(hostsFilesConfig.downloadedHostFiles, hostsFilesConfig.customHostsFiles);
	CHostsFilesManager::setDownloadTimeout(3600 * hostsFilesConfig.downloadTimeout);
	CHostsFilesManager::setDextra(dextraConfig.enabled);
	CHostsFilesManager::setDCS(dcsConfig.enabled);
	CHostsFilesManager::setDPlus(dplusConfig.enabled);
	CHostsFilesManager::setXLX(xlxConfig.enabled);

	// Setup Remote
	TRemote remoteConfig;
	m_config->getRemote(remoteConfig);
	LogInfo("Remote enabled: %d, port %u", int(remoteConfig.enabled), remoteConfig.port);
	m_thread->setRemote(remoteConfig.enabled, remoteConfig.password, remoteConfig.port);

	// Get final things ready
	m_thread->setIcomRepeaterHandler(repeaterProtocolFactory.getIcomProtocolHandler());
	m_thread->setHBRepeaterHandler(repeaterProtocolFactory.getHBProtocolHandler());
	m_thread->setDummyRepeaterHandler(repeaterProtocolFactory.getDummyProtocolHandler());
	m_thread->setInfoEnabled(true);
	m_thread->setEchoEnabled(true);
	m_thread->setDTMFEnabled(true);
	m_thread->setLog(true, log.logIRCDDBTraffic);

	return true;
}

void CDStarGatewayApp::sigHandlerExit(int sig)
{
	LogInfo("Caught signal : %s, shutting down gateway", strsignal(sig));

	if(g_app != nullptr && g_app->m_thread != nullptr) {
		g_app->m_thread->kill();
	}
}

void CDStarGatewayApp::sigHandlerFatal(int sig)
{
	LogFatal("Caught signal : %s", strsignal(sig));
	fprintf(stderr, "Caught signal : %s\n", strsignal(sig));
#ifdef DEBUG_DSTARGW
	std::stringstream stackTrace;
	stackTrace <<  boost::stacktrace::stacktrace();
	LogFatal("Stack Trace : \n%s", stackTrace.str().c_str());
	fprintf(stderr, "Stack Trace : \n%s\n", stackTrace.str().c_str());
#endif
	exit(3);
}

void CDStarGatewayApp::sigHandlerUSR(int sig)
{
	if(sig == SIGUSR1) {
		LogInfo("Caught signal : %s, updating host files", strsignal(sig));

		CHostsFilesManager::UpdateHostsAsync(); // call and forget
	}
}

void CDStarGatewayApp::terminateHandler()
{
#ifdef DEBUG_DSTARGW
	std::stringstream stackTrace;
	stackTrace <<  boost::stacktrace::stacktrace();
#endif

	std::exception_ptr eptr;
	eptr = std::current_exception(); 

	try {
        if (eptr != nullptr) {
            std::rethrow_exception(eptr);
        }
		else {
			LogFatal("Unhandled unknown exception occured");
			fprintf(stderr, "Unknown ex\n");
		}
    } catch(const std::exception& e) {
        LogFatal("Unhandled exception occured %s", e.what());
		fprintf(stderr, "Unhandled ex %s\n", e.what());
    }

#ifdef DEBUG_DSTARGW
	LogFatal("Stack Trace : \n%s", stackTrace.str().c_str());
#endif
	exit(2);
}
