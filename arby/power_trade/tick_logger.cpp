#include "power_trade/tick_logger.hpp"

namespace arby::power_trade
{

tick_logger::tick_logger(std::shared_ptr< connector > connector, std::string symbol, fs::path path)
: impl_(std::make_shared< impl >(connector, std::move(symbol), std::move(path)))
{
    asio::dispatch(bind_executor(impl_->get_executor(), std::bind(&impl::start, impl_)));
}

tick_logger::tick_logger(tick_logger &&other) = default;

tick_logger &
tick_logger::operator=(tick_logger &&other)
{
    destroy();
    impl_ = std::move(other.impl_);
    return *this;
}

void
tick_logger::destroy()
{
    if (impl_)
    {
        auto i = std::move(impl_);
        asio::dispatch(bind_executor(i->get_executor(), std::bind(&impl::stop, i)));
    }
}

tick_logger::~tick_logger()
{
    destroy();
}

tick_logger::impl::impl(std::shared_ptr< connector > connector, std::string symbol, fs::path path)
: connector_(std::move(connector))
, symbol_(std::move(symbol))
, logger_(get_executor(), std::move(path))
{
}

void
tick_logger::impl::start()
{
    assert(asioex::on_correct_thread(get_executor()));
    auto weak = weak_from_this();

    auto watch = [&](json::string type)
    {
        persistent_connections_.emplace_back(connector_->get_implementation()->watch_messages(
            std::move(type),
            [weak, symbol = symbol_](std::shared_ptr< connector::inbound_message const > payload)
            {
                auto to_string = [](std::string_view sv) { return std::string(sv.begin(), sv.end()); };

                auto s = json::value_to< std::string >(payload->object().at("symbol"));
                if (s == symbol)
                    if (auto self = weak.lock())
                        self->logger_.send(to_string(payload->view()));
            }));
    };

    watch("snapshot");
    watch("order_added");
    watch("order_deleted");
    watch("order_executed");
}

void
tick_logger::impl::stop()
{
    persistent_connections_.clear();
}

}   // namespace arby::power_trade
