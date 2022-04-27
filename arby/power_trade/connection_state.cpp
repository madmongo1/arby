//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/arby
//

#include "power_trade/connection_state.hpp"

#include <ostream>

namespace arby::power_trade
{

std::ostream &
operator<<(std::ostream &os, const connection_state &cstate)
{
    if (cstate.up())
        return os << "up";
    return os << "down (" << cstate.ec_.message() << ')';
}
}   // namespace arby::power_trade
