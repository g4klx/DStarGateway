/*
 *   Copyright (C) 2021-2024 by Geoffrey Merck F4FXL / KC3FRA
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

#include <gtest/gtest.h>

#include "DTMF.h"

const unsigned char DTMF_MASK[] = {0x82U, 0x08U, 0x20U, 0x82U, 0x00U, 0x00U, 0x82U, 0x00U, 0x00U};
const unsigned char DTMF_SIG[]  = {0x82U, 0x08U, 0x20U, 0x82U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U};

const unsigned char NULL_AMBE_DATA_BYTES[] = {0x9E, 0x8D, 0x32, 0x88, 0x26, 0x1A, 0x3F, 0x61, 0xE8};

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

namespace DTMFTests
{
    class DTMF_decode : public ::testing::Test {
    
    };

    TEST_F(DTMF_decode, decode_reflector_module_as_number)
    {
        unsigned char D[] = {DTMF_SIG[0],
                                DTMF_SIG[1],
                                DTMF_SIG[2],
                                DTMF_SIG[3],
                                (unsigned char)(DTMF_SIG[4] | DTMF_SYMD[0]),
                                (unsigned char)(DTMF_SIG[5] | DTMF_SYMD[1]),
                                DTMF_SIG[6],
                                (unsigned char)(DTMF_SIG[7] | DTMF_SYMD[2]),
                                (unsigned char)(DTMF_SIG[8] | DTMF_SYMD[3])};
        unsigned char Zero[] = {DTMF_SIG[0],
                                DTMF_SIG[1],
                                DTMF_SIG[2],
                                DTMF_SIG[3],
                                (unsigned char)(DTMF_SIG[4] | DTMF_SYM0[0]),
                                (unsigned char)(DTMF_SIG[5] | DTMF_SYM0[1]),
                                DTMF_SIG[6],
                                (unsigned char)(DTMF_SIG[7] | DTMF_SYM0[2]),
                                (unsigned char)(DTMF_SIG[8] | DTMF_SYM0[3])};

        unsigned char One[] = {DTMF_SIG[0],
                                DTMF_SIG[1],
                                DTMF_SIG[2],
                                DTMF_SIG[3],
                                (unsigned char)(DTMF_SIG[4] | DTMF_SYM1[0]),
                                (unsigned char)(DTMF_SIG[5] | DTMF_SYM1[1]),
                                DTMF_SIG[6],
                                (unsigned char)(DTMF_SIG[7] | DTMF_SYM1[2]),
                                (unsigned char)(DTMF_SIG[8] | DTMF_SYM1[3])};

        
        unsigned char Eight[] = {DTMF_SIG[0],
                                DTMF_SIG[1],
                                DTMF_SIG[2],
                                DTMF_SIG[3],
                                (unsigned char)(DTMF_SIG[4] | DTMF_SYM8[0]),
                                (unsigned char)(DTMF_SIG[5] | DTMF_SYM8[1]),
                                DTMF_SIG[6],
                                (unsigned char)(DTMF_SIG[7] | DTMF_SYM8[2]),
                                (unsigned char)(DTMF_SIG[8] | DTMF_SYM8[3])};
        
        unsigned char Four[] = {DTMF_SIG[0],
                                DTMF_SIG[1],
                                DTMF_SIG[2],
                                DTMF_SIG[3],
                                (unsigned char)(DTMF_SIG[4] | DTMF_SYM4[0]),
                                (unsigned char)(DTMF_SIG[5] | DTMF_SYM4[1]),
                                DTMF_SIG[6],
                                (unsigned char)(DTMF_SIG[7] | DTMF_SYM4[2]),
                                (unsigned char)(DTMF_SIG[8] | DTMF_SYM4[3])};
                            

        CDTMF dtmf;
        dtmf.decode(D, false);
        dtmf.decode(D, false);
        dtmf.decode(D, false);
        dtmf.decode(D, false);
        for(uint i = 0; i < 10U; i++) dtmf.decode(NULL_AMBE_DATA_BYTES, false);
        dtmf.decode(Zero, false);
        dtmf.decode(Zero, false);
        dtmf.decode(Zero, false);
        dtmf.decode(Zero, false);
        for(uint i = 0; i < 10U; i++) dtmf.decode(NULL_AMBE_DATA_BYTES, false);
        dtmf.decode(One, false);
        dtmf.decode(One, false);
        dtmf.decode(One, false);
        dtmf.decode(One, false);
        for(uint i = 0; i < 10U; i++) dtmf.decode(NULL_AMBE_DATA_BYTES, false);
        dtmf.decode(Eight, false);
        dtmf.decode(Eight, false);
        dtmf.decode(Eight, false);
        dtmf.decode(Eight, false);
        for(uint i = 0; i < 10U; i++) dtmf.decode(NULL_AMBE_DATA_BYTES, false);
        dtmf.decode(Zero, false);
        dtmf.decode(Zero, false);
        dtmf.decode(Zero, false);
        dtmf.decode(Zero, false);
        for(uint i = 0; i < 10U; i++) dtmf.decode(NULL_AMBE_DATA_BYTES, false);
        dtmf.decode(Four, false);
        dtmf.decode(Four, false);
        dtmf.decode(Four, false);
        dtmf.decode(Four, false);
        dtmf.decode(Four, true);
        for(uint i = 0; i < 10U; i++) dtmf.decode(NULL_AMBE_DATA_BYTES, false);

        EXPECT_TRUE(dtmf.hasCommand());
        EXPECT_STREQ(dtmf.translate().c_str(), "DCS018DL");
    }

    TEST_F(DTMF_decode, decode_reflector_module_as_letter)
    {
        unsigned char D[] = {DTMF_SIG[0],
                                DTMF_SIG[1],
                                DTMF_SIG[2],
                                DTMF_SIG[3],
                                (unsigned char)(DTMF_SIG[4] | DTMF_SYMD[0]),
                                (unsigned char)(DTMF_SIG[5] | DTMF_SYMD[1]),
                                DTMF_SIG[6],
                                (unsigned char)(DTMF_SIG[7] | DTMF_SYMD[2]),
                                (unsigned char)(DTMF_SIG[8] | DTMF_SYMD[3])};
        unsigned char Zero[] = {DTMF_SIG[0],
                                DTMF_SIG[1],
                                DTMF_SIG[2],
                                DTMF_SIG[3],
                                (unsigned char)(DTMF_SIG[4] | DTMF_SYM0[0]),
                                (unsigned char)(DTMF_SIG[5] | DTMF_SYM0[1]),
                                DTMF_SIG[6],
                                (unsigned char)(DTMF_SIG[7] | DTMF_SYM0[2]),
                                (unsigned char)(DTMF_SIG[8] | DTMF_SYM0[3])};

        unsigned char One[] = {DTMF_SIG[0],
                                DTMF_SIG[1],
                                DTMF_SIG[2],
                                DTMF_SIG[3],
                                (unsigned char)(DTMF_SIG[4] | DTMF_SYM1[0]),
                                (unsigned char)(DTMF_SIG[5] | DTMF_SYM1[1]),
                                DTMF_SIG[6],
                                (unsigned char)(DTMF_SIG[7] | DTMF_SYM1[2]),
                                (unsigned char)(DTMF_SIG[8] | DTMF_SYM1[3])};

        
        unsigned char Eight[] = {DTMF_SIG[0],
                                DTMF_SIG[1],
                                DTMF_SIG[2],
                                DTMF_SIG[3],
                                (unsigned char)(DTMF_SIG[4] | DTMF_SYM8[0]),
                                (unsigned char)(DTMF_SIG[5] | DTMF_SYM8[1]),
                                DTMF_SIG[6],
                                (unsigned char)(DTMF_SIG[7] | DTMF_SYM8[2]),
                                (unsigned char)(DTMF_SIG[8] | DTMF_SYM8[3])};                          

        CDTMF dtmf;
        dtmf.decode(D, false);
        dtmf.decode(D, false);
        dtmf.decode(D, false);
        dtmf.decode(D, false);
        for(uint i = 0; i < 10U; i++) dtmf.decode(NULL_AMBE_DATA_BYTES, false);
        dtmf.decode(Zero, false);
        dtmf.decode(Zero, false);
        dtmf.decode(Zero, false);
        dtmf.decode(Zero, false);
        for(uint i = 0; i < 10U; i++) dtmf.decode(NULL_AMBE_DATA_BYTES, false);
        dtmf.decode(One, false);
        dtmf.decode(One, false);
        dtmf.decode(One, false);
        dtmf.decode(One, false);
        for(uint i = 0; i < 10U; i++) dtmf.decode(NULL_AMBE_DATA_BYTES, false);
        dtmf.decode(Eight, false);
        dtmf.decode(Eight, false);
        dtmf.decode(Eight, false);
        dtmf.decode(Eight, false);
        for(uint i = 0; i < 10U; i++) dtmf.decode(NULL_AMBE_DATA_BYTES, false);
        dtmf.decode(D, false);
        dtmf.decode(D, false);
        dtmf.decode(D, false);
        dtmf.decode(D, false);
        for(uint i = 0; i < 10U; i++) dtmf.decode(NULL_AMBE_DATA_BYTES, true);

        EXPECT_TRUE(dtmf.hasCommand());
        EXPECT_STREQ(dtmf.translate().c_str(), "DCS018DL");
    }
}