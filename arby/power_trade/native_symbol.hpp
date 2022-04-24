//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/arby
//

#ifndef ARBY_ARBY_POWER_TRADE_NATIVE_SYMBOL_HPP
#define ARBY_ARBY_POWER_TRADE_NATIVE_SYMBOL_HPP

#include "config/json.hpp"
#include "trading/market_key.hpp"

namespace arby
{
namespace power_trade
{

json::string
native_symbol(trading::market_key const &in);

}   // namespace power_trade
}   // namespace arby

#endif   // ARBY_ARBY_POWER_TRADE_NATIVE_SYMBOL_HPP
