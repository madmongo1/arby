//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/arby
//

#ifndef ARBY_ARBY_TRADING_SPOT_MARKET_KEY_HPP
#define ARBY_ARBY_TRADING_SPOT_MARKET_KEY_HPP

#include <boost/json/string_view.hpp>
#include <boost/utility/string_view.hpp>

#include <iosfwd>
#include <string>
#include <string_view>

namespace arby
{
namespace trading
{

struct spot_market_key
{
    std::string base;
    std::string term;
};
std::ostream &
operator<<(std::ostream &os, spot_market_key const &key);
std::istream &
operator>>(std::istream &is, spot_market_key &key);
std::string
to_string(spot_market_key const &key);

spot_market_key
spot_key(boost::core::string_view sv);

spot_market_key
spot_key(std::string_view sv);

spot_market_key
spot_key(std::string const &sv);

spot_market_key
spot_key(const char *sz);

}   // namespace trading
}   // namespace arby

#endif   // ARBY_ARBY_TRADING_SPOT_MARKET_KEY_HPP
