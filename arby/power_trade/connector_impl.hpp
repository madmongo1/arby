//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/router
//

#ifndef ARBY_ARBY_POWER_TRADE_CONNECTOR_IMPL_HPP
#define ARBY_ARBY_POWER_TRADE_CONNECTOR_IMPL_HPP

#include "config/websocket.hpp"

#include <boost/asio/awaitable.hpp>

#include <deque>

namespace arby::power_trade
{
struct connector_impl : std::enable_shared_from_this< connector_impl >
{
    using executor_type               = asio::any_io_executor;
    using tcp_layer                   = tcp::socket;
    using tls_layer                   = asio::ssl::stream< tcp_layer >;
    using ws_stream                   = websocket::stream< tls_layer >;
    static constexpr char classname[] = "connector_impl";

    /// @brief Constructor
    /// @param exec The internal executor to use for IO
    /// @param sslctx ssl context
    connector_impl(executor_type exec, ssl::context &sslctx);

    executor_type const &
    get_executor() const
    {
        return exec_;
    }

    asio::awaitable< void >
    connect();

    void
    start();

    void
    stop();

    void
    send(std::string s);

    void
    interrupt();

  private:
    asio::awaitable< void >
    run(std::shared_ptr< connector_impl > self);

    asio::awaitable< void >
    run_connection();

    asio::awaitable< void >
    send_loop(ws_stream &ws);

    asio::awaitable< void >
    receive_loop(ws_stream &ws);

    asio::awaitable< tcp::resolver::results_type >
    resolve();

  private:
    // dependencies
    executor_type exec_;
    ssl::context &ssl_ctx_;

    // parameters
    std::string const host_ = "35.186.148.56", port_ = "4321", path_ = "/";

    // state
    sys::error_code           error_ = asio::error::not_connected;
    std::deque< std::string > send_queue_;
    asio::steady_timer        send_cv_ { get_executor() };
    asio::cancellation_signal interrupt_connection_;
    asio::cancellation_signal stop_;
    bool                      stopped_ = false;
};
}   // namespace arby::power_trade

#endif   // ARBY_ARBY_POWER_TRADE_CONNECTOR_IMPL_HPP
