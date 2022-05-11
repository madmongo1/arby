//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/arby
//

#ifndef ARBY_ARBY_DETAIL_HAS_STOP_SIGNAL_HPP
#define ARBY_ARBY_DETAIL_HAS_STOP_SIGNAL_HPP

#include "asioex/scoped_interrupt.hpp"
#include "config/asio.hpp"

namespace arby::detail
{
struct has_stop_signal
{
    void
    stop()
    {
        asioex::terminate(stop_signal_);
    }

    asio::cancellation_slot
    stop_slot()
    {
        return stop_signal_.slot();
    }

  private:
    asio::cancellation_signal stop_signal_;
};

}   // namespace arby::detail

#endif   // ARBY_ARBY_DETAIL_HAS_STOP_SIGNAL_HPP
