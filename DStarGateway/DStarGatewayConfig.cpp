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

CDStarGatewayConfig::CDStarGatewayConfig(const std::string& pathname) :
m_fileName(pathname),
m_general(),
m_paths(),
m_hostsFiles(),
m_aprs(),
m_dextra(),
m_dplus(),
m_dcs(),
m_remote(),
m_xlx(),
m_log(),
m_mqtt(),
#ifdef USE_GPSD
m_gpsd(),
#endif
m_daemon(),
m_accessControl(),
m_drats(),
m_repeaters(),
m_ircDDB()
{
}

bool CDStarGatewayConfig::load()
{
	bool ret = false;
	LogInfo("Loading Configuration from %s", m_fileName.c_str());
	CConfig cfg(m_fileName);

	ret = open(cfg);
	if (ret) {
		ret = loadGeneral(cfg) && ret;
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

	if (ret) {
		//properly size values
		m_general.callsign.resize(LONG_CALLSIGN_LENGTH - 1U, ' ');
		m_general.callsign.push_back('G');
	} else {
		LogError("Loading Configuration from %s failed", m_fileName.c_str());
	}

	return ret;
}

bool CDStarGatewayConfig::loadDaemon(const CConfig& cfg)
{
	bool ret = cfg.getValue("Daemon", "Daemon", m_daemon.daemon, false);
	ret = cfg.getValue("Daemon", "PidFile", m_daemon.pidFile, 0, 1024, "") && ret;
	ret = cfg.getValue("Daemon", "User", m_daemon.user, 0, 1024, "") && ret;
	return ret;
}

bool CDStarGatewayConfig::loadXLX(const CConfig& cfg)
{
	bool ret = cfg.getValue("XLX", "Enabled", m_xlx.enabled, true);

	return ret;
}

bool CDStarGatewayConfig::loadRemote(const CConfig& cfg)
{
	bool ret = cfg.getValue("Remote Commands", "Enabled", m_remote.enabled, false);
	ret = cfg.getValue("Remote Commands", "Port", m_remote.port, 1U, 65535U, 4242U) && ret;
	ret = cfg.getValue("Remote Commands", "Password", m_remote.password, 0, 1024, "") && ret;

	m_remote.enabled = m_remote.enabled && !m_remote.password.empty();

	return ret;
}

bool CDStarGatewayConfig::loadDextra(const CConfig& cfg)
{
	bool ret = cfg.getValue("Dextra", "Enabled", m_dextra.enabled, true);
	ret = cfg.getValue("Dextra", "MaxDongles", m_dextra.maxDongles, 1U, 5U, 5U) && ret;
	return ret;
}

bool CDStarGatewayConfig::loadDPlus(const CConfig& cfg)
{
	bool ret = cfg.getValue("D-Plus", "Enabled", m_dplus.enabled, true);
	ret = cfg.getValue("D-Plus", "MaxDongles", m_dplus.maxDongles, 1U, 5U, 5U) && ret;
	ret = cfg.getValue("D-Plus", "Login", m_dplus.login, 0, LONG_CALLSIGN_LENGTH, m_general.callsign) && ret;

	m_dplus.enabled = m_dplus.enabled && !m_dplus.login.empty();
	m_dplus.login = CUtils::ToUpper(m_dplus.login);

	return ret;
}

bool CDStarGatewayConfig::loadDCS(const CConfig& cfg)
{
	bool ret = cfg.getValue("DCS", "Enabled", m_dcs.enabled, true);
	return ret;
}

bool CDStarGatewayConfig::loadAPRS(const CConfig& cfg)
{
	bool ret = cfg.getValue("APRS", "Enabled", m_aprs.enabled, false);
#ifdef USE_GPSD
	std::string positionSource;
	ret = cfg.getValue("APRS", "PositionSource", positionSource, "Fixed", {"Fixed", "GPSD"}) && ret;
	if(ret) {
		if (positionSource == "Fixed")	m_aprs.m_positionSource = POSSRC_FIXED;
		else if (positionSource == "GPSD")	m_aprs.m_positionSource = POSSRC_GPSD;
	}
#else
	m_aprs.m_positionSource = POSSRC_FIXED;
#endif

	return ret;
}

bool CDStarGatewayConfig::loadLog(const CConfig& cfg)
{
	bool ret = cfg.getValue("Log", "DisplayLevel", m_log.displayLevel, 0U, 6U, 2U);
	ret = cfg.getValue("Log", "MQTTLevel", m_log.mqttLevel, 0U, 6U, 2U) && ret;

	ret = cfg.getValue("Log", "LogIRCDDBTraffic", m_log.logIRCDDBTraffic, false) && ret;

	return ret;
}

bool CDStarGatewayConfig::loadMQTT(const CConfig& cfg)
{
	bool ret = cfg.getValue("MQTT", "Address", m_mqtt.address, 1U, 25U, "127.0.0.1");
	ret = cfg.getValue("MQTT", "Port", m_mqtt.port, 1U, 65535U, 1883U) && ret;
	ret = cfg.getValue("MQTT", "Keepalive", m_mqtt.keepalive, 0U, 240U, 60U) && ret;

	ret = cfg.getValue("MQTT", "Authenticate", m_mqtt.authenticate, false) && ret;
	ret = cfg.getValue("MQTT", "Username", m_mqtt.username, 0, 1024, "mmdvm") && ret;
	ret = cfg.getValue("MQTT", "Password", m_mqtt.password, 0U, 30U, "mmdvm") && ret;

	ret = cfg.getValue("MQTT", "Name", m_mqtt.name, 0U, 30U, "dstar-gateway") && ret;

	return ret;
}

bool CDStarGatewayConfig::loadPaths(const CConfig& cfg)
{
	bool ret = cfg.getValue("Paths", "Data", m_paths.dataDir, 0, 2048, "/usr/local/share/dstargateway.d/");

	if (ret && m_paths.dataDir[m_paths.dataDir.length() - 1] != '/')
		m_paths.dataDir.push_back('/');

	//TODO 20211226 check if directory are accessible

	return ret;
}

bool CDStarGatewayConfig::loadHostsFiles(const CConfig& cfg)
{
	bool ret = cfg.getValue("Hosts Files", "HostsFiles",  m_hostsFiles.hostFiles, 0, 2048, "/usr/local/share/dstargateway.d/");
	ret = cfg.getValue("Hosts Files", "CustomHostsfiles", m_hostsFiles.customHostsFiles, 0, 2048, "/usr/local/share/dstargateway.d/hostsfiles.d/");
	ret = cfg.getValue("Hosts Files", "ReloadTime",       m_hostsFiles.reloadTime, 24U, 0xffffffffU, 72U);

	if (ret && m_hostsFiles.hostFiles[m_hostsFiles.hostFiles.length() - 1] != '/')
		m_hostsFiles.hostFiles.push_back('/');

	if (ret && m_hostsFiles.customHostsFiles[m_hostsFiles.customHostsFiles.length() - 1] != '/')
		m_hostsFiles.hostFiles.push_back('/');

	//TODO 20211226 check if directory are accessible

	return ret;
}

bool CDStarGatewayConfig::loadRepeaters(const CConfig& cfg)
{
	m_repeaters.clear();
	for (unsigned int i = 0U; i < 4U; i++) {
		std::string section = CStringUtils::string_format("Repeater %u", i + 1U);
		bool repeaterEnabled;

		bool ret = cfg.getValue(section, "Enabled", repeaterEnabled, false);
		if(!ret || !repeaterEnabled)
			continue;
		
		TRepeater * repeater = new TRepeater;
		ret = cfg.getValue(section, "Band", repeater->band, 1, 2, "B") && ret;
		ret = cfg.getValue(section, "Callsign", repeater->callsign, 0, LONG_CALLSIGN_LENGTH - 1, m_general.callsign);
		ret = cfg.getValue(section, "Address", repeater->address, 0, 15, "127.0.0.1") && ret;
		ret = cfg.getValue(section, "Port", repeater->port, 1U, 65535U, 20011U) && ret;

		std::string hwType;
		ret = cfg.getValue(section, "Type", hwType, "", {"HB", "Icom", "Dummy"}) && ret;
		if(ret) {
			if (hwType == "HB") 		repeater->hwType = HW_HOMEBREW;
			else if (hwType == "Icom")	repeater->hwType = HW_ICOM;
			else if (hwType == "Dummy")	repeater->hwType = HW_DUMMY;
		}

		ret = cfg.getValue(section, "Reflector", repeater->reflector, 0, LONG_CALLSIGN_LENGTH, "") && ret;
		ret = cfg.getValue(section, "ReflectorAtStartup", repeater->reflectorAtStartup, !repeater->reflector.empty()) && ret;

		std::string reconnect;
		ret = cfg.getValue(section, "ReflectorReconnect", reconnect, "Mever", {"Never", "Fixed", "5", "10", "15", "20", "25", "30", "60", "90", "120", "180"}) && ret;
		if(ret) {
			if (reconnect == "Never")		repeater->reflectorReconnect = RECONNECT_NEVER;
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
			else if(reconnect == "Fixed")	repeater->reflectorReconnect = RECONNECT_FIXED;
		}

		ret = cfg.getValue(section, "Frequency", repeater->frequency, 0.0, 1500.0, 434.0) && ret;	
		ret = cfg.getValue(section, "Offset", repeater->offset, -50.0, 50.0, 0.0) && ret;
		ret = cfg.getValue(section, "RangeKm", repeater->range, 0.0, 3000.0, 0.0) && ret;
		ret = cfg.getValue(section, "Latitude", repeater->latitude, -90.0, 90.0, m_general.latitude) && ret;
		ret = cfg.getValue(section, "Longitude", repeater->longitude, -180.0, 180.0, m_general.longitude) && ret;
		ret = cfg.getValue(section, "AGL", repeater->agl, 0, 1000.0, 0.0) && ret;
		ret = cfg.getValue(section, "Description1", repeater->description1, 0, 1024, m_general.description1) && ret;
		ret = cfg.getValue(section, "Description2", repeater->description2, 0, 1024, m_general.description2) && ret;
		ret = cfg.getValue(section, "URL", repeater->url, 0, 1024, m_general.url) && ret;;
		ret = cfg.getValue(section, "Band1", repeater->band1, 0, 255, 0) && ret;
		ret = cfg.getValue(section, "Band2", repeater->band2, 0, 255, 0) && ret;
		ret = cfg.getValue(section, "Band3", repeater->band3, 0, 255, 0) && ret;

		if (ret)
			m_repeaters.push_back(repeater);
		else
			delete repeater;
	}

	if (m_repeaters.size() == 0U) {
		LogError("Configuration error: no repeaters configured !");
		return false;
	}

	return true;
}

bool CDStarGatewayConfig::loadIrcDDB(const CConfig& cfg)
{
	bool ret = true;
	for(unsigned int i = 0U; i < 4U; i++) {
		std::string section = CStringUtils::string_format("IRCDDB %u", i + 1U);
		bool ircEnabled;

		ret = cfg.getValue(section, "Enabled", ircEnabled, false) && ret;
		if (!ircEnabled)
			continue;
		
		TircDDB* ircddb = new TircDDB;
		ret = cfg.getValue(section, "Hostname", ircddb->hostname, 0, 1024, "ircv4.openquad.net") && ret;
		ret = cfg.getValue(section, "Username", ircddb->username, 0, LONG_CALLSIGN_LENGTH - 1U, m_general.callsign) && ret;
		ret = cfg.getValue(section, "Password", ircddb->password, 0, 1024, "") && ret;

		if (ret)
			m_ircDDB.push_back(ircddb);
		else
			delete ircddb;
	}

	return ret;
}

bool CDStarGatewayConfig::loadGeneral(const CConfig& cfg)
{
	bool ret = cfg.getValue("General", "Callsign", m_general.callsign, 3, 8, "");
	ret = cfg.getValue("General", "Address", m_general.address, 0, 20, "0.0.0.0") && ret;
	ret = cfg.getValue("General", "HBAddress", m_general.hbAddress, 0, 20, "127.0.0.1") && ret;
	ret = cfg.getValue("General", "HBPort", m_general.hbPort, 1U, 65535U, 20010U) && ret;
	ret = cfg.getValue("General", "IcomAddress", m_general.icomAddress, 0, 20, "127.0.0.1") && ret;
	ret = cfg.getValue("General", "IcomPort", m_general.icomPort, 1U, 65535U, 20000U) && ret;
	ret = cfg.getValue("General", "Latitude", m_general.latitude, -90.0, 90.0, 0.0) && ret;
	ret = cfg.getValue("General", "Longitude", m_general.longitude, -180.0, 180.0, 0.0) && ret;
	ret = cfg.getValue("General", "Description1", m_general.description1, 0, 1024, "") && ret;
	ret = cfg.getValue("General", "Description2", m_general.description2, 0, 1024, "") && ret;
	ret = cfg.getValue("General", "URL", m_general.url, 0, 1024, "") && ret;
	
	std::string type;
	ret = cfg.getValue("General", "Type", type, "Repeater", {"Repeater", "Hotspot"}) && ret;
	if (type == "Repeater")		m_general.type = GT_REPEATER;
	else if(type == "Hotspot")	m_general.type = GT_HOTSPOT;

	std::string lang;
	ret = cfg.getValue("General", "Language", lang, "English_UK",
						{"English_UK", "Deutsch", "Dansk", "Francais", "Francais_2", "Italiano", "Polski",
						"English_US", "Espanol", "Svenska", "Nederlands_NL", "Nederlands_BE", "Norsk", "Portugues"}) && ret;;
	if (lang == "English_UK")			m_general.language = TL_ENGLISH_UK;
	else if (lang == "Deutsch")			m_general.language = TL_DEUTSCH;
	else if (lang == "Dansk")			m_general.language = TL_DANSK;
	else if (lang == "Francais")		m_general.language = TL_FRANCAIS;
	else if (lang == "Francais_2")		m_general.language = TL_FRANCAIS_2;
	else if (lang == "Italiano") 		m_general.language = TL_ITALIANO;
	else if (lang == "Polski")			m_general.language = TL_POLSKI;
	else if (lang == "English_US")		m_general.language = TL_ENGLISH_US;
	else if (lang == "Espanol")			m_general.language = TL_ESPANOL;
	else if (lang == "Svenska")			m_general.language = TL_SVENSKA;
	else if (lang == "Nederlands_NL")	m_general.language = TL_NEDERLANDS_NL;
	else if (lang == "Nederlands_BE")	m_general.language = TL_NEDERLANDS_BE;
	else if (lang == "Norsk")			m_general.language = TL_NORSK;
	else if (lang == "Portugues")		m_general.language = TL_PORTUGUES;

	CUtils::ToUpper(m_general.callsign);
	CUtils::clean(m_general.description1, "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789 .,&*()-+=@/?:;");
	CUtils::clean(m_general.description2, "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789 .,&*()-+=@/?:;");
	CUtils::clean(m_general.url, 		  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789 .,&*()-+=@/?:;");

	return ret;
}

#ifdef USE_GPSD
bool CDStarGatewayConfig::loadGPSD(const CConfig & cfg)
{
	bool ret = cfg.getValue("GPSD", "Address", m_gpsd.m_address, 0U, 15U, "127.0.0.1");
	ret = cfg.getValue("GPSD", "Port", m_gpsd.m_port, 0U, 5U, "2947") && ret;

	return ret;
}
#endif

bool CDStarGatewayConfig::loadAccessControl(const CConfig& cfg)
{
	bool ret = cfg.getValue("Access Control", "WhiteList", m_accessControl.whiteList, 0U, 2048U, "");
	ret = cfg.getValue("Access Control", "BlackList", m_accessControl.blackList, 0U, 2048U, "") && ret;
	ret = cfg.getValue("Access Control", "RestrictList", m_accessControl.restrictList, 0U, 2048U, "") && ret;
	
	return ret;
}

bool CDStarGatewayConfig::loadDRats(const CConfig& cfg)
{
	bool ret = cfg.getValue("D-Rats", "Enabled", m_drats.enabled, false);

	return ret;
}

bool CDStarGatewayConfig::open(CConfig& cfg)
{
	try {
		return cfg.load();
	}
	catch(...) {
		LogError("Can't read %s", m_fileName.c_str());
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

void CDStarGatewayConfig::getGeneral(TGeneral& general) const
{
	general = m_general;
}

void CDStarGatewayConfig::getIrcDDB(unsigned int ircddb, TircDDB& ircDDB) const
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

void CDStarGatewayConfig::getRepeater(unsigned int index, TRepeater& repeater) const
{
	repeater = *(m_repeaters[index]);
}

void CDStarGatewayConfig::getLog(TLog& log) const
{
	log = m_log;
}

void CDStarGatewayConfig::getMQTT(TMQTT& mqtt) const
{
	mqtt = m_mqtt;
}

void CDStarGatewayConfig::getPaths(Tpaths& paths) const
{
	paths = m_paths;
}

void CDStarGatewayConfig::getHostsFiles(THostsFiles& hostsFiles) const
{
	hostsFiles = m_hostsFiles;
}

void CDStarGatewayConfig::getAPRS(TAPRS& aprs) const
{
	aprs = m_aprs;
}

void CDStarGatewayConfig::getDExtra(TDextra& dextra) const
{
	dextra = m_dextra;
}

void CDStarGatewayConfig::getDPlus(TDplus& dplus) const
{
	dplus = m_dplus;
}

void CDStarGatewayConfig::getDCS(TDCS& dcs) const
{
	dcs = m_dcs;
}

void CDStarGatewayConfig::getRemote(TRemote& remote) const
{
	remote = m_remote;
}

void CDStarGatewayConfig::getXLX(TXLX& xlx) const
{
	xlx = m_xlx;
}

#ifdef USE_GPSD
void CDStarGatewayConfig::getGPSD(TGPSD& gpsd) const
{
	gpsd = m_gpsd;
}
#endif

void CDStarGatewayConfig::getDaemon(TDaemon& gen) const
{
	gen = m_daemon;
}

void CDStarGatewayConfig::getAccessControl(TAccessControl& accessControl) const
{
	accessControl = m_accessControl;
}

void CDStarGatewayConfig::getDRats(TDRats& drats) const
{
	drats = m_drats;
}
