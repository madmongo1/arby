//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/arby
//

#ifndef ARBY_ARBY_TRADING_AGGREGATE_BOOK_FEED_HPP
#define ARBY_ARBY_TRADING_AGGREGATE_BOOK_FEED_HPP

#include "config/signals.hpp"
#include "trading/aggregate_book_snapshot.hpp"
#include "util/cross_executor_connection.hpp"

#include <vector>

namespace arby
{
namespace trading
{

class aggregate_book_feed_iface
{
    using snapshot_signal = typename sigs::signal_type< void(std::shared_ptr< aggregate_book_snapshot const >),
                                                        sigs::keywords::mutex_type< sigs::dummy_mutex > >::type;

  public:
    using snapshot_slot = snapshot_signal::slot_type;

    aggregate_book_feed_iface() {}

    struct subscribe_result
    {
        std::shared_ptr< aggregate_book_snapshot const > snapshot;
        util::cross_executor_connection                  connection;
    };

    virtual asio::awaitable< subscribe_result >
    subscribe(snapshot_slot slot) = 0;

  protected:
    void
    update_snapshot(std::shared_ptr< aggregate_book_snapshot const > candidate)
    {
        assert(candidate);
        last_snapshot_ = candidate;
        snap_sig_(candidate);
    }

  protected:
    std::shared_ptr< aggregate_book_snapshot const > last_snapshot_;
    snapshot_signal                                  snap_sig_;
};

template < class Derived >
struct implement_aggregate_book_feed : aggregate_book_feed_iface
{
    implement_aggregate_book_feed()
    : aggregate_book_feed_iface()
    {
    }

    virtual asio::awaitable< subscribe_result >
    subscribe(snapshot_slot slot) override
    {
        using asio::co_spawn;
        using asio::use_awaitable;

        auto self = static_cast< Derived * >(this)->shared_from_this();

        auto   this_exec = co_await asio::this_coro::executor;
        auto &&my_exec   = self->get_executor();

        if (my_exec == this_exec)
        {
            co_return subscribe_result { .snapshot = last_snapshot_, .connection = { self, snap_sig_.connect(std::move(slot)) } };
        }
        else
        {
            co_return co_await co_spawn(
                my_exec,
                [&]() -> asio::awaitable< subscribe_result >
                {
                    co_return subscribe_result { .snapshot   = self->last_snapshot_,
                                                 .connection = { self, self->snap_sig_.connect(std::move(slot)) } };
                },
                use_awaitable);
        }
    }

  protected:
    std::shared_ptr< aggregate_book_snapshot >
    new_aggregate_book_snapshot()
    {
        auto derived = static_cast< Derived * >(this);
        auto deleter = [weak = derived->weak_from_this(), exec = derived->get_executor()](aggregate_book_snapshot *snap)
        {
            if (auto self = weak.lock())
                asio::dispatch(asio::bind_executor(
                    exec, [self, snap] { self->snapshot_buffer_.push_back(std::unique_ptr< aggregate_book_snapshot >(snap)); }));
            else
                delete snap;
        };

        auto pop_or_create = [&]
        {
            if (snapshot_buffer_.empty())
                return std::make_unique< aggregate_book_snapshot >();
            else
            {
                auto result = std::move(snapshot_buffer_.back());
                snapshot_buffer_.pop_back();
                return result;
            }
        };

        return std::shared_ptr< aggregate_book_snapshot >(pop_or_create(), deleter);
    }

  private:
    std::vector< std::unique_ptr< aggregate_book_snapshot > > snapshot_buffer_;
};



}   // namespace trading
}   // namespace arby

#endif   // ARBY_ARBY_TRADING_AGGREGATE_BOOK_FEED_HPP
