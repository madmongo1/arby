//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/arby
//

#ifndef ARBY_ARBY_POWER_TRADE_MARKET_DATA_TICK_HPP
#define ARBY_ARBY_POWER_TRADE_MARKET_DATA_TICK_HPP
#include "trading/types.hpp"

#include <boost/variant2/variant.hpp>

#include <memory>

namespace arby
{
namespace power_trade
{
namespace market_data
{

struct tick_base
{
    trading::timestamp_type timestamp;
};

struct impl_add_tick : tick_base
{
    std::string         order_id;
    trading::qty_type   qty;
    trading::price_type price;
    trading::side_type  side;
};

struct impl_delete_tick : tick_base
{
    std::string         order_id;
    trading::qty_type   qty;
    trading::price_type price;
};

struct impl_execute_tick : tick_base
{
    std::string         order_id;
    trading::qty_type   qty;
    trading::price_type price;
    trading::side_type  side;
};

struct impl_snapshot_tick : tick_base
{
};

using impl_tick_variant = boost::variant2::variant< impl_add_tick, impl_delete_tick, impl_execute_tick, impl_snapshot_tick >;

struct tick
{
    std::shared_ptr< impl_tick_variant const > impl_;
};

template < class F >
auto
visit(F &&);

}   // namespace market_data
}   // namespace power_trade
}   // namespace arby

#endif   // ARBY_ARBY_POWER_TRADE_MARKET_DATA_TICK_HPP
