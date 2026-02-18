/*
 *   Copyright (C) 2021-2026 by Geoffrey Merck F4FXL / KC3FRA
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

#include <array>
#include <gtest/gtest.h>

#include "DTMF.h"
#include "DStarDefines.h"

namespace DTMFTests
{
    using Frame = std::array<unsigned char, 9>;

    static Frame makeFrame(const unsigned char sym[4])
    {
        return Frame{
            DTMF_SIG[0],
            DTMF_SIG[1],
            DTMF_SIG[2],
            DTMF_SIG[3],
            static_cast<unsigned char>(DTMF_SIG[4] | sym[0]),
            static_cast<unsigned char>(DTMF_SIG[5] | sym[1]),
            DTMF_SIG[6],
            static_cast<unsigned char>(DTMF_SIG[7] | sym[2]),
            static_cast<unsigned char>(DTMF_SIG[8] | sym[3]),
        };
    }

    static void decode4(CDTMF& dtmf, const Frame& f, bool last = false)
    {
        dtmf.decode(f.data(), false);
        dtmf.decode(f.data(), false);
        dtmf.decode(f.data(), false);
        dtmf.decode(f.data(), last);
    }

    static void gap(CDTMF& dtmf, unsigned n = 10, bool end = false)
    {
        for (unsigned i = 0; i < n; ++i)
            dtmf.decode(NULL_AMBE_DATA_BYTES, end);
    }

    class DTMF_decode : public ::testing::Test {};

    TEST_F(DTMF_decode, decode_reflector_module_as_number)
    {
        const auto D    = makeFrame(DTMF_SYMD);
        const auto Zero = makeFrame(DTMF_SYM0);
        const auto One  = makeFrame(DTMF_SYM1);
        const auto Eight= makeFrame(DTMF_SYM8);
        const auto Four = makeFrame(DTMF_SYM4);

        CDTMF dtmf;

        decode4(dtmf, D);     gap(dtmf);
        decode4(dtmf, Zero);  gap(dtmf);
        decode4(dtmf, One);   gap(dtmf);
        decode4(dtmf, Eight); gap(dtmf);
        decode4(dtmf, Zero);  gap(dtmf);
        decode4(dtmf, Four);
        gap(dtmf, 10, true);

        EXPECT_TRUE(dtmf.hasCommand());
        EXPECT_STREQ(dtmf.translate().c_str(), "DCS018DL");
    }

    TEST_F(DTMF_decode, decode_reflector_module_as_letter)
    {
        const auto D    = makeFrame(DTMF_SYMD);
        const auto Zero = makeFrame(DTMF_SYM0);
        const auto One  = makeFrame(DTMF_SYM1);
        const auto Eight= makeFrame(DTMF_SYM8);

        CDTMF dtmf;

        decode4(dtmf, D);     gap(dtmf);
        decode4(dtmf, Zero);  gap(dtmf);
        decode4(dtmf, One);   gap(dtmf);
        decode4(dtmf, Eight); gap(dtmf);
        decode4(dtmf, D);
        gap(dtmf, 10, true);

        EXPECT_TRUE(dtmf.hasCommand());
        EXPECT_STREQ(dtmf.translate().c_str(), "DCS018DL");
    }

    TEST_F(DTMF_decode, decode_reflector_short_module_as_letter)
    {
        const auto D     = makeFrame(DTMF_SYMD);
        const auto One   = makeFrame(DTMF_SYM1);
        const auto Eight = makeFrame(DTMF_SYM8);

        CDTMF dtmf;

        decode4(dtmf, D);     gap(dtmf);
        decode4(dtmf, One);   gap(dtmf);
        decode4(dtmf, Eight); gap(dtmf);
        decode4(dtmf, D);
        gap(dtmf, 10, true);

        EXPECT_TRUE(dtmf.hasCommand());
        EXPECT_STREQ(dtmf.translate().c_str(), "DCS018DL");
    }

    TEST_F(DTMF_decode, decode_reflector_short_module_as_number)
    {
        const auto D     = makeFrame(DTMF_SYMD);
        const auto Zero  = makeFrame(DTMF_SYM0);
        const auto One   = makeFrame(DTMF_SYM1);
        const auto Eight = makeFrame(DTMF_SYM8);
        const auto Four  = makeFrame(DTMF_SYM4);

        CDTMF dtmf;

        decode4(dtmf, D);     gap(dtmf);
        decode4(dtmf, One);   gap(dtmf);
        decode4(dtmf, Eight); gap(dtmf);
        decode4(dtmf, Zero);  gap(dtmf);
        decode4(dtmf, Four);
        gap(dtmf, 10, true);

        EXPECT_TRUE(dtmf.hasCommand());
        EXPECT_STREQ(dtmf.translate().c_str(), "DCS018DL");
    }
}
