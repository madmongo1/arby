//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/arby
//

#ifndef ARBY_ARBY_POWER_TRADE_TICK_LOGGER_HPP
#define ARBY_ARBY_POWER_TRADE_TICK_LOGGER_HPP

#include "config/filesystem.hpp"
#include "config/json.hpp"
#include "config/signals.hpp"
#include "logging/data_log.hpp"
#include "power_trade/connector.hpp"

namespace arby
{
namespace power_trade
{

/// @brief Listens to tick events on the connection for an instrument and logs them to a file
/// @note must run on the same executor as the associated connector
struct tick_logger
{
    struct impl : std::enable_shared_from_this< impl >
    {
        using executor_type = connector::executor_type;

        impl(std::shared_ptr< connector > connector, std::string symbol, fs::path path);

        impl(impl const &) = delete;

        impl &
        operator=(impl const &) = delete;

        void
        start();

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
        on_message(std::string_view type, std::shared_ptr< json::object const > const &pmessage);

        static void
        _on_message(std::weak_ptr< impl > weak, std::string_view type, std::shared_ptr< json::object const > const &pmessage);

      private:
        std::shared_ptr< connector >           connector_;
        std::vector< sigs::scoped_connection > persistent_connections_;
        std::string                            symbol_;
        logging::data_log                      logger_;
    };

    using executor_type = impl::executor_type;

    tick_logger(std::shared_ptr< connector > connector, std::string symbol, fs::path path);
    tick_logger(tick_logger &&other);
    tick_logger &
    operator=(tick_logger &&other);
    ~tick_logger();

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

#endif   // ARBY_ARBY_POWER_TRADE_TICK_LOGGER_HPP
