/*
 *   Copyright (C) 2021-2022 by Geoffrey Merck F4FXL / KC3FRA
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


#pragma once

#include "Defs.h"

#ifdef USE_GPSD
#include <gps.h>
#include <string>

#include "APRSIdFrameProvider.h"
#include "Timer.h"

class CAPRSGPSDIdFrameProvider : public CAPRSIdFrameProvider
{
public:
    CAPRSGPSDIdFrameProvider(const std::string& gateway, const std::string& address, const std::string& port);

    void start() override;
    void close() override;
    void clock(unsigned int ms) override;
    bool wantsToSend() override;

protected:
    bool buildAPRSFramesInt(const CAPRSEntry * aprsEntry, std::vector<CAPRSFrame *>& frames) override;

private:
	std::string			m_gpsdAddress;
	std::string			m_gpsdPort;
	struct gps_data_t	m_gpsdData;
    bool                m_hasConnection;
    bool                m_warned;
    CTimer              m_retryTimer;
    CTimer              m_fixTimer;
    CTimer              m_moveTimer;
    CTimer              m_refreshTimer;
    bool                m_fixValid;
    double              m_latitude;
    double              m_longitude;
    bool                m_altitudeValid;
    double              m_altitude;
    bool                m_motionValid;
    double              m_speed;
    double              m_track;
    bool                m_sent;
    double              m_sentLatitude;
    double              m_sentLongitude;

    bool connect();
    void disconnect();
    void read();
};
#endif
