//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/arby
//

#ifndef ARBY_ARBY_TRADING_FEED_CONDITION_HPP
#define ARBY_ARBY_TRADING_FEED_CONDITION_HPP

#include "config/wise_enum.hpp"
#include "trading/types.hpp"

#include <iosfwd>

namespace arby::trading
{

WISE_ENUM(feed_state, good, stale, not_ready, error);
std::ostream &
operator<<(std::ostream &os, feed_state arg);
std::istream &
operator>>(std::istream &is, feed_state &arg);

struct feed_condition
{
    feed_state                 state = feed_state::not_ready;
    std::vector< std::string > errors;

    /// @brief update this condition with the error information in the other
    /// @param r reference to other condition
    void
    merge(feed_condition const &r);

    void merge(feed_state s);

    void reset(feed_state s = feed_state::good);
};

std::ostream &
operator<<(std::ostream &os, feed_condition arg);

}   // namespace arby::trading

#endif   // ARBY_ARBY_TRADING_FEED_CONDITION_HPP
