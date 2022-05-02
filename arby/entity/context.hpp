//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/arby
//

#ifndef ARBY_ENTITY_CONTEXT_HPP
#define ARBY_ENTITY_CONTEXT_HPP

#include "entity/entity_base.hpp"

#include <concepts>

namespace arby::entity
{
struct context
{
    template < std::derived_from< entity::entity_handle_base > Interface >
    std::shared_ptr< Interface >
    locate();
};

template < std::derived_from< entity::entity_handle_base > Interface >
std::shared_ptr< Interface >
context::locate()
{
}

}   // namespace arby::entity
#endif   // ARBY_ENTITY_CONTEXT_HPP