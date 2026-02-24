/*
 *   Copyright (c) 2024 by Geoffrey F4FXL / KC3FRA
 *   Copyright (C) 2026 by Jonathan Naylor G4KLX
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

#ifndef	HostsFilesManager_H
#define	HostsFilesManager_H

#include <string>
#include <future>

#include "CacheManager.h"
#include "Timer.h"
#include "DStarDefines.h"


class CHostsFilesManager {
public: 
    static void setHostFilesDirectories(const std::string& hostFilesDir, const std::string& customHostFilesDir);
    static void setDextra(bool enabled);
    static void setDCS(bool enabled);
    static void setDPlus(bool enabled);
    static void setXLX(bool enabled);
    static void setCache(CCacheManager* cache);
    static void clock(unsigned int ms);
    static void setDownloadTimeout(unsigned int seconds);
    static void UpdateHostsFromInternet();
    static void UpdateHostsFromLocal();
    static bool UpdateHosts();
    static std::future<bool> UpdateHostsAsync();

private:
    static std::string m_hostFilesDirectory;
    static std::string m_customFilesDirectory;

    static bool m_dextraEnabled;
    static bool m_dcsEnabled;
    static bool m_dplusEnabled;
    static bool m_xlxEnabled;

    static CCacheManager* m_cache;
    static CTimer m_downloadTimer;

    static void loadReflectors(const std::string& directory);
};

#endif
