/*
 *   Copyright (c) 2021-2022 by Geoffrey Merck F4FXL / KC3FRA
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

#include <boost/algorithm/string.hpp>

#include "StringUtils.h"

size_t CStringUtils::find_nth(const std::string& haystack, char needle, size_t nth) 
{
    size_t matches = 0U;
    auto haystackLength = haystack.length();

    for(size_t i = 0; i < haystackLength; i++) {
        if(haystack[i] == needle) {
            matches++;
            if(matches == nth)
                return i;
        }
    }

    return std::string::npos;
}

unsigned int CStringUtils::stringToPort(const std::string& s)
{
    unsigned int port = 0U;
    std::string ls = boost::trim_copy(s);

    if(!ls.empty() && std::all_of(ls.begin(), ls.end(), [](char c){ return c >= '0' && c <= '9'; })) {
        unsigned int portTemp = std::stoul(ls);
        if(portTemp > 0U && portTemp <= 65535U)
            port = portTemp;
    }

    return port;
}

//
// String utils
//
std::string& CStringUtils::toUpper(std::string& s)
{
	if(s.size() == 0) return s;
	for(unsigned int i = 0; i<s.size(); i++){
		s[i] = ::toupper(s[i]);
	}
	return s;
}

char* CStringUtils::toUpper(char* s)
{
	if(s == NULL)
		return s;

	unsigned int len;
	len = ::strlen(s);
	if(len == 0)
		return s;

	for(unsigned int i = 0; i<len; i++){
		s[i] = ::toupper(s[i]);
	}

	return s;
}

static char* str_ltrim(char* str){
    if (str == NULL)
        return NULL;

    // count leading space
    int i = 0;
    while((str[i] == ' ' || str[i] == '\t' || str[i] == '\r' || str[i] == '\n')) {
        i++;
    }

    if (i > 0) {
        size_t len = strlen(str);
        if (i == len)
            str[0] = 0;
        else
            memmove(str, str + i, len - i + 1); // move with the trailing '\0'
    }

    return str;
}

static char* str_rtrim(char* str) {
    if (str == NULL)
        return NULL;

    int len = strlen(str);
    int i;
    for(i = len - 1; i>=0; i--){
        if (str[i] == ' ' || str[i] == '\t' || str[i] == '\r' || str[i] == '\n'){
            str[i] = 0;
            continue;
        }
        if (str[i] != 0)
            break;
    }
    return str;
}

static inline char* str_trim(char* str) {
    return str_ltrim(str_rtrim(str));
}

char* CStringUtils::trimString(char *str)
{
	return str_trim(str);
}