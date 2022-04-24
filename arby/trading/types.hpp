//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/arby
//

#ifndef ARBY_ARBY_TRADING_TYPES_HPP
#define ARBY_ARBY_TRADING_TYPES_HPP

#include "config/wise_enum.hpp"

#include <boost/multiprecision/cpp_dec_float.hpp>

namespace arby::trading
{
using qty_type   = boost::multiprecision::cpp_dec_float_50;
using price_type = boost::multiprecision::cpp_dec_float_50;
using timestamp_type = std::chrono::system_clock::time_point;

WISE_ENUM(side_type, buy, sell)
std::ostream& operator<<(std::ostream& os, side_type side);
std::istream& operator>>(std::istream& is, side_type &side);

}   // namespace arby::trading

#endif   // ARBY_ARBY_TRADING_TYPES_HPP
