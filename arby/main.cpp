#include "asioex/async_semaphore.hpp"

#include <boost/asio.hpp>
#include <boost/asio/experimental/awaitable_operators.hpp>
#include <boost/asio/experimental/channel.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/filesystem.hpp>
#include <fmt/chrono.h>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <util/truncate.hpp>

#include <cassert>
#include <cstdio>
#include <deque>
#include <iostream>
#include <set>
#include <string>

namespace beast     = boost::beast;
namespace sys       = boost::system;
namespace websocket = beast::websocket;
namespace asio      = boost::asio;
namespace ip        = asio::ip;
namespace ssl       = asio::ssl;
using tcp           = ip::tcp;
namespace fs        = boost::filesystem;

std::string progname;

enum trade_side
{
    buy,
    sell
};

struct instance
{
    std::string                           name;
    std::size_t                           version;
    std::chrono::system_clock::time_point creation_time;
};

std::ostream &
operator<<(std::ostream &os, instance const &i)
{
    fmt::print(os, "{}:{}:{}", i.name, i.version, i.creation_time);
    return os;
}

bool
operator<(instance const &l, instance const &r)
{
    return std::tie(l.name, l.version, l.creation_time) <
           std::tie(r.name, r.version, r.creation_time);
}

struct monitor
{
    static std::unordered_map< std::string, std::size_t > next_index_;
    static std::set< instance >                           instances_;
    using instance_iter = std::set< instance >::iterator;

    struct sentinel
    {
        sentinel(instance_iter it)
        : iter(it)
        , valid(true)
        {
        }
        sentinel(sentinel &&other)
        : iter(other.iter)
        , valid(std::exchange(other.valid, false))
        {
        }

        sentinel &
        operator=(sentinel const &) = delete;
        ~sentinel()
        {
            if (valid)
                monitor::erase(iter);
        }

        instance_iter iter;
        bool          valid;
    };

    static sentinel
    record(std::string name)
    {
        auto version = next_index_[name]++;
        auto time    = std::chrono::system_clock::now();
        auto result  = sentinel(instances_
                                   .insert(instance { .name = std::move(name),
                                                       .version       = version,
                                                       .creation_time = time })
                                   .first);
        fmt::print("monitor: create {}\n", *result.iter);
        return result;
    }

    static void
    erase(instance_iter iter)
    {
        fmt::print("monitor: destroy {}\n", *iter);
        instances_.erase(iter);
    }

    asio::awaitable< void > static mon()
    {
        auto timer = asio::steady_timer(co_await asio::this_coro::executor);
        while (1)
        {
            timer.expires_after(std::chrono::seconds(3));
            co_await timer.async_wait(asio::use_awaitable);
            fmt::print("monitor::mon:-\n");
            for (auto &i : instances_)
            {
                fmt::print(" - {}\n", i);
            }
        }
    }
};
std::unordered_map< std::string, std::size_t > monitor::next_index_;
std::set< instance >                           monitor::instances_;

namespace power_trade
{
struct proxy_display_order
{
};
struct proxy_order_book
{
};
}   // namespace power_trade

struct power_trade_connector
{
    using tcp_layer = tcp::socket;
    using ssl_layer = ssl::stream< tcp_layer >;
    using ws_layer  = websocket::stream< ssl_layer >;

    static constexpr char classname[] = "power_trade_connector";

    struct impl : std::enable_shared_from_this< impl >
    {
        impl(asio::any_io_executor exec, ssl::context &sslctx)
        : exec_(exec)
        , ssl_ctx_(sslctx)
        {
        }

        void
        start()
        {
            asio::co_spawn(
                get_executor(),
                run(shared_from_this()),
                asio::bind_cancellation_slot(stop_.slot(), asio::detached));
        }

        void
        stop()
        {
            asio::dispatch(get_executor(),
                           [self = shared_from_this()]
                           {
                               fmt::print("{}::{}\n", classname, "stop");
                               assert(!self->stopped_);
                               self->stopped_ = true;
                               self->stop_.emit(
                                   asio::cancellation_type::terminal);
                           });
        }

        asio::any_io_executor const &
        get_executor() const
        {
            return exec_;
        }

        void
        send(std::string s)
        {
            send_queue_.push_back(std::move(s));
            send_cv_.cancel();
        }

      private:
        asio::awaitable< void >
        run(std::shared_ptr< impl > self)
        {
            auto sentinel = monitor::record("power_trade_connector::run");
            try
            {
                using asio::use_awaitable;
                using namespace asio::experimental::awaitable_operators;

                auto ws = websocket::stream< ssl_layer >(self->get_executor(),
                                                         self->ssl_ctx_);

                auto &ssl_stream = ws.next_layer();
                auto &sock       = ssl_stream.next_layer();

                co_await async_connect(sock, co_await resolve(), use_awaitable);

                if (!SSL_set_tlsext_host_name(ssl_stream.native_handle(),
                                              host_.c_str()))
                    throw sys::system_error(ERR_get_error(),
                                            asio::error::get_ssl_category());

                co_await ssl_stream.async_handshake(ssl::stream_base::client,
                                                    use_awaitable);

                co_await ws.async_handshake(host_, path_, use_awaitable);

                error_.clear();
                send_cv_.cancel();
                co_await(send_loop(ws) || receive_loop(ws));
            }
            catch (std::exception &e)
            {
                fmt::print("{}: exception: {}\n", __func__, e.what());
            }
            fmt::print("{}: complete\n", __func__);
        }

