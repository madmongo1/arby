//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/arby
//

#ifndef ARBY_ARBY_CONFIGURATION_HPP
#define ARBY_ARBY_CONFIGURATION_HPP

#include "impl/configuration_impl.hpp"

#include <memory>

namespace arby
{

struct configuration
{
    using implementation_class = configuration_impl;
    using implementation_type  = std::shared_ptr< implementation_class >;
    using executor_type        = implementation_class::executor_type;

    configuration(implementation_type impl);

    auto
    get_executor() const -> executor_type const &;

  private:
    implementation_type impl_;
};

}   // namespace arby

#endif   // ARBY_ARBY_CONFIGURATION_HPP
