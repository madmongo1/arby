//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/arby
//

#ifndef ARBY_LIB_UTIL_MONITOR_HPP
#define ARBY_LIB_UTIL_MONITOR_HPP

#include "config/asio.hpp"

#include <chrono>
#include <ostream>
#include <set>
#include <string>
#include <unordered_map>

namespace arby::util
{
struct instance
{
    std::string                           name;
    std::size_t                           version;
    std::chrono::system_clock::time_point creation_time;
};

std::ostream &
operator<<(std::ostream &os, instance const &i);

bool
operator<(instance const &l, instance const &r);

struct monitor
{
    static std::unordered_map< std::string, std::size_t > next_index_;
    static std::set< instance >                           instances_;
    using instance_iter = std::set< instance >::iterator;

    struct sentinel
    {
        sentinel(instance_iter it);
        sentinel(sentinel &&other);

        sentinel &
        operator=(sentinel const &) = delete;
        ~sentinel();

        instance_iter iter;
        bool          valid;
    };

    static sentinel
    record(std::string name);

    static void
    erase(instance_iter iter, std::exception_ptr ep);

    static asio::awaitable< void >
    mon();
};

}   // namespace arby::util

#endif   // ARBY_LIB_UTIL_MONITOR_HPP
