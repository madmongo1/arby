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

namespace arby
{
namespace power_trade
{

    struct orderbook_listener
    {
        orderbook_listener(asio::any_io_executor exec, std::shared_ptr<connector> connector, json::string symbol);
    };

}   // namespace power_trade
}   // namespace arby

#endif   // ARBY_ARBY_POWER_TRADE_ORDERBOOK_LISTENER_HPP
