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
struct entity_base : std::enable_shared_from_this< entity_base >
{
    WISE_ENUM_MEMBER(entity_state, not_started, prepared, started, stopped)
    template < class Stream >
    friend decltype(auto)
    operator<<(Stream &os, entity_state es)
    {
        return os << wise_enum::to_string(es);
    }

    virtual std::string_view
    classname() const;

    entity_base(asio::any_io_executor exec, entity_service entity_svc)
    : exec_(exec)
    , entity_svc_(entity_svc)
    {
    }

    void
    prepare()
    {
        assert(estate_ == not_started);
        do_prepare();
        estate_ = prepared;
    }

    void
    start()
    {
        using asio::bind_executor;
        using asio::dispatch;

        assert(estate_ == prepared);

        entity_version_ = entity_svc_.notify(key_, weak_from_this());

        dispatch(bind_executor(get_executor(), [self = shared_from_this()] { self->handle_start(); }));
        estate_ = started;
    }

    void
    stop()
    {
        using asio::bind_executor;
        using asio::dispatch;

        assert(estate_ == started);

        dispatch(bind_executor(get_executor(), [self = shared_from_this()] { self->handle_stop(); }));
        estate_ = stopped;
    }

    /// Summarise the object in a one-line report.
    /// @param buffer appends to this string. Does not add a newline.
    /// @note must be called on the entity's executor
    std::string
    summary() const;

    virtual ~entity_base() = default;

    asio::any_io_executor const &
    get_executor() const;

    entity_key const &
    key() const;

  private:
    virtual void
    extend_summary(std::string &buffer) const;

    virtual void
    do_prepare() {};

    virtual void
    handle_start() {};

    virtual void
    handle_stop() {};

    asio::any_io_executor exec_;
    entity_service        entity_svc_;
    entity_key            key_;
    int                   entity_version_ = -1;
    entity_state          estate_         = not_started;
};

template < class T >
concept is_entity_impl = std::is_base_of_v< entity_base, T >;

struct entity_handle_base
{
    virtual ~entity_handle_base() = default;
};

template < is_entity_impl T >
struct entity_handle
{
    template < class... Args >
    static auto
    create(Args &&...args) -> entity_handle;

    template < class U >
        requires std::is_base_of_v< T, U >
    entity_handle(std::shared_ptr< U > impl)
    : impl_(impl)
    {
        impl_->prepare();
        impl_->start();
    }

    asio::awaitable< std::string >
    summary() const
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

    entity_handle& operator=(entity_handle &&other)
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
entity_handle< T >::create(Args &&...args) -> entity_handle< T >
{
    auto impl = std::make_shared< T >(std::forward< Args >(args)...);
    impl->start();
    return entity_handle(impl);
}

}   // namespace entity
}   // namespace arby

#endif   // ARBY_ENTITY_BASE_HPP
