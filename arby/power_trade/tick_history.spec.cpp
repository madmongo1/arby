//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/arby
//
#include "power_trade/tick_history.hpp"

#include "testing/test_environment.spec.hpp"

#include <doctest/doctest.h>

#include <fstream>
#include <string>
#include <tuple>

using namespace arby;

namespace
{
struct
{
    std::ifstream ifs { testing::source_root() / "arby/power_trade/test_data/eth-usd.txt" };

    std::tuple< std::string, std::shared_ptr< json::object const > >
    next()
    {
        std::string                           buffer;
        std::string                           type = "eof";
        std::shared_ptr< json::object const > payload;
        if (std::getline(ifs, buffer))
        {
            auto pv = std::make_shared< json::value const >(json::parse(buffer));

            if (auto &outer = pv->as_object(); !outer.empty())
            {
                auto &[k, v] = *outer.begin();
                type.assign(k.begin(), k.end());
                payload = std::shared_ptr< json::object const >(pv, &v.as_object());
            }
        }
        return std::make_tuple(type, payload);
    }

} tick_source;

}   // namespace
TEST_SUITE("power_trade")
{
    TEST_CASE("tick_history")
    {
        auto hist = power_trade::tick_history();

        std::string                           type;
        std::shared_ptr< json::object const > payload;

        auto to_code = [](std::string const &type)
        {
            auto result = power_trade::tick_code();
            if (type == "snapshot")
                result = power_trade::tick_code::snapshot;
            else if (type == "order_added")
                result = power_trade::tick_code::add;
            else if (type == "order_deleted")
                result = power_trade::tick_code::remove;
            else if (type == "order_executed")
                result = power_trade::tick_code::execute;
            else
                FAIL("invalid code");
            return result;
        };

        auto process_next = [&]
        {
            std::tie(type, payload) = tick_source.next();
            hist.add_tick(power_trade::tick_record(to_code(type), payload));
        };

        // first snapshot
        process_next();
        auto s0 = hist.highest_sequence();
        hist.add_watermark(s0);

        // second snapshot
        process_next();
        auto s1 = hist.highest_sequence();
        hist.add_watermark(s1);

        // first snapshot destroyed
        hist.remove_watermark(s0);
        REQUIRE(!hist.history.empty());
        CHECK(hist.history.begin()->first == s1);

        // more snapshots
        process_next();
        auto s2 = hist.highest_sequence();
        hist.add_watermark(s2);

        process_next();
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