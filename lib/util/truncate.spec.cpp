//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/arby
//

#include "util/truncate.hpp"

#include <doctest/doctest.h>

using namespace arby;

TEST_SUITE("util")
{
    TEST_CASE("truncate")
    {
        std::string s = to_string(util::truncate("the cat sat on the mat", 14));
        CHECK(s == "the cat sat...");
    }
}