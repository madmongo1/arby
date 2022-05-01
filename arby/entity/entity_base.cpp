//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/arby
//

#include "entity_base.hpp"

#include "asioex/helpers.hpp"

#include <fmt/format.h>
#include <fmt/ostream.h>

namespace arby
{
namespace entity
{
std::string_view
entity_base::classname() const
{
    return "unnamed_entity";
}

std::string
entity_base::summary() const
{
    assert(asioex::on_correct_thread(get_executor()));

    auto buffer = fmt::format("{}[{}.{}] ({}) : ", classname(), key_, entity_version_, this->estate_);
    extend_summary(buffer);
    return buffer;
}

void
entity_base::extend_summary(std::string &) const
{
}

asio::any_io_executor const &
entity_base::get_executor() const
{
    return exec_;
}

entity_key const &
entity_base::key() const
{
    return key_;
}
void
entity_base::prepare()
{
    assert(estate_ == not_started);
    do_prepare();
    key_.lock();
    estate_ = prepared;
}
void
entity_base::start()
{
    using asio::bind_executor;
    using asio::dispatch;

    assert(estate_ == prepared);

    entity_version_ = entity_svc_.notify(key_, weak_from_this());

    dispatch(bind_executor(get_executor(), [self = shared_from_this()] { self->handle_start(); }));
    estate_ = started;
}
entity_base::entity_base(asio::any_io_executor exec, entity_service entity_svc)
    : exec_(exec)
    , entity_svc_(entity_svc)
{
}
void
entity_base::stop()
{
    using asio::bind_executor;
    using asio::dispatch;

    assert(estate_ == started);

    dispatch(bind_executor(get_executor(), [self = shared_from_this()] { self->handle_stop(); }));
    estate_ = stopped;
}
}   // namespace entity
}   // namespace arby