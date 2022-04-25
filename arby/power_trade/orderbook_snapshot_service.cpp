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
void
orderbook_snapshot_service::replay_tick(order_book &book, const tick_record &tick)
{
    auto to_price     = [](json::value const v) { return trading::price_type(v.as_string().c_str()); };
    auto to_qty       = [](json::value const v) { return trading::qty_type(v.as_string().c_str()); };
    auto to_side      = [](json::value const v) { return *wise_enum::from_string< trading::side_type >(v.as_string()); };
    auto to_timestamp = [](json::value const v)
    {
        using namespace std::chrono;
        return system_clock::time_point(microseconds(::atol(v.as_string().c_str())));
    };

    auto const &p = *tick.payload;

    switch (tick.code)
    {
    case tick_code::snapshot:
        book.reset();
        for (auto &e : p.at("buy").as_array())
        {
            auto &oe = e.as_object();
            book.add(oe.at("orderid").as_string(),
                     to_price(oe.at("price")),
                     to_qty(oe.at("quantity")),
                     trading::side_type::buy,
                     to_timestamp(oe.at("utc_timestamp")));
        }
        for (auto &e : p.at("sell").as_array())
        {
            auto &oe = e.as_object();
            book.add(oe.at("orderid").as_string(),
                     to_price(oe.at("price")),
                     to_qty(oe.at("quantity")),
                     trading::side_type::sell,
                     to_timestamp(oe.at("utc_timestamp")));
        }
        break;
    case tick_code::add:
        book.add(p.at("order_id").as_string(),
                 to_price(p.at("price")),
                 to_qty(p.at("quantity")),
                 to_side(p.at("side")),
                 to_timestamp(p.at("utc_timestamp")));
        break;
    case tick_code::remove:
        book.remove(p.at("order_id").as_string(), to_side(p.at("side")), to_timestamp(p.at("utc_timestamp")));
        break;

    case tick_code::execute:
        book.execute(
            p.at("order_id").as_string(), to_side(p.at("side")), to_qty(p.at("quantity")), to_timestamp(p.at("utc_timestamp")));
        break;
    }
}

std::unique_ptr< orderbook_snapshot >
orderbook_snapshot_service::process_tick(tick_record tick)
{
    if (tick.code == tick_code::snapshot)
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