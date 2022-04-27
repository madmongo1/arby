//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/arby
//

#ifndef ARBY_LIB_UTIL_DATA_LOG_HPP
#define ARBY_LIB_UTIL_DATA_LOG_HPP

#include "asioex/helpers.hpp"
#include "config/asio.hpp"
#include "config/filesystem.hpp"

namespace arby
{
namespace logging
{

/// @brief Logs data to a file asynchronously
struct data_log
{
    struct impl : std::enable_shared_from_this< impl >
    {
        impl(asio::any_io_executor exec, fs::path const &path);

        asio::awaitable< void >
        run();

        void
        stop();

        void
        send(std::string s);

        asio::any_io_executor const &
        get_executor() const;

      private:
        static void
        do_send(std::shared_ptr< impl > self, error_code ec, std::string s);

      private:
        asio::any_io_executor                                        exec_;
        asio::posix::stream_descriptor                               stream_;
        asio::experimental::channel< void(error_code, std::string) > channel_;
    };

    data_log(asio::any_io_executor exec, boost::filesystem::path const &path);

    void
    send(std::string s);

    ~data_log();

  private:
    std::shared_ptr< impl > impl_;
};

}   // namespace logging
}   // namespace arby

#endif   // ARBY_LIB_UTIL_DATA_LOG_HPP
