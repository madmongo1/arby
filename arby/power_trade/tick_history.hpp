//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/arby
//

#ifndef ARBY_ARBY_POWER_TRADE_TICK_HISTORY_HPP
#define ARBY_ARBY_POWER_TRADE_TICK_HISTORY_HPP

#include "config/json.hpp"
#include "config/wise_enum.hpp"

#include <memory>
#include <set>

namespace arby::power_trade
{
WISE_ENUM_CLASS(tick_code, snapshot, add, remove, execute)

template < class Stream >
decltype(auto)
operator<<(Stream &s, tick_code code)
{
    return s << wise_enum::to_string(code);
}

struct tick_record
{
    std::shared_ptr< json::object const > payload;
    tick_code                             code;
};

struct tick_history
{
    std::uint64_t                          next_sequence = 0;
    std::map< std::uint64_t, tick_record > history;
    std::multiset< std::uint64_t >         watermarks;

    void reset()
    {
        next_sequence = 0;
        history.clear();
        watermarks.clear();
    }

    void
    add_tick(tick_record tick)
    {
        history.emplace(next_sequence++, std::move(tick));
    }

    std::uint64_t
    highest_sequence() const
    {
        // note: will wrap to uint64_max if empty
        return next_sequence - 1;
    }

    void
    add_watermark(std::uint64_t wm)
    {
        watermarks.insert(wm);
    }

    void
    remove_watermark(std::uint64_t wm)
    {
        watermarks.erase(wm);

        if (watermarks.empty())
        {
            history.clear();
            return;
        }

        auto first = history.begin();
        auto last  = history.lower_bound(*watermarks.begin());
        history.erase(first, last);
    }
};
}   // namespace arby::power_trade

#endif   // ARBY_ARBY_POWER_TRADE_TICK_HISTORY_HPP
