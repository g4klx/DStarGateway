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

#pragma once

#ifndef USE_BOOST_HASH
#define USE_BOOST_HASH 0
#endif

#include <string>
#include <vector>
#include <sys/socket.h>
#if USE_BOOST_HASH
#include <boost/container_hash/hash.hpp>
#endif

#include "G2ProtocolHandler.h"
#include "NetUtils.h"

static void simple_hash_combine(std::size_t& seed, uint32_t value) {
#if USE_BOOST_HASH
    boost::hash_combine(seed, value);
#else
#if UINTPTR_MAX == 0xffffffffffffffffULL
    static const std::size_t FNV_PRIME = 0x100000001b3ULL;
#else
    static const std::size_t FNV_PRIME = 0x01000193U;
#endif
    seed ^= (std::size_t)value;
    seed *= FNV_PRIME;
#endif
}

struct sockaddr_storage_map {
    struct compAddrAndPort {
        bool operator() (const struct sockaddr_storage& a, const struct sockaddr_storage& b) const {
            return CNetUtils::match(a, b, IMT_ADDRESS_AND_PORT);
        }
    };
    struct hash {
        std::size_t operator() (const sockaddr_storage& a) const {
			switch(a.ss_family)
			{
				case AF_INET: {
					auto ptr4 = ((struct sockaddr_in *)&a);
					size_t res = AF_INET;
					simple_hash_combine(res, ptr4->sin_port);
					simple_hash_combine(res, ptr4->sin_addr.s_addr);
					return res;
				}
				case AF_INET6: {
					auto ptr6 = ((struct sockaddr_in6 *)&a);
					size_t res = AF_INET6;
					simple_hash_combine(res, ptr6->sin6_port);
                    auto in6Ptr = (unsigned int *)&(ptr6->sin6_addr);
					simple_hash_combine(res, in6Ptr[0]);
                    simple_hash_combine(res, in6Ptr[1]);
                    simple_hash_combine(res, in6Ptr[2]);
                    simple_hash_combine(res, in6Ptr[3]);
					return res;
				}
				default:
					return 0U;
			}
        }
    };
};

class CG2ProtocolHandlerPool
{
public:
    CG2ProtocolHandlerPool(unsigned short g2Port, const std::string& address = "");
    ~CG2ProtocolHandlerPool();

    bool open();
    void close();
    G2_TYPE read();
    CAMBEData * readAMBE();
    CHeaderData * readHeader();
    
    bool writeAMBE(const CAMBEData& data);
    bool writeHeader(const CHeaderData& header);

    void traverseNat(const std::string& address);

    void clock(unsigned int ms);

private:
    bool readPackets();
    CG2ProtocolHandler * findHandler(const struct sockaddr_storage& addr, IPMATCHTYPE matchType) const;
    CG2ProtocolHandler * findHandler(in_addr addr, unsigned int port, IPMATCHTYPE matchType) const;

    std::string m_address;
    unsigned int m_basePort;
    CUDPReaderWriter m_socket;
    std::vector<CG2ProtocolHandler *> m_pool;
	std::vector<CG2ProtocolHandler *>::iterator m_index;
};