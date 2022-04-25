//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/arby
//
#include "power_trade/tick_history.hpp"

#include <doctest/doctest.h>

using namespace arby;

namespace
{
auto
make_snap()
{
    return power_trade::tick_record { .payload = std::make_shared< json::object >(), .code = power_trade::tick_code::snapshot };
}

auto
make_add()
{
    return power_trade::tick_record { .payload = std::make_shared< json::object >(), .code = power_trade::tick_code::add };
}

}   // namespace
TEST_SUITE("power_trade")
{
    TEST_CASE("tick_history")
    {
        auto hist = power_trade::tick_history();

        // first snapshot
        hist.add_tick(make_snap());
        auto s0 = hist.highest_sequence();
        hist.add_watermark(s0);

        // second snapshot
        hist.add_tick(make_add());
        auto s1 = hist.highest_sequence();
        hist.add_watermark(s1);

        // first snapshot destroyed
        hist.remove_watermark(s0);
        REQUIRE(!hist.history.empty());
        CHECK(hist.history.begin()->first == s1);

        // more snapshots
        hist.add_tick(make_add());
        auto s2 = hist.highest_sequence();
        hist.add_watermark(s2);

        hist.add_tick(make_add());
        auto s3 = hist.highest_sequence();
        hist.add_watermark(s3);

        // now remove in random order
        hist.remove_watermark(s3);
        REQUIRE(!hist.history.empty());
        CHECK(hist.history.begin()->first == s1);

        hist.remove_watermark(s1);
        REQUIRE(!hist.history.empty());
        CHECK(hist.history.begin()->first == s2);

        hist.remove_watermark(s2);
        CHECK(hist.history.empty());
    }
}