//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/arby
//

#include "entity_key.hpp"

#include <fmt/format.h>
#include <fmt/ostream.h>
#include <openssl/sha.h>

namespace arby
{
namespace entity
{
key_values::key_values(std::shared_ptr< const std::unordered_map< std::string, std::string > > values)
: values_(std::move(values))
{
}
std::string const &
key_values::at(const std::string &key) const
{
    return values_->at(key);
}
bool
key_values::operator==(const key_values &other) const
{
    return values_.get() == other.values_.get();
}
key_values
mutable_key_values::lock() &&
{
    return key_values(std::shared_ptr< std::unordered_map< std::string, std::string > const >(std::move(values_)));
}
entity_key::entity_key(key_values values)
: values_(std::move(values))
, used_()
{
}
std::size_t
hash_value(const entity_key &key)
{
    assert(key.locked_);
    return key.cpphash_;
}
void
entity_key::lock()
{
    using boost::hash_combine;

    assert(!locked_);
    assert(cpphash_ == 0);

    SHA_CTX shactx;
    SHA1_Init(&shactx);

    for (auto &uk : used_)
    {
        hash_combine(cpphash_, uk);
        SHA1_Update(&shactx, uk.data(), uk.size());

        auto &v = values_.at(uk);
        hash_combine(cpphash_, values_.at(uk));
        SHA1_Update(&shactx, v.data(), v.size());
    }

    unsigned char digest[SHA_DIGEST_LENGTH];
    SHA1_Final(sha1hash_.data(), &shactx);

    locked_ = true;
}
std::string const &
entity_key::use(const std::string &key)
{
    assert(!locked_);
    std::string const &value = values_.at(key);
    used_.insert(key);
    return value;
}
void
entity_key::merge(const entity_key &other)
{
    assert(!locked_);
    assert(other.locked_);
    assert(values_ == other.values_);
    for (auto &&k : other.used_)
        used_.insert(k);
}

std::ostream &
operator<<(std::ostream &os, const entity_key &key)
{
    if (key.locked_)
    {
        auto to_hex = [](unsigned char nibble)
        {
            static const char digits[] = "0123456789abcdef";
            return digits[nibble];
        };
        for (unsigned char byte : key.sha1hash_)
        {
            os << to_hex(byte >> 4);
            os << to_hex(byte & 0xf);
        }
    }
    else
    {
        fmt::print(os, "[unlocked");
        for (auto &&k : key.used_)
            fmt::print(" [{} {}]", k, key.values_.at(k));
        fmt::print(os, "]");
    }

    return os;
}
}   // namespace entity
}   // namespace arby