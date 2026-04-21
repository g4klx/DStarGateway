/*
 *   Copyright (C) 2015,2016,2020,2022,2023,2025,2026 by Jonathan Naylor G4KLX
 *	 Copyright (C) 2018-2026 by BG5HHP
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

#if !defined USE_MQTT || USE_MQTT == 0

#include "Log.h"
#include "Utils.h"

#include <nlohmann/json.hpp>

#if defined(_WIN32) || defined(_WIN64)
#include <Windows.h>
#else
#include <sys/time.h>
#include <unistd.h>
#endif

#include <cstdio>
#include <cstdlib>
#include <cstdarg>
#include <ctime>
#include <cassert>
#include <cstring>

static unsigned int m_fileLevel = 0U;	// by default set file log disabled until it's initialized.
static std::string m_filePath;
static std::string m_fileRoot;

static FILE* m_fpLog = NULL;
static bool m_daemon = false;

static unsigned int m_displayLevel = 2U;

static struct tm m_tm;
static bool m_utc = true;

static char LEVELS[] = " DMIWEF";

static bool LogOpen()
{
	bool status = false;
	
	if (m_fileLevel == 0U)
		return true;

	time_t now;
	::time(&now);

	struct tm* tm;
	if(m_utc)
	{
		tm = ::gmtime(&now);
	}else{
		tm = ::localtime(&now);
	}

	if (tm->tm_mday == m_tm.tm_mday && tm->tm_mon == m_tm.tm_mon && tm->tm_year == m_tm.tm_year) {
		if (m_fpLog != NULL)
		    return true;
	} else {
		if (m_fpLog != NULL)
			::fclose(m_fpLog);
	}

	char filename[200U];
#if defined(_WIN32) || defined(_WIN64)
	::sprintf(filename, "%s\\%s-%04d-%02d-%02d.log", m_filePath.c_str(), m_fileRoot.c_str(), tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday);
#else
	::snprintf(filename, sizeof(filename), "%s/%s-%04d-%02d-%02d.log", m_filePath.c_str(), m_fileRoot.c_str(), tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday);
#endif

	if ((m_fpLog = ::fopen(filename, "a+t")) != NULL) {
		status = true;

#if !defined(_WIN32) && !defined(_WIN64)
		if (m_daemon)
			dup2(fileno(m_fpLog), fileno(stderr));
#endif
	}
	
	m_tm = *tm;

	return status;
}
void LogInitialise(unsigned int displayLevel, unsigned int mqttLevel) {
	(void)mqttLevel;
	LogInitialiseFile(false, "/tmp", "dstargateway", 2/*file level*/, displayLevel, true);
}

bool LogInitialiseFile(bool daemon, const std::string& filePath, const std::string& fileRoot, unsigned int fileLevel, unsigned int displayLevel, bool utc) {
	m_filePath     = filePath;
	m_fileRoot     = fileRoot;
	m_fileLevel    = fileLevel;
	m_displayLevel = displayLevel;
	m_utc          = utc;
	m_daemon       = daemon;

	if (m_daemon)
		m_displayLevel = 0U;

	LogFinalise();
	return ::LogOpen();
}

void LogFinalise()
{
	if (m_fpLog != NULL) {
		::fclose(m_fpLog);
		m_fpLog = NULL;
	}
}

void Log(unsigned int level, const char* fmt, ...)
{
    assert(fmt != NULL);
	// don't log if log level is disabled
	if ((level < m_fileLevel || m_fileLevel == 0) && (level < m_displayLevel || m_displayLevel == 0))
	{
		return;
	}
	char buffer[512U];
	unsigned int len;
#if defined(_WIN32) || defined(_WIN64)
	SYSTEMTIME st;
	::GetSystemTime(&st);

	len = ::snprintf(buffer, 512, "%c: %04u-%02u-%02u %02u:%02u:%02u.%03u ", LEVELS[level], st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
#else
	struct timeval now;
	::gettimeofday(&now, NULL);

	struct tm* tm;
	if(m_utc)
	{
		tm = ::gmtime(&now.tv_sec);
	}else{
		tm = ::localtime(&now.tv_sec);
	}

	len = ::snprintf(buffer, sizeof(buffer), "%c: %04d-%02d-%02d %02d:%02d:%02d.%03ld ", LEVELS[level], tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec, now.tv_usec / 1000L);
#endif

	va_list vl;
	va_start(vl, fmt);

	::vsnprintf(buffer + len, sizeof(buffer) - len, fmt, vl);

	va_end(vl);

	if (level >= m_fileLevel && m_fileLevel != 0U) {
		bool ret = ::LogOpen();
		if (!ret)
			return;

		::fprintf(m_fpLog, "%s\n", buffer);
		::fflush(m_fpLog);
	}

	if (level >= m_displayLevel && m_displayLevel != 0U) {
		::fprintf(stdout, "%s\n", buffer);
		::fflush(stdout);
	}

	if (level == 6U) {		// Fatal
		::fclose(m_fpLog);
		exit(1);
	}
}

#if 0
static void WriteJSON(const std::string& topLevel, nlohmann::json& json, bool retain)
{
	if (m_mqtt != nullptr) {
		nlohmann::json top;

		top[topLevel] = json;

		m_mqtt->publish("json", top.dump(), retain);
	}
}
#endif

void writeJSONStatus(const std::string& status)
{
#if 0
	nlohmann::json json;

	json["timestamp"] = CUtils::createTimestamp();
	json["message"]   = status;

	WriteJSON("status", json, false);
#endif
	LogMessage("status: %s", status.c_str());
}

void writeJSONLinking(const std::string& repeater, const std::string& reason, const std::string& protocol, const std::string& reflector)
{
#if 0
	nlohmann::json json;

	json["timestamp"] = CUtils::createTimestamp();
	json["repeater"]  = repeater;
	json["action"]    = "linking";
	json["reason"]    = reason;
	json["reflector"] = reflector;
	json["protocol"]  = protocol;

	WriteJSON("link", json, true);
#endif
	LogMessage("linking: rpt:%s, reason:%s, ref:%s, prot:%s", repeater.c_str(), reason.c_str(), reflector.c_str(), protocol.c_str());
}

void writeJSONUnlinked(const std::string& repeater, const std::string& reason)
{
#if 0
	nlohmann::json json;

	json["timestamp"] = CUtils::createTimestamp();
	json["repeater"]  = repeater;
	json["action"]    = "unlinked";
	json["reason"]    = reason;

	WriteJSON("link", json, true);
#endif
	LogMessage("unlinking: rpt:%s, reason:%s", repeater.c_str(), reason.c_str());
}

void writeJSONFailed(const std::string& repeater)
{
#if 0
	nlohmann::json json;

	json["timestamp"] = CUtils::createTimestamp();
	json["repeater"]  = repeater;
	json["action"]    = "failed";

	WriteJSON("link", json, true);
#endif
	LogMessage("failed: rpt:%s", repeater.c_str());
}

void writeJSONRelinking(const std::string& repeater, const std::string& protocol, const std::string& reflector)
{
#if 0
	nlohmann::json json;

	json["timestamp"] = CUtils::createTimestamp();
	json["repeater"]  = repeater;
	json["action"]    = "relinking";
	json["reflector"] = reflector;
	json["protocol"]  = protocol;

	WriteJSON("link", json, true);
#endif
	LogMessage("relinking: rpt:%s, ref:%s, prot:%s", repeater.c_str(), reflector.c_str(), protocol.c_str());
}

#endif // #if !defined USE_MQTT || USE_MQTT == 0