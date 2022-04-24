//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/arby
//

#ifndef ARBY_ARBY_TRADING_AGGREGATE_BOOK_HPP
#define ARBY_ARBY_TRADING_AGGREGATE_BOOK_HPP

#include "trading/market_key.hpp"
#include "trading/types.hpp"

#include <chrono>
#include <iosfwd>
#include <vector>

namespace arby::trading
{

struct price_depth
{
    price_type price;
    qty_type   depth;
};

struct bid_ladder : std::vector< price_depth >
{
};

struct offer_ladder : std::vector< price_depth >
{
    std::vector< price_depth > data_;
};

struct aggregate_book
{
    market_key     market;
    timestamp_type timestamp;
    bid_ladder     bids;
    offer_ladder   offers;
};

}   // namespace arby::trading

#endif   // ARBY_ARBY_TRADING_AGGREGATE_BOOK_HPP
