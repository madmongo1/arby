//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/arby
//

#include "util/table.hpp"

#include <fmt/chrono.h>
#include <fmt/ostream.h>

#include <algorithm>
#include <chrono>
#include <iterator>
#include <vector>

namespace arby::util
{
void
table::set(std::size_t row, std::size_t col, std::string s)
{
    data_[row][col] = std::move(s);
    rows            = std::max(rows, row);
}

std::vector< std::size_t >
table::calc_widths() const
{
    auto result = std::vector< std::size_t >();

    for (auto &[r, cdata] : data_)
        for (auto &[c, str] : cdata)
        {
            if (c >= result.size())
                result.resize(c + 1, 0);

            result[c] = std::max(result[c], str.size());
        }

    return result;
};

std::string
table::pad(std::string s, std::size_t field)
{
    auto pre  = (field - s.size()) / 2;
    s         = std::string(pre, ' ') + s;
    auto post = field - s.size();
    s += std::string(post, ' ');
    return s;
}

std::ostream &
operator<<(std::ostream &os, table const &tab)
{
    const auto widths = tab.calc_widths();

    const char *nl = "";
    for (auto &[r, cdata] : tab.data_)
    {
        os << nl;
        auto sep = std::string_view("");
        for (auto col = std::size_t(0); col < widths.size(); ++col)
        {
            auto idata = cdata.find(col);
            if (idata == cdata.end())
            {
                os << sep << tab.pad("", widths[col]);
            }
            else
            {
                os << sep << tab.pad(idata->second, widths[col]);
            }
            sep = " | ";
        }
        nl = "\n";
    }
    return os;
}

}   // namespace arby::util
