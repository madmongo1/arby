//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/arby
//

#ifndef ARBY_ARBY_REACTIVE_DETAIL_FIX_CONNECTOR_IMPL_HPP
#define ARBY_ARBY_REACTIVE_DETAIL_FIX_CONNECTOR_IMPL_HPP

#include "asioex/helpers.hpp"
#include "connection_state.hpp"
#include "entity/entity_base.hpp"
#include "entity/entity_key.hpp"

#include <string>

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

    entity::entity_key
    to_key() const;
};

void
merge(entity::entity_key &target, fix_connector_args const &args);

struct fix_connector_impl : entity::entity_base
{
    fix_connector_impl(asio::any_io_executor exec, ssl::context &sslctx, fix_connector_args args);

    std::string_view
    classname() const override;

  private:
    void
    extend_summary(std::string &buffer) const override;

    static asio::awaitable< void >
    run(std::shared_ptr< fix_connector_impl > self)
    {
        co_await asioex::spin();
    }
    void
    handle_start() override;
    void
    handle_stop() override;

  public:
    ssl::context &sslctx_;

    fix_connector_args args_;

    connection_state connstate_ = connection_down;
};

}   // namespace reactive
}   // namespace arby

#endif   // ARBY_ARBY_REACTIVE_DETAIL_FIX_CONNECTOR_IMPL_HPP
