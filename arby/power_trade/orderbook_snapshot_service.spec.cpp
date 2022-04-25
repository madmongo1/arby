//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/arby
//
#include "power_trade/orderbook_snapshot_service.hpp"

#include "config/json.hpp"

#include <doctest/doctest.h>

using namespace arby;
using namespace std::literals;
namespace
{
auto
make_snap()
{
    return power_trade::tick_record { .payload = std::make_shared< json::object >(json::parse(
                                                                                      R"__json(
{
    "server_utc_timestamp":"1000",
    "market_id":"0",
    "symbol":"BTC-USD",
    "buy":[{"price":"39628.00","quantity":"0.00250000","orderid":"1","utc_timestamp":"1"}],
    "sell":[{"price":"39630.00","quantity":"0.00250000","orderid":"2","utc_timestamp":"2"}]
}
)__json")
                                                                                      .as_object()),
                                      .code    = power_trade::tick_code::snapshot };
}

auto
make_add(json::string              orderid,
         trading::side_type        side,
         trading::price_type       price,
         trading::qty_type         qty,
         std::chrono::microseconds ts)
{
    return power_trade::tick_record { .payload = std::make_shared< json::object >(
                                          json::object({ { "orderid", orderid },
                                                         { "side", wise_enum::to_string(side) },
                                                         { "price", to_string(price) },
                                                         { "quantity", to_string(qty) },
                                                         { "utc_timestamp", std::to_string(ts.count()) } })),
                                      .code = power_trade::tick_code::add };
}
}   // namespace

TEST_SUITE("power_trade")
{
    TEST_CASE("orderbook_snapshot_service")
    {
        auto svc   = power_trade::orderbook_snapshot_service();
        auto snap1 = svc.process_tick(make_snap());
        CHECK(!snap1->book.bids_.empty());
        CHECK(snap1->book == svc.orderbook());
        auto snap2 =
            svc.process_tick(make_add("3", trading::side_type::sell, trading::price_type("39631.00"), trading::qty_type("1"), 3us));
        CHECK(snap1->book != snap2->book);
        CHECK(snap2->book == svc.orderbook());
        svc.deallocate_snapshot(std::move(snap1));
        snap1 =
            svc.process_tick(make_add("4", trading::side_type::sell, trading::price_type("39632.00"), trading::qty_type("1"), 4us));
        CHECK(snap1->book != snap2->book);
        CHECK(snap1->book == svc.orderbook());
    }
}
