//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/arby
//

#include "asioex/async_semaphore.hpp"
#include "config/websocket.hpp"
#include "power_trade/connector.hpp"
#include "power_trade/event_listener.hpp"
#include "power_trade/native_symbol.hpp"
#include "power_trade/orderbook_listener_impl.hpp"
#include "trading/market_key.hpp"
#include "util/monitor.hpp"

#include <boost/asio.hpp>
#include <boost/asio/experimental/awaitable_operators.hpp>
#include <boost/asio/experimental/channel.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/filesystem.hpp>
#include <boost/system/result.hpp>
#include <fmt/chrono.h>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <spdlog/spdlog.h>
#include <util/truncate.hpp>

#include <cassert>
#include <cstdio>
#include <deque>
#include <iostream>
#include <set>
#include <string>

namespace fs = boost::filesystem;
using namespace arby;

std::string progname;

#include <termios.h>

namespace
{
struct termios original_termios;
}

void
set_up_stdin()
{
    ::tcgetattr(STDIN_FILENO, &original_termios);
    auto my_termios = original_termios;
    my_termios.c_lflag &= ~(ICANON);
    std::atexit([]() { ::tcsetattr(STDIN_FILENO, TCSANOW, &original_termios); });

    //    ::cfmakeraw(&my_termios);
    ::tcsetattr(STDIN_FILENO, TCSANOW, &my_termios);
}

asio::awaitable< void >
monitor_quit(asio::cancellation_signal &sig, power_trade::connector &conn)
{
    auto sentinel = util::monitor::record("monitor_quit");
    using asio::redirect_error;
    using asio::use_awaitable;

    set_up_stdin();
    auto input = asio::posix::stream_descriptor(co_await asio::this_coro::executor, ::dup(STDIN_FILENO));

    for (;;)
    {
        char       buf;
        error_code ec;
        auto       size = co_await asio::async_read(input, asio::mutable_buffer(&buf, 1), redirect_error(use_awaitable, ec));
        if (ec)
            break;
        if (size && buf == 'q')
        {
            fmt::print("quitting\n");
            break;
        }
        if (size && buf == 'i')
        {
            fmt::print("interrupting\n");
            conn.interrupt();
        }
    }

    sig.emit(asio::cancellation_type::all);
    fmt::print("{}: done\n", __func__);
}

asio::awaitable< void >
check(ssl::context &sslctx)
{
    auto this_exec = co_await asio::this_coro::executor;

    auto sentinel = util::monitor::record("check");
    auto con      = std::make_shared< power_trade::connector >(this_exec, sslctx);

    auto watch1 = power_trade::event_listener::create(con, "heartbeat");
    auto watch2 = power_trade::orderbook_listener_impl::create(this_exec, con, trading::spot_key("eth/usd"));
    auto [w2con, snap] = watch2->subscribe([](std::shared_ptr< power_trade::orderbook_snapshot const > snap)
                      { spdlog::info("*** snapshot *** {}", snap); });
    spdlog::info("*** snapshot *** {}", snap);
    auto watch3 = power_trade::orderbook_listener_impl::create(this_exec, con, trading::spot_key("btc-usd"));

    asio::cancellation_signal cancel_sig;
    co_await monitor_quit(cancel_sig, *con);
}

asio::awaitable< void >
watch()
{
    using namespace asio::experimental::awaitable_operators;

    auto sigs = asio::signal_set(co_await asio::this_coro::executor, SIGINT);
    co_await (util::monitor::mon() || sigs.async_wait(asio::use_awaitable));
}

int
main(int argc, char **argv)
{
    using asio::co_spawn;
    using asio::detached;

    asio::io_context ioc;
    ssl::context     sslctx(ssl::context_base::tls_client);

    auto arg0 = fs::path(argv[0]);
    progname  = arg0.stem().string();

    spdlog::set_level(spdlog::level::debug);
    fmt::print("{}: starting\n", progname);

    try
    {
        co_spawn(ioc, watch(), detached);
        co_spawn(ioc, check(sslctx), detached);
        ioc.run();
    }
    catch (const std::exception &e)
    {
        fmt::print(stderr, "{}: exception: {}\n", progname, e.what());
        return 127;
    }

    fmt::print("{}: done\n", progname);
}