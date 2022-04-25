//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/arby
//

#ifndef ARBY_ARBY_POWER_TRADE_ORDERBOOK_LISTENER_HPP
#define ARBY_ARBY_POWER_TRADE_ORDERBOOK_LISTENER_HPP

#include "power_trade/orderbook_listener_impl.hpp"
#include "trading/market_key.hpp"

namespace arby
{
namespace power_trade
{

    struct orderbook_listener
    {
        using impl_class = orderbook_listener_impl;
        using impl_type = std::shared_ptr<impl_class>;
        using executor_type = impl_class::executor_type;
        using snapshot_class = impl_class::snapshot_class;
        using snapshot_type = impl_class::snapshot_type;
        static constexpr char classname[] = "power_trade::orderbook_listener";

        orderbook_listener(asio::any_io_executor exec, std::shared_ptr<connector> connector, trading::market_key market);

        executor_type const& get_executor() const { return impl_->get_executor(); }

      private:
        impl_type impl_;
    };

}   // namespace power_trade
}   // namespace arby

#endif   // ARBY_ARBY_POWER_TRADE_ORDERBOOK_LISTENER_HPP
