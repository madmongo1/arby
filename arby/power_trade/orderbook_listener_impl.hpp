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

#include "config/signals.hpp"
#include "power_trade/connector.hpp"
#include "power_trade/native_symbol.hpp"
#include "power_trade/order_book.hpp"
#include "power_trade/orderbook_snapshot_service.hpp"
#include "power_trade/tick_history.hpp"
#include "trading/aggregate_book_feed.hpp"
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

    using executor_type  = asio::any_io_executor;
    using snapshot_class = orderbook_snapshot;
    using snapshot_type  = std::shared_ptr< snapshot_class const >;

    using signal_type = typename sigs::signal_type< void(snapshot_type), sigs::keywords::mutex_type< sigs::dummy_mutex > >::type;
    using slot_type   = signal_type::slot_type;

    static std::shared_ptr< orderbook_listener_impl >
    create(asio::any_io_executor exec, std::shared_ptr< connector > connector, trading::market_key symbol);

    orderbook_listener_impl(asio::any_io_executor exec, std::shared_ptr< connector > connector, trading::market_key symbol);

    void
    start();

    void
    stop();

    std::tuple< sigs::connection, snapshot_type >
    subscribe(slot_type slot);

  private:
    asio::awaitable< void >
    run(std::shared_ptr< orderbook_listener_impl > self);

    void
    on_connection_state(connection_state cstate);

    void
    on_command_response(std::shared_ptr< connector::inbound_message const > payload);

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
    void
    on_execute(json::object const &payload);

    static void
    _handle_command_response(std::weak_ptr< orderbook_listener_impl >            weak,
                             std::shared_ptr< connector::inbound_message const > payload);

    static void
    _handle_tick(std::weak_ptr< orderbook_listener_impl >            weak,
                 std::shared_ptr< connector::inbound_message const > payload,
                 tick_code                                           code);

    void
    on_tick(tick_record tick);

    void
    update();

    std::array< std::tuple< json::string, json::string >, 2 >
    make_message_filter() const
    {
        auto result = std::array< std::tuple< json::string, json::string >, 2 > { { { "symbol", native_symbol(symbol_) },
                                                                                    { "market_id", "0" } } };
        return result;
    }

    std::string
    build_source_id() const;

    trading::market_key const symbol_;
    json::string const        my_subscribe_id_ = build_subscribe_id();
    std::string const         source_id_       = build_source_id();

    std::shared_ptr< connector > connector_;
    connection_state             connstate_;

    util::cross_executor_connection connection_state_conn_;
    util::cross_executor_connection cmd_response_conn_;
    util::cross_executor_connection tick_con_[4];

    asio::cancellation_signal stop_monitoring_books_;

    orderbook_snapshot_service        snapshot_service_;
    std::shared_ptr< snapshot_class > snapshot_;
    signal_type                       signal_;

    // data for building the snapshot
    trading::feed_condition connection_condition_;
    trading::feed_condition book_condition_;
};

}   // namespace power_trade
}   // namespace arby

#endif   // ARBY_ARBY_POWER_TRADE_ORDERBOOK_LISTENER_IMPL_HPP
