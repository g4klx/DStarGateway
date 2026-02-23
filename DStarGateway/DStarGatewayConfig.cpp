/*
 *   Copyright (C) 2010,2011,2012,2026 by Jonathan Naylor G4KLX
 *   Copyright (c) 2017 by Thomas A. Early N7TAE
 *   Copyright (c) 2021 by Geoffrey Merck F4FXL / KC3FRA
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

#include <string>
#include <sstream>
#include <iostream>

#include "Utils.h"
#include "DStarGatewayConfig.h"
#include "DStarDefines.h"
#include "Log.h"
#include "StringUtils.h"

CDStarGatewayConfig::CDStarGatewayConfig(const std::string &pathname)
: m_fileName(pathname)
{

}

bool CDStarGatewayConfig::load()
{
	bool ret = false;
	LogInfo("Loading Configuration from %s", m_fileName.c_str());
	CConfig cfg(m_fileName);

	ret = open(cfg);
	if(ret) {
		ret = loadGateway(cfg) && ret;
		ret = loadIrcDDB(cfg) && ret;
		ret = loadRepeaters(cfg) && ret;
		ret = loadPaths(cfg) && ret;
		ret = loadHostsFiles(cfg) && ret;
		ret = loadLog(cfg) && ret;
		ret = loadMQTT(cfg) && ret;
		ret = loadAPRS(cfg) && ret;
		ret = loadDextra(cfg) && ret;
		ret = loadDCS(cfg) && ret;
		ret = loadDPlus(cfg) && ret;
		ret = loadRemote(cfg) && ret;
		ret = loadXLX(cfg) && ret;
#ifdef USE_GPSD
		ret = loadGPSD(cfg) && ret;
#endif
		ret = loadDaemon(cfg) && ret;
		ret = loadAccessControl(cfg) && ret;
		ret = loadDRats(cfg) && ret;
	}

	if(ret) {
		//properly size values
		m_gateway.callsign.resize(LONG_CALLSIGN_LENGTH - 1U, ' ');
		m_gateway.callsign.push_back('G');
	}
	else {
		LogError("Loading Configuration from %s failed", m_fileName.c_str());
	}

	return ret;
}

bool CDStarGatewayConfig::loadDaemon(const CConfig & cfg)
{
	bool ret = cfg.getValue("Daemon", "daemon", m_daemon.daemon, false);
	ret = cfg.getValue("Daemon", "pidfile", m_daemon.pidFile, 0, 1024, "") && ret;
	ret = cfg.getValue("Daemon", "user", m_daemon.user, 0, 1024, "") && ret;
	return ret;
}

bool CDStarGatewayConfig::loadXLX(const CConfig & cfg)
{
	bool ret = cfg.getValue("XLX", "enabled", m_xlx.enabled, true);

	return ret;
}

bool CDStarGatewayConfig::loadRemote(const CConfig & cfg)
{
	bool ret = cfg.getValue("Remote Commands", "enabled", m_remote.enabled, false);
	ret = cfg.getValue("Remote Commands", "port", m_remote.port, 1U, 65535U, 4242U) && ret;
	ret = cfg.getValue("Remote Commands", "password", m_remote.password, 0, 1024, "") && ret;

	m_remote.enabled = m_remote.enabled && !m_remote.password.empty();

	return ret;
}

bool CDStarGatewayConfig::loadDextra(const CConfig & cfg)
{
	bool ret = cfg.getValue("Dextra", "enabled", m_dextra.enabled, true);
	ret = cfg.getValue("Dextra", "maxDongles", m_dextra.maxDongles, 1U, 5U, 5U) && ret;
	return ret;
}

bool CDStarGatewayConfig::loadDPlus(const CConfig & cfg)
{
	bool ret = cfg.getValue("D-Plus", "enabled", m_dplus.enabled, true);
	ret = cfg.getValue("D-Plus", "maxDongles", m_dplus.maxDongles, 1U, 5U, 5U) && ret;
	ret = cfg.getValue("D-Plus", "login", m_dplus.login, 0, LONG_CALLSIGN_LENGTH, m_gateway.callsign) && ret;

	m_dplus.enabled = m_dplus.enabled && !m_dplus.login.empty();
	m_dplus.login = CUtils::ToUpper(m_dplus.login);

	return ret;
}

bool CDStarGatewayConfig::loadDCS(const CConfig & cfg)
{
	bool ret = cfg.getValue("DCS", "enabled", m_dcs.enabled, true);
	return ret;
}

bool CDStarGatewayConfig::loadAPRS(const CConfig & cfg)
{
	bool ret = cfg.getValue("APRS", "enabled", m_aprs.enabled, false);
#ifdef USE_GPSD
	std::string positionSource;
	ret = cfg.getValue("APRS", "positionSource", positionSource, "fixed", {"fixed", "gpsd"}) && ret;
	if(ret) {
		if(positionSource == "fixed")	m_aprs.m_positionSource = POSSRC_FIXED;
		else if(positionSource == "gpsd")	m_aprs.m_positionSource = POSSRC_GPSD;
	}
#else
	m_aprs.m_positionSource = POSSRC_FIXED;
#endif

	return ret;
}

bool CDStarGatewayConfig::loadLog(const CConfig & cfg)
{
	bool ret = cfg.getValue("Log", "displayLevel", m_log.displayLevel, 0U, 6U, 2U);
	ret = cfg.getValue("Log", "mqttLevel", m_log.mqttLevel, 0U, 6U, 2U) && ret;

	ret = cfg.getValue("Log", "logIRCDDBTraffic", m_log.logIRCDDBTraffic, false) && ret;

	return ret;
}

bool CDStarGatewayConfig::loadMQTT(const CConfig & cfg)
{
	bool ret = cfg.getValue("MQTT", "address", m_mqtt.address, 1U, 25U, "127.0.0.1");
	ret = cfg.getValue("MQTT", "port", m_mqtt.port, 1U, 65535U, 1883U) && ret;
	ret = cfg.getValue("MQTT", "keepalive", m_mqtt.keepalive, 0U, 240U, 60U) && ret;

	ret = cfg.getValue("MQTT", "authenticate", m_mqtt.authenticate, false) && ret;
	ret = cfg.getValue("MQTT", "username", m_mqtt.username, 0, 1024, "mmdvm") && ret;
	ret = cfg.getValue("MQTT", "password", m_mqtt.password, 0U, 30U, "mmdvm") && ret;

	ret = cfg.getValue("MQTT", "name", m_mqtt.name, 0U, 30U, "dstar-gateway") && ret;

	return ret;
}

bool CDStarGatewayConfig::loadPaths(const CConfig & cfg)
{
	bool ret = cfg.getValue("Paths", "data", m_paths.dataDir, 0, 2048, "/usr/local/share/dstargateway.d/");

	if(ret && m_paths.dataDir[m_paths.dataDir.length() - 1] != '/') {
		m_paths.dataDir.push_back('/');
	}

	//TODO 20211226 check if directory are accessible

	return ret;
}

bool CDStarGatewayConfig::loadHostsFiles(const CConfig & cfg)
{
	bool ret = cfg.getValue("Hosts Files", "downloadedHostsFiles", m_hostsFiles.downloadedHostFiles, 0, 2048, "/usr/local/share/dstargateway.d/");
	ret = cfg.getValue("Hosts Files", "customHostsfiles", m_hostsFiles.customHostsFiles, 0, 2048, "/usr/local/share/dstargateway.d/hostsfiles.d/");
	ret = cfg.getValue("Hosts Files", "downloadTimer", m_hostsFiles.downloadTimeout, 24U, 0xffffffffU, 72U);

	if(ret && m_hostsFiles.downloadedHostFiles[m_hostsFiles.downloadedHostFiles.length() - 1] != '/') {
		m_hostsFiles.downloadedHostFiles.push_back('/');
	}

	if(ret && m_hostsFiles.customHostsFiles[m_hostsFiles.customHostsFiles.length() - 1] != '/') {
		m_hostsFiles.downloadedHostFiles.push_back('/');
	}

	//TODO 20211226 check if directory are accessible

	return ret;
}

bool CDStarGatewayConfig::loadRepeaters(const CConfig & cfg)
{
	m_repeaters.clear();
	for(unsigned int i = 0; i < 4; i++) {
		std::string section = CStringUtils::string_format("Repeater %u", i + 1U);
		bool repeaterEnabled;

		bool ret = cfg.getValue(section, "enabled", repeaterEnabled, false);
		if(!ret || !repeaterEnabled)
			continue;
		
		TRepeater * repeater = new TRepeater;
		ret = cfg.getValue(section, "band", repeater->band, 1, 2, "B") && ret;
		ret = cfg.getValue(section, "callsign", repeater->callsign, 0, LONG_CALLSIGN_LENGTH - 1, m_gateway.callsign);
		ret = cfg.getValue(section, "address", repeater->address, 0, 15, "127.0.0.1") && ret;
		ret = cfg.getValue(section, "port", repeater->port, 1U, 65535U, 20011U) && ret;

		std::string hwType;
		ret = cfg.getValue(section, "type", hwType, "", {"hb", "icom"}) && ret;
		if(ret) {
			if(hwType == "hb") 			repeater->hwType = HW_HOMEBREW;
			else if(hwType == "icom")	repeater->hwType = HW_ICOM;
		}

		ret = cfg.getValue(section, "reflector", repeater->reflector, 0, LONG_CALLSIGN_LENGTH, "") && ret;
		ret = cfg.getValue(section, "reflectorAtStartup", repeater->reflectorAtStartup, !repeater->reflector.empty()) && ret;

		std::string reconnect;
		ret = cfg.getValue(section, "reflectorReconnect", reconnect, "never", {"never", "fixed", "5", "10", "15", "20", "25", "30", "60", "90", "120", "180"}) && ret;
		if(ret) {
			if(reconnect == "never")		repeater->reflectorReconnect = RECONNECT_NEVER;
			else if(reconnect == "5")		repeater->reflectorReconnect = RECONNECT_5MINS;
			else if(reconnect == "10")		repeater->reflectorReconnect = RECONNECT_10MINS;
			else if(reconnect == "15")		repeater->reflectorReconnect = RECONNECT_15MINS;
			else if(reconnect == "20")		repeater->reflectorReconnect = RECONNECT_20MINS;
			else if(reconnect == "25")		repeater->reflectorReconnect = RECONNECT_25MINS;
			else if(reconnect == "30")		repeater->reflectorReconnect = RECONNECT_30MINS;
			else if(reconnect == "60")		repeater->reflectorReconnect = RECONNECT_60MINS;
			else if(reconnect == "90")		repeater->reflectorReconnect = RECONNECT_90MINS;
			else if(reconnect == "120")		repeater->reflectorReconnect = RECONNECT_120MINS;
			else if(reconnect == "180")		repeater->reflectorReconnect = RECONNECT_180MINS;
			else if(reconnect == "fixed")	repeater->reflectorReconnect = RECONNECT_FIXED;
		}

		ret = cfg.getValue(section, "frequency", repeater->frequency, 0.0, 1500.0, 434.0) && ret;	
		ret = cfg.getValue(section, "offset", repeater->offset, -50.0, 50.0, 0.0) && ret;
		ret = cfg.getValue(section, "rangeKm", repeater->range, 0.0, 3000.0, 0.0) && ret;
		ret = cfg.getValue(section, "latitude", repeater->latitude, -90.0, 90.0, m_gateway.latitude) && ret;
		ret = cfg.getValue(section, "longitude", repeater->longitude, -180.0, 180.0, m_gateway.longitude) && ret;
		ret = cfg.getValue(section, "agl", repeater->agl, 0, 1000.0, 0.0) && ret;
		ret = cfg.getValue(section, "description1", repeater->description1, 0, 1024, m_gateway.description1) && ret;
		ret = cfg.getValue(section, "description2", repeater->description2, 0, 1024, m_gateway.description2) && ret;
		ret = cfg.getValue(section, "url", repeater->url, 0, 1024, m_gateway.url) && ret;;
		ret = cfg.getValue(section, "band1", repeater->band1, 0, 255, 0) && ret;
		ret = cfg.getValue(section, "band2", repeater->band2, 0, 255, 0) && ret;
		ret = cfg.getValue(section, "band3", repeater->band3, 0, 255, 0) && ret;

		if(ret) {
			m_repeaters.push_back(repeater);
		}
		else {
			delete repeater;
		}
	}

	if(m_repeaters.size() == 0U) {
		LogError("Configuration error: no repeaters configured !");
		return false;
	}

	return true;
}

bool CDStarGatewayConfig::loadIrcDDB(const CConfig & cfg)
{
	bool ret = true;
	for(unsigned int i = 0; i < 4; i++) {
		std::string section = CStringUtils::string_format("IRCDDB %u", i + 1U);
		bool ircEnabled;

		ret = cfg.getValue(section, "enabled", ircEnabled, false) && ret;
		if(!ircEnabled)
			continue;
		
		TircDDB * ircddb = new TircDDB;
		ret = cfg.getValue(section, "hostname", ircddb->hostname, 0, 1024, "ircv4.openquad.net") && ret;
		ret = cfg.getValue(section, "username", ircddb->username, 0, LONG_CALLSIGN_LENGTH - 1, m_gateway.callsign) && ret;
		ret = cfg.getValue(section, "password", ircddb->password, 0, 1024, "") && ret;

		if(ret) {
			m_ircDDB.push_back(ircddb);
		}
		else {
			delete ircddb;
		}
	}

	return ret;
}

bool CDStarGatewayConfig::loadGateway(const CConfig & cfg)
{
	bool ret = cfg.getValue("gateway", "callsign", m_gateway.callsign, 3, 8, "");
	ret = cfg.getValue("gateway", "address", m_gateway.address, 0, 20, "0.0.0.0") && ret;
	ret = cfg.getValue("gateway", "hbAddress", m_gateway.hbAddress, 0, 20, "127.0.0.1") && ret;
	ret = cfg.getValue("gateway", "hbPort", m_gateway.hbPort, 1U, 65535U, 20010U) && ret;
	ret = cfg.getValue("gateway", "icomAddress", m_gateway.icomAddress, 0, 20, "127.0.0.1") && ret;
	ret = cfg.getValue("gateway", "icomPort", m_gateway.icomPort, 1U, 65535U, 20000U) && ret;
	ret = cfg.getValue("gateway", "latitude", m_gateway.latitude, -90.0, 90.0, 0.0) && ret;
	ret = cfg.getValue("gateway", "longitude", m_gateway.longitude, -180.0, 180.0, 0.0) && ret;
	ret = cfg.getValue("gateway", "description1", m_gateway.description1, 0, 1024, "") && ret;
	ret = cfg.getValue("gateway", "description2", m_gateway.description2, 0, 1024, "") && ret;
	ret = cfg.getValue("gateway", "url", m_gateway.url, 0, 1024, "") && ret;
	
	std::string type;
	ret = cfg.getValue("gateway", "type", type, "repeater", {"repeater", "hotspot"}) && ret;
	if(type == "repeater")		m_gateway.type = GT_REPEATER;
	else if(type == "hotspot")	m_gateway.type = GT_HOTSPOT;

	std::string lang;
	ret = cfg.getValue("gateway", "language", lang, "english_uk",
						{"english_uk", "deutsch", "dansk", "francais", "francais_2", "italiano", "polski",
						"english_us", "espanol", "svenska", "nederlands_nl", "nederlands_be", "norsk", "portugues"}) && ret;;
	if(lang == "english_uk")		m_gateway.language = TL_ENGLISH_UK;
	else if(lang == "deutsch")		m_gateway.language = TL_DEUTSCH;
	else if(lang == "dansk")		m_gateway.language = TL_DANSK;
	else if(lang == "francais")		m_gateway.language = TL_FRANCAIS;
	else if(lang == "francais_2")	m_gateway.language = TL_FRANCAIS_2;
	else if(lang == "italiano") 	m_gateway.language = TL_ITALIANO;
	else if(lang == "polski")		m_gateway.language = TL_POLSKI;
	else if(lang == "english_us")	m_gateway.language = TL_ENGLISH_US;
	else if(lang == "espanol")		m_gateway.language = TL_ESPANOL;
	else if(lang == "svenska")		m_gateway.language = TL_SVENSKA;
	else if(lang == "nederlands_nl")m_gateway.language = TL_NEDERLANDS_NL;
	else if(lang == "nederlands_be")m_gateway.language = TL_NEDERLANDS_BE;
	else if(lang == "norsk")		m_gateway.language = TL_NORSK;
	else if(lang == "portugues")	m_gateway.language = TL_PORTUGUES;

	CUtils::ToUpper(m_gateway.callsign);
	CUtils::clean(m_gateway.description1, "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789 .,&*()-+=@/?:;");
	CUtils::clean(m_gateway.description2, "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789 .,&*()-+=@/?:;");
	CUtils::clean(m_gateway.url, 		  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789 .,&*()-+=@/?:;");

	return ret;
}

#ifdef USE_GPSD
bool CDStarGatewayConfig::loadGPSD(const CConfig & cfg)
{
	bool ret = cfg.getValue("gpsd", "address", m_gpsd.m_address, 0U, 15U, "127.0.0.1");
	ret = cfg.getValue("gpsd", "port", m_gpsd.m_port, 0U, 5U, "2947") && ret;

	return ret;
}
#endif

bool CDStarGatewayConfig::loadAccessControl(const CConfig & cfg)
{
	bool ret = cfg.getValue("Access Control", "whiteList", m_accessControl.whiteList, 0U, 2048U, "");
	ret = cfg.getValue("Access Control", "blackList", m_accessControl.blackList, 0U, 2048U, "") && ret;
	ret = cfg.getValue("Access Control", "restrictList", m_accessControl.restrictList, 0U, 2048U, "") && ret;
	
	return ret;
}

bool CDStarGatewayConfig::loadDRats(const CConfig & cfg)
{
	bool ret = cfg.getValue("DRats", "enabled", m_drats.enabled, false);

	return ret;
}

bool CDStarGatewayConfig::open(CConfig & cfg)
{
	try {
		return cfg.load();
	}
	catch(...) {
		LogError("Can't read %s\n", m_fileName.c_str());
		return false;
	}
	return true;
}

CDStarGatewayConfig::~CDStarGatewayConfig()
{
	while (m_repeaters.size()) {
		delete m_repeaters.back();
		m_repeaters.pop_back();
	}

	while(m_ircDDB.size()) {
		delete m_ircDDB.back();
		m_ircDDB.pop_back();
	}
}

void CDStarGatewayConfig::getGateway(TGateway & gateway) const
{
	gateway = m_gateway;
}

void CDStarGatewayConfig::getIrcDDB(unsigned int ircddb, TircDDB & ircDDB) const
{
	ircDDB = *(m_ircDDB[ircddb]);
}

unsigned int CDStarGatewayConfig::getRepeaterCount() const
{
	return m_repeaters.size();
}

unsigned int CDStarGatewayConfig::getIrcDDBCount() const
{
	return m_ircDDB.size();
}

void CDStarGatewayConfig::getRepeater(unsigned int index, TRepeater & repeater) const
{
	repeater = *(m_repeaters[index]);
}

void CDStarGatewayConfig::getLog(TLog & log) const
{
	log = m_log;
}

void CDStarGatewayConfig::getMQTT(TMQTT & mqtt) const
{
	mqtt = m_mqtt;
}

void CDStarGatewayConfig::getPaths(Tpaths & paths) const
{
	paths = m_paths;
}

void CDStarGatewayConfig::getHostsFiles(THostsFiles & hostsFiles) const
{
	hostsFiles = m_hostsFiles;
}

void CDStarGatewayConfig::getAPRS(TAPRS & aprs) const
{
	aprs = m_aprs;
}

void CDStarGatewayConfig::getDExtra(TDextra & dextra) const
{
	dextra = m_dextra;
}

void CDStarGatewayConfig::getDPlus(TDplus & dplus) const
{
	dplus = m_dplus;
}

void CDStarGatewayConfig::getDCS(TDCS & dcs) const
{
	dcs = m_dcs;
}

void CDStarGatewayConfig::getRemote(TRemote & remote) const
{
	remote = m_remote;
}

void CDStarGatewayConfig::getXLX(TXLX & xlx) const
{
	xlx = m_xlx;
}

#ifdef USE_GPSD
void CDStarGatewayConfig::getGPSD(TGPSD & gpsd) const
{
	gpsd = m_gpsd;
}
#endif

void CDStarGatewayConfig::getDaemon(TDaemon & gen) const
{
	gen = m_daemon;
}

void CDStarGatewayConfig::getAccessControl(TAccessControl & accessControl) const
{
	accessControl = m_accessControl;
}

void CDStarGatewayConfig::getDRats(TDRats & drats) const
{
	drats = m_drats;
}
