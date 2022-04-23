//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/router
//

#ifndef ARBY_ARBY_POWER_TRADE_ORDERBOOK_LISTENER_HPP
#define ARBY_ARBY_POWER_TRADE_ORDERBOOK_LISTENER_HPP

#include "power_trade/connector.hpp"

#include <boost/variant2.hpp>

namespace arby
{
namespace power_trade
{

    struct orderbook_listener
    : util::has_executor_base
    , std::enable_shared_from_this< orderbook_listener >
    {
        static constexpr char classname[] = "orderbook_listener";

        static std::shared_ptr< orderbook_listener >
        create(asio::any_io_executor        exec,
               std::shared_ptr< connector > connector,
               json::string                 symbol);

        orderbook_listener(asio::any_io_executor        exec,
                           std::shared_ptr< connector > connector,
                           json::string                 symbol);

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

        using message = boost::variant2::
            variant< orderbook_snapshot, orderbook_add, orderbook_remove >;

        asio::awaitable< void >
        run(std::shared_ptr< orderbook_listener > self);

        void
        on_connection_state(connection_state cstate);

        void
        on_command_response(std::shared_ptr< json::object const > payload);

        json::string
        build_subscribe_id() const;

        asio::awaitable< void >
        monitor_book_messages(std::shared_ptr< orderbook_listener > self);

        void
        on_snapshot(json::object const &payload);
        void
        on_add(json::object const &payload);
        void
        on_remove(json::object const &payload);

        static void
        _handle_command_response(std::weak_ptr< orderbook_listener >   weak,
                                 std::shared_ptr< json::object const > payload);

        static void
        _handle_snapshot(std::weak_ptr< orderbook_listener >   weak,
                         std::shared_ptr< json::object const > payload);

        void
        handle_snapshot(std::shared_ptr< json::object const > payload);

        static void
        _handle_add(std::weak_ptr< orderbook_listener >   weak,
                         std::shared_ptr< json::object const > payload);

        void
        handle_add(std::shared_ptr< json::object const > payload);

        static void
        _handle_remove(std::weak_ptr< orderbook_listener >   weak,
                         std::shared_ptr< json::object const > payload);

        void
        handle_remove(std::shared_ptr< json::object const > payload);

        json::string const symbol_;
        json::string const my_subscribe_id_ = build_subscribe_id();

        std::shared_ptr< connector > connector_;
        connection_state             connstate_;

        util::cross_executor_connection connection_state_conn_;
        util::cross_executor_connection cmd_response_conn_, snapshot_conn_,
            add_conn_, remove_conn_;

        asio::cancellation_signal stop_monitoring_books_;

        asio::experimental::channel< void(error_code, message) > msg_channel_ {
            get_executor()
        };
    };

}   // namespace power_trade
}   // namespace arby

#endif   // ARBY_ARBY_POWER_TRADE_ORDERBOOK_LISTENER_HPP
