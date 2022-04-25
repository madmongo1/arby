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
    /// @brief The current condition of the entity producing the snapshot
    feed_condition condition;

    /// @brief The identity of the entity producing the snapshot
    std::string source;

    /// @brief The time the snapshot was constructed
    trading::timestamp_type timestamp = std::chrono::system_clock::now();

    /// @brief The time of construction of the upstream data
    trading::timestamp_type upstream_time = std::chrono::system_clock::now();

    /// @brief a collection of snapshots that were used to construct this snapshot
    std::vector< std::shared_ptr< feed_snapshot const > > parents_ = {};

    void
    print(std::ostream &os) const;

    virtual ~feed_snapshot() = default;

  protected:
    virtual void
    print_impl(std::ostream &os) const;
};
template < class T, std::enable_if_t< std::is_base_of_v< feed_snapshot, std::decay_t< T > > > * = nullptr >
std::ostream &
operator<<(std::ostream &os, T const &snap)
{
    snap.print(os);
    return os;
}

template < class T, std::enable_if_t< std::is_base_of_v< feed_snapshot, std::decay_t< T > > > * = nullptr >
std::ostream &
operator<<(std::ostream &os, std::shared_ptr< T const > const &snap)
{
    if (snap)
        snap->print(os);
    else
        os << "<null>";
    return os;
}

template < class T, std::enable_if_t< std::is_base_of_v< feed_snapshot, std::decay_t< T > > > * = nullptr >
std::ostream &
operator<<(std::ostream &os, T const *snap)
{
    if (snap)
        snap->print(os);
    else
        os << "<null>";
    return os;
}

using feed_snapshot_ptr = std::shared_ptr< feed_snapshot const >;

}   // namespace trading
}   // namespace arby

#endif   // ARBY_ARBY_TRADING_FEED_SNAPSHOT_HPP
