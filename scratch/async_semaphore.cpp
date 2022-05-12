//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/arby
//

#include "config/asio.hpp"

#include <fmt/ostream.h>

namespace arby
{
namespace asioex
{

template < class Executor, class = void >
struct basic_async_semaphore;

template < class Executor >
struct basic_async_semaphore< Executor, std::enable_if_t< asio::execution::is_executor_v< asio::any_io_executor > > >
{
    using executor_type = Executor;

    struct blip
    {
    };

    basic_async_semaphore(Executor const &exec, int count = 1)
    : ch_(exec)
    , count_(count)
    {
    }

    template < BOOST_ASIO_COMPLETION_TOKEN_FOR(void(error_code))
                   CompletionHandler BOOST_ASIO_DEFAULT_COMPLETION_TOKEN_TYPE(Executor) >
    BOOST_ASIO_INITFN_RESULT_TYPE(CompletionHandler, void(error_code))
    acquire(CompletionHandler &&handler BOOST_ASIO_DEFAULT_COMPLETION_TOKEN(Executor))
    {
        auto op = [this, coro = asio::coroutine(), post = true]< class Self >(Self &&self, error_code ec = {}, blip = {}) mutable
        {
            BOOST_ASIO_CORO_REENTER(coro) for (;;)
            {
                if (count_ <= 0)
                {
                    post = false;
                    BOOST_ASIO_CORO_YIELD
                    ch_.async_receive(std::move(self));
                    if (ec)
                        goto done;
                }
                --count_;
                if (post)
                {
                    post = false;
                    BOOST_ASIO_CORO_YIELD
                    asio::post(asio::experimental::append(std::move(self), ec));
                }
            done:
                self.complete(ec);
                break;
            }
        };

        return asio::async_compose< CompletionHandler, void(error_code) >(std::move(op), handler, *this);
    }

    bool
    try_acquire()
    {
        if (count_ > 0)
        {
            --count_;
            return true;
        }
        return false;
    }

    void
    release()
    {
        ++count_;
        ch_.try_send(error_code(), blip());
    }

    executor_type
    get_executor()
    {
        return ch_.get_executor();
    }

  private:
    asio::experimental::channel< Executor, void(error_code, blip) > ch_;
    int                                                             count_;
};

using async_semaphore = basic_async_semaphore< asio::any_io_executor >;

}   // namespace asioex

namespace coro
{
struct semaphore
{
    struct blip
    {
    };

    semaphore(asio::any_io_executor exec, int count = 1)
    : ch_(exec)
    , count_(count)
    {
    }

    asio::awaitable< void >
    acquire()
    {
        if (count_ <= 0)
            co_await ch_.async_receive(asio::use_awaitable);
        --count_;
        co_return;
    }

    bool
    try_acquire()
    {
        if (count_ > 0)
        {
            --count_;
            return true;
        }
        return false;
    }

    void
    release()
    {
        ++count_;
        ch_.try_send(error_code(), blip());
    }

    asio::experimental::channel< void(error_code, blip) > ch_;
    int                                                   count_;
};

}   // namespace coro

asio::awaitable< void >
proc(std::shared_ptr< asioex::async_semaphore > sem, int i)
{
    fmt::print("entering proc {}\n", i);

    if (!sem->try_acquire())
        co_await sem->acquire(asio::use_awaitable);

    fmt::print("performing proc {}\n", i);

    auto t =
        asio::steady_timer(co_await asio::this_coro::executor, asio::steady_timer::clock_type::now() + std::chrono::seconds(1));
    co_await t.async_wait(asio::use_awaitable);

    fmt::print("releasing proc {}\n", i);
    sem->release();
}

asio::awaitable< void >
comain()
{
    auto exec = co_await asio::this_coro::executor;

    auto sem = std::make_shared< asioex::async_semaphore >(exec, 1);
    for (int i = 0; i < 10; ++i)
        asio::co_spawn(exec, proc(sem, i), asio::detached);
}
}   // namespace arby

using namespace arby;

int
main()
{
    asio::io_context ioc;
    asio::co_spawn(ioc, arby::comain(), asio::detached);
    ioc.run();
}
