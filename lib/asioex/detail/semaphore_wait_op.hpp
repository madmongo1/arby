//
// Copyright (c) 2021 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/asio_experiments
//

#ifndef ASIOEX_DETAIL_SEMAPHORE_WAIT_OP
#define ASIOEX_DETAIL_SEMAPHORE_WAIT_OP

#include <asioex/detail/bilist_node.hpp>
#include <asioex/error_code.hpp>

namespace asioex
{
struct async_semaphore_base;

namespace detail
{
struct semaphore_wait_op : detail::bilist_node
{
    semaphore_wait_op(async_semaphore_base *host);

    virtual void complete(error_code) = 0;

    async_semaphore_base *host_;
};

}   // namespace detail
}   // namespace asioex

#endif

#include <asioex/detail/impl/semaphore_wait_op.hpp>