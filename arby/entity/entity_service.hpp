//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/arby
//

#ifndef ARBY_ENTITY_ENTITY_SERVICE_HPP
#define ARBY_ENTITY_ENTITY_SERVICE_HPP

#include "config/asio.hpp"
#include "entity/entity_key.hpp"

#include <any>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

namespace arby::entity
{

struct entity_base;
struct entity_handle_base;

struct entity_service
{
    struct impl
    {
        std::unordered_map< entity_key, std::weak_ptr< entity::entity_handle_base > > cache_;
        std::set< std::weak_ptr< entity_base >, std::owner_less<> >                   impl_cache_;
        std::mutex                                                                    m_;

        asio::awaitable< void >
        summary(std::string &body);

        void
        notify_create(std::weak_ptr< entity_base > const &weak)
        {
            auto lock = std::unique_lock(m_);
            impl_cache_.insert(weak);
        }

        void
        notify_destroy(std::weak_ptr< entity_base > const &weak)
        {
            auto lock = std::unique_lock(m_);
            impl_cache_.erase(weak);
        }

        template < class F >
        void
        enumerate(F &&f)
        {
            auto lock       = std::unique_lock(m_);
            auto cache_copy = this->impl_cache_;
            lock.unlock();
            for (auto &weak : cache_copy)
                if (auto p = weak.lock())
                    f(p);
        }
    };

    using implementation_type = impl *;

    static implementation_type
    get_implementation()
    {
        static impl i;
        return &i;
    }

    asio::awaitable< void >
    summary(std::string &body)
    {
        return get_implementation()->summary(body);
    }

    void
    notify_create(std::weak_ptr< entity_base > const &weak)
    {
        get_implementation()->notify_create(weak);
    }

    void
    notify_destroy(std::weak_ptr< entity_base > const &weak)
    {
        get_implementation()->notify_destroy(weak);
    }

    template < class F >
    void
    enumerate(F &&f)
    {
        get_implementation()->enumerate(f);
    }

    template < class T, class F >
    std::shared_ptr< T >
    require(entity_key const &key, F &&f)
    {
        auto &&impl      = get_implementation();
        auto   lock      = std::unique_lock(impl->m_);
        auto  &weak      = impl->cache_[key];
        auto   candidate = std::static_pointer_cast< T >(weak.lock());
        if (!candidate)
        {
            candidate = f();
            weak      = candidate;
        }
        return candidate;
    }
};

}   // namespace arby::entity

#endif   // ARBY_ENTITY_SERVICE_HPP
