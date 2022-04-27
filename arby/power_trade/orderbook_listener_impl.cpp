//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/arby
//

#include "orderbook_listener_impl.hpp"

#include "asioex/helpers.hpp"
#include "util/monitor.hpp"

#include <fmt/ostream.h>
#include <spdlog/spdlog.h>
#include <util/truncate.hpp>

#include <functional>

namespace arby
{
namespace power_trade
{

using namespace std::literals;

void
orderbook_listener_impl::on_connection_state(connection_state cstate)
{
    connstate_ = cstate;
    spdlog::info("{}::{} {}", this->source_id_, __func__, cstate);
    if (connstate_.down())
    {
        connection_condition_.reset(trading::feed_state::stale);
        connection_condition_.errors.push_back("connection dropped");
        book_condition_.merge(trading::feed_state::stale);
    }
    else
    {
        connection_condition_.reset(trading::feed_state::good);
        auto req = json::value({ { "subscribe",
                                   { { "market_id", "0" },
                                     { "symbol", native_symbol(symbol_) },
                                     { "type", "snap_full_updates" },
                                     { "interval", "0" },
                                     { "user_tag", my_subscribe_id_ } } } });
        connector_->send(json::serialize(req));
    }
    update();
}

std::shared_ptr< orderbook_listener_impl >
orderbook_listener_impl::create(asio::any_io_executor exec, std::shared_ptr< connector > connector, trading::market_key symbol)
{
    auto impl = std::make_shared< orderbook_listener_impl >(std::move(exec), std::move(connector), std::move(symbol));
    impl->start();
    return impl;
}

orderbook_listener_impl::orderbook_listener_impl(asio::any_io_executor        exec,
                                                 std::shared_ptr< connector > connector,
                                                 trading::market_key          symbol)
: util::has_executor_base(std::move(exec))
, connector_(std::move(connector))
, symbol_(std::move(symbol))
{
    connection_condition_.state = trading::feed_state::not_ready;
    connection_condition_.errors.push_back("connection state unknown");
    book_condition_.state = trading::feed_state::not_ready;
    book_condition_.errors.push_back("no book");

    snapshot_ = std::make_shared< orderbook_snapshot >();
    snapshot_->condition.merge(connection_condition_);
    snapshot_->condition.merge(book_condition_);
    snapshot_->source = source_id_;
}

void
orderbook_listener_impl::start()
{
    asio::co_spawn(get_executor(), run(shared_from_this()), asio::detached);
}

namespace
{
template < class F, class Filter >
struct filter_message
{
    filter_message(F next, Filter filters)
    : next(std::move(next))
    , filters(std::move(filters))
    {
    }

