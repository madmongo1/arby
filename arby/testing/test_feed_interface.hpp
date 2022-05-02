//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/arby
//

#ifndef ARBY_ARBY_TESTING_TEST_FEED_INTERFACE_HPP
#define ARBY_ARBY_TESTING_TEST_FEED_INTERFACE_HPP

#include "entity/entity_base.hpp"

namespace arby
{
namespace testing
{

struct test_feed_impl_interface : entity::entity_base
{
};

struct test_feed_interface : entity::entity_handle< test_feed_impl_interface >
{
};

}   // namespace testing
}   // namespace arby

#endif   // ARBY_ARBY_TESTING_TEST_FEED_INTERFACE_HPP
