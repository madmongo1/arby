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

namespace arby
{
namespace reactive
{

struct fix_connector_args
{
    std::string sender_comp_id;
    std::string target_comp_id;
    std::string socket_connect_host;
    std::string socket_connect_port;
    bool        use_ssl;
};

struct fix_connector_impl : entity::entity_base
{
    fix_connector_impl(asio::any_io_executor exec, ssl::context &sslctx, fix_connector_args args, entity::entity_service esvc)
    : entity::entity_base(exec, esvc)
    , sslctx_(sslctx)
    , args_(std::move(args))
    {
    }

    std::string_view
    classname() const override;

  private:
    void
    extend_summary(std::string &buffer) const override;

  public:
    ssl::context &sslctx_;

    fix_connector_args args_;

    connection_state connstate_ = connection_down;
};

struct fix_connector : entity::entity_handle< fix_connector_impl >
{
    fix_connector(asio::any_io_executor exec, ssl::context &sslctx, fix_connector_args args, entity::entity_service esvc = {})
    : entity::entity_handle< fix_connector_impl >(std::make_shared< fix_connector_impl >(exec, sslctx, std::move(args), esvc))
    {
    }
};

}   // namespace reactive
}   // namespace arby

#endif   // ARBY_FIX_CONNECTOR_HPP
