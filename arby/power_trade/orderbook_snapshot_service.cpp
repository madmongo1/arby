//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/arby
//

#include "orderbook_snapshot_service.hpp"

#include <fmt/format.h>
#include <fmt/ostream.h>

namespace arby
{
namespace power_trade
{
namespace
{
template < class... Ts >
struct overloaded : Ts...
{
    using Ts::operator()...;
};
}   // namespace

void
orderbook_snapshot_service::replay_tick(order_book &book, const tick_record &tick)
{
    auto visitor = overloaded {
        [&book](tick_record::snapshot const &snap)
        {
            book.reset();
            for (auto &bid : snap.bids)
                book.add(bid);
            for (auto &offer : snap.offers)
                book.add(offer);
        },
        [&book](tick_record::add const &a) { book.add(a); },
        [&book](tick_record::remove const &r) { book.remove(r); },
        [&book](tick_record::execute const &e) { book.execute(e); },
    };

    boost::variant2::visit(visitor, tick.as_variant());
}

std::unique_ptr< orderbook_snapshot >
orderbook_snapshot_service::process_tick(tick_record tick)
{
    auto visitor = overloaded { [&](tick_record::snapshot const &snap)
                                {
                                    tick_history_.reset();
                                    current_generation_ += 1;
                                } };

    if (holds_alternative< tick_record::snapshot >(tick.as_variant()))
    {
        tick_history_.reset();
        current_generation_ += 1;
    }

    tick_history_.add_tick(tick);
    replay_tick(order_book_, tick);

    auto new_snap = allocate_snapshot();
    if (new_snap->generation == current_generation_)
    {
        auto old_seq = new_snap->sequence;
        auto itick   = tick_history_.history.find(new_snap->sequence);
        if (itick == tick_history_.history.end())
        {
            new_snap->book = order_book_;
        }
        else
        {
            while (++itick != tick_history_.history.end())
                replay_tick(new_snap->book, itick->second);
            tick_history_.remove_watermark(old_seq);
        }
    }
    else
    {
        new_snap->book = order_book_;
    }

    new_snap->generation    = current_generation_;
    new_snap->sequence      = tick_history_.highest_sequence();
    new_snap->timestamp     = std::chrono::system_clock::now();
    new_snap->upstream_time = order_book_.last_update_;

    tick_history_.add_watermark(new_snap->sequence);

    return new_snap;
}

std::unique_ptr< orderbook_snapshot >
orderbook_snapshot_service::allocate_snapshot()
{
    if (free_snaps_.empty())
        return std::make_unique< orderbook_snapshot >();
    else
    {
        auto result = std::move(free_snaps_.back());
        free_snaps_.pop_back();
        return result;
    }
}

void
orderbook_snapshot_service::deallocate_snapshot(std::unique_ptr< orderbook_snapshot > snap)
{
    snap->parents_.clear();
    free_snaps_.push_back(std::move(snap));
}

void
orderbook_snapshot::print_impl(std::ostream &os) const
{
    feed_snapshot::print_impl(os);
    fmt::print(os, "[generation {}][sequence {}][TOB {}/{}]", generation, sequence, book.top_bid_str(), book.top_offer_str());
}
}   // namespace power_trade
}   // namespace arby