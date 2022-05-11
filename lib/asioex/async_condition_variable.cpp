//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/arby
//

#include "async_condition_variable.hpp"

namespace arby
{
namespace asioex
{
async_condition_variable::async_condition_variable(async_condition_variable::executor_type exec)
    : exec_(exec)
{
}
const async_condition_variable::executor_type &
async_condition_variable::get_executor() const
{
    return exec_;
}
asio::awaitable< void >
async_condition_variable::wait()
{
    error_code ec;
    co_await timer_.async_wait(asio::redirect_error(asio::use_awaitable, ec));
    if (asio::cancellation_state cstate = co_await asio::this_coro::cancellation_state;
        cstate.cancelled() == asio::cancellation_type::none)
        ec.clear();
    else
        ec = asio::error::operation_aborted;
    if (ec)
        throw system_error(ec);
}
void
async_condition_variable::notify_one()
{
    timer_.cancel_one();
}
}   // namespace asioex
}   // namespace arby