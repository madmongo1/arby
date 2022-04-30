//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/arby
//

#ifndef ARBY_WEB_HTTP_APP
#define ARBY_WEB_HTTP_APP

#include "config/asio.hpp"
#include "config/http.hpp"

namespace arby::web
{
struct http_app_base
{
    virtual ~http_app_base() = default;

    virtual asio::awaitable< bool >
    operator()(tcp::socket &stream, http::request< http::string_body > &request);
};

struct http_app
{
    http_app(std::shared_ptr< http_app_base > impl);

    template < class T, class... Args >
    static http_app
    create(Args &&...args);

    asio::awaitable< bool >
    operator()(tcp::socket &stream, http::request< http::string_body > &request);

  private:
    std::shared_ptr< http_app_base > impl_;
};

// implementation

template < class T, class... Args >
http_app
http_app::create(Args &&...args)
{
    return http_app(std::make_shared< T >(std::forward< Args >(args)...));
}

}   // namespace arby::web

#endif
