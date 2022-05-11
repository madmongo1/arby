//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/arby
//

#ifndef ARBY_ARBY_IMPL_CONFIGURATION_SERVICE_HPP
#define ARBY_ARBY_IMPL_CONFIGURATION_SERVICE_HPP

#include "configuration.hpp"
#include "entity/invariants.hpp"

namespace arby
{
struct configuration_service
{
    std::shared_ptr< configuration >
    require(entity::invariants invariants)
    {
        auto exec = asio::make_strand(invariants.require< asio::io_context::executor_type >());

        auto lock      = std::unique_lock(m_);
        auto candidate = cache_.lock();
        if (!candidate)
        {
            auto impl = std::make_shared< configuration_impl >(exec);
            candidate = std::make_shared< configuration >(impl);
            cache_    = candidate;
            lock.unlock();
        }
        return candidate;
    }

    std::mutex                     m_;
    std::weak_ptr< configuration > cache_;
};
}   // namespace arby

#endif   // ARBY_ARBY_IMPL_CONFIGURATION_SERVICE_HPP