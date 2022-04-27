//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/arby
//

#ifndef ARBY_ARBY_POWER_TRADE_ORDER_BOOK_HPP
#define ARBY_ARBY_POWER_TRADE_ORDER_BOOK_HPP

#include "config/wise_enum.hpp"
#include "power_trade/tick_record.hpp"
#include "trading/types.hpp"

#include <chrono>
#include <list>
#include <map>
#include <string>

namespace arby::power_trade
{
struct order_qty
{
    std::string       orderid;
    trading::qty_type qty;

    friend bool
    operator==(order_qty const &l, order_qty const &r)
    {
        return l.orderid == r.orderid && l.qty == r.qty;
    }
};

struct level_data
{
    using qty_list = std::list< order_qty >;

    trading::qty_type aggregate_depth { 0 };
    qty_list          orders {};

    friend bool
    operator==(level_data const &l, level_data const &r)
    {
        return l.aggregate_depth == r.aggregate_depth && l.orders == r.orders;
    }
};

struct order_book
{
    std::string
    top_bid_str() const;
    std::string
    top_offer_str() const;

    void
    add(tick_record::add const &r);

    void
    remove(tick_record::remove const &r);

    void
    execute(tick_record::execute const &e);

    void
    reset();

    friend bool
    operator==(order_book const &l, order_book const &r);

    friend std::ostream &
    operator<<(std::ostream &os, order_book const &book);

    using offer_ladder      = std::map< trading::price_type, level_data, std::less<> >;
    using offer_order_cache = std::map< std::string, std::tuple< offer_ladder::iterator, level_data::qty_list ::iterator > >;

    using bid_ladder      = std::map< trading::price_type, level_data, std::greater<> >;
    using bid_order_cache = std::map< std::string, std::tuple< bid_ladder::iterator, level_data::qty_list ::iterator > >;

    std::chrono::system_clock::time_point last_update_;

    offer_ladder      offers_;
    offer_order_cache offer_cache_;
    trading::qty_type aggregate_offers_;

    bid_ladder        bids_;
    bid_order_cache   bid_cache_;
    trading::qty_type aggregate_bids_;

    order_book &
    operator=(order_book const &r)
    {
        offers_           = r.offers_;
        aggregate_offers_ = r.aggregate_offers_;
        bids_             = r.bids_;
        aggregate_bids_   = r.aggregate_bids_;
        last_update_      = r.last_update_;

        // now rebuild the indexes

        for (auto iladder = bids_.begin(); iladder != bids_.end(); ++iladder)
        {
            auto &[price, detail] = *iladder;
            for (auto iqty = detail.orders.begin(); iqty != detail.orders.end(); ++iqty)
            {
                auto &[orderid, qty] = *iqty;
                bid_cache_.emplace(orderid, std::make_tuple(iladder, iqty));
            }
        }

        for (auto iladder = offers_.begin(); iladder != offers_.end(); ++iladder)
        {
            auto &[price, detail] = *iladder;
            for (auto iqty = detail.orders.begin(); iqty != detail.orders.end(); ++iqty)
            {
                auto &[orderid, qty] = *iqty;
                offer_cache_.emplace(orderid, std::make_tuple(iladder, iqty));
            }
        }

        return *this;
    }
};

}   // namespace arby::power_trade

#endif   // ARBY_ARBY_POWER_TRADE_ORDER_BOOK_HPP
