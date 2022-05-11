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
#include "entity/invariants.hpp"

#include <any>
#include <concepts>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <typeinfo>
#include <unordered_map>

namespace arby::entity
{

struct entity_base;
struct entity_handle_base;

struct entity_service;

struct service_base
{
    virtual ~service_base() = default;

    virtual std::string const &
    classname() const = 0;

    virtual std::type_index
    index() const = 0;

    std::shared_ptr< entity_handle_base > virtual construct(entity::invariants const &invariants,
                                                            entity_service           &cache,
                                                            std::any const           &key) = 0;
};

template < std::derived_from< entity_handle_base > Interface, const char *ClassName >
struct basic_service : service_base
{
    std::string const &
    classname() const override
    {
        static const std::string x = ClassName;
        return x;
    }
    std::type_index
    index() const
    {
        return typeid(Interface);
    };

    std::shared_ptr< entity_handle_base >
    construct(entity::invariants const &invariants, entity_service &esvc, std::any const &key) override
    {
        if (auto igen = generators_.find(key.type()); igen != generators_.end())
        {
            return igen->second(invariants, esvc, key);
        }
    }

    std::unordered_map<
        std::string,
        std::function< std::shared_ptr< Interface >(entity::invariants const &, entity_service &, std::any const &) > >
        generators_;
};

struct entity_service
{
    struct impl
    {
        std::unordered_map< std::type_index, std::unique_ptr< service_base > > interface_services;
        entity::invariants                                                                      invariants_;
        std::unordered_map< entity_key, std::weak_ptr< entity::entity_handle_base > >           cache_;
        std::set< std::weak_ptr< entity_base >, std::owner_less<> >                             impl_cache_;
        std::mutex                                                                              m_;

        virtual ~impl() = default;

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

    /// Either locate
    /// @tparam T
    /// @tparam F
    /// @param key
    /// @param f
    /// @return
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

    /// Add invariants to the invariants set.
    ///
    /// Ther service's invariants are system-configured serivces that entities will require in order to run.
    /// @note it is not safe to add an invariant while any entity is running or while any other thread is accessing the service.
    /// Entities should be added at startup and left alone.
    /// @tparam Invariants
    /// @param is
    template < class... Invariants >
    void
    add_invariants(Invariants... is)
    {
        auto &&impl = get_implementation();
        (impl->invariants_.template add(std::forward< Invariants >(is)), ...);
    }

    /// Locate the required entity handle type, creating and startiing if if necessary.
    /// @tparam T is the type of entity handle required
    /// @return a shared pointer containing the entity handle
    template < std::derived_from< entity::entity_handle_base > T >
    std::shared_ptr< T >
    locate(std::any arg = std::any())
    {
        /*
        auto &&impl = get_implementation();
        auto &&type = typeid(T);
        auto& svc = impl->require_service(typeid(T));
        auto [key, handle] = svc->construct(*this, std::move(arg));
*/
        //        auto  &service = impl->services_.at(typeid(T));
        return std::shared_ptr< T >();
    }
};

struct config_service
{
//    std::shared_ptr<entity_handle_base> construct(entity_service svc, )
};

}   // namespace arby::entity

#endif   // ARBY_ENTITY_SERVICE_HPP
