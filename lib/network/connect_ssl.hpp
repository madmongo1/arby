//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/router
//

#ifndef ARBY_LIB_NETWORK_CONNECT_SSL_HPP
#define ARBY_LIB_NETWORK_CONNECT_SSL_HPP

#include "config/asio.hpp"
#include "config/websocket.hpp"

namespace arby::network
{
asio::awaitable< tcp::resolver ::results_type >
resolve(std::string const &host, std::string const &service);

asio::awaitable< tcp::endpoint >
connect(tcp::socket &sock, std::string const &host, std::string const &service);

asio::awaitable< void >
connect(ssl::stream< tcp::socket > &stream,
        std::string const          &host,
        std::string const          &port);

asio::awaitable< void >
connect(websocket::stream< ssl::stream< tcp::socket > > &stream,
        std::string const                               &host,
        std::string const                               &port,
        std::string const                               &path);
}   // namespace arby::network
#endif   // ARBY_LIB_NETWORK_CONNECT_SSL_HPP
