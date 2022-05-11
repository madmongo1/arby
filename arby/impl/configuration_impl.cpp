//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/arby
//

#include "configuration_impl.hpp"

#include "asioex/helpers.hpp"
#include "asioex/scoped_interrupt.hpp"

namespace arby
{
json::value const *
seek(json::value const *root, std::string_view path)
{
    if (!root)
        return root;

    while (!path.empty() && path[0] == '/')
        path = path.substr(1);

    if (path.empty())
        return root;

    auto next    = path.find('/');
    auto segment = path.substr(0, next);
    if (next == std::string_view::npos)
        path = std::string_view(segment.data() + segment.size(), 0);
    else
        path = path.substr(next);

    auto o = root->if_object();
    if (!o)
        return nullptr;
    root = o->if_contains(segment);
    return seek(root, path);
}

json::value const &
seek(json::value const &root, std::string_view path)
{
    auto p = seek(&root, path);
    if (!p)
    {
        static const auto v = json::value();
        return v;
    }

    return *p;
}

std::string_view
configuration_impl::classname() const
{
    return "ConfigurationImpl";
}

asio::awaitable< void >
configuration_impl::run(std::shared_ptr< configuration_impl > self)
{
    co_await asioex::spin();
}

auto
configuration_impl::get_executor() const -> executor_type const &
{
    return exec_;
}

auto
configuration_impl::sub_snapshot(snapshot_type const &snap, std::string_view path) -> snapshot_type
{
    switch (snap.status)
    {
    case configuration_status::good:
        if (auto p = seek(snap.value.get(), path); p)
            return snapshot_type { .status = configuration_status::good,
                                   .value  = std::shared_ptr< json::value const >(snap.value, p) };
        else
            return snapshot_type { .status = configuration_status::error, .value = empty_value() };
        break;
    case configuration_status::not_ready:
    case configuration_status::error:
        return snapshot_type { .status = snap.status, .value = empty_value() };
    }
}

}   // namespace arby