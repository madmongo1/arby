//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/arby
//
//

#include "logging/data_log.hpp"

#include "config/asio.hpp"
#include "config/filesystem.hpp"

#include <boost/filesystem.hpp>
#include <doctest/doctest.h>

using namespace arby;

TEST_SUITE("logging")
{
    TEST_CASE("data log")
    {
        asio::io_context ioc;
        {
            auto              path = fs::temp_directory_path() / "test.txt";
            logging::data_log logger(ioc.get_executor(), path);

            logger.send("The");
            logger.send("cat");
            logger.send("sat");
        }
        ioc.run();
    }
}
