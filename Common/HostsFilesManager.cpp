/*
 *   Copyright (c) 2024 by Geoffrey F4FXL / KC3FRA
 *   Copytight (C) 2026 by Jonathan Naylor G4KLX
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

#include <cassert>

#include <fstream>
#include <nlohmann/json.hpp>

#include "HostsFilesManager.h"
#include "StringUtils.h"
#include "UDPReaderWriter.h"
#include "Log.h"

std::string CHostsFilesManager::m_hostFilesDirectory("");
std::string CHostsFilesManager::m_customFilesDirectory("");

bool CHostsFilesManager::m_dextraEnabled = false;
bool CHostsFilesManager::m_dcsEnabled = false;
bool CHostsFilesManager::m_dplusEnabled = false;
bool CHostsFilesManager::m_xlxEnabled = false;

CCacheManager * CHostsFilesManager::m_cache = nullptr;
CTimer CHostsFilesManager::m_reloadTimer(1000U, 60 * 60 * 24);

void CHostsFilesManager::setHostFilesDirectories(const std::string & hostFilesDir, const std::string & customHostFilesDir)
{
    m_hostFilesDirectory.assign(hostFilesDir);
    m_customFilesDirectory.assign(customHostFilesDir);
}

void CHostsFilesManager::setDextra(bool enabled)
{
    m_dextraEnabled = enabled;
}

void CHostsFilesManager::setDPlus(bool enabled)
{
    m_dplusEnabled = enabled;
}

void CHostsFilesManager::setDCS(bool enabled)
{
    m_dcsEnabled = enabled;
}

void CHostsFilesManager::setXLX(bool enabled)
{
    m_xlxEnabled = enabled;
}

void CHostsFilesManager::setCache(CCacheManager * cache)
{
    assert(cache != nullptr);
    m_cache = cache;
}

void CHostsFilesManager::clock(unsigned int ms)
{
    m_reloadTimer.clock(ms);

    if (m_reloadTimer.hasExpired()) {
	LogInfo("Reloading hosts files after %u hours", m_reloadTimer.getTimeout() / 3600U);
        UpdateHostsAsync(); // call and forget
	m_reloadTimer.start();
    }
}

void CHostsFilesManager::setReloadTime(unsigned int seconds)
{
    m_reloadTimer.start(seconds);
}

void CHostsFilesManager::UpdateHostsFromInternet()
{
    CHostsFilesManager::loadReflectors(m_hostFilesDirectory);
}

void CHostsFilesManager::UpdateHostsFromLocal()
{
    CHostsFilesManager::loadReflectors(m_customFilesDirectory);
}

bool CHostsFilesManager::UpdateHosts()
{
    UpdateHostsFromInternet();

    UpdateHostsFromLocal();

    return true;
}

std::future<bool> CHostsFilesManager::UpdateHostsAsync()
{
    auto fut = std::async(std::launch::async, UpdateHosts);

    return fut;
}

void CHostsFilesManager::loadReflectors(const std::string& directory)
{
	if (m_cache == nullptr) {
		LogWarning("HostsFilesManager cache not initilized");
		return;
	}

	unsigned int dplusCount = 0U;
	unsigned int dextraCount = 0U;
	unsigned int dcsCount = 0U;

	std::string filename = directory + "/" + JSON_HOSTS_FILE_NAME;

	try {
		std::fstream file(filename);

		nlohmann::json data = nlohmann::json::parse(file);

		bool hasData = data["reflectors"].is_array();
		if (!hasData)
			throw;

		nlohmann::json::array_t hosts = data["reflectors"];
		for (const auto& it : hosts) {
			std::string reflector = it["name"];

			// unsigned short port = it["port"];

			std::string type = it["reflector_type"];

			bool isNull = it["ipv4"].is_null();
			if (isNull)
				continue;

			std::string ipv4 = it["ipv4"];

			bool locked = false;
			bool hasLocked = it.contains("locked");
			if (hasLocked)
				locked = it["locked"];

			reflector.resize(LONG_CALLSIGN_LENGTH - 1U, ' ');
			reflector += "G";

			if (type == "REF") {
				if (m_dplusEnabled) {
					m_cache->updateGateway(reflector, ipv4, DP_DPLUS, locked, true);
					dplusCount++;
				}
			} else if (type == "XRF") {
				if (m_dextraEnabled) {
					m_cache->updateGateway(reflector, ipv4, DP_DEXTRA, locked, true);
					dextraCount++;
				}
			} else if (type == "DCS") {
				if (m_dcsEnabled) {
					m_cache->updateGateway(reflector, ipv4, DP_DCS, locked, true);
					dcsCount++;
				}
			} else {
				LogWarning("Unknown type of \"%s\" found in %s", type.c_str(), filename.c_str());
			}
		}
	}
	catch (...) {
		LogError("Unable to load/parse JSON file %s", filename.c_str());
		return;
	}

	if (m_dplusEnabled)
		LogInfo("Loaded %u D-Plus hosts from %s", dplusCount, filename.c_str());
	if (m_dextraEnabled)
		LogInfo("Loaded %u Dextra hosts from %s", dextraCount, filename.c_str());
	if (m_dcsEnabled)
		LogInfo("Loaded %u DCS hosts from %s", dcsCount, filename.c_str());
}

