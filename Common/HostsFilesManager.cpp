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

#include "HostsFilesManager.h"
#include "HostFile.h"
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
CTimer CHostsFilesManager::m_downloadTimer(1000U, 60 * 60 * 24);

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
    m_downloadTimer.clock(ms);

    if (m_downloadTimer.hasExpired()) {
	LogInfo("Downloading hosts files after %u hours", m_downloadTimer.getTimeout() / 3600U);
        UpdateHostsAsync(); // call and forget
	m_downloadTimer.start();
    }
}

void CHostsFilesManager::setDownloadTimeout(unsigned int seconds)
{
    m_downloadTimer.start(seconds);
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

void CHostsFilesManager::loadReflectors(const std::string & directory)
{
    if (m_xlxEnabled) {
		std::string fileName = directory + "/" + XLX_HOSTS_FILE_NAME;
		loadReflectors(fileName, DP_DCS);
	}
	
	if (m_dplusEnabled) {
		std::string fileName = directory + "/" + DPLUS_HOSTS_FILE_NAME;
		loadReflectors(fileName, DP_DPLUS);
	}

	if (m_dextraEnabled) {
		std::string fileName = directory + "/" + DEXTRA_HOSTS_FILE_NAME;
		loadReflectors(fileName, DP_DEXTRA);
	}

	if (m_dcsEnabled) {
		std::string fileName = directory + "/" + DCS_HOSTS_FILE_NAME;
		loadReflectors(fileName, DP_DCS);
	}
}

void CHostsFilesManager::loadReflectors(const std::string & hostFileName, DSTAR_PROTOCOL proto)
{
    if(m_cache == nullptr) {
        LogDebug("HostsFilesManager cache not initilized");
        return;
    }

	unsigned int count = 0U;

	CHostFile hostFile(hostFileName, false);
	for (unsigned int i = 0U; i < hostFile.getCount(); i++) {
		std::string reflector = hostFile.getName(i);
		in_addr address    = CUDPReaderWriter::lookup(hostFile.getAddress(i));
		bool lock          = hostFile.getLock(i);

		if (address.s_addr != INADDR_NONE) {
			unsigned char* ucp = (unsigned char*)&address;

			std::string addrText;
			addrText = CStringUtils::string_format("%u.%u.%u.%u", ucp[0U] & 0xFFU, ucp[1U] & 0xFFU, ucp[2U] & 0xFFU, ucp[3U] & 0xFFU);

			if (lock)
				LogInfo("Locking %s to %s", reflector.c_str(), addrText.c_str());

			reflector.resize(LONG_CALLSIGN_LENGTH - 1U, ' ');
			reflector += "G";
			m_cache->updateGateway(reflector, addrText, proto, lock, true);

			count++;
		}
	}

	std::string protoString;
	switch (proto)
	{
	case DP_DEXTRA:
		protoString =  "DExtra";
		break;
	case DP_DCS:
		protoString = "DCS";
		break;
	case DP_DPLUS:
		protoString = "DPlus";
		break;
	default:
		// ???
		break;
	}

	LogInfo("Loaded %u of %u %s hosts from %s", count, hostFile.getCount(), protoString.c_str() , hostFileName.c_str());
}
