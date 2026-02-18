/*
 *   Copyright (C) 2012,2013 by Jonathan Naylor G4KLX
 *   Copyright (c) Thomas A. Early N7TAE
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

#include <string>

class CDTMF {
public:
	CDTMF();
	~CDTMF();

	bool decode(const unsigned char* ambe, bool end);

	bool hasCommand() const;

	std::string translate();

	void reset();

private:
	std::string  m_data;
	std::string  m_command;
	bool         m_pressed;
	unsigned int m_releaseCount;
	unsigned int m_pressCount;
	char         m_lastChar;

	std::string processReflector(const std::string& prefix, const std::string& command) const;
	std::string processCCS(const std::string& command) const;
};

const unsigned char DTMF_MASK[] = {0x82U, 0x08U, 0x20U, 0x82U, 0x00U, 0x00U, 0x82U, 0x00U, 0x00U};
const unsigned char DTMF_SIG[]  = {0x82U, 0x08U, 0x20U, 0x82U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U};

const unsigned char DTMF_SYM_MASK[] = {0x10U, 0x40U, 0x08U, 0x20U};
const unsigned char DTMF_SYM0[]     = {0x00U, 0x40U, 0x08U, 0x20U};
const unsigned char DTMF_SYM1[]     = {0x00U, 0x00U, 0x00U, 0x00U};
const unsigned char DTMF_SYM2[]     = {0x00U, 0x40U, 0x00U, 0x00U};
const unsigned char DTMF_SYM3[]     = {0x10U, 0x00U, 0x00U, 0x00U};
const unsigned char DTMF_SYM4[]     = {0x00U, 0x00U, 0x00U, 0x20U};
const unsigned char DTMF_SYM5[]     = {0x00U, 0x40U, 0x00U, 0x20U};
const unsigned char DTMF_SYM6[]     = {0x10U, 0x00U, 0x00U, 0x20U};
const unsigned char DTMF_SYM7[]     = {0x00U, 0x00U, 0x08U, 0x00U};
const unsigned char DTMF_SYM8[]     = {0x00U, 0x40U, 0x08U, 0x00U};
const unsigned char DTMF_SYM9[]     = {0x10U, 0x00U, 0x08U, 0x00U};
const unsigned char DTMF_SYMA[]     = {0x10U, 0x40U, 0x00U, 0x00U};
const unsigned char DTMF_SYMB[]     = {0x10U, 0x40U, 0x00U, 0x20U};
const unsigned char DTMF_SYMC[]     = {0x10U, 0x40U, 0x08U, 0x00U};
const unsigned char DTMF_SYMD[]     = {0x10U, 0x40U, 0x08U, 0x20U};
const unsigned char DTMF_SYMS[]     = {0x00U, 0x00U, 0x08U, 0x20U};
const unsigned char DTMF_SYMH[]     = {0x10U, 0x00U, 0x08U, 0x20U};