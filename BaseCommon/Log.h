/*
 *   Copyright (C) 2015,2016,2020,2022,2023,2026 by Jonathan Naylor G4KLX
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

#if !defined(LOG_H)
#define	LOG_H

#include <string>

#define	LogDebug(fmt, ...)	Log(1U, fmt, ##__VA_ARGS__)
#define	LogMessage(fmt, ...)	Log(2U, fmt, ##__VA_ARGS__)
#define	LogInfo(fmt, ...)	Log(3U, fmt, ##__VA_ARGS__)
#define	LogWarning(fmt, ...)	Log(4U, fmt, ##__VA_ARGS__)
#define	LogError(fmt, ...)	Log(5U, fmt, ##__VA_ARGS__)
#define	LogFatal(fmt, ...)	Log(6U, fmt, ##__VA_ARGS__)

extern void Log(unsigned int level, const char* fmt, ...);

extern void LogInitialise(unsigned int displayLevel, unsigned int mqttLevel);
extern void LogFinalise();

extern void writeJSONStatus(const std::string& status);
extern void writeJSONLinking(const std::string& repeater, const std::string& reason, const std::string& protocol, const std::string& reflector);
extern void writeJSONUnlinked(const std::string& reason, const std::string& repeater);
extern void writeJSONFailed(const std::string& repeater);
extern void writeJSONRelinking(const std::string& repeater, const std::string& protocol, const std::string& reflector);

#endif
