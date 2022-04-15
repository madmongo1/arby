//
// Copyright (c) 2021 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/asio_experiments
//

#ifndef ASIOEX_IMPL_ASYNC_SEMAPHORE_BASE_HPP
#define ASIOEX_IMPL_ASYNC_SEMAPHORE_BASE_HPP

#include <asioex/error_code.hpp>
#include <asioex/async_semaphore.hpp>
#include <asioex/detail/semaphore_wait_op.hpp>

namespace asioex
{
async_semaphore_base::async_semaphore_base(int initial_count)
: waiters_()
, count_(initial_count)
{
}

async_semaphore_base::~async_semaphore_base()
{
    detail::bilist_node *p = &waiters_;
    while (p->next_ != &waiters_)
    {
        detail::bilist_node *current = p;
        p                            = p->next_;
        static_cast< detail::semaphore_wait_op * >(current)->complete(
            asio::error::operation_aborted);
    }
}

void
async_semaphore_base::add_waiter(detail::semaphore_wait_op *waiter)
{
    waiter->link_before(&waiters_);
}

int
async_semaphore_base::count() const noexcept
{
    return count_;
}

void
async_semaphore_base::release()
{
    count_ += 1;

    // release a pending operations
    if (waiters_.next_ == &waiters_)
        return;

    decrement();
    static_cast< detail::semaphore_wait_op * >(waiters_.next_)
        ->complete(std::error_code());
}

bool
async_semaphore_base::try_acquire()
{
    bool acquired = false;
    if (count_ > 0)
    {
        --count_;
        acquired = true;
    }
    return acquired;
}

int
async_semaphore_base::decrement()
{
    BOOST_ASIO_ASSERT(count_ > 0);
    return --count_;
}

}   // namespace asioex

#endif