//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/arby
//

#ifndef ARBY_WEB_ENTITY_DETAIL_APP_HPP
#define ARBY_WEB_ENTITY_DETAIL_APP_HPP

#include "config/http.hpp"
#include "web/http_app.hpp"
#include "entity/entity_service.hpp"

namespace arby
{
namespace web
{

struct entity_detail_app : http_app_base
{
    virtual asio::awaitable< bool >
    operator()(tcp::socket &stream, http::request< http::string_body > &request, std::cmatch& match) override;

    entity::entity_service entity_service_ = entity::entity_service();
};

}   // namespace web
}   // namespace arby

#endif   // ARBY_WEB_ENTITY_DETAIL_APP_HPP
