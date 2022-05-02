//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/arby
//

#ifndef ARBY_ARBY_REACTIVE_ENTITY_MANAGEMENT_HPP
#define ARBY_ARBY_REACTIVE_ENTITY_MANAGEMENT_HPP

#include "entity/entity_service.hpp"
#include "entity/invariants.hpp"
#include "reactive/fix_connector.hpp"
#include "ssl_context.hpp"

namespace arby::reactive
{
std::shared_ptr< fix_connector >
prepare_fix_connector(entity::invariants invariants, entity::entity_service svc, std::any arguments)
{
    auto args = std::any_cast< fix_connector_args >(arguments);

    auto key = entity::entity_key("reactive::fix_connector");
    merge(key, args);
    key.lock();

    return svc.require< fix_connector >(key,
                                        [&]
                                        {
                                            auto executor =
                                                asio::make_strand(invariants.require< asio::io_context::executor_type >());
                                            auto &sslctx = invariants.require< ssl_context >().to_context();
                                            return std::make_shared< fix_connector >(executor, sslctx, std::move(args));
                                        });
}
}   // namespace arby::reactive

#endif   // ARBY_ARBY_REACTIVE_ENTITY_MANAGEMENT_HPP
