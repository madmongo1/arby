//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/arby
//

#ifndef ARBY_LIB_ASIOEX_ASYNC_CONDITION_VARIABLE_HPP
#define ARBY_LIB_ASIOEX_ASYNC_CONDITION_VARIABLE_HPP

#include "config/asio.hpp"

namespace arby::asioex
{

struct async_condition_variable
{
    using executor_type = asio::any_io_executor;

    async_condition_variable(executor_type exec);

    executor_type const &
    get_executor() const;

    asio::awaitable< void >
    wait();

    void
    notify_one();

  private:
    executor_type      exec_;
    asio::steady_timer timer_ { get_executor(), asio::steady_timer ::time_point ::max() };
};

}   // namespace arby::asioex

#endif   // ARBY_LIB_ASIOEX_ASYNC_CONDITION_VARIABLE_HPP
