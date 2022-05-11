//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/arby
//

#include "configuration.hpp"

#include <fmt/ostream.h>
#include <spdlog/spdlog.h>
namespace arby
{
configuration::configuration(implementation_type impl)
: impl_(impl)
{
    using asio::bind_cancellation_slot;
    using asio::co_spawn;

    auto stop_handler = [impl = impl_](std::exception_ptr ep)
    {
        try
        {
            if (ep)
                std::rethrow_exception(ep);
            spdlog::debug("configuration - stop");
        }
        catch (std::exception &e)
        {
            spdlog::debug("configuration - stop *exception* : {}", e.what());
        }
    };

    auto token = bind_cancellation_slot(impl_->stop_slot(), stop_handler);
    co_spawn(get_executor(), configuration_impl::run(impl_), token);
}

auto
configuration::get_executor() const -> executor_type const &
{
    return impl_->get_executor();
}

}   // namespace arby