//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/router
//

#include "orderbook_listener.hpp"

#include "util/monitor.hpp"

#include <fmt/ostream.h>
#include <util/truncate.hpp>

#include <functional>

namespace arby
{
namespace power_trade
{
    void
    orderbook_listener::on_connection_state(connection_state cstate)
    {
        connstate_ = cstate;
        fmt::print("{}::{} {}\n", classname, __func__, cstate);
        if (connstate_.down())
        {
            stop_monitoring_books_.emit(asio::cancellation_type::terminal);
            msg_channel_.reset();
        }
        else
        {
            auto req =
                json::value({ { "subscribe",
                                { { "market_id", "0" },
                                  { "symbol", symbol_ },
                                  { "type", "snap_full_updates" },
                                  { "interval", "0" },
                                  { "user_tag", my_subscribe_id_ } } } });
            connector_->send(json::serialize(req));
        }
    }

    std::shared_ptr< orderbook_listener >
    orderbook_listener::create(asio::any_io_executor        exec,
                               std::shared_ptr< connector > connector,
                               json::string                 symbol)
    {
        auto impl = std::make_shared< orderbook_listener >(
            std::move(exec), std::move(connector), std::move(symbol));
        impl->start();
        return impl;
    }

    orderbook_listener::orderbook_listener(
        asio::any_io_executor        exec,
        std::shared_ptr< connector > connector,
        json::string                 symbol)
    : util::has_executor_base(std::move(exec))
    , connector_(std::move(connector))
    , symbol_(std::move(symbol))
    {
    }

    void
    orderbook_listener::start()
    {
        asio::co_spawn(get_executor(), run(shared_from_this()), asio::detached);
    }

    asio::awaitable< void >
    orderbook_listener::run(std::shared_ptr< orderbook_listener > self)
    {
        auto [conn, connstate] = co_await connector_->watch_connection_state(
            connector::connection_state_slot(
                [weak = weak_from_this()](connection_state state)
                {
                    if (auto self = weak.lock())
                        asio::post(asio::bind_executor(
                            self->get_executor(),
                            [self, state]
                            { self->on_connection_state(state); }));
                }));

        cmd_response_conn_ = co_await connector_->watch_messages(
            "command_response",
            std::bind(&orderbook_listener::_handle_command_response,
                      weak_from_this(),
                      std::placeholders::_1));

        snapshot_conn_ = co_await connector_->watch_messages(
            "snapshot",
            std::bind(&orderbook_listener::_handle_snapshot,
                      weak_from_this(),
                      std::placeholders::_1));

        remove_conn_ = co_await connector_->watch_messages(
            "order_deleted",
            std::bind(&orderbook_listener::_handle_remove,
                      weak_from_this(),
                      std::placeholders::_1));

        add_conn_ = co_await connector_->watch_messages(
            "order_added",
            std::bind(&orderbook_listener::_handle_add,
                      weak_from_this(),
                      std::placeholders::_1));

        connection_state_conn_ = std::move(conn);
        on_connection_state(connstate);
    }

    void
    orderbook_listener::_handle_command_response(
        std::weak_ptr< orderbook_listener >   weak,
        std::shared_ptr< json::object const > payload)
    {
        fmt::print("{}::{}({})\n", classname, __func__, *payload);
        auto self = weak.lock();
        if (!self)
            return;

        try
        {
            if (payload->at("user_tag").as_string() != self->my_subscribe_id_)
                return;
            asio::dispatch(asio::bind_executor(
                self->get_executor(),
                [self, payload] { self->on_command_response(payload); }));
        }
        catch (std::exception &e)
        {
            fmt::print("{}[{}]::\"command_response\" - exception: {}\n",
                       classname,
                       self->symbol_,
                       e.what());
        }
    }

    void
    orderbook_listener::_handle_snapshot(
        std::weak_ptr< orderbook_listener >   weak,
        std::shared_ptr< json::object const > payload)
    {
        /*
        fmt::print("{}::{}({})\n",
                   classname,
                   __func__,
                   util::truncate(json::serialize(*payload)));
                   */
        if (auto self = weak.lock())
            asio::post(asio::bind_executor(
                self->get_executor(),
                std::bind(
                    &orderbook_listener::handle_snapshot, self, payload)));
    }

    void
    orderbook_listener::handle_snapshot(
        std::shared_ptr< json::object const > payload)
    {
        auto msg = orderbook_snapshot { .payload = payload };
        if (!msg_channel_.try_send(error_code(), std::move(msg)))
        {
            msg_channel_.async_send(error_code(),
                                    std::move(msg),
                                    [self = shared_from_this()](error_code) {});
        }
    }

