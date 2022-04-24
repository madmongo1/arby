//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/arby
//

#include "order_book.hpp"

#include <fmt/chrono.h>
#include <fmt/ostream.h>

#include <algorithm>
#include <chrono>
#include <iterator>

namespace arby::power_trade
{

struct table
{
    std::map< std::size_t, std::map< std::size_t, std::string > > data_;
    std::size_t                                                   rows = 0;

    void
    set(std::size_t row, std::size_t col, std::string s)
    {
        data_[row][col] = std::move(s);
        rows            = std::max(rows, row);
    }

    std::vector< std::size_t >
    calc_widths() const
    {
        auto result = std::vector< std::size_t >();

        for (auto &[r, cdata] : data_)
            for (auto &[c, str] : cdata)
            {
                if (c >= result.size())
                    result.resize(c + 1, 0);

                result[c] = std::max(result[c], str.size());
            }

        return result;
    };

    static std::string
    pad(std::string s, std::size_t field)
    {
        auto pre  = (field - s.size()) / 2;
        s         = std::string(pre, ' ') + s;
        auto post = field - s.size();
        s += std::string(post, ' ');
        return s;
    }

    friend std::ostream &
    operator<<(std::ostream &os, table const &tab)
    {
        const auto widths = tab.calc_widths();

        const char *nl = "";
        for (auto &[r, cdata] : tab.data_)
        {
            os << nl;
            auto sep = std::string_view("");
            for (auto col = std::size_t(0); col < widths.size(); ++col)
            {
                auto idata = cdata.find(col);
                if (idata == cdata.end())
                {
                    os << sep << pad("", widths[col]);
                }
                else
                {
                    os << sep << pad(idata->second, widths[col]);
                }
                sep = " | ";
            }
            nl = "\n";
        }
        return os;
    }
};

std::ostream &
operator<<(std::ostream &os, const order_book &book)
{
    fmt::print(
        os, "last_update: {}, bid depth: {}, offer depth: {}\n", book.last_update_, book.aggregate_bids_, book.aggregate_offers_);

    auto max_levels = std::size_t(10);

    auto tab = table();

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
                price_type                            price,
                qty_type                              quantity,
                side_type                             side,
                std::chrono::system_clock::time_point timestamp)
{
    if (timestamp < last_update_)
        throw std::runtime_error("updates out of order");

    if (side == buy)
    {
        auto ilevel = bids_.find(price);
        if (ilevel == bids_.end())
            ilevel = bids_.emplace(price, level_data()).first;
        auto &detail = ilevel->second;
        detail.aggregate_depth += quantity;
        auto iqty            = detail.orders.insert(detail.orders.end(), quantity);
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
        auto iqty              = detail.orders.insert(detail.orders.end(), quantity);
        offer_cache_[order_id] = std::make_tuple(ilevel, iqty);
        aggregate_offers_ += quantity;
    }

    last_update_ = timestamp;
}
void
order_book::remove(const json::string &order_id, side_type side, std::chrono::system_clock::time_point timestamp)
{
    if (timestamp < last_update_)
        throw std::runtime_error("updates out of order");

    if (side == buy)
    {
        auto icache = bid_cache_.find(order_id);
        if (icache == bid_cache_.end())
            return;
        auto &[ilevel, iqty] = icache->second;
        auto &detail         = ilevel->second;
        detail.aggregate_depth -= *iqty;
        aggregate_bids_ -= *iqty;
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
        detail.aggregate_depth -= *iqty;
        aggregate_offers_ -= *iqty;
        detail.orders.erase(iqty);
        if (detail.orders.empty())
            offers_.erase(ilevel);
        offer_cache_.erase(icache);
    }

    last_update_ = timestamp;
}
void
order_book::reset()
{
    last_update_ = std::chrono::system_clock::time_point ::min();
    offers_.clear();
    offer_cache_.clear();
    bids_.clear();
    bid_cache_.clear();
}
}   // namespace arby::power_trade
