//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/arby
//

#ifndef ARBY_ENTITY_ENTITY_SERVICE_HPP
#define ARBY_ENTITY_ENTITY_SERVICE_HPP

#include "config/asio.hpp"
#include "entity/entity_key.hpp"

#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

namespace arby::entity
{

struct entity_base;

struct entity_service
{
    struct impl
    {
        struct entry
        {
            std::weak_ptr< entity_base > weak_impl;
            int                          version;
        };

        struct info
        {
            std::map< int, std::weak_ptr< entity_base > > weak_;
            int                                           next_version = 0;
        };

        std::unordered_map< std::string, info > cache_;
        std::mutex                              m_;

        asio::awaitable< void >
        summary(std::string &body);

        int
        notify(entity_key const &key, std::weak_ptr< entity_base > const &weak)
        {
            auto  digest     = key.sha1_digest();
            auto  lock       = std::unique_lock(m_);
            auto &i          = cache_[digest];
            auto  version    = i.next_version++;
            i.weak_[version] = weak;
            return version;
        }

        void
        remove(entity_key const &key, int version)
        {
            auto lock = std::unique_lock(m_);
            if (auto iinfo = cache_.find(key.sha1_digest()); iinfo != cache_.end())
            {
                iinfo->second.weak_.erase(version);
                if (iinfo->second.weak_.empty())
                    cache_.erase(iinfo);
            }
        }

        template < class F >
        void
        enumerate(F &&f)
        {
            auto lock       = std::unique_lock(m_);
            auto cache_copy = this->cache_;
            lock.unlock();
            for (auto &[k, info] : cache_copy)
                for (auto &[version, weak] : info.weak_)
                    if (auto p = weak.lock())
                        f(p);
        }

        std::shared_ptr< entity_base >
        hash_lookup(std::string const &sha1hash, int version)
        {
            std::shared_ptr< entity_base > result;

            auto lock = std::unique_lock(m_);

            if (auto iinfo = cache_.find(sha1hash); iinfo != cache_.end())
            {
                if (auto iversion = iinfo->second.weak_.find(version); iversion != iinfo->second.weak_.end())
                    result = iversion->second.lock();
            }

            return result;
        }

        std::vector< std::shared_ptr< entity_base > >
        hash_lookup(std::string const &sha1hash)
        {
            std::vector< std::shared_ptr< entity_base > > result;

            auto lock = std::unique_lock(m_);

            if (auto iinfo = cache_.find(sha1hash); iinfo != cache_.end())
            {
                for (auto &[version, weak] : iinfo->second.weak_)
                    if (auto p = weak.lock())
                        result.push_back(p);
            }

            return result;
        }
    };

    using implementation_type = impl *;

    static implementation_type
    get_implementation()
    {
        static impl i;
        return &i;
    }

    asio::awaitable< void >
    summary(std::string &body)
    {
        return get_implementation()->summary(body);
    }

    int
    notify(entity_key const &key, std::weak_ptr< entity_base > const &weak)
    {
        return get_implementation()->notify(key, weak);
    }
    void
    remove(entity_key const &key, int version)
    {
        return get_implementation()->remove(key, version);
    }

    std::shared_ptr< entity_base >
    hash_lookup(std::string const &sha1hash, int version)
    {
        return get_implementation()->hash_lookup(sha1hash, version);
    }

    std::vector< std::shared_ptr< entity_base > >
    hash_lookup(std::string const &sha1hash)
    {
        return get_implementation()->hash_lookup(sha1hash);
    }

    template < class F >
    void
    enumerate(F &&f)
    {
        get_implementation()->enumerate(f);
    }
};

}   // namespace arby::entity

#endif   // ARBY_ENTITY_SERVICE_HPP
