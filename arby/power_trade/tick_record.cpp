//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/arby
//

#include "power_trade/tick_record.hpp"

#include <fmt/ostream.h>
#include <spdlog/spdlog.h>

namespace arby::power_trade
{
namespace
{
constexpr auto to_price     = [](json::value const &v) { return trading::price_type(v.as_string().c_str()); };
constexpr auto to_qty       = [](json::value const &v) { return trading::qty_type(v.as_string().c_str()); };
constexpr auto to_side      = [](json::value const &v) { return *wise_enum::from_string< trading::side_type >(v.as_string()); };
constexpr auto to_timestamp = [](json::value const &v)
{ return std::chrono::system_clock::time_point(std::chrono::nanoseconds (::atol(v.as_string().c_str()))); };
}   // namespace

auto
tick_record::make_snapshot(json::object const &p) -> snapshot
{
    snapshot result;

    for (auto &e : p.at("buy").as_array())
    {
        auto &oe = e.as_object();
        result.bids.push_back(add { .order_id  = json::value_to< std::string >(oe.at("orderid")),
                                    .price     = to_price(oe.at("price")),
                                    .qty       = to_qty(oe.at("quantity")),
                                    .timestamp = to_timestamp(oe.at("utc_timestamp")),
                                    .side      = trading::side_type::buy });
    }
    for (auto &e : p.at("sell").as_array())
    {
        auto &oe = e.as_object();
        result.offers.push_back(add { .order_id  = json::value_to< std::string >(oe.at("orderid")),
                                      .price     = to_price(oe.at("price")),
                                      .qty       = to_qty(oe.at("quantity")),
                                      .timestamp = to_timestamp(oe.at("utc_timestamp")),
                                      .side      = trading::side_type::sell });
    }

    return result;
}

auto
tick_record::construct2(tick_code code, std::shared_ptr< json::object const > const &payload) -> impl_var
{
    auto &p = *payload;

    switch (code)
    {
    case tick_code::add:
        spdlog::debug("tick_record::{}({}, {}", __func__, code, *payload);
        return add { .order_id  = json::value_to< std::string >(p.at("order_id")),
                     .price     = to_price(p.at("price")),
                     .qty       = to_qty(p.at("quantity")),
                     .timestamp = to_timestamp(p.at("utc_timestamp")),
                     .side      = to_side(p.at("side")) };
    case tick_code::remove:
        spdlog::debug("tick_record::{}({}, {}", __func__, code, *payload);
        return remove { .order_id  = json::value_to< std::string >(p.at("order_id")),
                        .timestamp = to_timestamp(p.at("utc_timestamp")),
                        .side      = to_side(p.at("side")) };
    case tick_code::execute:
        spdlog::debug("tick_record::{}({}, {}", __func__, code, *payload);
        return execute { .order_id  = json::value_to< std::string >(p.at("order_id")),
                         .price     = to_price(p.at("price")),
                         .qty       = to_qty(p.at("quantity")),
                         .timestamp = to_timestamp(p.at("utc_timestamp")),
                         .side      = to_side(p.at("side")) };
    default:
        assert(!"logic error");
    case tick_code::snapshot:
        return make_snapshot(p);
    }
}

auto
tick_record::construct(tick_code code, std::shared_ptr< json::object const > const &payload) -> implementation_type
{
    return std::make_shared< impl_var >(construct2(code, payload));
}

tick_record::tick_record(tick_code code, std::shared_ptr< json::object const > payload)
: impl_(construct(code, payload))
, code_(code)
{
}

}   // namespace arby::power_trade