/*
 * Copyright (C) 2008-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

// <COMPONENT>: atomic
// <FILE-TYPE>: component public header

#ifndef ATOMIC_OPS_ENUM_HPP
#define ATOMIC_OPS_ENUM_HPP

namespace ATOMIC
{
/*!
 * A type that tells the memory ordering semantics for a particular memory operation.  The
 * ordering semantics specify the visibility of this memory operation on other processors
 * w.r.t. other memory operations on this processor.  These enum values are for OPS::CompareAndSwap()
 * operations.
 */
enum BARRIER_CS
{
    BARRIER_CS_NONE, ///< Compare/swap operation has no special ordering semantics.
    BARRIER_CS_PREV, ///< Compare/swap visibile after all previous memory operations from this processor.
    BARRIER_CS_NEXT  ///< Compare/swap visible before all subsequent memory operations from this processor.
};

/*!
 * Tells the memory ordering semantics of an OPS::Swap() operation.
 */
enum BARRIER_SWAP
{
    BARRIER_SWAP_NONE, ///< Swap operation has no special ordering semantics.
    BARRIER_SWAP_PREV, ///< Swap visibile after all previous memory operations from this processor.
    BARRIER_SWAP_NEXT  ///< Swap visible before all subsequent memory operations from this processor.
};

/*!
 * Tells the memory ordering semantics of an OPS::Store() operation.
 */
enum BARRIER_ST
{
    BARRIER_ST_NONE, ///< Store operation has no special ordering semantics.
    BARRIER_ST_PREV  ///< Store visibile after all previous memory operations from this processor.
};

/*!
 * Tells the memory ordering semantics of an OPS::Load() operation.
 */
enum BARRIER_LD
{
    BARRIER_LD_NONE, ///< Load operation has no special ordering semantics.
    BARRIER_LD_NEXT  ///< Load gets value before all subsequent memory operations visible from this processor.
};

} // namespace ATOMIC
#endif // file guard
