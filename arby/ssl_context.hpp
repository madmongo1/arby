//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/arby
//

#ifndef ARBY_ARBY_SSL_CONTEXT_HPP
#define ARBY_ARBY_SSL_CONTEXT_HPP

#include "config/asio.hpp"

namespace arby
{

struct ssl_context
{
    ssl_context(ssl::context &impl)
    : impl_(&impl)
    {
    }

    operator ssl::context &() const { return *impl_; }

    ssl::context& to_context() const { return *impl_; }

  private:
    ssl::context *impl_;
};

}   // namespace arby

#endif   // ARBY_ARBY_SSL_CONTEXT_HPP
