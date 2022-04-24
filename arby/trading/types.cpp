//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/arby
//

#include "trading/types.hpp"

namespace arby::trading
{
std::ostream &
operator<<(std::ostream &os, side_type side)
{
    return os << wise_enum::to_string(side);
}

std::istream &
operator>>(std::istream &is, side_type &side)
{
    std::string buffer;
    is >> buffer;
    auto opt = wise_enum::from_string< side_type >(buffer);
    if (opt.has_value())
        side = *opt;
    else
        is.setstate(std::ios_base::failbit);
    return is;
}

}   // namespace arby::trading