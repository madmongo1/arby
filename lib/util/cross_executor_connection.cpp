//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/router
//

#include "cross_executor_connection.hpp"

namespace arby
{
namespace util
{
    cross_executor_connection::cross_executor_connection(
        std::shared_ptr< has_executor_base > owner,
        boost::signals2::connection          connection)
    : owner_(std::move(owner))
    , connection_(std::move(connection))
    {
    }
    cross_executor_connection::cross_executor_connection(
        cross_executor_connection &&other)
    : owner_(std::move(other.owner_))
    , connection_(std::move(other.connection_))
    {
    }
    cross_executor_connection &
    cross_executor_connection::operator=(cross_executor_connection &&other)
    {
        disconnect();
        owner_      = std::exchange(other.owner_, {});
        connection_ = std::exchange(other.connection_, {});
        return *this;
    }
    void
    cross_executor_connection::disconnect() noexcept
    {
        if (owner_)
        {
            asio::dispatch(owner_->get_executor(),
                           [owner = owner_, connection = connection_]
                           { connection.disconnect(); });
        }
    }
}   // namespace util
}   // namespace arby