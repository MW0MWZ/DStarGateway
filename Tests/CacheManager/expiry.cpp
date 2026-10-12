/*
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

#include <chrono>
#include <thread>
#include <gtest/gtest.h>

#include "CacheManager.h"

namespace CacheManagerTests
{
    class CacheManager_expiry : public ::testing::Test {};

    TEST_F(CacheManager_expiry, find_miss_does_not_insert)
    {
        CUserCache users;
        CRepeaterCache repeaters;
        CGatewayCache gateways;

        EXPECT_EQ(users.find("NOBODY  "), nullptr);
        EXPECT_EQ(repeaters.find("NOWHERE "), nullptr);
        EXPECT_EQ(gateways.find("NOGW   G"), nullptr);

        EXPECT_EQ(users.getCount(), 0U);
        EXPECT_EQ(repeaters.getCount(), 0U);
        EXPECT_EQ(gateways.getCount(), 0U);
    }

    TEST_F(CacheManager_expiry, user_moves_only_on_newer_timestamp)
    {
        CUserCache users;

        users.update("MW0MWZ  ", "MW0MWZ B", "2026-10-10 10:00:00");
        users.update("MW0MWZ  ", "VA3UV  C", "2026-10-10 09:00:00");
        EXPECT_EQ(users.find("MW0MWZ  ")->getRepeater(), "MW0MWZ B");

        users.update("MW0MWZ  ", "VA3UV  C", "2026-10-10 11:00:00");
        EXPECT_EQ(users.find("MW0MWZ  ")->getRepeater(), "VA3UV  C");
    }

    TEST_F(CacheManager_expiry, records_expire_and_touch_restarts_lifetime)
    {
        CUserRecord rec("MW0MWZ  ", "MW0MWZ B", "2026-10-10 10:00:00");
        std::this_thread::sleep_for(std::chrono::milliseconds(5));

        EXPECT_TRUE(rec.isExpired(std::chrono::milliseconds(1)));
        EXPECT_FALSE(rec.isExpired(std::chrono::minutes(5)));

        rec.touch();
        EXPECT_FALSE(rec.isExpired(std::chrono::seconds(1)));
    }

    TEST_F(CacheManager_expiry, learned_gateway_becomes_locked)
    {
        CGatewayCache gateways;

        gateways.update("XRF001 G", "1.2.3.4", DP_DEXTRA, false, false);   // learned from ircDDB
        EXPECT_FALSE(gateways.find("XRF001 G")->isLocked());

        gateways.update("XRF001 G", "1.2.3.4", DP_DEXTRA, false, true);    // hosts file
        EXPECT_TRUE(gateways.find("XRF001 G")->isLocked());

        gateways.update("XRF001 G", "5.6.7.8", DP_DCS, false, false);      // ircDDB can't unlock it
        EXPECT_TRUE(gateways.find("XRF001 G")->isLocked());
        EXPECT_EQ(gateways.find("XRF001 G")->getProtocol(), DP_DEXTRA);
    }

    TEST_F(CacheManager_expiry, local_repeater_is_locked)
    {
        CRepeaterCache repeaters;

        repeaters.update("GB7AB  C", "MW0MWZ G", true);                    // local repeater
        repeaters.update("GB7AB  C", "OTHER  G", false);                   // ircDDB
        EXPECT_TRUE(repeaters.find("GB7AB  C")->isLocked());
        EXPECT_EQ(repeaters.find("GB7AB  C")->getGateway(), "MW0MWZ G");

        repeaters.prune();
        EXPECT_EQ(repeaters.getCount(), 1U);
    }

    TEST_F(CacheManager_expiry, find_gateway_miss_releases_lock)
    {
        CCacheManager cache;

        // The second call used to deadlock
        EXPECT_EQ(cache.findGateway("NOGW   G"), nullptr);
        EXPECT_EQ(cache.findGateway("NOGW   G"), nullptr);
    }

    TEST_F(CacheManager_expiry, clear_users_on_reconnect)
    {
        CCacheManager cache;

        cache.updateUser("MW0MWZ  ", "MW0MWZ B", "MW0MWZ G", "9.9.9.9", "2026-10-10 10:00:00", DP_DEXTRA, false, false);
        CUserData* user = cache.findUser("MW0MWZ  ");
        ASSERT_NE(user, nullptr);
        EXPECT_EQ(user->getGateway(), "MW0MWZ G");
        delete user;

        cache.clearUsers();
        EXPECT_EQ(cache.findUser("MW0MWZ  "), nullptr);
    }
}
