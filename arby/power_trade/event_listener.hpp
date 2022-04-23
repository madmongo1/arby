//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/router
//

#ifndef ARBY_ARBY_POWER_TRADE_EVENT_LISTENER_HPP
#define ARBY_ARBY_POWER_TRADE_EVENT_LISTENER_HPP

#include "config/json.hpp"
#include "power_trade/connector.hpp"

namespace arby
{
namespace power_trade
{

    struct event_listener : std::enable_shared_from_this< event_listener >
    {
        using executor_type = connector_impl::executor_type;

        static std::shared_ptr< event_listener >
        create(std::shared_ptr< connector > connector,
               json::string const          &primary);

        event_listener(std::shared_ptr< connector > connector);

      private:
        event_listener(event_listener const &) = delete;

        event_listener &
        operator=(event_listener const &) = delete;

        void
        start(json::string const &primary);

        executor_type const &
        get_executor() const
        {
            return connector_->get_implementation().get_executor();
        }

      private:
        void
        on_message(std::shared_ptr< json::object const > const &pmessage);

      private:
        std::shared_ptr< connector >       connector_;
        boost::signals2::scoped_connection message_connection_;
    };

}   // namespace power_trade
}   // namespace arby

#endif   // ARBY_ARBY_POWER_TRADE_EVENT_LISTENER_HPP
