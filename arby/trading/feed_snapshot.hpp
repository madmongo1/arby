//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/arby
//

#ifndef ARBY_ARBY_TRADING_FEED_SNAPSHOT_HPP
#define ARBY_ARBY_TRADING_FEED_SNAPSHOT_HPP

#include "trading/feed_condition.hpp"
#include <memory>

namespace arby
{
namespace trading
{

struct feed_snapshot
{
    feed_condition condition;

    virtual ~feed_snapshot() = default;
};

using feed_snapshot_ptr = std::shared_ptr<feed_snapshot const>;

}   // namespace trading
}   // namespace arby

#endif   // ARBY_ARBY_TRADING_FEED_SNAPSHOT_HPP
