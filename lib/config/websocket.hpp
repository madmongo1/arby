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
