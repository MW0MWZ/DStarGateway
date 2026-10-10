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
#include <string>
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

    static const unsigned char* symbol(char key)
    {
        switch (key) {
            case '0': return DTMF_SYM0; case '1': return DTMF_SYM1; case '2': return DTMF_SYM2;
            case '3': return DTMF_SYM3; case '4': return DTMF_SYM4; case '5': return DTMF_SYM5;
            case '6': return DTMF_SYM6; case '7': return DTMF_SYM7; case '8': return DTMF_SYM8;
            case '9': return DTMF_SYM9; case 'A': return DTMF_SYMA; case 'B': return DTMF_SYMB;
            case 'C': return DTMF_SYMC; case 'D': return DTMF_SYMD; case '*': return DTMF_SYMS;
            default:  return DTMF_SYMH;
        }
    }

    // Key a whole command, then end the transmission
    static std::string send(const std::string& keys)
    {
        CDTMF dtmf;
        for (char key : keys) {
            decode4(dtmf, makeFrame(symbol(key)));
            gap(dtmf);
        }
        gap(dtmf, 10, true);

        return dtmf.translate();
    }

    class DTMF_decode : public ::testing::Test {};

    TEST_F(DTMF_decode, malformed_commands_are_ignored)
    {
        // These used to throw from std::stoul and take the gateway down
        for (const char* keys : { "C12", "D#12", "#D67205", "#*030C", "B#5", "*1#C", "12348", "12" })
            EXPECT_STREQ(send(keys).c_str(), "") << keys;
    }

    TEST_F(DTMF_decode, valid_commands_still_translate)
    {
        EXPECT_STREQ(send("D67205").c_str(), "DCS672EL");
        EXPECT_STREQ(send("D307C").c_str(),  "DCS307CL");
        EXPECT_STREQ(send("*030C").c_str(),  "REF030CL");
        EXPECT_STREQ(send("B001A").c_str(),  "XRF001AL");
        EXPECT_STREQ(send("#").c_str(),      "       U");
        EXPECT_STREQ(send("0").c_str(),      "       I");
        EXPECT_STREQ(send("00").c_str(),     "       I");
        EXPECT_STREQ(send("**").c_str(),     "       L");
    }

#ifndef USE_CCS
    TEST_F(DTMF_decode, ccs_commands_are_ignored_without_ccs)
    {
        EXPECT_STREQ(send("A").c_str(),    "");
        EXPECT_STREQ(send("1234").c_str(), "");
    }
#endif


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
