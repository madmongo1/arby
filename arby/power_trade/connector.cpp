//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/arby
//

#include "connector.hpp"

namespace arby
{
namespace power_trade
{
    connector::connector(asio::any_io_executor exec, ssl::context &ioc)
    : impl_(std::make_shared< connector_impl >(exec, ioc))
    {
        impl_->start();
    }

    asio::awaitable< util::cross_executor_connection >
    connector::watch_messages(json::string type, message_slot slot)
    {
        using asio::co_spawn;
        using asio::use_awaitable;

        auto this_exec = co_await asio::this_coro::executor;
        auto my_exec   = impl_->get_executor();

        if (this_exec == my_exec)
        {
            co_return util::cross_executor_connection { impl_, impl_->watch_messages(std::move(type), std::move(slot)) };
        }
        else
        {
            co_return co_await co_spawn(
                my_exec,
                [&]() -> asio::awaitable< util::cross_executor_connection > {
                    co_return util::cross_executor_connection { impl_, impl_->watch_messages(std::move(type), std::move(slot)) };
                },
                use_awaitable);
        }
    }

    asio::awaitable< std::tuple< util::cross_executor_connection, connection_state > >
    connector::watch_connection_state(connection_state_slot slot)
    {
        using asio::co_spawn;
        using asio::use_awaitable;

        auto this_exec = co_await asio::this_coro::executor;
        auto my_exec   = impl_->get_executor();

        connection_state current;

        if (this_exec == my_exec)
        {
            auto conn = impl_->watch_connection_state(current, std::move(slot));
            co_return std::make_tuple(util::cross_executor_connection(impl_, conn), current);
        }
        else
        {
            co_return co_await co_spawn(
                my_exec,
                [&]() -> asio::awaitable< std::tuple< util::cross_executor_connection, connection_state > >

                {
                    auto conn = impl_->watch_connection_state(current, std::move(slot));
                    co_return std::make_tuple(util::cross_executor_connection(impl_, conn), current);
                },
                use_awaitable);
        }
    }

}   // namespace power_trade
}   // namespace arby