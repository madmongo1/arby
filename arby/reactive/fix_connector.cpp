//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/arby
//

#include "fix_connector.hpp"

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
}   // namespace reactive
}   // namespace arby