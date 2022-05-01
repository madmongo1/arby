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
key_values::empty()
{
    static values_map const data;
    static auto const       values = std::shared_ptr< values_map const >(&data, []< class T >(T *) noexcept {});
    return key_values(values);
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

namespace
{
void
stringify(std::string &target, unsigned char const (&digest)[SHA_DIGEST_LENGTH])
{
    target.resize(SHA_DIGEST_LENGTH * 2);

    auto to_hex = [](unsigned char nibble)
    {
        static const char digits[] = "0123456789abcdef";
        return digits[nibble];
    };
    auto out = target.data();
    for (unsigned char byte : digest)
    {
        *out++ = to_hex(byte >> 4);
        *out++ = to_hex(byte & 0xf);
    }
}
}   // namespace

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
    SHA1_Final(digest, &shactx);
    stringify(sha1hash_, digest);

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
        os << key.sha1hash_;
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

bool
entity_key::operator==(const entity_key &other) const
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

bool
entity_key::operator<(entity_key const &other) const
{
    assert(locked_);
    assert(other.locked_);

    if (!(used_ < other.used_))
        return false;

    for (auto &&k : used_)
        if (!(values_.at(k) < other.values_.at(k)))
            return false;

    return true;
}

std::string const &
entity_key::sha1_digest() const
{
    assert(locked_);
    return sha1hash_;
}

}   // namespace entity
}   // namespace arby