//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/arby
//

#include "entity_detail_app.hpp"

#include <boost/algorithm/string.hpp>

namespace arby
{
namespace web
{
namespace
{
unsigned int
to_value(char c)
{
    switch (c)
    {
    case '0':
        return 0;
    case '1':
        return 1;
    case '2':
        return 2;
    case '3':
        return 3;
    case '4':
        return 4;
    case '5':
        return 5;
    case '6':
        return 6;
    case '7':
        return 7;
    case '8':
        return 8;
    case '9':
        return 9;
    case 'a':
        return 10;
    case 'b':
        return 11;
    case 'c':
        return 12;
    case 'd':
        return 13;
    case 'e':
        return 14;
    case 'f':
        return 15;
    case 'A':
        return 10;
    case 'B':
        return 11;
    case 'C':
        return 12;
    case 'D':
        return 13;
    case 'E':
        return 14;
    case 'F':
        return 15;
    }
    throw std::invalid_argument("invalid hex digit");
}

std::array< unsigned char, 20 >
to_sha1_digest(std::string const &s)
{
    if (s.size() != 40)
        throw std::invalid_argument("to_sha1_digest: invalid arg");

    std::array< unsigned char, 20 > result;

    auto dest  = result.begin();
    auto first = s.begin();
    while (first != s.end())
    {
        auto h  = to_value(*first++);
        auto l  = to_value(*first++);
        *dest++ = static_cast< unsigned char >(h * 16 + l);
    }
    return result;
}
}   // namespace

asio::awaitable< bool >
entity_detail_app::operator()(tcp::socket &stream, http::request< http::string_body > &request, std::cmatch &match)
{
    auto candidates = std::vector< std::shared_ptr< entity::entity_base > >();
    auto sha_key    = match[1].str();
    boost::algorithm::to_lower(sha_key);

    if (match[2].matched)
    {
        if (auto p = entity_service_.hash_lookup(sha_key, ::atoi(match[2].str().c_str())); p)
            candidates.push_back(p);
    }
    else
        candidates = entity_service_.hash_lookup(sha_key);

    co_return false;
}
}   // namespace web
}   // namespace arby