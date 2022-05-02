//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/arby
//

#include "fix_connector_impl.hpp"

namespace arby
{
namespace reactive
{
std::string_view
fix_connector_impl::classname() const
{
    return "reactive::fix_connector";
}

void
fix_connector_impl::extend_summary(std::string &buffer) const
{
}

void
fix_connector_impl::handle_start()
{

}
void
fix_connector_impl::handle_stop()
{
}
fix_connector_impl::fix_connector_impl(asio::any_io_executor exec, ssl::context &sslctx, fix_connector_args args)
    : entity::entity_base(exec)
    , sslctx_(sslctx)
    , args_(std::move(args))
{
}

entity::entity_key
fix_connector_args::to_key() const
{
    auto key = entity::entity_key();
    key.set("reactive.SenderCompId", sender_comp_id);
    key.set("reactive.TargetCompId", target_comp_id);
    key.set("reactive.SocketConnectHost", socket_connect_host);
    key.set("reactive.SocketConnectPort", socket_connect_port);
    key.set("reactive.UseSSL", std::to_string(use_ssl));
    return key;
}

void
merge(entity::entity_key &target, const fix_connector_args &args)
{
    target.merge(entity::entity_key(args.to_key()));
}
}   // namespace reactive
}   // namespace arby