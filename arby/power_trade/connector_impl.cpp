//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/router
//

#include "connector_impl.hpp"

#include "network/connect_ssl.hpp"
#include "util/monitor.hpp"
#include "util/truncate.hpp"

#include <boost/scope_exit.hpp>
#include <fmt/chrono.h>
#include <fmt/format.h>
#include <fmt/ostream.h>

namespace arby::power_trade
{

using namespace std::literals;

connector_impl::connector_impl(executor_type exec, ssl::context &sslctx)
: exec_(exec)
, ssl_ctx_(sslctx)
{
}

void
connector_impl::start()
{
    asio::co_spawn(get_executor(),
                   run(shared_from_this()),
                   asio::bind_cancellation_slot(stop_.slot(), asio::detached));
}

void
connector_impl::stop()
{
    asio::dispatch(get_executor(),
                   [self = shared_from_this()]
                   {
                       fmt::print("{}::{}\n", classname, "stop");
                       assert(!self->stopped_);
                       self->stopped_ = true;
                       self->stop_.emit(asio::cancellation_type::terminal);
                   });
}
void
connector_impl::send(std::string s)
{
    send_queue_.push_back(std::move(s));
    send_cv_.cancel();
}

void
connector_impl::interrupt()
{
    interrupt_connection_.emit(asio::cancellation_type::all);
}

asio::awaitable< void >
connector_impl::run(std::shared_ptr< connector_impl > self)
{
    using asio::use_awaitable;
    auto sentinel = util::monitor::record("power_trade_connector::run");

    for (;;)
    {
        try
        {
            co_await run_connection();
        }
        catch (std::exception &e)
        {
            fmt::print(
                "{}::{} : exception : {}", classname, __func__, e.what());
        }
        if (stopped_)
            break;
        auto t = asio::steady_timer(get_executor(), 2s);
        fmt::print("{}::{} : reconnect delay...\n", classname, __func__);
        co_await t.async_wait(use_awaitable);
    }
}

asio::awaitable< void >
connector_impl::run_connection()
{
    auto sentinel =
        util::monitor::record("power_trade_connector::run_connection");
    try
    {
        using asio::bind_cancellation_slot;
        using asio::co_spawn;
        using asio::use_awaitable;
        using namespace asio::experimental::awaitable_operators;

        auto ws = ws_stream(get_executor(), ssl_ctx_);

        co_await interruptible_connect(ws);

        fmt::print("{}::{} : websocket connected!\n", classname, __func__);

        error_.clear();
        send_cv_.cancel();

        auto forward_signal = asio::cancellation_signal();

        auto my_slot = (co_await asio::this_coro::cancellation_state).slot();
        my_slot.assign(
            [&](asio::cancellation_type type)
            {
                fmt::print("{}::run_connection : stopped!\n", classname);
                forward_signal.emit(type);
            });
        BOOST_SCOPE_EXIT_ALL(&)
        {
            my_slot.clear();
        };

        auto ic_slot = interrupt_connection_.slot();
        ic_slot.assign(
            [&](asio::cancellation_type type)
            {
                fmt::print("{}::run_connection : interrupted!\n", classname);
                forward_signal.emit(type);
            });
        BOOST_SCOPE_EXIT_ALL(&)
        {
            ic_slot.clear();
        };

        co_await co_spawn(
            get_executor(),
            send_loop(ws) || receive_loop(ws),
            bind_cancellation_slot(forward_signal.slot(), use_awaitable));
    }
    catch (boost::system::system_error &se)
    {
        error_ = se.code();
        fmt::print("{}: exception: {}\n", __func__, se.what());
    }
    catch (std::exception &e)
    {
        error_ = asio::error::basic_errors::no_such_device;
        fmt::print("{}: exception: {}\n", __func__, e.what());
    }
    fmt::print("{}: complete\n", __func__);
}

asio::awaitable< void >
connector_impl::interruptible_connect(ws_stream &stream)
{
    using asio::bind_cancellation_slot;
    using asio::co_spawn;
    using asio::use_awaitable;

    auto sentinel =
        util::monitor::record(fmt::format("{}::{}()", classname, __func__));

    bool interrupted       = false;
    auto cancel_connect    = asio::cancellation_signal();
    auto forward_interrupt = [&](asio::cancellation_type type)
    {
        interrupted = true;
        cancel_connect.emit(type);
    };

    {
        auto ic_slot = interrupt_connection_.slot();
        ic_slot.assign(forward_interrupt);
        BOOST_SCOPE_EXIT_ALL(ic_slot)
        {
            ic_slot.clear();
        };

        auto my_slot = (co_await asio::this_coro::cancellation_state).slot();
        my_slot.assign(forward_interrupt);
        BOOST_SCOPE_EXIT_ALL(my_slot)
        {
            my_slot.clear();
        };

        co_await co_spawn(
            get_executor(),
            network::connect(stream, host_, port_, path_),
            bind_cancellation_slot(cancel_connect.slot(), use_awaitable));
    }
    if (interrupted)
        throw system_error(asio::error::operation_aborted, "interrupted");
}

asio::awaitable< void >
connector_impl::send_loop(ws_stream &ws)
{
    auto sentinel = util::monitor::record("power_trade_connector::send_loop");
    using asio::redirect_error;
    using asio::use_awaitable;

    try
    {
        for (;;)
        {
            while (!error_ && send_queue_.empty())
            {
                send_cv_.expires_at(asio::steady_timer::time_point::max());
                error_code ec;
                co_await send_cv_.async_wait(redirect_error(use_awaitable, ec));
            }
            if (error_)
                break;
            assert(!send_queue_.empty());
            fmt::print("{}: sending {}\n", __func__, send_queue_.front());
            co_await ws.async_write(asio::buffer(send_queue_.front()),
                                    use_awaitable);
            send_queue_.pop_front();
        }
    }
    catch (std::exception &e)
    {
        fmt::print("{}: exception: {}\n", __func__, e.what());
    }
    fmt::print("{}: complete\n", __func__);
}

asio::awaitable< void >
connector_impl::receive_loop(ws_stream &ws)
{
    auto sentinel =
        util::monitor::record("power_trade_connector::receive_loop");
    using asio::redirect_error;
    using asio::use_awaitable;

    try
    {
        error_code         ec;
        beast::flat_buffer buf;
        buf.max_size(10'000'000);
        for (; !ec;)
        {
            auto size =
                co_await ws.async_read(buf, redirect_error(use_awaitable, ec));
            if (ec)
            {
                fmt::print("{}: read error: {}\n", __func__, ec.message());
                break;
            }

            auto now = std::chrono::system_clock::now();

            auto data = json::string_view(
                static_cast< char const * >(buf.data().data()),
                buf.data().size());

            auto handled = handle_message_data(data, ec);
            if (ec)
                break;

            buf.consume(size);
            if (!handled)
            {
                fmt::print("{}::{} : unhandled {} : {}\n",
                           classname,
                           __func__,
                           now,
                           util::truncate(data));
            }
        }

        if (!error_)
            error_ = ec;
        send_cv_.cancel();
    }
    catch (std::exception &e)
    {
        fmt::print("{}: exception: {}\n", __func__, e.what());
    }
    fmt::print("{}: complete\n", __func__);
}

bool
connector_impl::handle_message_data(json::string_view data, error_code &ec)
{
    auto pjson = std::make_shared< json::value >(json::parse(data, ec));
    if (ec)
        return false;

    if (auto pobject = pjson->if_object())
    {
        if (pobject->size() == 1)
        {
            auto key     = pobject->begin()->key();
            auto payload = std::shared_ptr< json::object const >(
                pjson, pobject->begin()->value().if_object());

            if (!payload)
                goto unrecognised;

            auto map_iter = signal_map_.find(key);
            if (map_iter == signal_map_.end())
                return false;
            auto &sig = map_iter->second;
            if (sig.empty())
            {
                signal_map_.erase(map_iter);
                return false;
            }
            else
            {
                map_iter->second(payload);
                return true;
            }
        }
    }
unrecognised:
    fmt::print(
        "{}::{} unrecognised JSON format: {}", classname, __func__, data);
    return false;
}

boost::signals2::connection
connector_impl::watch_messages(json::string message_type, message_slot slot)
{
    auto &sig = signal_map_[message_type];
    return sig.connect(std::move(slot));
}

}   // namespace arby::power_trade
