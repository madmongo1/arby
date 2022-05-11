//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/arby
//

#ifndef ARBY_ARBY_POWER_TRADE_DETAIL_CONNECTOR_IMPL_HPP
#define ARBY_ARBY_POWER_TRADE_DETAIL_CONNECTOR_IMPL_HPP

#include "config/json.hpp"
#include "config/websocket.hpp"
#include "power_trade/connection_state.hpp"
#include "trading/types.hpp"
#include "util/cross_executor_connection.hpp"

#include <boost/asio/awaitable.hpp>
#include <boost/functional/hash.hpp>
#include <boost/signals2.hpp>
#include <boost/unordered_map.hpp>

#include <deque>
#include <functional>
#include <iosfwd>
#include <tuple>

namespace arby::power_trade::detail
{

struct field_matcher
{
    using field     = std::tuple< json::string, json::value >;
    using field_set = std::vector< field >;

    field_matcher(field_set required)
    : required_(sorted(std::move(required)))
    , hash_(compute_hash(required_))
    {
    }

    bool
    matches(json::object const &r) const
    {
        for (auto &[k, lv] : required_)
        {
            if (auto rv = r.if_contains(k); !rv)
                return false;
            else if (*rv != lv)
                return false;
        }
        return true;
    }

  private:
    friend std::size_t
    hash_value(field_matcher const &fm)
    {
        return fm.hash_;
    }

    friend bool
    operator==(field_matcher const &l, field_matcher const &r)
    {
        return l.hash_ == r.hash_ && l.required_ == r.required_;
    }

    static field_set
    sorted(field_set in)
    {
        auto pred = [](field const &l, field const &r) { return get< 0 >(l) < get< 0 >(r); };

        std::sort(in.begin(), in.end(), pred);
        return in;
    }

    static std::size_t
    compute_hash(field_set const &fs)
    {
        constexpr auto shash = std::hash< std::size_t >();
        constexpr auto khash = std::hash< json::string >();
        constexpr auto vhash = std::hash< json::value >();

        auto seed = shash(fs.size());
        for (auto &[k, v] : fs)
        {
            boost::hash_combine(seed, khash(k));
            boost::hash_combine(seed, vhash(v));
        }
        return seed;
    }

    field_set   required_;
    std::size_t hash_;
};

struct message_matcher
{
    message_matcher(json::string type, field_matcher::field_set fields = {})
    : type_(std::move(type))
    , fields_(std::move(fields))
    {
    }

    json::object const *
    if_match(json::object const &o) const
    {
        json::object const *result = nullptr;

        if (o.size() != 1)
            return nullptr;

        auto &[k, v] = *o.begin();
        if (k != type_)
            return nullptr;

        result = v.if_object();
        if (!fields_.matches(*result))
            result = nullptr;

        return result;
    }

    json::string  type_;
    field_matcher fields_;
};

struct connector_impl
: util::has_executor_base
, std::enable_shared_from_this< connector_impl >
{
    using executor_type               = asio::any_io_executor;
    using tcp_layer                   = tcp::socket;
    using tls_layer                   = asio::ssl::stream< tcp_layer >;
    using ws_stream                   = websocket::stream< tls_layer >;
    static constexpr char classname[] = "connector_impl";

    class inbound_message
    {
        trading::timestamp_type timestamp_;
        beast::flat_buffer      buffer_;
        json::value             value_;
        json::string            type_;
        json::object const     *object_;

      public:
        inbound_message(std::size_t capacity)
        : buffer_(capacity)
        {
        }

        json::object const& object() const
        {
            return *object_;
        }

        beast::flat_buffer &
        prepare()
        {
            timestamp_ = std::chrono::system_clock::now();
            object_    = nullptr;
            type_.clear();
            value_ = nullptr;
            buffer_.clear();
            return buffer_;
        }

        std::string_view
        view() const
        {
            auto d = buffer_.data();
            return std::string_view(static_cast< const char * >(d.data()), d.size());
        }

        json::string const &
        type() const
        {
            return type_;
        }

        trading::timestamp_type
        timestamp() const
        {
            return timestamp_;
        }

        void
        commit()
        {
            timestamp_ = std::chrono::system_clock::now();
            auto v     = view();
            auto v1    = json::string_view(v.begin(), v.end());
            value_     = json::parse(v);
            if (auto outer = value_.if_object(); outer && !outer->empty())
            {
                auto &[k, v] = *outer->begin();
                type_.assign(k.begin(), k.end());
                object_ = v.if_object();
            }
        }
    };

    // Note that the signal type is not thread-safe. You must only interact with
    // the signals while on the same executor and thread as the connector
    using message_signal =
        boost::signals2::signal_type< void(std::shared_ptr< inbound_message const >),
                                      boost::signals2::keywords::mutex_type< boost::signals2::dummy_mutex > >::type;

    using message_slot          = message_signal::slot_type;
    using message_extended_slot = message_signal::extended_slot_type;

    using connection_state_signal =
        boost::signals2::signal_type< void(connection_state),
                                      boost::signals2::keywords::mutex_type< boost::signals2::dummy_mutex > >::type;

    using connection_state_slot          = connection_state_signal::slot_type;
    using connection_state_extended_slot = connection_state_signal::extended_slot_type;

    /// @brief Constructor
    /// @param exec The internal executor to use for IO
    /// @param sslctx ssl context
    connector_impl(executor_type exec, ssl::context &sslctx);

    asio::awaitable< void >
    connect();

    void
    start();

    void
    stop();

    void
    send(std::string s);

    void
    interrupt();

    boost::signals2::connection
    watch_messages(json::string message_type, message_slot slot);

    boost::signals2::connection
    watch_connection_state(connection_state &current, connection_state_slot slot);

  private:
    asio::awaitable< void >
    run(std::shared_ptr< connector_impl > self);

    asio::awaitable< void >
    run_connection();

    asio::awaitable< void >
    send_loop(ws_stream &ws);

    asio::awaitable< void >
    receive_loop(ws_stream &ws);

    asio::awaitable< void >
    interruptible_connect(ws_stream &stream);

    /// @brief Attempt to handle an incoming message
    /// @return boolean value indicating that the message was dispatched to at
    /// least one listener
    bool
    handle_message(std::shared_ptr< inbound_message const > pmessage);

    void
    set_connection_state(error_code ec);

  private:
    // dependencies
    ssl::context &ssl_ctx_;

    // parameters
//    std::string const host_ = "35.186.148.56", port_ = "4321", path_ = "/";
    std::string const host_ = "proxy.prod.powertrade.dev", port_="4321", path_="/";

    struct sv_comp_equ
    : boost::hash< boost::string_view >
    , std::equal_to<>
    {
        using is_transparent = void;
        using boost::hash< boost::string_view >::operator();
        using std::equal_to<>::                  operator();
        bool
        operator()(json::string const &s) const
        {
            return (*this)(boost::string_view(s.data(), s.size()));
        }
    };

    connection_state_signal connstate_signal_;
    connection_state        connstate_ { asio::error::not_connected };

    using signal_map = boost::unordered_map< json::string, message_signal, sv_comp_equ, sv_comp_equ >;
    signal_map signal_map_;

    // state
    std::deque< std::string > send_queue_;
    asio::steady_timer        send_cv_ { get_executor() };
    asio::cancellation_signal interrupt_connection_;
    asio::cancellation_signal stop_;
    bool                      stopped_ = false;
};
}   // namespace arby::power_trade::detail

#endif   // ARBY_ARBY_POWER_TRADE_DETAIL_CONNECTOR_IMPL_HPP
