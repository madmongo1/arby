//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/arby
//

#ifndef ARBY_ARBY_TRADING_ENTITY_HPP
#define ARBY_ARBY_TRADING_ENTITY_HPP

#include "config/asio.hpp"

namespace arby
{
namespace trading
{

using entity_key = std::string;

struct entity_service_impl;

struct entity_impl : std::enable_shared_from_this< entity_impl >
{
    using executor_type = asio::any_io_executor;

    entity_impl(entity_service_impl *service, executor_type exec, entity_key key, int version);

    void
    start();

    void
    stop();

    executor_type const &
    get_executor() const
    {
        return exec_;
    }

    template < class T >
    static auto
    shared_from(T *p) -> std::shared_ptr< T >
    {
        return std::static_pointer_cast< T >(p->shared_from_this());
    }

    template < class T >
    static auto
    weak_from(T *p) -> std::weak_ptr< T >
    {
        return std::static_pointer_cast< T >(p->weak_from_this());
    }

    /// @brief Produce a detailed report of the object
    /// @return a string containing formatted text
    virtual asio::awaitable< std::string >
    detail() const = 0;

    /// @brief Produce a one-line summary of the object
    /// @return a string containing text
    virtual asio::awaitable< std::string >
    summary() const = 0;

    entity_key const &
    key() const
    {
        return key_;
    }

    std::uint64_t
    version() const
    {
        return version_;
    }

  private:
    virtual void handle_start() = 0;
    virtual void handle_stop() = 0;

  private:
    executor_type        exec_;
    entity_service_impl *service_;

    /// @brief Key summarising the creation arguments of this entity in such a way that this may be used as an index for entity
    /// lookup
    const entity_key key_;

    /// @brief every entity_impl created has a unique version, even if it shares a key with another
    const std::uint64_t version_;
};

struct entity_impl_service
{
    asio::any_io_executor exec_;

    /// @brief Enumerate all entities into an HTML body
    /// @param body
    /// @return
    asio::awaitable< void >
    enumerate(std::string &body) const;

    /// @brief A map of all instances of an entity_impl.
    /// @note if an entity is destroyed and quickly recreated, the impl of the destroyed entity may still be shutting down.
    /// In this case, more than one version of the same entity may exist at once. But only one should be in the running state.
    std::unordered_multimap< entity_key, std::weak_ptr< entity_impl > > cache_;
};

/// @brief Base class of any object that can be managed by entity caches.
///
/// All entities are associated with an executor and control the lifetime of the underlying implementation
struct entity
{
    using executor_type = entity_impl::executor_type;

    entity(std::shared_ptr< entity_impl > impl)
    : impl_(impl)
    {
        assert(impl_);
        impl_->start();
    }

    entity(entity const &) = delete;
    entity &
    operator=(entity const &) = delete;
    ~entity()
    {
        if (impl_)
            impl_->stop();
    }

    executor_type const &
    get_executor() const
    {
        return impl_->get_executor();
    }

  private:
    executor_type                  exec_;
    std::shared_ptr< entity_impl > impl_;
};

}   // namespace trading
}   // namespace arby

#endif   // ARBY_ARBY_TRADING_ENTITY_HPP
