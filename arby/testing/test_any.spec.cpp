//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/arby
//
#include <doctest/doctest.h>
#include <any>

TEST_SUITE("testing")
{
    TEST_CASE("std::any")
    {
        auto v = std::any();
        auto& t = v.type();
        CHECK(t == typeid(void));
    }

}