//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/arby
//

#include "spot_market_key.hpp"

#include <boost/algorithm/string.hpp>
#include <fmt/format.h>

#include <istream>
#include <optional>
#include <ostream>

namespace arby
{
namespace trading
{
namespace
{
std::optional< spot_market_key >
spot_key_impl(std::string_view sv)
{
    auto common_separator = [](char x)
    {
        switch (x)
        {
        case '/':
        case '-':
        case ' ':
        case '_':
            return true;
        default:
            return false;
        }
    };
    auto isep = std::find_if(sv.begin(), sv.end(), common_separator);
    if (isep == sv.end())
        throw std::invalid_argument(fmt::format("trading::spot_key: invalid input [{}]", sv));

    auto result = spot_market_key { .base = std::string(sv.begin(), isep), .term = std::string(isep + 1, sv.end()) };
    boost::algorithm::to_lower(result.base);
    boost::algorithm::to_lower(result.term);
    return result;
}
}   // namespace

std::ostream &
operator<<(std::ostream &os, spot_market_key const &key)
{
    // use to_string to allow the stream to format the resulting string for us
    return os << to_string(key);
}

std::istream &
operator>>(std::istream &is, spot_market_key &key)
{
    std::string buffer;
    is >> buffer;
    if (auto o = spot_key_impl(buffer))
        key = *std::move(o);
    else
        is.setstate(std::ios::failbit);
    return is;
}

std::string
to_string(spot_market_key const &key)
{
    return fmt::format("{}/{}", key.base, key.term);
}

spot_market_key
spot_key(boost::core::string_view sv)
{
    return spot_key(std::string_view(sv.data(), sv.size()));
}

spot_market_key
spot_key(std::string const &sv)
{
    return spot_key(std::string_view(sv.data(), sv.size()));
}

spot_market_key
spot_key(const char *sz)
{
    return spot_key(std::string_view(sz));
}

spot_market_key
spot_key(std::string_view sv)
{
    if (auto o = spot_key_impl(sv))
        return *o;
    else
        throw std::invalid_argument(fmt::format("trading::spot_key: invalid input [{}]", sv));
}

}   // namespace trading
}   // namespace arby