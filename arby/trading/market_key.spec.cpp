//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/arby
//

#include "trading/market_key.hpp"

#include <doctest/doctest.h>

using namespace arby;

TEST_SUITE("trading")
{
    TEST_CASE("market_key")
    {
        auto mk = trading::market_key();
        CHECK(to_string(mk) == "/");

        mk = trading::market_key(trading::spot_key("uSd/Jpy"));
        CHECK(to_string(mk) == "usd/jpy");
    }
}