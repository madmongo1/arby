//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/arby
//

#ifndef ARBY_ARBY_TRADING_ENTITY_SERVICE_HPP
#define ARBY_ARBY_TRADING_ENTITY_SERVICE_HPP

#include "trading/entity.hpp"

namespace arby::trading
{
struct entity_service
{
    using executor_type = asio:an
    struct impl
    {
    };

    entity_service(asio::io_context & ioc)
    : ioc_(&ioc)
    {


    }

    asio::io_context* ioc_;
};

}   // namespace arby::trading

#endif   // ARBY_ARBY_TRADING_ENTITY_SERVICE_HPP
