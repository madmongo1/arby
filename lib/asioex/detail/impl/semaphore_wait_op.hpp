//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/arby
//

#ifndef ASIOEX_DETAIL_IMPL_SEMAPHORE_WAIT_OP
#define ASIOEX_DETAIL_IMPL_SEMAPHORE_WAIT_OP

#include <asioex/detail/semaphore_wait_op.hpp>

namespace arby::asioex
{
namespace detail
{
    semaphore_wait_op::semaphore_wait_op(async_semaphore_base *host)
    : host_(host)
    {
    }
}   // namespace detail
}   // namespace arby::asioex

#endif