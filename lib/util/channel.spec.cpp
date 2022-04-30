//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/arby
//

#include "../../arby/logging/data_log.hpp"
#include "config/asio.hpp"

#include <boost/filesystem.hpp>
#include <doctest/doctest.h>

using namespace arby;

TEST_SUITE("scratch")
{
    TEST_CASE("channel close")
    {
        using asio::awaitable;
        using asio::co_spawn;
        using asio::detached;
        using asio::redirect_error;
        using asio::use_awaitable;
        using asio::experimental::as_tuple;

        asio::io_context ioc;

        asio::experimental::channel< void(error_code, std::string) > ch(ioc, 10);
        CHECK(ch.is_open());
        CHECK(ch.try_send(error_code(), "Hello"));
        CHECK(ch.try_send(error_code(), "World"));
        CHECK(ch.try_send(asio::error::eof, std::string()));

        co_spawn(
            ioc,
            [&]() -> awaitable< void >
            {
                error_code  e;
                std::string s;
                std::tie(e, s) = co_await ch.async_receive(as_tuple(use_awaitable));
                CHECK(!e);
                CHECK(s == "Hello");
                std::tie(e, s) = co_await ch.async_receive(as_tuple(use_awaitable));
                CHECK(!e);
                CHECK(s == "World");
                std::tie(e, s) = co_await ch.async_receive(as_tuple(use_awaitable));
                CHECK(e == asio::error::eof);
                CHECK(s == "");
            },
            [](std::exception_ptr ep) { CHECK(ep == nullptr); });

        ioc.run();
    }
}