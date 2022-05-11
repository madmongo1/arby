//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/arby
//
#include "configuration_service.hpp"

#include <doctest/doctest.h>

using namespace arby;

TEST_SUITE("configuration")
{
    TEST_CASE("seek")
    {
        const char def[] = R"__json(
{
    "foo":"bar",
    "cat": {
        "name": "felix",
        "legs": {
            "left": "black",
            "right": "white"
        }
    }
}
)__json";
        auto       value = json::parse(def);

        SUBCASE("normalised")
        {
            auto p = seek(&value, "/");
            REQUIRE(p);
            CHECK(*p == value);

            p = seek(&value, "/foo");
            REQUIRE(p);
            CHECK(*p == "bar");

            p = seek(&value, "/cat/legs");
            REQUIRE(p);
            CHECK(*p == json::object({ { "left", "black" }, { "right", "white" } }));
            p = seek(p, "/left");
            CHECK(p != nullptr);
            CHECK(*p == "black");
        }

        SUBCASE("trailing slash")
        {
            auto p = seek(&value, "//");
            REQUIRE(p);
            CHECK(*p == value);

            p = seek(&value, "/foo/");
            REQUIRE(p);
            CHECK(*p == "bar");

            p = seek(&value, "/cat/legs/");
            REQUIRE(p);
            CHECK(*p == json::object({ { "left", "black" }, { "right", "white" } }));
            p = seek(p, "/left/");
            CHECK(p != nullptr);
            CHECK(*p == "black");
        }

        SUBCASE("trailing slash no leading slash")
        {
            auto p = seek(&value, "/");
            REQUIRE(p);
            CHECK(*p == value);

            p = seek(&value, "foo/");
            REQUIRE(p);
            CHECK(*p == "bar");

            p = seek(&value, "cat/legs/");
            REQUIRE(p);
            CHECK(*p == json::object({ { "left", "black" }, { "right", "white" } }));
            p = seek(p, "left/");
            CHECK(p != nullptr);
            CHECK(*p == "black");
        }

        SUBCASE("no leading slash")
        {
            auto p = seek(&value, "");
            REQUIRE(p);
            CHECK(*p == value);

            p = seek(&value, "foo");
            REQUIRE(p);
            CHECK(*p == "bar");

            p = seek(&value, "cat/legs");
            REQUIRE(p);
            CHECK(*p == json::object({ { "left", "black" }, { "right", "white" } }));
            p = seek(p, "left");
            CHECK(p != nullptr);
            CHECK(*p == "black");
        }
    }

    TEST_CASE("configuration object")
    {
        auto               svc = configuration_service();
        entity::invariants invariants;

        auto ioc = asio::io_context();

        invariants.add(ioc.get_executor());

        auto cfg = svc.require(invariants);

        REQUIRE(cfg != nullptr);
    }
}