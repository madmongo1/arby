//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/arby
//

#ifndef ARBY_FIX_CONNECTOR_HPP
#define ARBY_FIX_CONNECTOR_HPP

#include "config/wise_enum.hpp"
#include "connection_state.hpp"
#include "entity/entity_base.hpp"
#include "reactive/impl/fix_connector_impl.hpp"

namespace arby
{
namespace reactive
{

struct fix_connector : entity::entity_handle< fix_connector_impl >
{
    fix_connector(std::shared_ptr< fix_connector_impl > impl)
    : entity::entity_handle< fix_connector_impl >(impl)
    {
    }

    fix_connector(asio::any_io_executor exec, ssl::context &sslctx, fix_connector_args args)
    : entity::entity_handle< fix_connector_impl >(exec, sslctx, args)
    {
    }
};

}   // namespace reactive
}   // namespace arby

#endif   // ARBY_FIX_CONNECTOR_HPP
