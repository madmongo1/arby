//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/arby
//
#include "power_trade/native_symbol.hpp"

#include <doctest/doctest.h>

using namespace arby;

TEST_SUITE("power_trade")
{
    TEST_CASE("native_symbol")
    {
        auto mk     = trading::market_key(trading::spot_key("usd/jpy"));
        auto native = power_trade::native_symbol(mk);
        CHECK(native == "USD-JPY");
    }
}