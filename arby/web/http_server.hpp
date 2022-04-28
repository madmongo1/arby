//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/arby
//

#ifndef ARBY_WEB_HTTP_SERVER
#define ARBY_WEB_HTTP_SERVER

#include "config/asio.hpp"

namespace arby::web
{
struct http_server
{
    static constexpr char classname[] = "http_server";
    using executor_type               = asio::any_io_executor;

  private:
    class impl : public std::enable_shared_from_this< impl >
    {
        executor_type exec_;

      public:
        std::string const host;
        std::string const port;

        asio::cancellation_signal stop_signal;

        impl(executor_type exec, std::string host, std::string port);

        executor_type const &
        get_executor() const;

        asio::awaitable< void >
        run();

        void
        stop();

      private:
        asio::awaitable< void >
        serve(tcp::endpoint where);

        asio::awaitable< void >
        serve(tcp::resolver::results_type::iterator first, tcp::resolver::results_type::iterator last);

        asio::awaitable< void >
        serve(tcp::resolver::results_type range);

        asio::awaitable< void >
        session(tcp::socket sock);
    };

    struct epilog;

    executor_type exec_;

    std::vector< std::shared_ptr< impl > > impls_;

  public:
    http_server(executor_type exec);
    http_server(http_server &&other);
    http_server &
    operator=(http_server &&);
    ~http_server();

    void
    serve(std::string host, std::string port);

    void
    shutdown();

    executor_type const &
    get_executor() const;
};
}   // namespace arby::web

#endif
