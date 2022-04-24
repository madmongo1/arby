//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/arby
//
#ifndef ARBY_CONFIG_WEBSOCKET_HPP
#define ARBY_CONFIG_WEBSOCKET_HPP

#include "config/asio.hpp"

#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>

namespace arby
{
namespace beast     = boost::beast;
namespace websocket = beast::websocket;

}   // namespace arby

#endif
