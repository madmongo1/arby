//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/arby
//

#ifndef ARBY_ARBY_POWER_TRADE_CONNECTION_STATE_HPP
#define ARBY_ARBY_POWER_TRADE_CONNECTION_STATE_HPP

#include "config/asio.hpp"

#include <iosfwd>

namespace arby::power_trade
{

struct connection_state
{
    connection_state(error_code ec = asio::error::not_connected)
    : ec_(ec)
    {
    }

    void
    set(error_code ec)
    {
        ec_ = ec;
    }

    bool
    up() const
    {
        return !ec_;
    }

    bool
    down() const
    {
        return ec_.failed();
    }

    friend std::ostream &
    operator<<(std::ostream &os, connection_state const &cstate);

  private:
    error_code ec_;
};

}   // namespace arby::power_trade

#endif   // ARBY_ARBY_POWER_TRADE_CONNECTION_STATE_HPP
