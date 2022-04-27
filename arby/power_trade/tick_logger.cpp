#include "power_trade/tick_logger.hpp"

namespace arby::power_trade
{

tick_logger::tick_logger(std::shared_ptr< connector > connector, json::string symbol, fs::path path)
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

tick_logger::~tick_logger()
{
    if (impl_)
    {
        asio::dispatch(bind_executor(impl_->get_executor(), std::bind(&impl::stop, impl_)));
        impl_.reset();
    }
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
    persistent_connections_.push_back(
        connector_->get_implementation()->watch_messages("snapshot",
                                                         [weak, symbol = symbol_](std::shared_ptr< json::object const > payload)
                                                         {
                                                             auto s = json::value_to< std::string >(payload->at("symbol"));
                                                             if (s == symbol)
                                                                 if (auto self = weak.lock())
                                                                     self->logger_.send(std::move(s));
                                                         }));
    persistent_connections_.push_back(
        connector_->get_implementation()->watch_messages("order_added",
                                                         [weak, symbol = symbol_](std::shared_ptr< json::object const > payload)
                                                         {
                                                             auto s = json::value_to< std::string >(payload->at("symbol"));
                                                             if (s == symbol)
                                                                 if (auto self = weak.lock())
                                                                     self->logger_.send(std::move(s));
                                                         }));
    persistent_connections_.push_back(
        connector_->get_implementation()->watch_messages("order_deleted",
                                                         [weak, symbol = symbol_](std::shared_ptr< json::object const > payload)
                                                         {
                                                             auto s = json::value_to< std::string >(payload->at("symbol"));
                                                             if (s == symbol)
                                                                 if (auto self = weak.lock())
                                                                     self->logger_.send(std::move(s));
                                                         }));
    persistent_connections_.push_back(
        connector_->get_implementation()->watch_messages("order_executed",
                                                         [weak, symbol = symbol_](std::shared_ptr< json::object const > payload)
                                                         {
                                                             auto s = json::value_to< std::string >(payload->at("symbol"));
                                                             if (s == symbol)
                                                                 if (auto self = weak.lock())
                                                                     self->logger_.send(std::move(s));
                                                         }));
}

}   // namespace arby::power_trade
