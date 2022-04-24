//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/arby
//

#include "native_symbol.hpp"

#include <boost/algorithm/string.hpp>
#include <fmt/format.h>

namespace arby
{
namespace power_trade
{
namespace
{
struct converter
{
    static thread_local std::string buffer;

    json::string
    operator()(trading::spot_market_key const &in) const
    {
        buffer = fmt::format("{}-{}", in.base, in.term);
        boost::algorithm::to_upper(buffer);
        return json::string(buffer.begin(), buffer.end());
    }
};

thread_local std::string converter::buffer;
}   // namespace

json::string
native_symbol(trading::market_key const &in)
{
    return visit(converter(), in.as_variant());
}

}   // namespace power_trade
}   // namespace arby