//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/router
//

#include "connector_impl.hpp"

#include "util/monitor.hpp"
#include "util/truncate.hpp"

#include <fmt/format.h>
#include <fmt/chrono.h>
#include <fmt/ostream.h>

namespace arby::power_trade
{
connector_impl::connector_impl(executor_type exec,
                               ssl::context                   &sslctx)
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

asio::awaitable< void >
connector_impl::run(std::shared_ptr< connector_impl > self)
{
    auto sentinel = util::monitor::record("power_trade_connector::run");
    try
    {
        using asio::use_awaitable;
        using namespace asio::experimental::awaitable_operators;

        auto ws = ws_stream(self->get_executor(), self->ssl_ctx_);

        auto &ssl_stream = ws.next_layer();
        auto &sock       = ssl_stream.next_layer();

        co_await async_connect(sock, co_await resolve(), use_awaitable);

        if (!SSL_set_tlsext_host_name(ssl_stream.native_handle(),
                                      host_.c_str()))
            throw sys::system_error(ERR_get_error(),
                                    asio::error::get_ssl_category());

        co_await ssl_stream.async_handshake(ssl::stream_base::client,
                                            use_awaitable);

        co_await ws.async_handshake(host_, path_, use_awaitable);

        error_.clear();
        send_cv_.cancel();
        co_await (send_loop(ws) || receive_loop(ws));
    }
    catch (std::exception &e)
    {
        fmt::print("{}: exception: {}\n", __func__, e.what());
    }
    fmt::print("{}: complete\n", __func__);
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

            auto data =
                std::string_view(static_cast< char const * >(buf.data().data()),
                                 buf.data().size());
            auto now = std::chrono::system_clock::now();
            fmt::print("{} : {} : {}\n", __func__, now, util::truncate(data));
            buf.consume(size);
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
asio::awaitable< tcp::resolver::results_type >
connector_impl::resolve()
{
    auto sentinel = util::monitor::record("power_trade_connector::resolve");
    using asio::use_awaitable;

    auto resolver = tcp::resolver(co_await asio::this_coro::executor);
    co_return co_await resolver.async_resolve(host_, port_, use_awaitable);
}

}   // namespace arby::power_trade
