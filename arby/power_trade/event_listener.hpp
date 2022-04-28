//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/arby
//

#ifndef ARBY_ARBY_POWER_TRADE_EVENT_LISTENER_HPP
#define ARBY_ARBY_POWER_TRADE_EVENT_LISTENER_HPP

#include "config/json.hpp"
#include "power_trade/connector.hpp"

namespace arby
{
namespace power_trade
{

/// @brief an event listener is part of the connection object. It shares the same executor.
struct event_listener
{
    struct impl : std::enable_shared_from_this< impl >
    {
        using executor_type = connector::executor_type;

        static std::shared_ptr< impl >
        create(std::shared_ptr< connector > connector, json::string const &primary);

        impl(std::shared_ptr< connector > connector);

        impl(impl const &) = delete;

        impl &
        operator=(impl const &) = delete;

        void
        start(json::string const &primary);

        void
        stop();

        executor_type const &
        get_executor() const
        {
            return connector_->get_executor();
        }

      private:
        void
        on_connection_state(connection_state stat);

        void
        on_message(std::shared_ptr< connector::inbound_message const > const &pmessage);

      private:
        std::shared_ptr< connector >       connector_;
        boost::signals2::scoped_connection status_connection_;
        boost::signals2::scoped_connection message_connection_;
    };

    using executor_type = impl::executor_type;

    event_listener(std::shared_ptr< connector > connector, json::string const &primary);
    event_listener(event_listener &&other);
    event_listener &
    operator=(event_listener &&other);
    ~event_listener();

    executor_type const &
    get_executor() const
    {
        return impl_->get_executor();
    }

  private:
    void
    destroy();

    std::shared_ptr< impl > impl_;
};

}   // namespace power_trade
}   // namespace arby

#endif   // ARBY_ARBY_POWER_TRADE_EVENT_LISTENER_HPP
