#include <boost/asio.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/filesystem.hpp>
#include <fmt/format.h>

#include <cstdio>
#include <iostream>

namespace beast     = boost::beast;
namespace sys       = boost::system;
namespace websocket = beast::websocket;
namespace asio      = boost::asio;
namespace ip        = asio::ip;
namespace ssl       = asio::ssl;
using tcp           = ip::tcp;
namespace fs        = boost::filesystem;

std::string progname;

struct power_trade_connector
{
    using tcp_layer = tcp::socket;
    using ssl_layer = ssl::stream< tcp_layer >;

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
                get_executor(), run(shared_from_this()), asio::detached);
        }

        asio::any_io_executor const &
        get_executor() const
        {
            return exec_;
        }

      private:
        asio::awaitable< void >
        run(std::shared_ptr< impl > self)
        {
            using asio::use_awaitable;

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

            for (;;)
            {
                beast::flat_buffer buf;
                auto size = co_await ws.async_read(buf, use_awaitable);
                auto seq  = buf.data();
                auto data = std::string_view(
                    reinterpret_cast< const char * >(seq.data()), seq.size());
                fmt::print("message: {}\n", data);
                buf.consume(size);
            }
        }

        asio::awaitable< tcp::resolver::results_type >
        resolve()
        {
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
    };

    power_trade_connector(asio::any_io_executor exec, ssl::context &ioc)
    : impl_(std::make_shared< impl >(exec, ioc))
    {
        impl_->start();
    }

    std::shared_ptr< impl > impl_;
};

asio::awaitable< void >
check(ssl::context &sslctx)
{
    auto con =
        power_trade_connector(co_await asio::this_coro::executor, sslctx);
}

int
main(int argc, char **argv)
{
    asio::io_context ioc;
    ssl::context     sslctx(ssl::context_base::tls_client);

    auto arg0 = fs::path(argv[0]);
    progname  = arg0.stem().string();

    try
    {
        asio::co_spawn(ioc, check(sslctx), asio::detached);
        ioc.run();
    }
    catch (const std::exception &e)
    {
        fmt::print(stderr, "{}: exception: {}\n", progname, e.what());
        return 127;
    }

    fmt::print("{}: done\n", progname);
}