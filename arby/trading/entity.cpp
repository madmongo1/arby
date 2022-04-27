//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/arby
//

#include "entity.hpp"

#include <fmt/format.h>

#include <map>
#include "trading/entity_service.hpp"

namespace arby
{
namespace trading
{
namespace
{
asio::awaitable< std::string >
summarise(std::shared_ptr< entity_impl > impl)
{
    // spawn a thread on the impl's executor to build the summary string
    co_return co_await co_spawn(impl->get_executor(), impl->summary(), asio::use_awaitable);
}
}   // namespace

asio::awaitable< void >
entity_impl_service::enumerate(std::string &body) const
{
    using asio::redirect_error;
    using asio::use_awaitable;

    auto this_exec = co_await asio::this_coro::executor;
    auto waiting   = int(0);
    auto cv        = asio::steady_timer(this_exec, asio::steady_timer::time_point::max());
    std::multimap< std::tuple< entity_key, std::uint64_t >, std::string > rows;

    for (auto &[key, weak] : this->cache_)
    {
        if (auto strong = weak.lock())
        {
            ++waiting;
            // start async task with completion handler that notifies the condition variable this task should complete on the
            // current executor
            asio::co_spawn(this_exec,
                           summarise(strong),
                           [&](std::exception_ptr, std::string summary)
                           {
                               rows.emplace(std::make_tuple(strong->key(), strong->version()), summary);
                               if (--waiting == 0)
                                   cv.cancel_one();
                               ;
                           });
        }
    }

    error_code ec;
    while (waiting)
        co_await cv.async_wait(redirect_error(use_awaitable, ec));

    // summary is now ready
    for (auto &[k, summary] : rows)
    {
        auto &[key, version] = k;
        body += fmt::format("{}:{} {}\n", key, version, summary);
    }

    co_return;
}

entity_impl::entity_impl(entity_service_impl *service, entity_impl::executor_type exec, entity_key key, int version)
    : service_(service)
    , exec_(std::move(exec))
    , key_(std::move(key))
    , version_(version)
{
}
void
entity_impl::start()
{
    handle_start();
}

}   // namespace trading
}   // namespace arby