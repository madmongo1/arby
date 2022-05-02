//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/arby
//
#include "entity/invariants.hpp"
#include <doctest/doctest.h>
#include <string>

using namespace arby;

TEST_SUITE("invariants")
{
    TEST_CASE("general")
    {
        std::string s = "hi";

        entity::invariants i;
        i.add(s);

        auto p = i.query<std::string>();
        CHECK(p != nullptr);
        CHECK(*p == s);
        std::string const* sc = nullptr;
        CHECK_NOTHROW(sc = &i.require<std::string>());
        REQUIRE(sc != nullptr);
        CHECK(*sc == s);
    }
}