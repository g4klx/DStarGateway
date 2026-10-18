/*
 *   Copyright (C) 2021-2022 by Geoffrey Merck F4FXL / KC3FRA
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

#ifdef USE_GPSD
#include <cerrno>
#include <cmath>
#include <cstring>
#include <ctime>
#include <boost/algorithm/string.hpp>

#include "APRSGPSDIdFrameProvider.h"
#include "StringUtils.h"
#include "Log.h"

// Beacon on first fix, then 60 s while moving, hourly when parked.
// 50 m clears GPS wander and the ~18 m APRS position resolution.
const unsigned int RETRY_SECS               = 30U;
const unsigned int FIX_TIMEOUT_SECS         = 30U;
const unsigned int MOBILE_MIN_INTERVAL_SECS = 60U;
const unsigned int STATIONARY_INTERVAL_SECS = 3600U;
const double       MOVED_DISTANCE_METRES    = 50.0;

CAPRSGPSDIdFrameProvider::CAPRSGPSDIdFrameProvider(const std::string& gateway, const std::string& address, const std::string& port) :
CAPRSIdFrameProvider(gateway, 20U),
m_gpsdAddress(address),
m_gpsdPort(port),
m_gpsdData(),
m_hasConnection(false),
m_warned(false),
m_retryTimer(1000U, RETRY_SECS),
m_fixTimer(1000U, FIX_TIMEOUT_SECS),
m_moveTimer(1000U, MOBILE_MIN_INTERVAL_SECS),
m_refreshTimer(1000U, STATIONARY_INTERVAL_SECS),
m_fixValid(false),
m_latitude(0.0),
m_longitude(0.0),
m_altitudeValid(false),
m_altitude(0.0),
m_motionValid(false),
m_speed(0.0),
m_track(0.0),
m_sent(false),
m_sentLatitude(0.0),
m_sentLongitude(0.0)
{

}

void CAPRSGPSDIdFrameProvider::start()
{
    // gpsd may not be up yet; clock() retries
    if (!connect())
        m_retryTimer.start();
}

void CAPRSGPSDIdFrameProvider::close()
{
    if (m_hasConnection) {
        ::gps_stream(&m_gpsdData, WATCH_DISABLE, NULL);
        disconnect();
    }
}

bool CAPRSGPSDIdFrameProvider::connect()
{
    int ret = ::gps_open(m_gpsdAddress.c_str(), m_gpsdPort.c_str(), &m_gpsdData);
    if (ret != 0) {
        // -1: reason in errno (system error, or a libgps code on old libgps)
        int err = (ret == -1) ? errno : ret;
        const char* reason = (err < 0) ? ::gps_errstr(err) : ::strerror(err);

        // Warn once per outage
        if (!m_warned)
            LogWarning("Unable to connect to gpsd at %s:%s - %s, retrying every %u seconds", m_gpsdAddress.c_str(), m_gpsdPort.c_str(), reason, RETRY_SECS);
        m_warned = true;
        return false;
    }

    ::gps_stream(&m_gpsdData, WATCH_ENABLE | WATCH_JSON, NULL);

    m_hasConnection = true;
    m_warned        = false;

    LogMessage("Connected to GPSD");

    return true;
}

void CAPRSGPSDIdFrameProvider::disconnect()
{
    ::gps_close(&m_gpsdData);

    m_hasConnection = false;
    m_fixValid      = false;
    m_fixTimer.stop();
}

void CAPRSGPSDIdFrameProvider::clock(unsigned int ms)
{
    CAPRSIdFrameProvider::clock(ms);

    if (m_hasConnection) {
        read();
    } else {
        m_retryTimer.clock(ms);
        if (m_retryTimer.hasExpired()) {
            if (connect())
                m_retryTimer.stop();
            else
                m_retryTimer.start();
        }
    }

    // Don't beacon a stale fix
    m_fixTimer.clock(ms);
    if (m_fixTimer.hasExpired()) {
        m_fixValid = false;
        m_fixTimer.stop();
    }

    m_moveTimer.clock(ms);
    m_refreshTimer.clock(ms);
}

// Drain gpsd and keep the latest fix: gps_read() returns one message.
void CAPRSGPSDIdFrameProvider::read()
{
    // Bounded; the rest is read next time
    for (unsigned int n = 0U; n < 50U && ::gps_waiting(&m_gpsdData, 0); n++) {
        // Non-TPV messages leave stale flags set
        m_gpsdData.set = 0U;

#if GPSD_API_MAJOR_VERSION >= 7
        char message[1024];
        int ret = ::gps_read(&m_gpsdData, message, sizeof(message));
#else
        int ret = ::gps_read(&m_gpsdData);
#endif
        if (ret < 0) {
            LogWarning("Lost the connection to gpsd, retrying every %u seconds", RETRY_SECS);
            disconnect();
            m_warned = true;
            m_retryTimer.start();
            return;
        }

        if (ret == 0)
            return;

        // TPV reports only
        if ((m_gpsdData.set & MODE_SET) != MODE_SET)
            continue;

        // Any 2D/3D fix (GPS, DGPS, RTK...). gpsd doesn't range-check.
        bool latlonSet = (m_gpsdData.set & LATLON_SET) == LATLON_SET;
        double lat     = m_gpsdData.fix.latitude;
        double lon     = m_gpsdData.fix.longitude;
        bool latlonOK  = std::isfinite(lat) && std::isfinite(lon) &&
                         ::fabs(lat) <= 90.0 && ::fabs(lon) <= 180.0;
        if (m_gpsdData.fix.mode < MODE_2D || !latlonSet || !latlonOK) {
            m_fixValid = false;
            m_fixTimer.stop();
            continue;
        }

        m_latitude  = lat;
        m_longitude = lon;

#if GPSD_API_MAJOR_VERSION >= 9
        m_altitude = m_gpsdData.fix.altMSL;
#else
        m_altitude = m_gpsdData.fix.altitude;
#endif
        m_altitudeValid = (m_gpsdData.fix.mode >= MODE_3D) && std::isfinite(m_altitude) &&
                          m_altitude >= -1000.0 && m_altitude <= 100000.0;

        bool speedSet = (m_gpsdData.set & SPEED_SET) == SPEED_SET;
        bool trackSet = (m_gpsdData.set & TRACK_SET) == TRACK_SET;
        m_speed       = m_gpsdData.fix.speed;
        m_track       = m_gpsdData.fix.track;
        m_motionValid = speedSet && trackSet && std::isfinite(m_speed) && std::isfinite(m_track);

        m_fixValid = true;
        m_fixTimer.start();
    }
}

bool CAPRSGPSDIdFrameProvider::wantsToSend()
{
    if (!m_fixValid)
        return false;

    // First fix
    if (!m_sent)
        return true;

    // Parked: hourly refresh
    if (m_refreshTimer.hasExpired())
        return true;

    // Moving: at most once a minute
    if (!m_moveTimer.hasExpired())
        return false;

    // Distance from the last position sent (equirectangular)
    const double DEG_TO_RAD   = 0.017453292519943295;
    const double EARTH_RADIUS = 6371000.0;
    const double PI           = 3.141592653589793;

    double dLon = (m_longitude - m_sentLongitude) * DEG_TO_RAD;
    if (dLon > PI)
        dLon -= 2.0 * PI;
    else if (dLon < -PI)
        dLon += 2.0 * PI;

    double dLat   = (m_latitude - m_sentLatitude) * DEG_TO_RAD;
    double midLat = (m_latitude + m_sentLatitude) * 0.5 * DEG_TO_RAD;

    double x = dLon * ::cos(midLat);
    double distance = ::sqrt(x * x + dLat * dLat) * EARTH_RADIUS;

    return distance > MOVED_DISTANCE_METRES;
}

bool CAPRSGPSDIdFrameProvider::buildAPRSFramesInt(const CAPRSEntry * entry, std::vector<CAPRSFrame *>& frames)
{
    if (entry == NULL || !m_fixValid)
        return false;

    time_t now;
    ::time(&now);
    struct tm* tm = ::gmtime(&now);

    std::string desc;
    if (entry->getBand().length() > 1U) {
        if (entry->getFrequency() != 0.0)
            desc = CStringUtils::string_format("Data %.5lfMHz", entry->getFrequency());
        else
            desc = "Data";
    } else {
        if (entry->getFrequency() != 0.0)
            desc = CStringUtils::string_format("Voice %.5lfMHz %c%.4lfMHz",
                    entry->getFrequency(),
                    entry->getOffset() < 0.0 ? '-' : '+',
                    ::fabs(entry->getOffset()));
        else
            desc = "Voice";
    }

    std::string band;
    if (entry->getFrequency() >= 1200.0)
        band = "1.2";
    else if (entry->getFrequency() >= 420.0)
        band = "440";
    else if (entry->getFrequency() >= 144.0)
        band = "2m";
    else if (entry->getFrequency() >= 50.0)
        band = "6m";
    else if (entry->getFrequency() >= 28.0)
        band = "10m";

    double tempLat  = ::fabs(m_latitude);
    double tempLong = ::fabs(m_longitude);

    double latitude  = ::floor(tempLat);
    double longitude = ::floor(tempLong);

    latitude  = (tempLat  - latitude)  * 60.0 + latitude  * 100.0;
    longitude = (tempLong - longitude) * 60.0 + longitude * 100.0;

    std::string lat = CStringUtils::string_format("%07.2lf", latitude);
    std::string lon = CStringUtils::string_format("%08.2lf", longitude);

    // Convert commas to periods in the latitude and longitude
    boost::replace_all(lat, ",", ".");
    boost::replace_all(lon, ",", ".");

    std::string altitude;
    if (m_altitudeValid)
        altitude = CStringUtils::string_format("/A=%06.0lf", m_altitude * 3.28084);

    // Object: RNG data extension straight after the symbol, as the fixed provider does
    std::string body = CStringUtils::string_format(";%-7s%-2s*%02d%02d%02dz%s%cD%s%caRNG%04.0lf%s %s %s\r\n",
                                                        entry->getCallsign().c_str(), entry->getBand().c_str(),
                                                        tm->tm_mday, tm->tm_hour, tm->tm_min,
                                                        lat.c_str(), (m_latitude < 0.0)  ? 'S' : 'N',
                                                        lon.c_str(), (m_longitude < 0.0) ? 'W' : 'E',
                                                        entry->getRange() * 0.6214, altitude.c_str(), band.c_str(), desc.c_str());

    CAPRSFrame * frame = new CAPRSFrame(m_gateway + "-S",
                                    "APD5T1",
                                    { "TCPIP*", "qAC" , m_gateway + "-GS" },
                                    body, APFT_OBJECT);

    frames.push_back(frame);

    if (entry->getBand().length() == 1U) {
        // Course 001-360 (000 = unknown); speed m/s -> knots
        std::string motion;
        if (m_motionValid) {
            int course = int(::lround(m_track)) % 360;
            if (course <= 0)
                course += 360;
            long speed = ::lround(m_speed * 1.943844);
            if (speed < 0L)
                speed = 0L;
            else if (speed > 999L)
                speed = 999L;
            motion = CStringUtils::string_format("%03d/%03ld", course, speed);
        }

        body = CStringUtils::string_format("!%s%cD%s%c&%s%s",
            lat.c_str(), (m_latitude < 0.0)  ? 'S' : 'N',
            lon.c_str(), (m_longitude < 0.0) ? 'W' : 'E',
            motion.c_str(), altitude.c_str());

        frame = new CAPRSFrame(m_gateway,
                                    "APD5T2",
                                    { "TCPIP*", "qAC" , m_gateway + "-GS" },
                                    body, APFT_POSITION);

        frames.push_back(frame);
    }

    m_sent          = true;
    m_sentLatitude  = m_latitude;
    m_sentLongitude = m_longitude;
    m_moveTimer.start();
    m_refreshTimer.start();

    return true;
}
#endif
