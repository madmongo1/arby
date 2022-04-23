//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/router
//

#ifndef ARBY_ARBY_POWER_TRADE_ORDER_BOOK_HPP
#define ARBY_ARBY_POWER_TRADE_ORDER_BOOK_HPP

#include "config/json.hpp"
#include "config/wise_enum.hpp"

#include <boost/multiprecision/cpp_dec_float.hpp>

#include <chrono>
#include <list>
#include <map>

namespace arby::power_trade
{
using price_type = boost::multiprecision::cpp_dec_float_50;
using qty_type   = boost::multiprecision::cpp_dec_float_50;

WISE_ENUM(side_type, buy, sell)

struct order_detail
{
    qty_type quantity;
};

struct level_data
{
    using qty_list = std::list< qty_type >;

    qty_type aggregate_depth { 0 };
    qty_list orders {};
};

struct order_book
{
    void
    add(json::string const                   &order_id,
        price_type                            price,
        qty_type                              quantity,
        side_type                             side,
        std::chrono::system_clock::time_point timestamp);

    void
    remove(json::string const                   &order_id,
           side_type                             side,
           std::chrono::system_clock::time_point timestamp);

    void
    reset();

    friend std::ostream &
    operator<<(std::ostream &os, order_book const &book);

    using offer_ladder      = std::map< price_type, level_data, std::less<> >;
    using offer_order_cache = std::map<
        json::string,
        std::tuple< offer_ladder::iterator, level_data::qty_list ::iterator > >;

    using bid_ladder      = std::map< price_type, level_data, std::greater<> >;
    using bid_order_cache = std::map<
        json::string,
        std::tuple< bid_ladder::iterator, level_data::qty_list ::iterator > >;

    std::chrono::system_clock::time_point last_update_;

    offer_ladder      offers_;
    offer_order_cache offer_cache_;
    qty_type          aggregate_offers_;

    bid_ladder      bids_;
    bid_order_cache bid_cache_;
    qty_type        aggregate_bids_;
};

}   // namespace arby::power_trade

#endif   // ARBY_ARBY_POWER_TRADE_ORDER_BOOK_HPP
