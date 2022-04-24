//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/arby
//

#ifndef ASIOEX_DETAIL_BILIST_NODE_HPP
#define ASIOEX_DETAIL_BILIST_NODE_HPP

#include <boost/asio/detail/config.hpp>

namespace arby::asioex
{
namespace detail
{
    struct bilist_node
    {
        inline bilist_node();

        bilist_node(bilist_node const &) BOOST_ASIO_DELETED;

        bilist_node &
        operator=(bilist_node const &) BOOST_ASIO_DELETED;

        inline ~bilist_node();

        inline void
        unlink();

        inline void
        link_before(bilist_node *next);

        bilist_node *next_;
        bilist_node *prev_;
    };

    bilist_node::bilist_node()
    : next_(this)
    , prev_(this)
    {
    }

    bilist_node::~bilist_node()
    {
    }

    inline void
    bilist_node::unlink()
    {
        auto p   = prev_;
        auto n   = next_;
        n->prev_ = p;
        p->next_ = n;
    }

    void
    bilist_node::link_before(bilist_node *next)
    {
        next_        = next;
        prev_        = next->prev_;
        prev_->next_ = this;
        next->prev_  = this;
    }

}   // namespace detail
}   // namespace arby::asioex

#endif