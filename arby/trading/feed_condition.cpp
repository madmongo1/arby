//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/arby
//

#include "feed_condition.hpp"

#include <fmt/ostream.h>
#include <fmt/ranges.h>

namespace arby
{
namespace trading
{

std::ostream &
operator<<(std::ostream &os, feed_state arg)
{
    return os << wise_enum::to_string(arg);
}
std::ostream &
operator<<(std::ostream &os, feed_condition arg)
{
    fmt::print(os, "[state {}][errors {}]", arg.state, arg.errors);
    return os;
}

void
feed_condition::merge(const feed_condition &r)
{
    merge(r.state);
    errors.insert(errors.end(), r.errors.begin(), r.errors.end());
}

void
feed_condition::merge(feed_state s)
{
    state = std::max(state, s);
}

void
feed_condition::reset(feed_state s)
{
    state = s;
    errors.clear();
}

}   // namespace trading
}   // namespace arby