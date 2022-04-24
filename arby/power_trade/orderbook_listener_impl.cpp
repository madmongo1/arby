//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/arby
//

#include "orderbook_listener_impl.hpp"

#include "util/monitor.hpp"

#include <fmt/ostream.h>
#include <util/truncate.hpp>

#include <functional>

namespace arby
{
namespace power_trade
{

using namespace std::literals;

void
orderbook_listener_impl::on_connection_state(connection_state cstate)
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
        order_book_.reset();
        auto req = json::value({ { "subscribe",
                                   { { "market_id", "0" },
                                     { "symbol", native_symbol(symbol_) },
                                     { "type", "snap_full_updates" },
                                     { "interval", "0" },
                                     { "user_tag", my_subscribe_id_ } } } });
        connector_->send(json::serialize(req));
    }
}

std::shared_ptr< orderbook_listener_impl >
orderbook_listener_impl::create(asio::any_io_executor exec, std::shared_ptr< connector > connector, trading::market_key symbol)
{
    auto impl = std::make_shared< orderbook_listener_impl >(std::move(exec), std::move(connector), std::move(symbol));
    impl->start();
    return impl;
}

orderbook_listener_impl::orderbook_listener_impl(asio::any_io_executor        exec,
                                                 std::shared_ptr< connector > connector,
                                                 trading::market_key          symbol)
: util::has_executor_base(std::move(exec))
, connector_(std::move(connector))
, symbol_(std::move(symbol))
{
}

void
orderbook_listener_impl::start()
{
    asio::co_spawn(get_executor(), run(shared_from_this()), asio::detached);
}

namespace
{
template < class F, class Filter >
struct filter_message
{
    filter_message(F next, Filter filters)
    : next(std::move(next))
    , filters(std::move(filters))
    {
    }

    void
    operator()(std::shared_ptr< json::object const > payload) const
    {
        for (auto &[k, v] : filters)
        {
            auto pv = payload->if_contains(k);
            if (!pv)
                return;
            auto ps = pv->if_string();
            if (!ps)
                return;
            if (*ps != v)
                return;
        }
        next(payload);
    }
    Filter filters;
    F      next;
};

}   // namespace

asio::awaitable< void >
orderbook_listener_impl::run(std::shared_ptr< orderbook_listener_impl > self)
{
    auto [conn, connstate] = co_await connector_->watch_connection_state(connector::connection_state_slot(
        [weak = weak_from_this()](connection_state state)
        {
            if (auto self = weak.lock())
                asio::post(asio::bind_executor(self->get_executor(), [self, state] { self->on_connection_state(state); }));
        }));

    cmd_response_conn_ = co_await connector_->watch_messages(
        "command_response", std::bind(&orderbook_listener_impl::_handle_command_response, weak_from_this(), std::placeholders::_1));

    snapshot_conn_ = co_await connector_->watch_messages(
        "snapshot",
        filter_message(std::bind(&orderbook_listener_impl::_handle_snapshot, weak_from_this(), std::placeholders::_1),
                       make_message_filter()));

    remove_conn_ = co_await connector_->watch_messages(
        "order_deleted",
        filter_message(std::bind(&orderbook_listener_impl::_handle_remove, weak_from_this(), std::placeholders::_1),
                       make_message_filter()));

    add_conn_ = co_await connector_->watch_messages(
        "order_added",
        filter_message(std::bind(&orderbook_listener_impl::_handle_add, weak_from_this(), std::placeholders::_1),
                       make_message_filter()));

    connection_state_conn_ = std::move(conn);
    on_connection_state(connstate);
}

void
orderbook_listener_impl::_handle_command_response(std::weak_ptr< orderbook_listener_impl > weak,
                                                  std::shared_ptr< json::object const >    payload)
{
    fmt::print("{}::{}({})\n", classname, __func__, *payload);
    auto self = weak.lock();
    if (!self)
        return;

    try
    {
        if (payload->at("user_tag").as_string() != self->my_subscribe_id_)
            return;
        asio::dispatch(asio::bind_executor(self->get_executor(), [self, payload] { self->on_command_response(payload); }));
    }
    catch (std::exception &e)
    {
        fmt::print("{}[{}]::\"command_response\" - exception: {}\n", classname, self->symbol_, e.what());
    }
}

void
orderbook_listener_impl::_handle_message(std::weak_ptr< orderbook_listener_impl > weak, message msg)
{
    /*
    fmt::print("{}::{}({})\n",
               classname,
               __func__,
               util::truncate(json::serialize(*payload)));
               */
    if (auto self = weak.lock())
        asio::post(
            asio::bind_executor(self->get_executor(), std::bind(&orderbook_listener_impl::handle_message, self, std::move(msg))));
}

