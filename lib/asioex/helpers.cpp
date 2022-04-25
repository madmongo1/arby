//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/arby
//

#include "helpers.hpp"

namespace arby
{
namespace asioex
{
bool
on_correct_thread(asio::any_io_executor const &exec)
{
    auto const &type = exec.target_type();
    if (type == typeid(asio::strand< asio::io_context::executor_type >))
        return exec.target< asio::strand< asio::io_context::executor_type > >()->running_in_this_thread();
    if (type == typeid(asio::strand< asio::thread_pool::executor_type >))
        return exec.target< asio::strand< asio::thread_pool::executor_type > >()->running_in_this_thread();
    if (type == typeid(asio::io_context::executor_type))
        return exec.target< asio::io_context::executor_type >()->running_in_this_thread();
    if (type == typeid(asio::thread_pool::executor_type))
        return exec.target< asio::thread_pool::executor_type >()->running_in_this_thread();
    assert(!"unsupported executor type");
    return false;
}
}   // namespace asioex
}   // namespace arby