//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/arby
//

#ifndef ARBY_ARBY_POWER_TRADE_ORDERBOOK_LISTENER_IMPL_HPP
#define ARBY_ARBY_POWER_TRADE_ORDERBOOK_LISTENER_IMPL_HPP

#include "power_trade/connector.hpp"
#include "power_trade/native_symbol.hpp"
#include "power_trade/order_book.hpp"
#include "trading/market_key.hpp"

#include <boost/variant2.hpp>

namespace arby
{
namespace power_trade
{

/// @brief Implementation class to monitor a Power Trade order book.
///
/// This class maintains the power-trade-specific representation of an order
/// book and delivers generic snapshots to interested observers. It is an
/// implementation of a generic orderbook_feed. Upon reconnect, the listener
/// will rebuild the entire order book via a snapshot request. It will not
/// seek to persist data to local storage.
///
/// @note This class maintains its own executor, which may not be the same
/// executor as the Power Trade connector it holds. Therefore, any events
/// emitted from the connector must be marshalled onto our own executor via
/// POST. This also separates the processing of our data from the io loop.
struct orderbook_listener_impl
: util::has_executor_base
, std::enable_shared_from_this< orderbook_listener_impl >
{
    static constexpr char classname[] = "orderbook_listener_impl";

    static std::shared_ptr< orderbook_listener_impl >
    create(asio::any_io_executor exec, std::shared_ptr< connector > connector, trading::market_key symbol);

    orderbook_listener_impl(asio::any_io_executor exec, std::shared_ptr< connector > connector, trading::market_key symbol);

    void
    start();

  private:
    struct command_response
    {
        std::shared_ptr< json::object const > payload;
    };

    struct orderbook_snapshot
    {
        std::shared_ptr< json::object const > payload;
    };

    struct orderbook_add
    {
        std::shared_ptr< json::object const > payload;
    };

    struct orderbook_remove
    {
        std::shared_ptr< json::object const > payload;
    };

    using message = boost::variant2::variant< orderbook_snapshot, orderbook_add, orderbook_remove >;

    asio::awaitable< void >
    run(std::shared_ptr< orderbook_listener_impl > self);

    void
    on_connection_state(connection_state cstate);

    void
    on_command_response(std::shared_ptr< json::object const > payload);

    json::string
    build_subscribe_id() const;

    asio::awaitable< void >
    monitor_book_messages(std::shared_ptr< orderbook_listener_impl > self);

    void
    on_snapshot(json::object const &payload);
    void
    on_add(json::object const &payload);
    void
    on_remove(json::object const &payload);

    static void
    _handle_command_response(std::weak_ptr< orderbook_listener_impl > weak, std::shared_ptr< json::object const > payload);

    static void
    _handle_message(std::weak_ptr< orderbook_listener_impl > weak, message const message);
    void
    handle_message(message payload);

    static void
    _handle_snapshot(std::weak_ptr< orderbook_listener_impl > weak, std::shared_ptr< json::object const > payload);

    static void
    _handle_add(std::weak_ptr< orderbook_listener_impl > weak, std::shared_ptr< json::object const > payload);

    static void
    _handle_remove(std::weak_ptr< orderbook_listener_impl > weak, std::shared_ptr< json::object const > payload);

    std::array< std::tuple< json::string, json::string >, 2 >
    make_message_filter() const
    {
        auto result = std::array< std::tuple< json::string, json::string >, 2 > { { { "symbol", native_symbol(symbol_) },
                                                                                    { "market_id", "0" } } };
        return result;
    }

    trading::market_key const symbol_;
    json::string const        my_subscribe_id_ = build_subscribe_id();

    std::shared_ptr< connector > connector_;
    connection_state             connstate_;

    util::cross_executor_connection connection_state_conn_;
    util::cross_executor_connection cmd_response_conn_, snapshot_conn_, add_conn_, remove_conn_;

    asio::cancellation_signal stop_monitoring_books_;

    asio::experimental::channel< void(error_code, message) > msg_channel_ { get_executor() };

    order_book order_book_;
};

}   // namespace power_trade
}   // namespace arby

#endif   // ARBY_ARBY_POWER_TRADE_ORDERBOOK_LISTENER_IMPL_HPP
