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

#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

namespace arby::entity
{

using entity_key = std::string;

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

        std::unordered_map< entity_key, info > cache_;
        std::mutex                             m_;

        asio::awaitable< void >
        summary(std::string &body);

        int
        notify(entity_key const &key, std::weak_ptr< entity_base > const &weak)
        {
            auto  lock       = std::unique_lock(m_);
            auto &i          = cache_[key];
            auto  version    = i.next_version++;
            i.weak_[version] = weak;
            return version;
        }

        void
        remove(entity_key const &key, int version)
        {
            auto lock = std::unique_lock(m_);
            if (auto iinfo = cache_.find(key); iinfo != cache_.end())
            {
                iinfo->second.weak_.erase(version);
                if (iinfo->second.weak_.empty())
                    cache_.erase(iinfo);
            }
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
};

struct entity_base : std::enable_shared_from_this< entity_base >
{
    entity_base(asio::any_io_executor exec, entity_service entity_svc)
    : exec_(exec)
    , entity_svc_(entity_svc)
    {
    }

    void
    prepare()
    {
    }

    void
    start()
    {
    }

    virtual ~entity_base() = default;

    asio::any_io_executor const &
    get_executor() const
    {
        return exec_;
    }

    asio::any_io_executor exec_;
    entity_service        entity_svc_;
};

}   // namespace arby::entity

#endif   // ARBY_ENTITY_SERVICE_HPP