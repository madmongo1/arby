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

struct entity_impl : std::enable_shared_from_this< entity_impl >
{
    using executor_type = asio::any_io_executor;

    virtual void
    start() = 0;
    virtual void
    stop() = 0;

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

  private:
    executor_type exec_;
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
