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

void
merge(entity::entity_key &target, const fix_connector_args &args)
{
    target.set("reactive.SenderCompId", args.sender_comp_id);
    target.set("reactive.TargetCompId", args.target_comp_id);
    target.set("reactive.SocketConnectHost", args.socket_connect_host);
    target.set("reactive.SocketConnectPort", args.socket_connect_port);
}
}   // namespace reactive
}   // namespace arby