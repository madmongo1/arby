//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/arby
//

#include <cstdio>
#include <boost/asio.hpp>
#include <boost/asio/experimental/channel.hpp>

namespace net = boost::asio;
using net::ip::tcp;
using channel_type = net::experimental::channel<void(boost::system::error_code, std::size_t)>;

// To cause a crash run
// $ nc localhost 55555

net::awaitable<void> foo(channel_type& ch)
{
  try {
    co_await ch.async_receive(net::use_awaitable);
  } catch (std::exception& e) {
    std::printf("echo Exception: %s\n", e.what());
  }
}

net::awaitable<void> listener(channel_type& ch)
{
  try {
    auto executor = co_await net::this_coro::executor;
    tcp::acceptor acceptor(executor, {tcp::v4(), 55555});
    co_await acceptor.async_accept(net::use_awaitable);
    ch.cancel(); // <===== Causes the crash.
  } catch (std::exception& e) {
    std::printf("Error1: %s\n", e.what());
  }
}

int main()
{
  try {
    boost::asio::io_context ioc(1);
    channel_type ch{ioc.get_executor()};
    net::co_spawn(ioc, listener(ch), net::detached);
    net::co_spawn(ioc, foo(ch), net::detached);
    ioc.run();
  } catch (std::exception& e) {
    std::printf("Error2: %s\n", e.what());
  }
}