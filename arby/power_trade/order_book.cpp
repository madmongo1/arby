//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/arby
//

#include "order_book.hpp"

#include "util/table.hpp"

#include <fmt/chrono.h>
#include <fmt/ostream.h>

#include <algorithm>
#include <chrono>
#include <iterator>

namespace arby::power_trade
{

std::ostream &
operator<<(std::ostream &os, const order_book &book)
{
    fmt::print(
        os, "last_update: {}, bid depth: {}, offer depth: {}\n", book.last_update_, book.aggregate_bids_, book.aggregate_offers_);

    auto max_levels = std::size_t(10);

    auto tab = util::table();

    std::size_t row = 0;

    {
        auto itenth = std::next(book.offers_.begin(), std::min(book.offers_.size(), max_levels));

        auto first = std::make_reverse_iterator(itenth);
        auto last  = book.offers_.rend();

        while (first != last)
        {
            auto &[price, detail] = *first++;
            tab.set(row, 1, to_string(price));
            tab.set(row, 2, to_string(detail.aggregate_depth));
            ++row;
        }
    }

    {
        auto first = book.bids_.begin();
        auto last  = std::next(first, std::min(book.bids_.size(), max_levels));
        while (first != last)
        {
            auto &[price, detail] = *first++;
            tab.set(row, 1, to_string(price));
            tab.set(row, 0, to_string(detail.aggregate_depth));
            ++row;
        }
    }

    os << tab << "\n";

    return os;
}
void
order_book::add(const json::string                   &order_id,
                trading::price_type                   price,
                trading::qty_type                     quantity,
                trading::side_type                    side,
                std::chrono::system_clock::time_point timestamp)
{
    last_update_ = std::max(last_update_, timestamp);

    if (side == trading::buy)
    {
        auto ilevel = bids_.find(price);
        if (ilevel == bids_.end())
            ilevel = bids_.emplace(price, level_data()).first;
        auto &detail = ilevel->second;
        detail.aggregate_depth += quantity;
        auto iqty            = detail.orders.insert(detail.orders.end(), order_qty { .orderid = order_id, .qty = quantity });
        bid_cache_[order_id] = std::make_tuple(ilevel, iqty);
        aggregate_bids_ += quantity;
    }
    else
    {
        auto ilevel = offers_.find(price);
        if (ilevel == offers_.end())
            ilevel = offers_.emplace(price, level_data()).first;
        auto &detail = ilevel->second;
        detail.aggregate_depth += quantity;
        auto iqty              = detail.orders.insert(detail.orders.end(), order_qty { .orderid = order_id, .qty = quantity });
        offer_cache_[order_id] = std::make_tuple(ilevel, iqty);
        aggregate_offers_ += quantity;
    }

    last_update_ = timestamp;
}
void
order_book::remove(const json::string &order_id, trading::side_type side, std::chrono::system_clock::time_point timestamp)
{
    if (timestamp < last_update_)
        throw std::runtime_error("updates out of order");

    if (side == trading::buy)
    {
        auto icache = bid_cache_.find(order_id);
        if (icache == bid_cache_.end())
            return;
        auto &[ilevel, iqty] = icache->second;
        auto &detail         = ilevel->second;
        detail.aggregate_depth -= iqty->qty;
        aggregate_bids_ -= iqty->qty;
        detail.orders.erase(iqty);
        if (detail.orders.empty())
            bids_.erase(ilevel);
        bid_cache_.erase(icache);
    }
    else
    {
        auto icache = offer_cache_.find(order_id);
        if (icache == offer_cache_.end())
            return;
        auto &[ilevel, iqty] = icache->second;
        auto &detail         = ilevel->second;
        detail.aggregate_depth -= iqty->qty;
        aggregate_offers_ -= iqty->qty;
        detail.orders.erase(iqty);
        if (detail.orders.empty())
            offers_.erase(ilevel);
        offer_cache_.erase(icache);
    }

    last_update_ = timestamp;
}

bool
operator==(order_book const &l, order_book const &r)
{
    return l.last_update_ == r.last_update_ && l.aggregate_bids_ == r.aggregate_bids_ &&
           l.aggregate_offers_ == r.aggregate_offers_ && l.offers_ == r.offers_ && l.bids_ == r.bids_;
}

void
order_book::execute(json::string const                   &order_id,
                    trading::side_type                    side,
                    trading::qty_type                     qty,
                    std::chrono::system_clock::time_point timestamp)
{
    if (timestamp < last_update_)
        throw std::runtime_error("updates out of order");

    if (side == trading::buy)
    {
        auto icache = bid_cache_.find(order_id);
        if (icache == bid_cache_.end())
            return;
        auto &[ilevel, iqty] = icache->second;
        auto &detail         = ilevel->second;
        if (iqty->qty -= qty == 0)
        {
            detail.orders.erase(iqty);
            bid_cache_.erase(order_id);
            if (detail.orders.empty())
                bids_.erase(ilevel->first);
        }
        aggregate_bids_ -= qty;
    }
    else
    {
        auto icache = offer_cache_.find(order_id);
        if (icache == offer_cache_.end())
            return;
        auto &[ilevel, iqty] = icache->second;
        auto &detail         = ilevel->second;
        if (iqty->qty -= qty == 0)
        {
            detail.orders.erase(iqty);
            offer_cache_.erase(order_id);
            if (detail.orders.empty())
                offers_.erase(ilevel->first);
        }
        aggregate_bids_ -= qty;
    }

    last_update_ = timestamp;
}

void
order_book::reset()
{
    last_update_ = std::chrono::system_clock::time_point ::min();
    bids_.clear();
    bid_cache_.clear();
    aggregate_bids_ = 0;
    offers_.clear();
    offer_cache_.clear();
    aggregate_offers_ = 0;
}

std::string
order_book::top_bid_str() const
{
    if (bids_.empty())
        return "";
    return fmt::format("{}@{}", bids_.begin()->second.aggregate_depth, bids_.begin()->first);
}

std::string
order_book::top_offer_str() const
{
    if (offers_.empty())
        return "";
    return fmt::format("{}@{}", offers_.begin()->second.aggregate_depth, offers_.begin()->first);
}

}   // namespace arby::power_trade
