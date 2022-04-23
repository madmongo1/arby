//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/router
//

#include "event_listener.hpp"

#include "util/truncate.hpp"

#include <fmt/format.h>
#include <fmt/ostream.h>

namespace arby
{
namespace power_trade
{

    std::shared_ptr< event_listener >
    event_listener::create(std::shared_ptr< connector > connector,
                           json::string const          &primary)
    {
        auto impl = std::make_shared< event_listener >(std::move(connector));
        impl->start(primary);
        return impl;
    }

    event_listener::event_listener(std::shared_ptr< connector > connector)
    : connector_(std::move(connector))
    , message_connection_()
    {
    }

    void
    event_listener::start(json::string const &primary)
    {
        message_connection_ = connector_->get_implementation().watch_messages(
            primary,
            connector_impl::message_slot(
                [weak = weak_from_this()](
                    std::shared_ptr< json::object const > pmessage)
                {
                    if (auto self = weak.lock())
                        asio::post(asio::bind_executor(
                            self->get_executor(),
                            [self, pmessage] { self->on_message(pmessage); }));
                }));
    }

    void
    event_listener::on_message(
        const std::shared_ptr< const json::object > &pmessage)
    {
        fmt::print("event_listener::{} - {}\n",
                   __func__,
                   util::truncate(json::serialize(*pmessage), 1024));
    }

}   // namespace power_trade
}   // namespace arby