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

    auto buffer = fmt::format("{}[{}.{}] : ", classname(), key_, entity_version_);
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
}   // namespace entity
}   // namespace arby