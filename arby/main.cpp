//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/arby
//

#include "asioex/async_semaphore.hpp"
#include "asioex/scoped_interrupt.hpp"
#include "config/websocket.hpp"
#include "configuration.hpp"
#include "power_trade/connector.hpp"
#include "power_trade/event_listener.hpp"
#include "power_trade/native_symbol.hpp"
#include "power_trade/orderbook_listener_impl.hpp"
#include "power_trade/tick_logger.hpp"
#include "reactive/fix_connector.hpp"
#include "ssl_context.hpp"
#include "trading/market_key.hpp"
#include "util/monitor.hpp"
#include "web/entity_detail_app.hpp"
#include "web/entity_summary_app.hpp"
#include "web/http_server.hpp"

#include <boost/asio.hpp>
#include <boost/asio/experimental/awaitable_operators.hpp>
#include <boost/asio/experimental/channel.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/filesystem.hpp>
#include <boost/scope_exit.hpp>
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
monitor_keys(std::unordered_map< char, sigs::signal< void() > > &signals)
{
    auto sentinel = util::monitor::record(__func__);
    using asio::redirect_error;
    using asio::use_awaitable;

    set_up_stdin();
    auto input = asio::posix::stream_descriptor(co_await asio::this_coro::executor, ::dup(STDIN_FILENO));

    auto cstate = co_await asio::this_coro::cancellation_state;
    for (; cstate.cancelled() == asio::cancellation_type::none;)
    {
        char       buf;
        error_code ec;
        auto       size = co_await asio::async_read(input, asio::mutable_buffer(&buf, 1), redirect_error(use_awaitable, ec));
        if (ec)
            break;
        if (!size)
            continue;
        signals[buf]();
    }
}


asio::awaitable< void >
check(ssl::context &sslctx)
{
    using asio::use_awaitable;
    using namespace std::literals;

    auto sentinel  = util::monitor::record("check");
    auto this_exec = co_await asio::this_coro::executor;

    auto                                               stop_monitor = asio::cancellation_signal();
    std::unordered_map< char, sigs::signal< void() > > key_signals;
    sigs::scoped_connection                            qcon0 = key_signals['q'].connect([&] { asioex::terminate(stop_monitor); });

    auto                         svc = entity::entity_service();
    reactive::fix_connector_args reactive_args { .sender_comp_id      = "POWERTRADE_MD_1",
                                                 .target_comp_id      = "SWITCHBOARD_DEMO",
                                                 .socket_connect_host = "fix.demo.switchboard.reactivemarkets.com",
                                                 .socket_connect_port = "8289",
                                                 .use_ssl             = true };
    auto                         reactive1 = svc.locate< reactive::fix_connector >(reactive_args.to_key());

    auto cfg = svc.locate< configuration >();

    auto reactive = std::make_unique< reactive::fix_connector >(
        this_exec,
        sslctx,
        reactive::fix_connector_args { .sender_comp_id      = "POWERTRADE_MD_1",
                                       .target_comp_id      = "SWITCHBOARD_DEMO",
                                       .socket_connect_host = "fix.demo.switchboard.reactivemarkets.com",
                                       .socket_connect_port = "8289",
                                       .use_ssl             = true });
    auto rcon = key_signals['r'].connect([&] { reactive.reset(); });

    auto http_server = web::http_server(this_exec);
    http_server.serve("localhost", "8080");
    http_server.add_app("^/entities/?$", web::http_app::create< web::entity_summary_app >());
    http_server.add_app("^/entities/([0123456789abcdef]{40})(?:.([0-9]+))?/?$", web::http_app::create< web::entity_detail_app >());

    sigs::scoped_connection qcon1 = key_signals['q'].connect([&] { http_server.shutdown(); });

    auto con     = std::make_shared< power_trade::connector >(this_exec, sslctx);
    auto watch1  = std::make_unique< power_trade::event_listener >(con, "heartbeat");
    auto eth_log = std::make_unique< power_trade::tick_logger >(con, "ETH-USD", fs::temp_directory_path() / "eth-usd.txt");

    auto                    watch2 = power_trade::orderbook_listener_impl::create(this_exec, con, trading::spot_key("eth/usd"));
    sigs::scoped_connection w2con;
    std::shared_ptr< power_trade::orderbook_snapshot const > snap;
    std::tie(w2con, snap) = watch2->subscribe([](std::shared_ptr< power_trade::orderbook_snapshot const > snap)
                                              { spdlog::info("*** snapshot *** {}", snap); });
    spdlog::info("*** snapshot *** {}", snap);
    //    auto watch3         = power_trade::orderbook_listener_impl::create(this_exec, con, trading::spot_key("btc-usd"));
    //    auto [w3con, snap3] = watch3->subscribe([](std::shared_ptr< power_trade::orderbook_snapshot const > snap)
    //                                            { spdlog::info("*** snapshot *** {}", snap); });
    //    spdlog::info("*** snapshot *** {}", snap3);

    /*
asio::cancellation_signal cancel_sig;
co_await monitor_quit(cancel_sig, *con);
     */
    co_await co_spawn(
        this_exec, [&] { return monitor_keys(key_signals); }, asio::bind_cancellation_slot(stop_monitor.slot(), use_awaitable));
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

    asio::io_context  ioc;
    asio::thread_pool threadpool(6);
    ssl::context      sslctx(ssl::context_base::tls_client);

    auto arg0 = fs::path(argv[0]);
    progname  = arg0.stem().string();

    spdlog::set_level(spdlog::level::debug);
    fmt::print("{}: starting\n", progname);

    auto svc = entity::entity_service();
    svc.add_invariants(ioc.get_executor(), ssl_context(sslctx), threadpool.get_executor());
    //    svc.add_entity_service(entity::entity_interface_service<trading::aggregate_book, "AggregateBook">());

    try
    {
        //        co_spawn(ioc, watch(), detached);
        co_spawn(ioc, check(sslctx), detached);
        ioc.run();
        threadpool.join();
    }
    catch (const std::exception &e)
    {
        fmt::print(stderr, "{}: exception: {}\n", progname, e.what());
        return 127;
    }

    fmt::print("{}: done\n", progname);
}