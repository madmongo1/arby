//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/arby
//

#ifndef ARBY_ARBY_IMPL_CONFIGURATION_IMPL_HPP
#define ARBY_ARBY_IMPL_CONFIGURATION_IMPL_HPP

#include "asioex/async_condition_variable.hpp"
#include "config/json.hpp"
#include "config/signals.hpp"
#include "config/wise_enum.hpp"
#include "detail/has_stop_signal.hpp"
#include <unordered_map>

namespace arby
{

json::value const *
seek(json::value const *root, std::string_view path);

WISE_ENUM_CLASS(configuration_status, good, not_ready, error)
template < class S >
decltype(auto)
operator<<(S &os, configuration_status e)
{
    return os << wise_enum::to_string(e);
}

struct configuration_snapshot
{
    configuration_status                 status = configuration_status::not_ready;
    std::shared_ptr< json::value const > value  = std::make_shared< json::value >(nullptr);
};

struct configuration_impl : detail::has_stop_signal
{
    using snapshot_class = configuration_snapshot;
    using snapshot_type  = snapshot_class;

    using executor_type = asio::any_io_executor;

    using signal_type = typename sigs::signal_type< void(snapshot_type), sigs::keywords::mutex_type< sigs::dummy_mutex > >::type;
    using slot_type   = signal_type::slot_type;

    configuration_impl(executor_type exec)
    : exec_(exec)
    {
    }

    std::string_view
    classname() const;

    /// Update the config
    ///
    /// @note Must be executed on the correct executor
    /// @param v
    void
    update(json::value v)
    {
        snapshot_ = snapshot_type { .status = configuration_status::good, .value = std::make_shared< json::value >(std::move(v)) };

        for (auto &&[path, sub] : subs_)
        {
            sub.last = sub_snapshot(snapshot_, path);
            sub.signal(sub.last);
        }
    }

    std::tuple< sigs::connection, snapshot_type >
    subscribe(std::string const &path, slot_type slot)
    {
        auto isub = subs_.find(path);
        if (isub == subs_.end())
        {
            isub              = subs_.emplace(path, subscription()).first;
            isub->second.last = sub_snapshot(snapshot_, path);
        }
        auto con = isub->second.signal.connect(std::move(slot));
        return { con, isub->second.last };
    }

    void
    summary(std::string &buffer) const;

    static asio::awaitable< void >
    run(std::shared_ptr< configuration_impl > self);

    auto
    get_executor() const -> executor_type const &;

  private:
    static auto
    sub_snapshot(snapshot_type const &snap, std::string_view path) -> snapshot_type;

    static std::shared_ptr< json::value const >
    empty_value()
    {
        auto              nodelete = [](auto *) {};
        static const auto empty    = json::value();
        auto              pval     = std::shared_ptr< json::value const >(&empty, nodelete);
        return pval;
    }

    static snapshot_type
    default_snapshot()
    {
        auto              nodelete = [](auto *) {};
        static const auto empty    = json::value();
        auto              snap     = snapshot_type { .status = configuration_status::not_ready,
                                                     .value  = std::shared_ptr< json::value const >(&empty, nodelete) };
        return snap;
    }

    snapshot_type snapshot_ = default_snapshot();

    struct subscription
    {
        configuration_snapshot last = default_snapshot();
        signal_type            signal;
    };
    executor_type                                   exec_;
    std::unordered_map< std::string, subscription > subs_;
};

}   // namespace arby

#endif   // ARBY_ARBY_IMPL_CONFIGURATION_IMPL_HPP
