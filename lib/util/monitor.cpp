//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/arby
//

#include "monitor.hpp"

#include <fmt/chrono.h>
#include <fmt/ostream.h>

#include <tuple>

namespace arby::util
{
std::unordered_map< std::string, std::size_t > monitor::next_index_;
std::set< instance >                           monitor::instances_;

std::ostream &
operator<<(std::ostream &os, instance const &i)
{
    fmt::print(os, "{}:{}:{}", i.name, i.version, i.creation_time);
    return os;
}

bool
operator<(instance const &l, instance const &r)
{
    return std::tie(l.name, l.version, l.creation_time) < std::tie(r.name, r.version, r.creation_time);
}

monitor::sentinel::sentinel(monitor::instance_iter it)
: iter(it)
, valid(true)
{
}
monitor::sentinel::sentinel(monitor::sentinel &&other)
: iter(other.iter)
, valid(std::exchange(other.valid, false))
{
}
monitor::sentinel::~sentinel()
{
    if (valid)
        monitor::erase(iter);
}
monitor::sentinel
monitor::record(std::string name)
{
    auto version = next_index_[name]++;
    auto time    = std::chrono::system_clock::now();
    auto result =
        sentinel(instances_.insert(instance { .name = std::move(name), .version = version, .creation_time = time }).first);
    fmt::print("monitor: create {}\n", *result.iter);
    return result;
}
void
monitor::erase(monitor::instance_iter iter)
{
    fmt::print("monitor: destroy {}\n", *iter);
    instances_.erase(iter);
}
asio::awaitable< void >
monitor::mon()
{
    auto timer = asio::steady_timer(co_await asio::this_coro::executor);
    while (1)
    {
        timer.expires_after(std::chrono::seconds(60));
        co_await timer.async_wait(asio::use_awaitable);
        fmt::print("monitor::mon:-\n");
        for (auto &i : instances_)
        {
            fmt::print(" - {}\n", i);
        }
    }
}
}   // namespace arby::util
