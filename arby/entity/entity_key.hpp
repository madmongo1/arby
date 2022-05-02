//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/arby
//

#ifndef ARBY_ENTITY_KEY_HPP
#define ARBY_ENTITY_KEY_HPP

#include <boost/functional/hash.hpp>

#include <array>
#include <cassert>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>

namespace arby
{
namespace entity
{

struct key_values
{
    using values_map = std::unordered_map< std::string, std::string >;

    static key_values
    empty();

    key_values(std::shared_ptr< std::unordered_map< std::string, std::string > const > values);

    std::string const &
    at(std::string const &key) const;

    std::string const *
    query(std::string const &key) const;

    void
    set(std::string const &key, std::string value)
    {
        if (!values_.unique())
            values_ = std::make_shared< values_map >(*values_);
        (*values_)[key] = std::move(value);
    }

    bool
    operator==(key_values const &other) const;

  private:
    std::shared_ptr< std::unordered_map< std::string, std::string > > values_;
};

struct entity_key
{
    using map_type = std::map< std::string, std::string >;

    explicit entity_key(map_type values = map_type());

    friend std::size_t
    hash_value(entity_key const &key);

    void
    lock();

    void
    set(std::string const &key, std::string value)
    {
        copy_check()->values[key] = std::move(value);
    }

    void
    merge(entity_key const &other);

    std::string const &
    sha1_digest() const;

    bool
    operator==(entity_key const &other) const;

    bool
    operator<(entity_key const &other) const;

    friend std::ostream &
    operator<<(std::ostream &os, entity_key const &key);

    bool
    locked() const
    {
        return impl_->locked;
    }

    struct impl
    {
        map_type    values;
        std::size_t cpphash = 0;
        std::string sha1hash;
        bool        locked = false;

        impl
        clone() const
        {
            return impl(values);
        }

        impl(map_type m = map_type())
        : values(std::move(m))
        , cpphash(0)
        , sha1hash()
        , locked(false)
        {
        }

        impl(impl const &other) = delete;
        impl(impl &&other)      = default;

        impl &
        operator=(impl const &other) = delete;
    };

  private:
    std::shared_ptr< impl > const &
    copy_check()
    {
        if (!impl_.unique())
            impl_ = std::make_shared< impl >(impl_->clone());
        return impl_;
    }

    std::shared_ptr< impl > impl_;
};

struct entity_moniker
{
    std::string classname;
    entity_key  key;
};

inline std::size_t
hash_value(entity_moniker const &arg)
{
    std::size_t seed = 0;
    boost::hash_combine(seed, arg.classname);
    boost::hash_combine(seed, arg.key);
    return seed;
}

inline auto
as_tie(entity_moniker const &arg)
{
    return std::tie(arg.classname, arg.key);
}

inline bool
operator==(entity_moniker const &l, entity_moniker const &r)
{
    return as_tie(l) == as_tie(r);
}

inline bool
operator<(entity_moniker const &l, entity_moniker const &r)
{
    return as_tie(l) < as_tie(r);
}

}   // namespace entity
}   // namespace arby

namespace std
{
template <>
struct hash< arby::entity::entity_key >
{
    std::size_t
    operator()(arby::entity::entity_key const &arg) const
    {
        return hash_value(arg);
    }
};

template <>
struct hash< arby::entity::entity_moniker >
{
    std::size_t
    operator()(arby::entity::entity_key const &arg) const
    {
        return hash_value(arg);
    }
};

}   // namespace std
#endif   // ARBY_ENTITY_KEY_HPP