    void
    orderbook_listener::_handle_add(
        std::weak_ptr< orderbook_listener >   weak,
        std::shared_ptr< json::object const > payload)
    {
        /*
        fmt::print("{}::{}({})\n",
                   classname,
                   __func__,
                   util::truncate(json::serialize(*payload)));
                   */
        if (auto self = weak.lock())
            asio::post(asio::bind_executor(
                self->get_executor(),
                std::bind(&orderbook_listener::handle_add, self, payload)));
    }

    void
    orderbook_listener::handle_add(
        std::shared_ptr< json::object const > payload)
    {
        auto msg = orderbook_add { .payload = payload };
        if (!msg_channel_.try_send(error_code(), std::move(msg)))
        {
            msg_channel_.async_send(error_code(),
                                    std::move(msg),
                                    [self = shared_from_this()](error_code) {});
        }
    }

    void
    orderbook_listener::_handle_remove(
        std::weak_ptr< orderbook_listener >   weak,
        std::shared_ptr< json::object const > payload)
    {
        /*
        fmt::print("{}::{}({})\n",
                   classname,
                   __func__,
                   util::truncate(json::serialize(*payload)));
                   */
        if (auto self = weak.lock())
            asio::post(asio::bind_executor(
                self->get_executor(),
                std::bind(&orderbook_listener::handle_remove, self, payload)));
    }

    void
    orderbook_listener::handle_remove(
        std::shared_ptr< json::object const > payload)
    {
        auto msg = orderbook_remove { .payload = payload };
        if (!msg_channel_.try_send(error_code(), std::move(msg)))
        {
            msg_channel_.async_send(error_code(),
                                    std::move(msg),
                                    [self = shared_from_this()](error_code) {});
        }
    }

    json::string
    orderbook_listener::build_subscribe_id() const
    {
        std::string result = "order_book:";
        result.append(symbol_.begin(), symbol_.end());
        return json::string(result);
    }

    void
    orderbook_listener::on_command_response(
        std::shared_ptr< json::object const > payload)
    {
        fmt::print("{}::{}({})\n", classname, __func__, payload);

        using asio::bind_cancellation_slot;
        using asio::co_spawn;
        using asio::detached;

        if (auto &code = payload->at("error_code"); code == "0")
        {
            msg_channel_.reset();
            // start running the order book monitor
            co_spawn(get_executor(),
                     monitor_book_messages(shared_from_this()),
                     bind_cancellation_slot(stop_monitoring_books_.slot(),
                                            detached));
        }
        else
        {
            // error mode
        }
    }

    asio::awaitable< void >
    orderbook_listener::monitor_book_messages(
        std::shared_ptr< orderbook_listener > self)
    {
        using asio::use_awaitable;

        auto sentinel = util::monitor::record(
            fmt::format("{}[{}]::{}", classname, symbol_, __func__));

        auto actual_op = [this](message const &which)
        {
            if (holds_alternative< orderbook_snapshot >(which))
                on_snapshot(*get< orderbook_snapshot >(which).payload);
            else if (holds_alternative< orderbook_add >(which))
                on_add(*get< orderbook_add >(which).payload);
            else if (holds_alternative< orderbook_remove >(which))
                on_remove(*get< orderbook_remove >(which).payload);
        };
        for (;;)
        {
            error_code err;
            bool       received = false;
            do
            {
                auto op = [&](error_code ec, message const &which)
                {
                    err = ec;
                    if (!err.failed())
                        actual_op(which);
                };
                received = msg_channel_.try_receive(op);
                if (err.failed())
                    throw system_error(err);
            } while (received);
            auto which = co_await msg_channel_.async_receive(use_awaitable);
            actual_op(which);
        }
    }

    void
    orderbook_listener::on_snapshot(json::object const &payload)
    {
        fmt::print("{}::{}({})\n",
                   classname,
                   __func__,
                   util::truncate(json::serialize(payload)));
    }
    void
    orderbook_listener::on_add(json::object const &payload)
    {
        fmt::print("{}::{}({})\n",
                   classname,
                   __func__,
                   util::truncate(json::serialize(payload)));
    }
    void
    orderbook_listener::on_remove(json::object const &payload)
    {
        fmt::print("{}::{}({})\n",
                   classname,
                   __func__,
                   util::truncate(json::serialize(payload)));
    }

}   // namespace power_trade
}   // namespace arby