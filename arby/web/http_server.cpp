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
#include "web/http_app.hpp"

#include <fmt/ostream.h>
#include <fmt/ranges.h>
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

http_server::impl::impl(asio::any_io_executor exec, std::string host, std::string port, std::shared_ptr< app_store > apps)
: exec_(exec)
, host(host)
, port(port)
, apps_(apps)
{
}

asio::awaitable< void >
http_server::impl::run()
{
    auto sentinel = util::monitor::record(fmt::format("{}::{}()", classname, __func__));

    auto ex = co_await asio::this_coro::executor;

    co_await serve(co_await resolve(host, port));
}

namespace
{

template < class T >
struct printer;

template <>
struct printer< tcp::resolver::results_type >
{
    friend std::ostream &
    operator<<(std::ostream &os, printer const &p)
    {
        os << "(";
        const char *sep = "";
        for (auto &&r : p.arg)
        {
            os << sep << r.endpoint();
            sep = ", ";
        }
        return os << ")";
    }

    tcp::resolver::results_type const &arg;
};

template < class T >
printer< T >
print(T const &x)
{
    return printer< T > { x };
}
}   // namespace

asio::awaitable< void >
http_server::impl::serve(tcp::resolver::results_type endpoints)
{
    spdlog::debug("{}::{}({})", classname, __func__, print(endpoints));
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

struct http_server::epilog
{
    std::shared_ptr< http_server::impl > self;
    std::string                          service;
    bool                                 deleted = false;

    ~epilog()
    {
        assert(!deleted);
        deleted = true;
    }

    void
    operator()(std::exception_ptr ep) const
    {
        try
        {
            if (ep)
                std::rethrow_exception(ep);
            spdlog::info("http_server[{}:{}]::serve - {} finished", self->host, self->port, service);
        }
        catch (std::exception &e)
        {
            spdlog::error("http_server[{}:{}]::serve - {} exception: {}", self->host, self->port, service, e.what());
        }
    }
};

asio::awaitable< void >
http_server::protect(std::shared_ptr< impl > impl, asio::awaitable< void > a)
{
    co_return co_await std::move(a);
}

asio::awaitable< void >
http_server::impl::serve(tcp::endpoint ep)
{
    using asio::co_spawn;
    using asio::use_awaitable;
    using asio::this_coro::executor;

    auto sentinel = util::monitor::record(fmt::format("{}::{}({})", classname, __func__, ep));

    auto epilog = [this](std::exception_ptr ep)
    {
        try
        {
            if (ep)
                std::rethrow_exception(ep);
            spdlog::info("http_server[{}:{}]::serve - serve finished", host, port);
        }
        catch (std::exception &e)
        {
            spdlog::error("http_server[{}:{}]::serve - serve exception: {}", host, port, e.what());
        }
    };

    auto run_session = [](std::shared_ptr< impl > self, tcp::socket &&sock) { return session(self, std::move(sock)); };

    for (;;)
    {
        auto acceptor = tcp::acceptor(co_await executor, ep, true);
        auto sock     = tcp::socket(co_await executor);
        co_await acceptor.async_accept(sock, use_awaitable);
        co_spawn(co_await executor, run_session(shared_from_this(), std::move(sock)), epilog);
    }
}

asio::awaitable< void >
http_server::impl::session(std::shared_ptr< impl > self, tcp::socket sock)
{
    using asio::use_awaitable;
    using namespace asio::experimental::awaitable_operators;
    using namespace std::literals;

    auto sentinel = util::monitor::record(fmt::format("{}::{}({})", classname, __func__, sock.remote_endpoint()));

    auto timer   = asio::steady_timer(sock.get_executor());
    auto timeout = [&timer]() -> asio::awaitable< void >
    {
        timer.expires_after(5s);
        error_code ec;
        co_await timer.async_wait(asio::redirect_error(use_awaitable, ec));
    };

    auto request = http::request< http::string_body >();
    auto rxbuf   = beast::flat_buffer();
    auto again   = false;
    do
    {
        request.clear();
        request.body().clear();

        if (auto which = co_await (timeout() || http::async_read(sock, rxbuf, request, use_awaitable)); which.index() == 0)
            break;

        spdlog::info("{}::{}({}) {} {}", classname, __func__, sock.remote_endpoint(), request.method_string(), request.target());

        // match path against installed services

        auto handled = false;
        again        = false;

        for (auto &entry : self->apps_->store_)
        {
            auto match = std::cmatch();
            if (std::regex_match(request.target().data(), request.target().data() + request.target().size(), match, entry.re))
            {
                again   = co_await entry.app(sock, request);
                handled = true;
                break;
            }
        }

        if (!handled)
        {
            auto response = http::response< http::string_body >();
            response.result(http::status::not_found);
            response.version(request.version());
            response.keep_alive(request.keep_alive());
            response.body() = fmt::format("No app registered that matches {}\r\n", request.target());
            response.body() += "Valid route regexes are:\r\n";
            for (auto &entry : self->apps_->store_)
                response.body() += entry.def + "\r\n";
            response.prepare_payload();
            co_await http::async_write(sock, response, use_awaitable);
            again = !response.need_eof();
        }

        // ... or default
    } while (again);

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
    spdlog::debug("{}::{}", classname, __func__);
    asioex::terminate(stop_signal);
}

// ===== app_store =====

http_server::app_store::app_store(asio::any_io_executor exec)
: exec_(exec)
{
}

void
http_server::app_store::add_app(std::string def, http_app app)
{
    auto re = std::regex(def, std::regex_constants::icase);
    store_.push_back(app_entry { .def = std::move(def), .re = std::move(re), .app = std::move(app) });
}

// ===== http_server =====

http_server::http_server(executor_type exec)
: exec_(std::move(exec))
, impls_()
, apps_(std::make_unique< app_store >(exec_))
{
}

http_server::http_server(http_server &&other)
: impls_(std::move(other.impls_))
, apps_(std::move(other.apps_))
{
}

http_server &
http_server::operator=(http_server &&other)
{
    shutdown();
    impls_ = std::move(other.impls_);
    apps_  = std::move(other.apps_);
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
    auto my_impl = std::make_shared< impl >(get_executor(), host, port, apps_);
    impls_.push_back(my_impl);
    co_spawn(my_impl->get_executor(),
             std::bind(&impl::run, my_impl),
             asio::bind_cancellation_slot(my_impl->stop_signal.slot(), epilog { .self = my_impl, .service = "serve" }));
}

void
http_server::add_app(std::string text, http_app app)
{
    apps_->add_app(std::move(text), std::move(app));
}
}   // namespace arby::web