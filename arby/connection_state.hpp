//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/arby
//

#ifndef ARBY_CONNECTION_STATE_HPP
#define ARBY_CONNECTION_STATE_HPP

#include "config/wise_enum.hpp"

namespace arby
{

WISE_ENUM(connection_state, connection_down, connection_unauthenticated, connection_up)
template<class Stream>
decltype(auto) operator<<(Stream& os, connection_state cs)
{
    return os << wise_enum::to_string(cs);
}


}   // namespace arby

#endif   // ARBY_CONNECTION_STATE_HPP
