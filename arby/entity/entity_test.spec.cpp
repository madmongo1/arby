//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/arby
//

#include <doctest/doctest.h>

#include <cassert>
#include <memory>

template < class T >
struct enable_shared_from_other
{
    void
    init_shared_from_other(std::shared_ptr< T > self)
    {
        assert(weak_.expired());
        weak_ = self;
    }

    std::weak_ptr< T >
    weak_from_this()
    {
        return weak_;
    }

    std::shared_ptr< T >
    shared_from_this()
    {
        return weak_;
    }

  private:
    std::weak_ptr< T > weak_;
};

TEST_SUITE("entity_test")
{
    TEST_CASE("shared")
    {
        struct component2 : enable_shared_from_other< component2 >
        {
            bool
            has_parent()
            {
                auto w = weak_from_this();
                if (w.lock())
                    return true;
                else
                    return false;
            }
        };
        struct outer_component : std::enable_shared_from_this< outer_component >
        {
            void
            start()
            {
                c2.init_shared_from_other(std::shared_ptr< component2 >(shared_from_this(), &c2));
            }

            component2 c2;
        };

        auto outer = std::make_shared< outer_component >();
        outer->start();
        auto c2 = std::shared_ptr< component2 >(outer, &outer->c2);
        CHECK(c2->has_parent());
    }
}