    void
    operator()(std::shared_ptr< json::object const > payload) const
    {
        for (auto &[k, v] : filters)
        {
            auto pv = payload->if_contains(k);
            if (!pv)
                return;
            auto ps = pv->if_string();
            if (!ps)
                return;
            if (*ps != v)
                return;
        }
        next(payload);
    }
    Filter filters;
    F      next;
};

}   // namespace

asio::awaitable< void >
orderbook_listener_impl::run(std::shared_ptr< orderbook_listener_impl > self)
{
    auto [conn, connstate] = co_await connector_->watch_connection_state(connector::connection_state_slot(
        [weak = weak_from_this()](connection_state state)
        {
            if (auto self = weak.lock())
                asio::post(asio::bind_executor(self->get_executor(), [self, state] { self->on_connection_state(state); }));
        }));

    cmd_response_conn_ = co_await connector_->watch_messages(
        "command_response", std::bind(&orderbook_listener_impl::_handle_command_response, weak_from_this(), std::placeholders::_1));

    tick_con_[0] = co_await connector_->watch_messages(
        "snapshot",
        filter_message(
            std::bind(&orderbook_listener_impl::_handle_tick, weak_from_this(), std::placeholders::_1, tick_code::snapshot),
            make_message_filter()));

    tick_con_[1] = co_await connector_->watch_messages(
        "order_added",
        filter_message(std::bind(&orderbook_listener_impl::_handle_tick, weak_from_this(), std::placeholders::_1, tick_code::add),
                       make_message_filter()));

    tick_con_[2] = co_await connector_->watch_messages(
        "order_deleted",
        filter_message(
            std::bind(&orderbook_listener_impl::_handle_tick, weak_from_this(), std::placeholders::_1, tick_code::remove),
            make_message_filter()));

    tick_con_[3] = co_await connector_->watch_messages(
        "order_executed",
        filter_message(
            std::bind(&orderbook_listener_impl::_handle_tick, weak_from_this(), std::placeholders::_1, tick_code::execute),
            make_message_filter()));

    connection_state_conn_ = std::move(conn);
    on_connection_state(connstate);
}

void
orderbook_listener_impl::_handle_command_response(std::weak_ptr< orderbook_listener_impl > weak,
                                                  std::shared_ptr< json::object const >    payload)
{
    spdlog::trace("{}::{}({})", classname, __func__, *payload);
    auto self = weak.lock();
    if (!self)
        return;

    try
    {
        if (payload->at("user_tag").as_string() != self->my_subscribe_id_)
            return;
        asio::dispatch(asio::bind_executor(self->get_executor(), [self, payload] { self->on_command_response(payload); }));
    }
    catch (std::exception &e)
    {
        fmt::print("{}[{}]::\"command_response\" - exception: {}\n", classname, self->symbol_, e.what());
    }
}

json::string
orderbook_listener_impl::build_subscribe_id() const
{
    auto result = "order_book:"s + to_string(symbol_);
    return json::string(result);
}

void
orderbook_listener_impl::on_command_response(std::shared_ptr< json::object const > payload)
{
    spdlog::trace("{}::{}({})", classname, __func__, util::truncate(*payload));

    if (auto &code = payload->at("error_code"); code == "0")
    {
        book_condition_.reset(trading::not_ready);
        book_condition_.errors.push_back("subscribed");
    }
    else
    {
        book_condition_.reset(trading::error);
        book_condition_.errors.push_back("failed to subscribe");
    }
    update();
}

void
orderbook_listener_impl::_handle_tick(std::weak_ptr< orderbook_listener_impl > weak,
                                      std::shared_ptr< const json::object >    payload,
                                      tick_code                                code)
{
    auto self = weak.lock();
    if (!self)
    {
        spdlog::trace("{}::{} weak pointer expired, dropping tick: {} {}", classname, __func__, code, util::truncate(*payload));
        return;
    }

    try
    {
        asio::dispatch(asio::bind_executor(
            self->get_executor(), [self, tick = tick_record { code, payload }]() mutable { self->on_tick(std::move(tick)); }));
    }
    catch (std::exception &e)
    {
        spdlog::error("{}[{}]::{} exception: {}", classname, self->symbol_, __func__, e.what());
    }
}
void
orderbook_listener_impl::on_tick(tick_record tick)
{
    auto deleter = [weak = weak_from_this()](orderbook_snapshot *psnap) noexcept
    {
        auto up = std::unique_ptr< orderbook_snapshot >(psnap);
        if (auto self = weak.lock())
        {
            asio::dispatch(asio::bind_executor(self->get_executor(),
                                               [self, up = std::move(up)]() mutable
                                               { self->snapshot_service_.deallocate_snapshot(std::move(up)); }));
        }
    };
    snapshot_ = std::shared_ptr< orderbook_snapshot >(snapshot_service_.process_tick(std::move(tick)).release(), deleter);

    book_condition_.reset(trading::good);
    update();
}

auto
orderbook_listener_impl::subscribe(slot_type slot) -> std::tuple< sigs::connection, snapshot_type >
{
    assert(asioex::on_correct_thread(get_executor()));
    return std::make_tuple(signal_.connect(std::move(slot)), snapshot_);
}
std::string
orderbook_listener_impl::build_source_id() const
{
    return fmt::format("{}[{}]", classname, symbol_);
}

void
orderbook_listener_impl::update()
{
    snapshot_->condition.reset(trading::good);
    snapshot_->condition.merge(connection_condition_);
    snapshot_->condition.merge(book_condition_);
    signal_(snapshot_);
}

}   // namespace power_trade
}   // namespace arby