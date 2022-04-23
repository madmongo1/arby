//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/router
//

#ifndef ARBY_ARBY_POWER_TRADE_CONNECTOR_HPP
#define ARBY_ARBY_POWER_TRADE_CONNECTOR_HPP

#include "power_trade/connector_impl.hpp"

namespace arby
{
namespace power_trade
{
    struct connector
    {
        static constexpr char classname[] = "power_trade::connector";

        connector(asio::any_io_executor exec, ssl::context &ioc);

        ~connector()
        {
            if (impl_)
                impl_->stop();
        }

        connector(connector &&other)
        : impl_(std::move(other.impl_))
        {
        }

        connector &
        operator=(connector &&other)
        {
            if (impl_)
                impl_->stop();
            impl_ = std::move(other.impl_);
            return *this;
        }

        void
        send(std::string s)
        {
            asio::dispatch(impl_->get_executor(),
                           [s = std::move(s), impl = impl_]
                           { impl->send(std::move(s)); });
        }

        void
        interrupt()
        {
            asio::dispatch(impl_->get_executor(),
                           [impl = impl_] { impl->interrupt(); });
        }

        connector_impl &
        get_implementation()
        {
            return *impl_;
        }

        connector_impl const &
        get_implementation() const
        {
            return *impl_;
        }

      private:
        std::shared_ptr< connector_impl > impl_;
    };

}   // namespace power_trade
}   // namespace arby

#endif   // ARBY_ARBY_POWER_TRADE_CONNECTOR_HPP
