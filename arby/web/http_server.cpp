//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/arby
//

#include "web/http_server.hpp"

#include "asioex/helpers.hpp"
#include "asioex/scoped_interrupt.hpp"
#include "config/http.hpp"
#include "util/monitor.hpp"

#include <fmt/ostream.h>
#include <spdlog/spdlog.h>

#include <cassert>

namespace arby::web
{

namespace
{
asio::awaitable< tcp::resolver::results_type >
resolve(std::string const &host, std::string const &port)
{
    auto resolver = tcp::resolver(co_await asio::this_coro::executor);
    auto results  = co_await resolver.async_resolve(host, port, asio::use_awaitable);
    co_return results;
}
}   // namespace

http_server::impl::impl(asio::any_io_executor exec, std::string host, std::string port)
: exec_(exec)
, host(host)
, port(port)
{
}

asio::awaitable< void >
http_server::impl::run()
{
    auto ex = co_await asio::this_coro::executor;

    co_await serve(co_await resolve(host, port));
}

asio::awaitable< void >
http_server::impl::serve(tcp::resolver::results_type endpoints)
{
    return serve(endpoints.begin(), endpoints.end());
}

asio::awaitable< void >
http_server::impl::serve(tcp::resolver::results_type::iterator first, tcp::resolver::results_type::iterator last)
{
    using namespace asio::experimental::awaitable_operators;

    assert(first != last);
    auto next = std::next(first);
    if (next == last)
        return serve(first->endpoint());
    else
        return (serve(first->endpoint()) && serve(next, last));
}

asio::awaitable< void >
http_server::impl::serve(tcp::endpoint ep)
{
    using asio::co_spawn;
    using asio::use_awaitable;
    using asio::this_coro::executor;

    auto sentinel = util::monitor::record(fmt::format("{}::{}({})", classname, __func__, ep));

    try
    {
        auto acceptor = tcp::acceptor(co_await executor, ep);
        acceptor.listen();
        for (;;)
        {
            auto sock = tcp::socket(co_await executor);
            co_await acceptor.async_accept(sock, use_awaitable);
            co_spawn(co_await executor,
                     session(std::move(sock)),
                     [self = shared_from_this()](std::exception_ptr ep)
                     {
                         try
                         {
                             if (ep)
                                 std::rethrow_exception(ep);
                             spdlog::info("http_server[{}:{}]::serve - session finished", self->host, self->port);
                         }
                         catch (std::exception &e)
                         {
                             spdlog::error("http_server[{}:{}]::serve - session exception: {}", self->host, self->port, e.what());
                         }
                     });
        }
    }
    catch (std::exception &e)
    {
        spdlog::error("http_server[{}:{}]::serve - accept exception: {}", host, port, e.what());
    }
}

asio::awaitable< void >
http_server::impl::session(tcp::socket sock)
{
    using asio::use_awaitable;
    auto sentinel = util::monitor::record(fmt::format("{}::{}({})", classname, __func__, sock.remote_endpoint()));

    auto request  = http::request< http::string_body >();
    auto response = http::response< http::string_body >();
    auto rxbuf    = beast::flat_buffer();
    do
    {
        request.clear();
        request.body().clear();
        co_await http::async_read(sock, rxbuf, request, use_awaitable);

        // match path against installed services

        response.clear();
        response.body().clear();
        response.body() = "Hello, World!\r\n";
        response.prepare_payload();
        co_await http::async_write(sock, response, use_awaitable);
    } while (!response.need_eof());

    co_return;
}

auto
http_server::impl::get_executor() const -> executor_type const &
{
    return exec_;
}

void
http_server::impl::stop()
{
    assert(asioex::on_correct_thread(get_executor()));
    asioex::terminate(stop_signal);
}

http_server::http_server(executor_type exec)
: exec_(std::move(exec))
, impls_()
{
}

http_server::http_server(http_server &&other)
: impls_(std::move(other.impls_))
{
}

http_server &
http_server::operator=(http_server &&other)
{
    shutdown();
    impls_ = std::move(other.impls_);
    return *this;
}

http_server::~http_server()
{
    shutdown();
}

void
http_server::shutdown()
{
    auto impls = std::exchange(impls_, {});
    for (auto impl : impls)
        asio::dispatch(asio::bind_executor(impl->get_executor(), std::bind(&impl::stop, impl)));
}

auto
http_server::get_executor() const -> executor_type const &
{
    return exec_;
}

void
http_server::serve(std::string host, std::string port)
{
    auto my_impl = std::make_shared< impl >(get_executor(), host, port);
    impls_.push_back(my_impl);
    co_spawn(my_impl->get_executor(),
             std::bind(&impl::run, my_impl),
             asio::bind_cancellation_slot(
                 my_impl->stop_signal.slot(),
                 [my_impl](std::exception_ptr ep)
                 {
                     try
                     {
                         if (ep)
                             std::rethrow_exception(ep);
                         spdlog::info("web::http_server[{}:{}] run completed", my_impl->host, my_impl->port);
                     }
                     catch (std::exception &e)
                     {
                         spdlog::error("web::http_server[{}:{}] exception: {}", my_impl->host, my_impl->port, e.what());
                     }
                 }));
}

}   // namespace arby::web