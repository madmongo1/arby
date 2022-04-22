//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/router
//

#include "connector.hpp"

namespace arby
{
namespace power_trade
{
    connector::connector(asio::any_io_executor exec, ssl::context &ioc)
        : impl_(std::make_shared< connector_impl >(exec, ioc))
    {
        impl_->start();
    }
}   // namespace power_trade
}   // namespace arby