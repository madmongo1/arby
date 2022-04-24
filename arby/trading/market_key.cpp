//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/arby
//

#include "market_key.hpp"

namespace arby::trading
{

std::string to_string(market_key const& mk)
{
    auto to_string_op = [](auto const& x) { return to_string(x); };
    return visit(to_string_op, mk.as_variant());
}

std::ostream& operator<<(std::ostream& os, market_key const& key)
{
    return os << to_string(key);
}



}   // namespace arby::trading
