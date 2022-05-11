//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/arby
//

#ifndef ARBY_ENTITY_BASE_HPP
#define ARBY_ENTITY_BASE_HPP
#include "config/wise_enum.hpp"
#include "entity/entity_service.hpp"

#include <concepts>

namespace arby
{
namespace entity
{
template < class T >
struct enable_shared_from_that : std::enable_shared_from_this< T >
{
    template < std::derived_from< T > U >
    static std::shared_ptr< U >
    shared_from_that(U *that)
    {
        return std::static_pointer_cast< U >(that->shared_from_this());
    }

    template < std::derived_from< T > U >
    static std::shared_ptr< U >
    shared_from_that(U const *that)
    {
        return std::static_pointer_cast< U const >(that->shared_from_this());
    }

    template < std::derived_from< T > U >
    static std::weak_ptr< U >
    weak_from_that(U *that)
    {
        return std::static_pointer_cast< U >(that->weak_from_this());
    }

    template < std::derived_from< T > U >
    static std::weak_ptr< U >
    shared_from_that(U const *that)
    {
        return std::static_pointer_cast< U const >(that->weak_from_this());
    }
};

struct entity_base : enable_shared_from_that< entity_base >
{
    WISE_ENUM_MEMBER(entity_state, not_started, started, stopped)

    template < class Stream >
    friend decltype(auto)
    operator<<(Stream &os, entity_state es)
    {
        return os << wise_enum::to_string(es);
    }

    virtual std::string_view
    classname() const;

    entity_base(asio::any_io_executor exec);

    ///
    /// @note runs on the executor of the creator. Part of the construction cycle
    void
    start();

    /// @note must be called on the entity's executor
    void
    stop();

    /// Summarise the object in a one-line report.
    /// @param buffer appends to this string. Does not add a newline.
    /// @note must be called on the entity's executor
    std::string
    summary() const;

    virtual ~entity_base() = default;

    asio::any_io_executor const &
    get_executor() const;

  private:
    virtual void
    extend_summary(std::string &buffer) const;

    virtual void
    handle_start() {};

    virtual void
    handle_stop() {};

    asio::any_io_executor exec_;
    entity_state          estate_ = not_started;
};

template < class T >
concept is_entity_impl = std::is_base_of_v< entity_base, T >;

struct entity_handle_base
{
    virtual asio::awaitable< std::string >
    summary() const = 0;

    virtual asio::any_io_executor
    get_executor() const = 0;

    virtual ~entity_handle_base() = default;
};

template < is_entity_impl T >
struct entity_handle : entity_handle_base
{
    using implementation_class = T;
    using implementation_type  = std::shared_ptr< implementation_class >;

    template < class U >
        requires std::is_base_of_v< T, U >
    entity_handle(std::shared_ptr< U > impl)
    : impl_(impl)
    {
    }

    template < class... Args >
    entity_handle(asio::any_io_executor exec, Args &&...args)
    : impl_(construct(exec, std::forward< Args >(args)...))
    {
    }

    asio::any_io_executor
    get_executor() const override;
    asio::awaitable< std::string >
    summary() const override
    {
        using asio::awaitable;
        using asio::co_spawn;
        using asio::use_awaitable;

        auto   this_exec = co_await asio::this_coro::executor;
        auto &&my_exec   = impl_->get_executor();
        if (this_exec == my_exec)
        {
            co_return impl_->summary();
        }
        else
        {
            co_return co_await co_spawn(
                my_exec, [impl = impl_]() -> awaitable< std::string > { co_return impl->summary(); }, use_awaitable);
        }
    }

    entity_handle(entity_handle &&other)
    : impl_(std::move(other.impl_))
    {
    }

    entity_handle &
    operator=(entity_handle &&other)
    {
        if (&other != this)
        {
            stop_check();
            impl_ = std::move(other.impl_);
        }
        return *this;
    }

    ~entity_handle() { stop_check(); }

  private:
    template < class... Args >
    static auto
    construct(Args &&...args) -> std::shared_ptr< T >;

    void
    stop_check()
    {
        using asio::bind_executor;
        using asio::dispatch;

        if (impl_)
        {
            dispatch(bind_executor(impl_->get_executor(), [i = impl_] { i->stop(); }));
        }
    }

    std::shared_ptr< T > impl_;
};

template < is_entity_impl T >
template < class... Args >
auto
entity_handle< T >::construct(Args &&...args) -> std::shared_ptr< T >
{
    auto                 uimpl = std::make_unique< T >(std::forward< Args >(args)...);
    std::shared_ptr< T > impl { uimpl.release(),
                                [](T *p)
                                {
                                    auto w   = p->weak_from_this();
                                    auto svc = entity::entity_service();
                                    svc.notify_destroy(w);
                                    delete p;
                                } };
    auto svc = entity::entity_service();
    svc.notify_create(impl);
    impl->start();
    return impl;
}

template < is_entity_impl T >
asio::any_io_executor
entity_handle< T >::get_executor() const
{
    return impl_->get_executor();
}

}   // namespace entity
}   // namespace arby

#endif   // ARBY_ENTITY_BASE_HPP
