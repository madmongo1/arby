//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/arby
//

#include "web/http_app.hpp"

#include <fmt/format.h>
#include <fmt/ostream.h>

namespace arby::web
{

asio::awaitable< bool >
http_app_base::operator()(tcp::socket &stream, http::request< http::string_body > &request)
{
    using asio::use_awaitable;

    auto response = http::response< http::string_body >();
    response.result(http::status::not_found);
    response.version(request.version());
    response.keep_alive(request.keep_alive());
    response.body() = fmt::format("No app registered that matches {}\r\n", request.target());
    response.prepare_payload();
    co_await http::async_write(stream, response, use_awaitable);
    co_return !response.need_eof();
}

http_app::http_app(std::shared_ptr< http_app_base > impl)
: impl_(impl)
{
}

}   // namespace arby::web