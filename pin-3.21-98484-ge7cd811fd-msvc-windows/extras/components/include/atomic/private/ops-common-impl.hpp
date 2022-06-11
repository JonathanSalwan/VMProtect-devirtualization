/*
 * Copyright (C) 2011-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

// <COMPONENT>: atomic
// <FILE-TYPE>: component private header

#ifndef ATOMIC_PRIVATE_OPS_COMMON_IMPL_HPP
#define ATOMIC_PRIVATE_OPS_COMMON_IMPL_HPP

#include "atomic/exponential-backoff.hpp"
#include "atomic/ops-enum.hpp"

namespace ATOMIC
{
/*
 * Generic implementations of some operations using compare-and-swap.  These implementations
 * are used on architectures that have compare-and-swap, but do not have an instruction that
 * directly implements the operation in question.  Clients should not use this class directly,
 * but should use OPS instead.
 */
namespace OPS_IMPL
{
template< unsigned size >
static inline void CompareAndSwap(volatile void* location, const void* oldVal, void* newVal, BARRIER_CS ignored);

template< unsigned size > static inline void Load(volatile const void* location, void* val, BARRIER_LD ignored);

// ---------- Swap ----------

template< typename T >
inline void SwapByCompareAndSwap(volatile void* location, void* oldVal, const void* newVal, BARRIER_SWAP barrier)
{
    T oldv;
    T newv;
    T val = *static_cast< const T* >(newVal);

    BARRIER_CS myBarrier = (barrier == BARRIER_SWAP_NONE)   ? BARRIER_CS_NONE
                           : (barrier == BARRIER_SWAP_PREV) ? BARRIER_CS_PREV
                                                            : BARRIER_CS_NEXT;

    EXPONENTIAL_BACKOFF<> backoff;
    do
    {
        backoff.Delay();
        Load< sizeof(T) >(location, static_cast< void* >(&oldv), BARRIER_LD_NONE);
        newv = val;
        CompareAndSwap< sizeof(T) >(location, static_cast< const void* >(&oldv), static_cast< void* >(&newv), myBarrier);
    }
    while (newv != oldv);

    *static_cast< T* >(oldVal) = oldv;
}

// ---------- Increment ----------

template< typename T >
static inline void IncrementByCompareAndSwap(volatile void* location, const void* inc, void* oldVal, BARRIER_CS barrier)
{
    T oldv;
    T newv;
    T increment = *static_cast< const T* >(inc);

    EXPONENTIAL_BACKOFF<> backoff;
    do
    {
        backoff.Delay();
        Load< sizeof(T) >(location, static_cast< void* >(&oldv), BARRIER_LD_NONE);
        newv = oldv + increment;
        CompareAndSwap< sizeof(T) >(location, static_cast< const void* >(&oldv), static_cast< void* >(&newv), barrier);
    }
    while (newv != oldv);

    *static_cast< T* >(oldVal) = oldv;
}
} // namespace OPS_IMPL

} // namespace ATOMIC
#endif // file guard
