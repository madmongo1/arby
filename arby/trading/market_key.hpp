//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/arby
//

#ifndef ARBY_ARBY_TRADING_MARKET_KEY_HPP
#define ARBY_ARBY_TRADING_MARKET_KEY_HPP

#include "trading/spot_market_key.hpp"

#include <boost/variant2.hpp>

namespace arby
{
namespace trading
{

struct market_key
{
    market_key(spot_market_key key)
    : var_(std::move(key))
    {
    }

    using vtype = boost::variant2::variant< spot_market_key >;

    vtype &
    as_variant()
    {
        return var_;
    }
    vtype const &
    as_variant() const
    {
        return var_;
    }

    vtype var_;
};

std::string
to_string(market_key const &mk);

std::ostream &
operator<<(std::ostream &os, market_key const &key);

}   // namespace trading
}   // namespace arby

#endif   // ARBY_ARBY_TRADING_MARKET_KEY_HPP
