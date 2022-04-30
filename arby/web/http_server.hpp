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
#include "web/http_app.hpp"

#include <regex>

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

        static
        asio::awaitable< void >
        session(std::shared_ptr<impl> self, tcp::socket sock);
    };

    asio::awaitable< void > static protect(std::shared_ptr< impl > impl, asio::awaitable< void > a);

    struct app_store
    {
        app_store(asio::any_io_executor exec);

        struct app_entry
        {
            std::string def;
            std::regex  re;
            http_app    app;
        };

        void
        add_app(std::string def, http_app app);

        asio::any_io_executor    exec_;
        std::vector< app_entry > store_;
    };

    struct epilog;

    executor_type exec_;

    std::vector< std::shared_ptr< impl > > impls_;
    std::unique_ptr< app_store >           apps_;

  public:
    http_server(executor_type exec);
    http_server(http_server &&other);
    http_server &
    operator=(http_server &&);
    ~http_server();

    void
    serve(std::string host, std::string port);

    void
    add_app(std::string re, http_app app);

    void
    shutdown();

    executor_type const &
    get_executor() const;
};
}   // namespace arby::web

#endif
