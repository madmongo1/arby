//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/arby
//

#include "event_listener.hpp"

#include "asioex/helpers.hpp"
#include "util/truncate.hpp"

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <cassert>

namespace arby
{
namespace power_trade
{

std::shared_ptr< event_listener::impl >
event_listener::impl::create(std::shared_ptr< connector > connector, json::string const &primary)
{
    auto candidate = std::make_shared< impl >(std::move(connector));
    candidate->start(primary);
    return candidate;
}

event_listener::impl::impl(std::shared_ptr< connector > connector)
: connector_(std::move(connector))
, message_connection_()
{
}

void
event_listener::impl::start(json::string const &primary)
{
    connection_state current_state;
    status_connection_ =
        connector_->get_implementation()->watch_connection_state(current_state,
                                                                 detail::connector_impl::connection_state_slot(
                                                                     [weak = weak_from_this()](connection_state stat)
                                                                     {
                                                                         if (auto self = weak.lock())
                                                                             self->on_connection_state(stat);
                                                                     }));

    message_connection_ = connector_->get_implementation()->watch_messages(
        primary,
        detail::connector_impl::message_slot(
            [weak = weak_from_this()](std::shared_ptr< json::object const > pmessage)
            {
                if (auto self = weak.lock())
                    self->on_message(pmessage);
            }));

    on_connection_state(current_state);
}

void
event_listener::impl::stop()
{
    status_connection_.disconnect();
    message_connection_.disconnect();
    connector_.reset();
}

void
event_listener::impl::on_message(const std::shared_ptr< const json::object > &pmessage)
{
    fmt::print("event_listener::{} - {}\n", __func__, util::truncate(json::serialize(*pmessage), 1024));
}

void
event_listener::impl::on_connection_state(connection_state stat)
{
    fmt::print("event_listener::{} - {}\n", __func__, stat);
}

//========================================== handle ===============================

event_listener::event_listener(std::shared_ptr< connector > connector, json::string const &primary)
: impl_(impl::create(connector, primary))
{
    assert(asioex::on_correct_thread(get_executor()));
}

event_listener::event_listener(event_listener &&other)
: impl_(std::move(other.impl_))
{
    assert(asioex::on_correct_thread(get_executor()));
}

event_listener &
event_listener::operator=(event_listener &&other)
{
    assert(asioex::on_correct_thread(get_executor()));
    destroy();
    impl_ = std::move(other.impl_);
    return *this;
}

event_listener::~event_listener()
{
    assert(asioex::on_correct_thread(get_executor()));
    destroy();
}

    void
    event_listener::destroy()
    {
        assert(asioex::on_correct_thread(get_executor()));
        if (impl_)
        {
            impl_->stop();
            impl_.reset();
        }
    }
}   // namespace power_trade
}   // namespace arby