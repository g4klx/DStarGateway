/*
 *   Copyright (C) 2010-2014,2018,2020,2026 by Jonathan Naylor G4KLX
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
#include <sstream>
#include <iostream>
#include <boost/algorithm/string.hpp>

#include "APRSISHandlerThread.h"
#include "DStarDefines.h"
#include "Utils.h"
#include "Defs.h"
#include "Log.h"
#include "Version.h"
#include "MQTTConnection.h"
#include "APRSFormater.h"
#include "APRSParser.h"

// #define	DUMP_TX


// In Log.cpp
extern CMQTTConnection* m_mqtt;

CAPRSISHandlerThread::CAPRSISHandlerThread(const std::string& callsign) :
CThread("APRS"),
m_username(callsign),
m_ssid(callsign),
m_queue(20U),
m_exit(false),
m_APRSReadCallbacks(),
m_filter(),
m_clientName(FULL_PRODUCT_NAME)
{
	assert(!callsign.empty());

	m_username[LONG_CALLSIGN_LENGTH - 1U] = ' ';
	boost::trim(m_username);
	boost::to_upper(m_username);

	m_ssid = m_ssid.substr(LONG_CALLSIGN_LENGTH - 1U, 1);
}

CAPRSISHandlerThread::CAPRSISHandlerThread(const std::string& callsign, const std::string& filter) :
CThread("APRS"),
m_username(callsign),
m_ssid(callsign),
m_queue(20U),
m_exit(false),
m_APRSReadCallbacks(),
m_filter(filter),
m_clientName(FULL_PRODUCT_NAME)
{
	assert(!callsign.empty());

	m_username[LONG_CALLSIGN_LENGTH - 1U] = ' ';
	boost::trim(m_username);
	boost::to_upper(m_username);

	m_ssid = m_ssid.substr(LONG_CALLSIGN_LENGTH - 1U, 1);
}

CAPRSISHandlerThread::~CAPRSISHandlerThread()
{
	std::vector<IReadAPRSFrameCallback *> callBacksCopy;
	callBacksCopy.assign(m_APRSReadCallbacks.begin(), m_APRSReadCallbacks.end());

	m_APRSReadCallbacks.clear();

	callBacksCopy.clear();

	m_username.clear();
}

bool CAPRSISHandlerThread::start()
{
	Create();
	Run();

	return true;
}

void* CAPRSISHandlerThread::Entry()
{
	LogInfo("Starting the APRS Writer thread");

#ifndef DEBUG_DSTARGW
	try {
#endif
		while (!m_exit) {
			if(!m_queue.empty()){
				auto frameStr = m_queue.getData();

				LogInfo("APRS Frame sent to IS ==> %s", frameStr.c_str());

				m_mqtt->publish("aprs-gateway/aprs", frameStr);
			}
#ifdef notdef
			{
				std::string line;
				int length = m_socket.readLine(line, APRS_READ_TIMEOUT);

				if (length < 0 || m_keepAliveTimer.hasExpired()) {
					m_connected = false;
					m_socket.close();
					LogError("Error when reading from the APRS server");
					startReconnectionTimer();
				}
				else if(length > 0 && line[0] == '#') {
					m_keepAliveTimer.start();
				}
				else if(line.length() > 0 && line[0] != '#') {
					m_keepAliveTimer.start();
					LogDebug("APRS Frame received from IS <== %s", line.c_str());
					CAPRSFrame readFrame;
					if(CAPRSParser::parseFrame(line, readFrame)) {
						for(auto cb : m_APRSReadCallbacks) {
							CAPRSFrame f(readFrame);
							cb->readAPRSFrame(f);
						}
					}
				}
			}
#endif
		}

		while (!m_queue.empty()) {
			auto s = m_queue.getData();
			s.clear();
		}
#ifndef DEBUG_DSTARGW
	}
	catch (std::exception& e) {
		std::string message(e.what());
		LogInfo("Exception raised in the APRS Writer thread - \"%s\"", message.c_str());
		throw;
	}
	catch (...) {
		LogInfo("Unknown exception raised in the APRS Writer thread");
		throw;
	}
#endif

	LogInfo("Stopping the APRS Writer thread");

	return NULL;
}

void CAPRSISHandlerThread::addReadAPRSCallback(IReadAPRSFrameCallback * cb)
{
	assert(cb != nullptr);
	m_APRSReadCallbacks.push_back(cb);
}

void CAPRSISHandlerThread::write(CAPRSFrame& frame)
{
	std::string frameString;
	if(CAPRSFormater::frameToString(frameString, frame)) {
		boost::trim_if(frameString, [] (char c) { return c == '\r' || c == '\n'; }); // trim all CRLF, we will add our own, just to make sure we get rid of any garbage that might come from slow data
		LogDebug("Queued APRS Frame : %s", frameString.c_str());
		frameString.append("\r\n");

		m_queue.addData(frameString);
	}
}

bool CAPRSISHandlerThread::isConnected() const
{
	return true;
}

void CAPRSISHandlerThread::stop()
{
	m_exit = true;

	Wait();
}

void CAPRSISHandlerThread::clock(unsigned int)
{
}

