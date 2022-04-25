//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/arby
//

#ifndef ARBY_ARBY_POWER_TRADE_ORDERBOOK_SNAPSHOT_SERVICE_HPP
#define ARBY_ARBY_POWER_TRADE_ORDERBOOK_SNAPSHOT_SERVICE_HPP

#include "power_trade/order_book.hpp"
#include "power_trade/tick_history.hpp"
#include "trading/feed_snapshot.hpp"

namespace arby
{
namespace power_trade
{

/// @brief A snapshot of the power trade order book
struct orderbook_snapshot : trading::feed_snapshot
{
    std::uint64_t generation = 0;
    std::uint64_t sequence   = 0;
    order_book book;

  protected:
    void
    print_impl(std::ostream &os) const override;
};

struct orderbook_snapshot_service
{
    tick_history                                         tick_history_       = {};
    std::uint64_t                                        current_generation_ = 0;
    order_book                                           order_book_         = {};
    std::vector< std::unique_ptr< orderbook_snapshot > > free_snaps_         = {};

    std::unique_ptr< orderbook_snapshot >
    process_tick(tick_record tick);

    static void
    replay_tick(order_book &book, tick_record const &tick);

    std::unique_ptr< orderbook_snapshot >
    allocate_snapshot();

    void
    deallocate_snapshot(std::unique_ptr< orderbook_snapshot > snap);

    order_book const &
    orderbook() const
    {
        return order_book_;
    }
};

}   // namespace power_trade
}   // namespace arby

#endif   // ARBY_ARBY_POWER_TRADE_ORDERBOOK_SNAPSHOT_SERVICE_HPP
