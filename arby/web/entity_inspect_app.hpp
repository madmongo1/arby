//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/arby
//

#ifndef ARBY_ENTITY_INSPECT_APP_HPP
#define ARBY_ENTITY_INSPECT_APP_HPP

#include "web/http_app.hpp"
#include "entity/entity_service.hpp"

namespace arby
{
namespace web
{

struct entity_inspect_app : http_app
{

    entity::entity_service entity_service_;
};

}   // namespace web
}   // namespace arby

#endif   // ARBY_ENTITY_INSPECT_APP_HPP
