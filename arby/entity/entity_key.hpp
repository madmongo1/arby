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
    key_values(std::shared_ptr< std::unordered_map< std::string, std::string > const > values);

    std::string const &
    at(std::string const &key) const;

    bool
    operator==(key_values const &other) const;

  private:
    std::shared_ptr< std::unordered_map< std::string, std::string > const > values_;
};

struct mutable_key_values
{
    std::unique_ptr< std::unordered_map< std::string, std::string > > values_;

    key_values
    lock() &&;
};

struct entity_key
{
    entity_key(key_values values);

    friend std::size_t
    hash_value(entity_key const &key);

    void
    lock();

    std::string const &
    use(std::string const &key);

    void
    merge(entity_key const &other);

    bool
    operator==(entity_key const &other)
    {
        assert(locked_);
        assert(other.locked_);

        if (cpphash_ != other.cpphash_)
            return false;
        if (used_ != other.used_)
            return false;
        for (auto &&k : used_)
            if (values_.at(k) != other.values_.at(k))
                return false;

        return true;
    }

    friend std::ostream &
    operator<<(std::ostream &os, entity_key const &key);

  private:
    key_values                      values_;
    std::set< std::string >         used_;
    std::size_t                     cpphash_;
    std::array< unsigned char, 20 > sha1hash_;
    bool                            locked_ = false;
};

struct unlocked_entity_key
{
    unlocked_entity_key(key_values values)
    : values_(std::move(values))
    , used_()
    {
    }

    key_values              values_;
    std::set< std::string > used_;
};

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
}   // namespace std
#endif   // ARBY_ENTITY_KEY_HPP
