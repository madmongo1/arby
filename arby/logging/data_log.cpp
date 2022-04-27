//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/arby
//

#include "logging/data_log.hpp"

namespace arby
{
namespace logging
{
void
data_log::impl::do_send(std::shared_ptr< impl > self, error_code ec, std::string s)
{
    assert(asioex::on_correct_thread(self->get_executor()));
    if (!self->channel_.try_send(ec, std::move(s)))
        self->channel_.async_send(ec, std::move(s), [self](error_code) {});
}

asio::awaitable< void >
data_log::impl::run()
{
    error_code  ec;
    std::string current_line;
    auto        handle_immediate = [&](error_code ec1, std::string s1)
    {
        ec           = ec1;
        current_line = std::move(s1);
    };

    for (;;)
    {
        if (!channel_.try_receive(handle_immediate))
            current_line = co_await channel_.async_receive(asio::redirect_error(asio::use_awaitable, ec));
        if (ec)
            break;

        current_line += '\n';
        co_await asio::async_write(stream_, asio::buffer(current_line), asio::redirect_error(asio::use_awaitable, ec));
        if (ec)
        {
            // spdlog::
            break;
        }
    }
    channel_.close();
}

asio::any_io_executor const &
data_log::impl::get_executor() const
{
    return exec_;
}

void
data_log::impl::send(std::string s)
{
    dispatch(bind_executor(get_executor(), std::bind(impl::do_send, shared_from_this(), error_code(), std::move(s))));
}
void
data_log::impl::stop()
{
    dispatch(bind_executor(get_executor(), std::bind(impl::do_send, shared_from_this(), asio::error::eof, std::string())));
}
data_log::impl::impl(asio::any_io_executor exec, const fs::path &path)
: exec_(exec)
, stream_(exec, ::open(path.c_str(), O_CREAT | O_WRONLY | O_APPEND | O_TRUNC, S_IRUSR | S_IWUSR))
, channel_(exec, 1024)
{
}

data_log::data_log(asio::any_io_executor exec, const boost::filesystem::path &path)
: impl_(std::make_shared< impl >(exec, path))
{
    co_spawn(impl_->get_executor(), impl_->run(), [self = impl_](std::exception_ptr ep) {});
}

data_log::~data_log()
{
    if (impl_)
        dispatch(bind_executor(impl_->get_executor(), std::bind(&impl::stop, impl_)));
}
void
data_log::send(std::string s)
{
    impl_->send(std::move(s));
}
}   // namespace logging
}   // namespace arby