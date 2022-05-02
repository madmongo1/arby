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

entity_key::entity_key(std::string classname, map_type values)
: impl_(std::make_shared< impl >(std::move(classname), std::move(values)))
{
}

std::size_t
hash_value(const entity_key &key)
{
    assert(key.locked());
    return key.impl_->cpphash;
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

    assert(locked());
    assert(impl_->cpphash == 0);

    SHA_CTX shactx;
    SHA1_Init(&shactx);

    impl_->cpphash = boost::hash_value(impl_->values);

    for (auto &[k, v] : impl_->values)
    {
        SHA1_Update(&shactx, k.data(), k.size());
        SHA1_Update(&shactx, v.data(), v.size());
    }

    unsigned char digest[SHA_DIGEST_LENGTH];
    SHA1_Final(digest, &shactx);
    stringify(impl_->sha1hash, digest);

    impl_->locked = true;
}

void
entity_key::merge(const entity_key &other)
{
    assert(!locked());
    auto &target = copy_check()->values;
    for (auto &[k, v] : other.impl_->values)
        // note: merged values will not overwrite existing values
        target.emplace(k, v);
}

std::ostream &
operator<<(std::ostream &os, const entity_key &key)
{
    if (key.locked())
    {
        fmt::print(os, "{}:{}", key.impl_->classname, key.impl_->sha1hash);
    }
    else
    {
        fmt::print(os, "[{}-unlocked", key.impl_->classname);
        for (auto &[k, v] : key.impl_->values)
            fmt::print(" [{} {}]", k, v);
        fmt::print(os, "]");
    }

    return os;
}

bool
entity_key::operator==(const entity_key &other) const
{
    assert(locked());
    assert(other.locked());

    if (impl_.get() == other.impl_.get())
        return true;

    return std::tie(impl_->classname, impl_->cpphash, impl_->sha1hash, impl_->values) ==
           std::tie(other.impl_->classname, other.impl_->cpphash, other.impl_->sha1hash, other.impl_->values);
}

bool
entity_key::operator<(entity_key const &other) const
{
    assert(locked());
    assert(other.locked());

    if (impl_.get() == other.impl_.get())
        return false;

    return std::tie(impl_->classname, impl_->cpphash, impl_->sha1hash, impl_->values) <
           std::tie(other.impl_->classname, other.impl_->cpphash, other.impl_->sha1hash, other.impl_->values);
}

std::string const &
entity_key::sha1_digest() const
{
    assert(locked());
    return impl_->sha1hash;
}

}   // namespace entity
}   // namespace arby