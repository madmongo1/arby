//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/arby
//

#ifndef ARBY_ARBY_POWER_TRADE_TICK_RECORD_HPP
#define ARBY_ARBY_POWER_TRADE_TICK_RECORD_HPP

#include "config/json.hpp"
#include "config/wise_enum.hpp"
#include "trading/types.hpp"

#include <boost/variant2.hpp>

#include <memory>

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
    struct add
    {
        std::string             order_id;
        trading::price_type     price;
        trading::qty_type       qty;
        trading::timestamp_type timestamp;
        trading::side_type      side;
    };

    struct snapshot
    {
        std::vector< add > bids;
        std::vector< add > offers;
    };

    static auto
    make_snapshot(json::object const &p) -> snapshot;

    struct remove
    {
        std::string             order_id;
        trading::timestamp_type timestamp;
        trading::side_type      side;
    };

    struct execute
    {
        std::string             order_id;
        trading::price_type     price;
        trading::qty_type       qty;
        trading::timestamp_type timestamp;
        trading::side_type      side;
    };

    tick_record(tick_code code, std::shared_ptr< json::object const > payload);

    using impl_var = boost::variant2::variant< add, remove, execute, snapshot >;

    impl_var const &
    as_variant() const
    {
        return *impl_;
    }

  private:
    using implementation_type = std::shared_ptr< impl_var const >;

    static impl_var
    construct2(tick_code code, std::shared_ptr< json::object const > const &payload);

    static implementation_type
    construct(tick_code code, std::shared_ptr< json::object const > const &payload);

    std::shared_ptr< impl_var const > impl_;
    tick_code                         code_;
};

}   // namespace arby::power_trade

#endif   // ARBY_ARBY_POWER_TRADE_TICK_RECORD_HPP
