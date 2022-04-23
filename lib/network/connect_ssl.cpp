//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/router
//

#include "connect_ssl.hpp"

#include "fmt/format.h"
#include "util/monitor.hpp"

namespace arby::network
{
asio::awaitable< tcp::resolver ::results_type >
resolve(std::string const &host, std::string const &service)
{
    using asio::use_awaitable;

    auto sentinel = util::monitor::record(
        fmt::format("{}({}:{})", __func__, host, service));

    auto resolver = tcp::resolver(co_await asio::this_coro::executor);
    co_return co_await resolver.async_resolve(host, service, use_awaitable);
}

asio::awaitable< tcp::endpoint >
connect(tcp::socket &sock, std::string const &host, std::string const &service)
{
    using asio::use_awaitable;

    auto sentinel = util::monitor::record(
        fmt::format("{}({}:{})", __func__, host, service));

    auto endpoints = co_await resolve(host, service);
    auto ep        = co_await asio::async_connect(sock, endpoints, use_awaitable);
    co_return ep;
}

asio::awaitable< void >
connect(ssl::stream< tcp::socket > &stream,
        std::string const          &host,
        std::string const          &service)
{
    using asio::use_awaitable;

    auto sentinel = util::monitor::record(
        fmt::format("{}({}:{})", __func__, host, service));

    co_await connect(stream.next_layer(), host, service);

    if (!SSL_set_tlsext_host_name(stream.native_handle(), host.c_str()))
        throw sys::system_error(ERR_get_error(),
                                asio::error::get_ssl_category());

    co_await stream.async_handshake(ssl::stream_base::client, use_awaitable);
}

asio::awaitable< void >
connect(websocket::stream< ssl::stream< tcp::socket > > &stream,
        std::string const                               &host,
        std::string const                               &service,
        std::string const                               &path)
{
    using asio::use_awaitable;

    auto sentinel = util::monitor::record(
        fmt::format("{}({}:{}{})", __func__, host, service, path));

    co_await connect(stream.next_layer(), host, service);

    co_await stream.async_handshake(host, path, use_awaitable);
}
}   // namespace arby::network
