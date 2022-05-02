//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/arby
//

#ifndef ARBY_ARBY_ENTITY_INVARIANTS_HPP
#define ARBY_ARBY_ENTITY_INVARIANTS_HPP

#include <any>
#include <cassert>
#include <stdexcept>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>

namespace arby
{
namespace entity
{

struct invariants
{
    template < class T >
    void
    add(T &&x)
    {
        using type  = std::remove_const_t< std::remove_reference_t< T > >;
        auto [i, b] = elements_.emplace(typeid(T), std::make_any< type >(std::forward< T >(x)));
        assert(b);
    }

    template < class T >
    T const &
    require() const
    {
        auto &elem = elements_.at(typeid(T));
        auto  ptr  = std::any_cast< T >(&elem);
        if (!ptr) throw std::invalid_argument("invariants: wrong type");
        return *ptr;
    }

    template < class T >
    T const *
    query() const
    {
        T const *result = nullptr;
        if (auto i = elements_.find(typeid(T)); i != elements_.end())
            result = std::any_cast< T >(&i->second);
        return result;
    }

  private:
    std::unordered_map< std::type_index, std::any > elements_;
};

}   // namespace entity
}   // namespace arby

#endif   // ARBY_ARBY_ENTITY_INVARIANTS_HPP
