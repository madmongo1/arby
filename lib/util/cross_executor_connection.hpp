//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/arby
//

#ifndef ARBY_LIB_UTIL_CROSS_EXECUTOR_CONNECTION_HPP
#define ARBY_LIB_UTIL_CROSS_EXECUTOR_CONNECTION_HPP

#include "config/asio.hpp"

#include <boost/signals2/connection.hpp>

namespace arby
{
namespace util
{

    struct has_executor_base
    {
        has_executor_base(asio::any_io_executor exec)
        : exec_(std::move(exec))
        {
        }

        virtual asio::any_io_executor const &
        get_executor() const
        {
            return exec_;
        }

      private:
        asio::any_io_executor exec_;
    };

    struct cross_executor_connection
    {
        cross_executor_connection(std::shared_ptr< has_executor_base > owner = {}, boost::signals2::connection connection = {});

        cross_executor_connection(cross_executor_connection &&other);

        cross_executor_connection &
        operator=(cross_executor_connection &&other);

        ~cross_executor_connection() { disconnect(); }

        void
        disconnect() noexcept;

      private:
        std::shared_ptr< has_executor_base > owner_;
        boost::signals2::connection          connection_;
    };

}   // namespace util
}   // namespace arby

#endif   // ARBY_LIB_UTIL_CROSS_EXECUTOR_CONNECTION_HPP
