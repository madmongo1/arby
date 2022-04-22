#include "util/truncate.hpp"

#include <cassert>

namespace arby::util
{

string_list::handle::handle(string_list *host, node *pnode)
: host(host)
, pnode(pnode)
{
}

string_list::handle::handle(handle &&other)
: host(other.host)
, pnode(std::exchange(other.pnode, nullptr))
{
}

string_list::handle &
string_list::handle::operator=(handle &&other)
{
    if (pnode)
        host->link(pnode);
    host  = other.host;
    pnode = std::exchange(other.pnode, nullptr);
    return *this;
}

string_list::handle::~handle()
{
    if (pnode)
        host->link(pnode);
}

void
string_list::link(node *pnode)
{
    assert(pnode->next == nullptr);
    pnode->next = first;
    first       = pnode;
}

string_list::node *
string_list::unlink()
{
    auto result = first;
    if (result)
    {
        first        = result->next;
        result->next = nullptr;
    }
    return result;
}

string_list::string_list()
: first(nullptr)
{
}

string_list::~string_list()
{
    while (auto current = unlink())
        delete current;
}

string_list::handle
string_list::pop()
{
    auto pnode = unlink();
    if (!pnode)
        pnode = new node;
    return handle(this, pnode);
}

string_list thread_local truncate_op_base::buffers_;

std::string_view
truncate_op_base::transform(std::string_view sv) const
{
    if (!handle.pnode && sv.size() > max_)
    {
        handle = buffers_.pop();
        auto s = &(handle.pnode->s);
        s->assign(sv.data(), sv.data() + max_ - 3);
        s->append(3, '.');
    }
    if (handle.pnode)
    {
        auto s = &(handle.pnode->s);
        sv     = std::string_view(s->data(), s->size());
    }
    return sv;
}

truncate_op< std::string_view >::truncate_op(std::string_view sv)
: truncate_op_base()
, sv_(transform(sv))
{
}

std::ostream &
operator<<(std::ostream &os, truncate_op< std::string_view > const &op)
{
    os << op.sv_;
    return os;
}

truncate_op< std::string_view >
truncate(std::string_view sv)
{
    return truncate_op< std::string_view >(sv);
}

}   // namespace util