void
orderbook_listener_impl::handle_message(message msg)
{
    if (!msg_channel_.try_send(error_code(), std::move(msg)))
    {
        msg_channel_.async_send(error_code(), std::move(msg), [self = shared_from_this()](error_code) {});
    }
}

void
orderbook_listener_impl::_handle_snapshot(std::weak_ptr< orderbook_listener_impl > weak,
                                          std::shared_ptr< json::object const >    payload)
{
    _handle_message(weak, orderbook_snapshot { .payload = payload });
}

void
orderbook_listener_impl::_handle_add(std::weak_ptr< orderbook_listener_impl > weak, std::shared_ptr< json::object const > payload)
{
    _handle_message(weak, orderbook_add { .payload = payload });
}

void
orderbook_listener_impl::_handle_remove(std::weak_ptr< orderbook_listener_impl > weak,
                                        std::shared_ptr< json::object const >    payload)
{
    _handle_message(weak, orderbook_remove { .payload = payload });
}

json::string
orderbook_listener_impl::build_subscribe_id() const
{
    auto result = "order_book:"s + to_string(symbol_);
    return json::string(result);
}

void
orderbook_listener_impl::on_command_response(std::shared_ptr< json::object const > payload)
{
    fmt::print("{}::{}({})\n", classname, __func__, *payload);

    using asio::bind_cancellation_slot;
    using asio::co_spawn;
    using asio::detached;

    if (auto &code = payload->at("error_code"); code == "0")
    {
        msg_channel_.reset();
        // start running the order book monitor

        co_spawn(
            get_executor(),
            [this]() { return monitor_book_messages(shared_from_this()); },
            bind_cancellation_slot(stop_monitoring_books_.slot(), detached));
    }
    else
    {
        // error mode
    }
}

asio::awaitable< void >
orderbook_listener_impl::monitor_book_messages(std::shared_ptr< orderbook_listener_impl > self)
{
    using asio::use_awaitable;

    auto last_update = std::chrono::system_clock::time_point::min();

    auto sentinel = util::monitor::record(fmt::format("{}[{}]::{}", classname, symbol_, __func__));

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

        if (order_book_.last_update_ > last_update)
        {
            last_update = order_book_.last_update_;
            fmt::print("{}[{}]::{} orderbook:\n{}\n", classname, symbol_, __func__, order_book_);
        }

        auto which = co_await msg_channel_.async_receive(use_awaitable);
        actual_op(which);
    }
}

void
orderbook_listener_impl::on_snapshot(json::object const &payload)
{
    fmt::print("{}::{}({})\n", classname, __func__, util::truncate(json::serialize(payload)));

    auto timestamp = std::chrono::system_clock::time_point(
        std::chrono::microseconds(::atol(payload.at("server_utc_timestamp").as_string().c_str())));

    auto &buys = payload.at("buy").as_array();
    for (auto &v : buys)
    {
        auto &obuy = v.as_object();
        order_book_.add(obuy.at("orderid").as_string(),
                        trading::price_type(obuy.at("price").as_string().c_str()),
                        trading::qty_type(obuy.at("quantity").as_string().c_str()),
                        trading::buy,
                        timestamp);
    }

    auto &sells = payload.at("sell").as_array();
    for (auto &v : sells)
    {
        auto &osell = v.as_object();
        order_book_.add(osell.at("orderid").as_string(),
                        trading::price_type(osell.at("price").as_string().c_str()),
                        trading::qty_type(osell.at("quantity").as_string().c_str()),
                        trading::sell,
                        timestamp);
    }
}
void
orderbook_listener_impl::on_add(json::object const &payload)
{
    fmt::print("{}::{}({})\n", classname, __func__, util::truncate(json::serialize(payload)));

    auto side  = wise_enum::from_string< trading::side_type >(payload.at("side").as_string());
    auto qty   = trading::qty_type(payload.at("quantity").as_string().c_str());
    auto price = trading::price_type(payload.at("price").as_string().c_str());
    auto tsval = ::atoll(payload.at("server_utc_timestamp").as_string().c_str());

    auto timestamp = std::chrono::system_clock::time_point(std::chrono::microseconds(tsval));
    order_book_.add(payload.at("order_id").as_string(), price, qty, side.value(), timestamp);
}
void
orderbook_listener_impl::on_remove(json::object const &payload)
{
    fmt::print("{}::{}({})\n", classname, __func__, util::truncate(json::serialize(payload)));
    auto side      = wise_enum::from_string< trading::side_type >(payload.at("side").as_string());
    auto timestamp = std::chrono::system_clock::time_point(
        std::chrono::microseconds(::atoll(payload.at("server_utc_timestamp").as_string().c_str())));
    order_book_.remove(payload.at("order_id").as_string(), side.value(), timestamp);
}

}   // namespace power_trade
}   // namespace arby