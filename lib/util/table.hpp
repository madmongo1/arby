//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/arby
//

#ifndef ARBY_ARBY_UTIL_TABLE_HPP
#define ARBY_ARBY_UTIL_TABLE_HPP

#include "config/json.hpp"

namespace arby::util
{
struct table
{
    std::map< std::size_t, std::map< std::size_t, std::string > > data_;
    std::size_t                                                   rows = 0;

    void
    set(std::size_t row, std::size_t col, std::string s);

    std::vector< std::size_t >
    calc_widths() const;

    static std::string
    pad(std::string s, std::size_t field);

    friend std::ostream &
    operator<<(std::ostream &os, table const &tab);
};

}   // namespace arby::util

#endif   // ARBY_ARBY_UTIL_TABLE_HPP
