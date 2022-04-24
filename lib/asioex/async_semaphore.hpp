//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/arby
//

#ifndef ASIOEX_ASYNC_SEMAPHORE_HPP
#define ASIOEX_ASYNC_SEMAPHORE_HPP

#include <asioex/detail/bilist_node.hpp>
#include <asioex/error_code.hpp>
#include <boost/asio/any_io_executor.hpp>
#include <boost/asio/async_result.hpp>
#include <boost/asio/detail/config.hpp>

namespace arby::asioex
{
namespace detail
{
    struct semaphore_wait_op;
}

struct async_semaphore_base
{
    inline async_semaphore_base(int initial_count);

    async_semaphore_base(async_semaphore_base const &) BOOST_ASIO_DELETED;

    async_semaphore_base &
    operator=(async_semaphore_base const &) BOOST_ASIO_DELETED;

    async_semaphore_base(async_semaphore_base &&) BOOST_ASIO_DELETED;

    async_semaphore_base &
    operator=(async_semaphore_base &&) BOOST_ASIO_DELETED;

    inline ~async_semaphore_base();

    /// @brief Attempt to immediately acquire the semaphore.
    /// @details This function attempts to acquire the semaphore without
    /// blocking or initiating an asynchronous operation.
    /// @returns true if the semaphore was acquired, false otherwise
    inline bool
    try_acquire();

    /// @brief Release the sempahore.
    /// @details This function immediately releases the semaphore. If there are
    /// pending async_acquire operations, then the least recent operation will
    /// commence completion.
    inline void
    release();

  protected:
    inline void
    add_waiter(detail::semaphore_wait_op *waiter);

    inline int
    decrement();

    BOOST_ASIO_NODISCARD inline int
    count() const noexcept;

  private:
    detail::bilist_node waiters_;
    int                 count_;
};

template < class Executor = asio::any_io_executor >
struct basic_async_semaphore : async_semaphore_base
{
    /// @brief The type of the default executor.
    using executor_type = Executor;

    /// Rebinds the socket type to another executor.
    template < typename Executor1 >
    struct rebind_executor
    {
        /// The socket type when rebound to the specified executor.
        typedef basic_async_semaphore< Executor1 > other;
    };

    /// @brief Construct an async_sempaphore
    /// @param exec is the default executor associated with the asyn_sempahore
    /// @param initial_count is the initial value of the internal counter.
    /// @pre initial_count >= 0
    /// @pre initial_count <= MAX_INT
    ///
    basic_async_semaphore(executor_type exec, int initial_count = 1);

    /// @brief return the default executor.
    executor_type const &
    get_executor() const;

    /// @brief Initiate an asynchronous acquire of the semaphore
    /// @details Multiple asynchronous acquire operations may be in progress at
    /// the same time. However, the caller must ensure that this function is not
    /// invoked from two threads simultaneously. When the semaphore's internal
    /// count is above zero, async acquire operations will complete in strict
    /// FIFO order. If the semaphore object is destoyed while an async_acquire
    /// is outstanding, the operation's completion handler will be invoked with
    /// the error_code set to error::operation_aborted. If the async_acquire
    /// operation is cancelled before completion, the completion handler will be
    /// invoked with the error_code set to error::operation_aborted. Successful
    /// acquisition of the semaphore is signalled to the caller when the
    /// completion handler is invoked with no error.
    /// @tparam CompletionHandler represents a completion token or handler which
    /// is invokable with the signature `void(error_code)`
    /// @param token is a completion token or handler matching the signature
    /// void(error_code)
    /// @note The completion handler will be invoked as if by `post` to the
    /// handler's associated executor. If no executor is associated with the
    /// completion handler, the handler will be invoked as if by `post` to the
    /// async_semaphore's associated default executor.
    template < BOOST_ASIO_COMPLETION_TOKEN_FOR(void(error_code))
                   CompletionHandler BOOST_ASIO_DEFAULT_COMPLETION_TOKEN_TYPE(executor_type) >
    BOOST_ASIO_INITFN_RESULT_TYPE(CompletionHandler, void(error_code))
    async_acquire(CompletionHandler &&token BOOST_ASIO_DEFAULT_COMPLETION_TOKEN(executor_type));

  private:
    executor_type exec_;
};

using async_semaphore = basic_async_semaphore<>;

}   // namespace arby::asioex

#endif

#include <asioex/impl/async_semaphore_base.hpp>
#include <asioex/impl/basic_async_semaphore.hpp>