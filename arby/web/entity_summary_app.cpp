//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/arby
//

#include "entity_summary_app.hpp"

#include "entity/entity_base.hpp"

#include <fmt/format.h>
#include <fmt/ostream.h>

namespace arby
{
namespace web
{
struct async_condition_variable
{
    using executor_type = asio::any_io_executor;

    async_condition_variable(executor_type exec)
    : exec_(exec)
    {
    }

    executor_type const &
    get_executor() const
    {
        return exec_;
    }

    asio::awaitable< void >
    wait()
    {
        error_code ec;
        co_await timer_.async_wait(asio::redirect_error(asio::use_awaitable, ec));
        if (asio::cancellation_state cstate = co_await asio::this_coro::cancellation_state;
            cstate.cancelled() == asio::cancellation_type::none)
            ec.clear();
        else
            ec = asio::error::operation_aborted;
        if (ec)
            throw system_error(ec);
    }

    void
    notify_one()
    {
        timer_.cancel_one();
    }

  private:
    executor_type      exec_;
    asio::steady_timer timer_ { get_executor(), asio::steady_timer ::time_point ::max() };
};

asio::awaitable< bool >
entity_summary_app::operator()(tcp::socket &stream, http::request< http::string_body > &request, std::cmatch &)
{
    auto this_exec = co_await asio::this_coro::executor;

    auto summaries   = std::map< void *, std::string >();
    auto cv          = async_condition_variable(this_exec);
    int  outstanding = 0;

    auto final = [&](void* p, std::string result)
    {
        summaries.emplace(p, result);
        --outstanding;
        cv.notify_one();
    };

    auto enumerator = [this_exec, final, &summaries](std::shared_ptr< entity::entity_base > const &p)
    {
        if (this_exec == p->get_executor())
        {
            summaries.emplace(p.get(), p->summary());
        }
        else
        {
            asio::dispatch(asio::bind_executor(p->get_executor(),
                                               [p, final, this_exec]() mutable
                                               {
                                                   auto result = p->summary();
                                                   asio::dispatch(asio::bind_executor(
                                                       this_exec, std::bind(final, p.get(), result)));
                                               }));
        }
    };

    entity_service_.enumerate(enumerator);

    while (outstanding)
        co_await cv.wait();

    using asio::use_awaitable;

    auto response = http::response< http::string_body >();
    response.result(http::status::ok);
    response.version(request.version());
    response.keep_alive(request.keep_alive());
    auto &body = response.body();
    body       = fmt::format("{} entities:\r\n", summaries.size());
    for (auto &[k, buffer] : summaries)
        body += fmt::format("{} - {}\r\n", k, buffer);
    response.prepare_payload();
    co_await http::async_write(stream, response, use_awaitable);
    co_return !response.need_eof();
}

}   // namespace web
}   // namespace arby