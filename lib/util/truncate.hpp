//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/arby
//

#include "config/json.hpp"

#include <boost/utility/string_view_fwd.hpp>

#include <list>
#include <new>
#include <string>
#include <string_view>
#include <utility>

namespace arby::util
{

struct string_list
{
    struct node
    {
        node       *next = nullptr;
        std::string s;
    };

    struct handle
    {
        string_list *host;
        node        *pnode;

        explicit handle(string_list *host, node *pnode);

        handle(handle &&other);

        handle &
        operator=(handle &&other);

        ~handle();
    };

    handle
    pop();

    string_list();

    string_list(string_list const &) = delete;

    string_list &
    operator=(string_list const &) = delete;

    ~string_list();

    node *
    unlink();

    void
    link(node *pnode);

  private:
    node *first = nullptr;
};

struct truncate_op_base
{
    std::string_view
    transform(std::string_view sv, std::size_t limit) const;

  private:
    mutable string_list::handle     handle { nullptr, nullptr };
    thread_local static string_list buffers_;
};

struct truncate_op : truncate_op_base
{
    truncate_op(std::string_view sv, std::size_t limit);

    friend std::ostream &
    operator<<(std::ostream &os, truncate_op const &op);

    friend std::string
    to_string(truncate_op const &op);

  private:
    std::string_view sv_;
};

// truncate_op< std::string_view >
// truncate(std::string_view sv, std::size_t limit = 256);

truncate_op
truncate(std::string const &s, std::size_t limit = 256);

truncate_op
truncate(boost::string_view sv, std::size_t limit = 256);

struct json_truncate_op
{
    json_truncate_op(json::object const &o, std::size_t limit);

    friend std::ostream &
    operator<<(std::ostream &os, json_truncate_op const &op);

    json::object const &o;
    std::size_t         limit;
};

json_truncate_op
truncate(json::object const &j, std::size_t limit = 256);

}   // namespace arby::util