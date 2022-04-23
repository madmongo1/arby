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

#include "config/json.hpp"
#include "config/websocket.hpp"

#include <boost/asio/awaitable.hpp>
#include <boost/signals2.hpp>
#include <boost/unordered_map.hpp>

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

    // Note that the signal type is not thread-safe. You must only interact with
    // the signals while on the same executor and thread as the connector
    using message_signal = boost::signals2::signal_type<
        void(std::shared_ptr< json::object const >),
        boost::signals2::keywords::mutex_type<
            boost::signals2::dummy_mutex > >::type;

    using message_slot          = message_signal::slot_type;
    using message_extended_slot = message_signal::extended_slot_type;

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

    boost::signals2::connection
    watch_messages(json::string message_type, message_slot slot);

  private:
    asio::awaitable< void >
    run(std::shared_ptr< connector_impl > self);

    asio::awaitable< void >
    run_connection();

    asio::awaitable< void >
    send_loop(ws_stream &ws);

    asio::awaitable< void >
    receive_loop(ws_stream &ws);

    asio::awaitable< void >
    interruptible_connect(ws_stream &stream);

    /// @brief Attempt to handle an incoming message
    /// @param data a string view representing the message payload
    /// @param ec output variable that records any errors. If this is true, the
    /// function will return false
    /// @return boolean value indicating that the message was dispatched to at
    /// least one listener
    bool
    handle_message_data(json::string_view data, error_code &ec);

  private:
    // dependencies
    executor_type exec_;
    ssl::context &ssl_ctx_;

    // parameters
    std::string const host_ = "35.186.148.56", port_ = "4321", path_ = "/";

    struct sv_comp_equ
    : boost::hash< boost::string_view >
    , std::equal_to<>
    {
        using is_transparent = void;
        using boost::hash< boost::string_view >::operator();
        using std::equal_to<>::                  operator();
    };

    using signal_map = boost::
        unordered_map< json::string, message_signal, sv_comp_equ, sv_comp_equ >;
    signal_map signal_map_;

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
