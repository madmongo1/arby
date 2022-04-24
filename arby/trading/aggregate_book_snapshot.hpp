//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/arby
//

#ifndef ARBY_ARBY_TRADING_AGGREGATE_BOOK_SNAPSHOT_HPP
#define ARBY_ARBY_TRADING_AGGREGATE_BOOK_SNAPSHOT_HPP

#include "trading/aggregate_book.hpp"
#include "trading/feed_snapshot.hpp"

namespace arby
{
namespace trading
{

struct aggregate_book_snapshot : feed_snapshot
{
    aggregate_book book;
};

}   // namespace trading
}   // namespace arby

#endif   // ARBY_ARBY_TRADING_AGGREGATE_BOOK_SNAPSHOT_HPP
