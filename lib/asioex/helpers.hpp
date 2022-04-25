//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/arby
//

#ifndef ARBY_LIB_ASIOEX_HELPERS_HPP
#define ARBY_LIB_ASIOEX_HELPERS_HPP

#include "config/asio.hpp"

namespace arby
{
namespace asioex
{

#ifndef NDEBUG
bool
on_correct_thread(asio::any_io_executor const &exec);
#endif

}   // namespace asioex
}   // namespace arby

#endif   // ARBY_LIB_ASIOEX_HELPERS_HPP
