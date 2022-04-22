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

template < class T >
struct truncate_op;

struct truncate_op_base
{
    std::string_view
    transform(std::string_view sv) const;

  private:
    static constexpr std::size_t    max_ = 132;
    mutable string_list::handle     handle { nullptr, nullptr };
    thread_local static string_list buffers_;
};

template <>
struct truncate_op< std::string_view > : truncate_op_base
{
    truncate_op(std::string_view sv);

    friend std::ostream &
    operator<<(std::ostream &os, truncate_op const &op);

  private:
    std::string_view sv_;
};

truncate_op< std::string_view >
truncate(std::string_view sv);

}   // namespace util