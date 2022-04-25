//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/arby
//

#include "feed_snapshot.hpp"

#include <fmt/chrono.h>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <fmt/ranges.h>

namespace arby
{
namespace trading
{

void
feed_snapshot::print(std::ostream &os) const
{
    fmt::print(os,
               "[condition {}][source {}][timestamp {}][upstream_time {}]",
               this->condition,
               source,
               this->timestamp,
               this->upstream_time);
    this->print_impl(os);
    fmt::print(os, "[parents {}]", this->parents_);
}

void
feed_snapshot::print_impl(std::ostream &) const
{
}

}   // namespace trading
}   // namespace arby