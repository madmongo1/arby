//
// Copyright (c) 2022 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/arby
//

#ifndef ARBY_LIB_ASIOEX_SCOPED_INTERRUPT_HPP
#define ARBY_LIB_ASIOEX_SCOPED_INTERRUPT_HPP

#include "asioex/config.hpp"

namespace arby::asioex
{
/// @brief Send a terminal signal to an asio::cancellation_signal.
///
/// @note reduces typing for common operations
/// @param sig is a reference to the signal to emit
inline void
terminate(asio::cancellation_signal &sig)
{
    sig.emit(asio::cancellation_type::terminal);
}

/// @brief Build a scoped cancellation handler for a cancellation_slot.
///
/// The main purpose of this class is to remove clutter and typing when all you want to do is make a sub-operation cancellable with
/// some degree of customisation. The provided interrupt handler is nullary - all filtering is done by the boilerplate in this
/// class. For most cancellations, which are terminal, this is sufficient. If you need anything more complex, write it by hand.
/// This class takes care of automatically clearing the interrupt handler prior to handling. This is also a common annoyance when
/// writing such a handler.
struct scoped_interrupt
{
    /// @brief Construct the scoped interrupt
    /// @tparam InterruptHandler A nullary function type.
    /// @param slot is the which will have the handler applied
    /// @param interrupt_handler Is the nullary function object. This will be called at most once, in the thread which emits the
    /// associated cnacellation_signal.
    /// @param type_allowed is a bitmask of event types that will be allowed to trigger the handler. Defaults to all.
    template < class InterruptHandler >
    scoped_interrupt(asio::cancellation_slot slot,
                     InterruptHandler      &&interrupt_handler,
                     asio::cancellation_type type_allowed = asio::cancellation_type::all)
    : slot_(std::move(slot))
    {
        slot_.assign(
            [next = std::forward< InterruptHandler >(interrupt_handler), type_allowed](asio::cancellation_type type)
            {
                if (static_cast< int >(type) & static_cast< int >(type_allowed))
                    next();
            });
    }

    /// @brief Construct the scoped interrupt
    /// @tparam InterruptHandler A nullary function type.
    /// @param signal is a reference to the associated signal.
    /// @param interrupt_handler Is the nullary function object. This will be called at most once, in the thread which emits the
    /// associated cancellation_signal.
    /// @param type_allowed is a bitmask of event types that will be allowed to trigger the handler. Defaults to all.
    /// @note this is a convenience constructor , provided so we don't have to waste more typing to get the slot.
    template < class InterruptHandler >
    scoped_interrupt(asio::cancellation_signal &signal,
                     InterruptHandler         &&interrupt_handler,
                     asio::cancellation_type    type = asio::cancellation_type::all)
    : scoped_interrupt(signal.slot(), std::forward< InterruptHandler >(interrupt_handler), type)
    {
    }

    ~scoped_interrupt() { reset(); }

    void
    reset() noexcept
    {
        slot_.clear();
    }

    scoped_interrupt &
    operator=(scoped_interrupt const &)        = delete;
    scoped_interrupt(scoped_interrupt const &) = delete;

  private:
    asio::cancellation_slot slot_;
};

}   // namespace arby::asioex

#endif   // ARBY_LIB_ASIOEX_SCOPED_INTERRUPT_HPP
