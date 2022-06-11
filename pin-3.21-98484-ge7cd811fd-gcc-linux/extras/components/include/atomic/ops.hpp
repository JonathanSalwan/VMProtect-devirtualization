/*
 * Copyright (C) 2005-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

// <COMPONENT>: atomic
// <FILE-TYPE>: component public header

#ifndef ATOMIC_OPS_HPP
#define ATOMIC_OPS_HPP

#include "atomic/exponential-backoff.hpp"
#include "atomic/ops-enum.hpp"
#include "atomic/private/ops-common-impl.hpp"
#include "atomic/private/backoff-impl.hpp"

#if defined(HOST_IA32)
#include "atomic/private/ia32/ops-impl.hpp"
#elif defined(HOST_IA32E)
#include "atomic/private/intel64/ops-impl.hpp"
#endif

namespace ATOMIC
{
/*! @brief  Low-level atomic memory operations.
 *
 * A collection of low-level atomic memory operations that can be performed on fundemental data types
 * (integers, pointers, etc.)  All of the operations are static template methods, so they automatically
 * adapt to whatever data type you use.
 *
 * @par Example:
 *                                                                                          \code
 *  #include "atomic/ops.hpp"
 *
 *  int Count;
 *
 *  void Foo()
 *  {
 *      ATOMIC::OPS::Increment(&Count, 1);
 *  }
 *                                                                                          \endcode
 */
class /*<UTILITY>*/ OPS
{
  public:
    /*!
     * Perform an atomic compare-and-swap.
     *
     *  @param[in] location     Pointer to the location to modify.
     *  @param[in] oldVal       The \a location is only changed if its value is
     *                           equal to \a oldVal.
     *  @param[in] newVal       The new value to write to \a location.
     *  @param[in] barrier      Tells the memory ordering semantics of this operation.
     *
     * @return  The value of \a location prior to the operation.  The \a newVal was stored
     *           only if the return value is equal to \a oldVal.
     */
    template< typename T > static T CompareAndSwap(volatile T* location, T oldVal, T newVal, BARRIER_CS barrier = BARRIER_CS_NONE)
    {
        OPS_IMPL::CompareAndSwap< sizeof(T) >(static_cast< volatile void* >(location), static_cast< const void* >(&oldVal),
                                              static_cast< void* >(&newVal), barrier);
        return newVal;
    }

    /*!
     * This is exactly the same as CompareAndSwap() except the return value is a
     * boolean.
     *
     *  @param[in] location     Pointer to the location to modify.
     *  @param[in] oldVal       The \a location is only changed if its value is
     *                           equal to \a oldVal.
     *  @param[in] newVal       The new value to write to \a location.
     *  @param[in] barrier      Tells the memory ordering semantics of this operation.
     *
     * @return  TRUE if \a newVal was stored to \a location.
     */
    template< typename T >
    static bool CompareAndDidSwap(volatile T* location, T oldVal, T newVal, BARRIER_CS barrier = BARRIER_CS_NONE)
    {
        OPS_IMPL::CompareAndSwap< sizeof(T) >(static_cast< volatile void* >(location), static_cast< const void* >(&oldVal),
                                              static_cast< void* >(&newVal), barrier);
        return (newVal == oldVal);
    }

    /*!
     * Perform an atomic swap.
     *
     *  @param[in]  location     Pointer to the location to modify.
     *  @param[in]  newVal       The new value to write to \a location.
     *  @param[in]  barrier      Tells the memory ordering semantics of this operation.
     *
     * @return  The value swapped out of \a location
     */
    template< typename T > static T Swap(volatile T* location, T newVal, BARRIER_SWAP barrier = BARRIER_SWAP_NONE)
    {
        T oldVal;

        OPS_IMPL::Swap< sizeof(T) >(static_cast< volatile void* >(location), static_cast< void* >(&oldVal),
                                    static_cast< const void* >(&newVal), barrier);

        return oldVal;
    }

    /*!
     * Atomically set the value of a location.  Any outside observer using an OPS method
     * is guaranteed to see either the old value or the new value (but not an intermediate value).
     *
     *  @param[in] location     Pointer to the location.
     *  @param[in] val          The value to write to \a location.
     *  @param[in] barrier      Tells the memory ordering semantics of this operation.
     */
    template< typename T > static void Store(volatile T* location, T val, BARRIER_ST barrier = BARRIER_ST_NONE)
    {
        OPS_IMPL::Store< sizeof(T) >(static_cast< volatile void* >(location), static_cast< const void* >(&val), barrier);
    }

    /*!
     * Atomically get the value of a location.  If another thread changes the location using an
     * OPS method, Load() is guaranteed to either return the old value or the new value (but not
     * an intermediate value).
     *
     *  @param[in] location     Pointer to the location.
     *  @param[in] barrier      Tells the memory ordering semantics of this operation.
     *
     * @return  The value at \a location.
     */
    template< typename T > static T Load(volatile const T* location, BARRIER_LD barrier = BARRIER_LD_NONE)
    {
        T val;
        OPS_IMPL::Load< sizeof(T) >(static_cast< volatile const void* >(location), static_cast< void* >(&val), barrier);
        return val;
    }

    /*!
     * Atomically increment a value.
     *
     *  @param[in] location   Pointer to the location to modify.
     *  @param[in] inc        The increment amount.
     *  @param[in] barrier    Tells the memory ordering semantics of this operation.
     *
     * @return  The value of \a location prior to being incremented.
     */
    template< typename T > static T Increment(volatile T* location, T inc, BARRIER_CS barrier = BARRIER_CS_NONE)
    {
        T oldVal;
        OPS_IMPL::Increment< sizeof(T) >(static_cast< volatile void* >(location), static_cast< const void* >(&inc),
                                         static_cast< void* >(&oldVal), barrier);
        return oldVal;
    }

    /*!
     * Atomically set a location if it is less than a value.  This allows you to
     * compute the maximum of a series of values.
     *
     *  @param[in] location   Pointer to the location to modify.
     *  @param[in] val        If \a location is less than \a val it is updated.
     */
    template< typename T > static void MaxValue(volatile T* location, T val)
    {
        EXPONENTIAL_BACKOFF<> backoff;

        T oldVal = Load(location);
        while (oldVal < val && !CompareAndDidSwap(location, oldVal, val))
        {
            backoff.Delay();
            oldVal = Load(location);
        }
    }

    /*!
     * Delay the calling thread in a spin loop, using a processor specific
     * "spin loop hint" if available.
     *
     *  @param[in] delay    The number of iterations for the spin loop.
     */
    static void Delay(unsigned delay) { ATOMIC_SpinDelay(static_cast< UINT32 >(delay)); }
};

} // namespace ATOMIC
#endif // file guard