        asio::awaitable< void >
        send_loop(ws_layer &ws)
        {
            auto sentinel = monitor::record("power_trade_connector::send_loop");
            using asio::redirect_error;
            using asio::use_awaitable;

            try
            {
                for (;;)
                {
                    while (!error_ && send_queue_.empty())
                    {
                        send_cv_.expires_at(
                            asio::steady_timer::time_point::max());
                        error_code ec;
                        co_await send_cv_.async_wait(
                            redirect_error(use_awaitable, ec));
                    }
                    if (error_)
                        break;
                    assert(!send_queue_.empty());
                    fmt::print(
                        "{}: sending {}\n", __func__, send_queue_.front());
                    co_await ws.async_write(asio::buffer(send_queue_.front()),
                                            use_awaitable);
                    send_queue_.pop_front();
                }
            }
            catch (std::exception &e)
            {
                fmt::print("{}: exception: {}\n", __func__, e.what());
            }
            fmt::print("{}: complete\n", __func__);
        }

        asio::awaitable< void >
        receive_loop(ws_layer &ws)
        {
            auto sentinel =
                monitor::record("power_trade_connector::receive_loop");
            using asio::redirect_error;
            using asio::use_awaitable;

            try
            {
                error_code         ec;
                beast::flat_buffer buf;
                buf.max_size(10'000'000);
                for (; !ec;)
                {
                    auto size = co_await ws.async_read(
                        buf, redirect_error(use_awaitable, ec));
                    if (ec)
                    {
                        fmt::print(
                            "{}: read error: {}\n", __func__, ec.message());
                        break;
                    }

                    auto data = std::string_view(
                        static_cast< char const * >(buf.data().data()),
                        buf.data().size());
                    fmt::print("{}: {} bytes\n", __func__, size);
                    fmt::print(
                        "{}: message: {}\n", __func__, util::truncate(data));
                    buf.consume(size);
                }

                if (!error_)
                    error_ = ec;
                send_cv_.cancel();
            }
            catch (std::exception &e)
            {
                fmt::print("{}: exception: {}\n", __func__, e.what());
            }
            fmt::print("{}: complete\n", __func__);
        }

        asio::awaitable< tcp::resolver::results_type >
        resolve()
        {
            auto sentinel = monitor::record("power_trade_connector::resolve");
            using asio::use_awaitable;

            auto resolver = tcp::resolver(co_await asio::this_coro::executor);
            co_return co_await resolver.async_resolve(
                host_, port_, use_awaitable);
        }

        asio::any_io_executor exec_;
        ssl::context         &ssl_ctx_;

        std::string host_ = "35.186.148.56";
        std::string port_ = "4321";
        std::string path_ = "/";

        asio::cancellation_signal stop_;
        bool                      stopped_ = false;
        error_code                error_   = asio::error::not_connected;
        std::deque< std::string > send_queue_;
        asio::steady_timer        send_cv_ { get_executor() };
    };

    power_trade_connector(asio::any_io_executor exec, ssl::context &ioc)
    : impl_(std::make_shared< impl >(exec, ioc))
    {
        impl_->start();
    }

    ~power_trade_connector()
    {
        if (impl_)
            impl_->stop();
    }

    power_trade_connector(power_trade_connector &&other)
    : impl_(std::move(other.impl_))
    {
    }

    power_trade_connector &
    operator=(power_trade_connector &&other)
    {
        if (impl_)
            impl_->stop();
        impl_ = std::move(other.impl_);
        return *this;
    }

    void
    send(std::string s)
    {
        asio::dispatch(impl_->get_executor(),
                       [s = std::move(s), impl = impl_]
                       { impl->send(std::move(s)); });
    }

    std::shared_ptr< impl > impl_;
};
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
    std::atexit([]()
                { ::tcsetattr(STDIN_FILENO, TCSANOW, &original_termios); });

    //    ::cfmakeraw(&my_termios);
    ::tcsetattr(STDIN_FILENO, TCSANOW, &my_termios);
}

asio::awaitable< void >
monitor_quit(asio::cancellation_signal &sig)
{
    auto sentinel = monitor::record("monitor_quit");
    using asio::redirect_error;
    using asio::use_awaitable;

    set_up_stdin();
    auto input = asio::posix::stream_descriptor(
        co_await asio::this_coro::executor, ::dup(STDIN_FILENO));

    for (;;)
    {
        char       buf;
        error_code ec;
        auto       size =
            co_await asio::async_read(input,
                                      asio::mutable_buffer(&buf, 1),
                                      redirect_error(use_awaitable, ec));
        if (ec)
            break;
        if (size && buf == 'q')
            break;
    }

    sig.emit(asio::cancellation_type::all);
    fmt::print("{}: done\n", __func__);
}

asio::awaitable< void >
check(ssl::context &sslctx)
{
    auto sentinel = monitor::record("check");
    auto con =
        power_trade_connector(co_await asio::this_coro::executor, sslctx);

    const static char req[] = R"json(
{
    "subscribe":
    {
        "market_id":"0",
        "symbol":"ETH-USD",
        "type":"snap_full_updates",
        "interval":"0",
        "user_tag":"another_anything_unique"
    }
})json";
    con.send(req);

    asio::cancellation_signal cancel_sig;
    co_await monitor_quit(cancel_sig);
}

asio::awaitable< void >
watch()
{
    using namespace asio::experimental::awaitable_operators;

    auto sigs = asio::signal_set(co_await asio::this_coro::executor, SIGINT);
    co_await(monitor::mon() || sigs.async_wait(asio::use_awaitable));
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