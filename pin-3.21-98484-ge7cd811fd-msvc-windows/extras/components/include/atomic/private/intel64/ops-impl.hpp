/*
 * Copyright (C) 2008-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

// <COMPONENT>: atomic
// <FILE-TYPE>: component private header

#ifndef ATOMIC_PRIVATE_INTEL64_OPS_IMPL_HPP
#define ATOMIC_PRIVATE_INTEL64_OPS_IMPL_HPP

#include "atomic/private/intel64/ops-impl-intel64-asm.hpp"

namespace ATOMIC
{
/*
 * Low-level implementation of fundemental atomic operations for the Intel(R) 64 architecture.
 * Clients should not use this class directly, but should use OPS instead.
 */
namespace OPS_IMPL
{
// ---------- Compare-And-Swap ----------

template< unsigned size >
static inline void CompareAndSwap(volatile void* location, const void* oldVal, void* newVal, BARRIER_CS ignored);

template<> inline void CompareAndSwap< 1 >(volatile void* location, const void* oldVal, void* newVal, BARRIER_CS ignored)
{
    ATOMIC_CompareAndSwap8(location, oldVal, newVal);
}

template<> inline void CompareAndSwap< 2 >(volatile void* location, const void* oldVal, void* newVal, BARRIER_CS ignored)
{
    ATOMIC_CompareAndSwap16(location, oldVal, newVal);
}

template<> inline void CompareAndSwap< 4 >(volatile void* location, const void* oldVal, void* newVal, BARRIER_CS ignored)
{
    ATOMIC_CompareAndSwap32(location, oldVal, newVal);
}

template<> inline void CompareAndSwap< 8 >(volatile void* location, const void* oldVal, void* newVal, BARRIER_CS ignored)
{
    ATOMIC_CompareAndSwap64(location, oldVal, newVal);
}

// ---------- Store ----------

template< unsigned size > static inline void Store(volatile void* location, const void* val, BARRIER_ST ignored);

template<> inline void Store< 1 >(volatile void* location, const void* val, BARRIER_ST ignored)
{
    *static_cast< volatile UINT8* >(location) = *static_cast< const UINT8* >(val);
}

template<> inline void Store< 2 >(volatile void* location, const void* val, BARRIER_ST ignored)
{
    *static_cast< volatile UINT16* >(location) = *static_cast< const UINT16* >(val);
}

template<> inline void Store< 4 >(volatile void* location, const void* val, BARRIER_ST ignored)
{
    *static_cast< volatile UINT32* >(location) = *static_cast< const UINT32* >(val);
}

template<> inline void Store< 8 >(volatile void* location, const void* val, BARRIER_ST ignored)
{
    *static_cast< volatile UINT64* >(location) = *static_cast< const UINT64* >(val);
}

// ---------- Load ----------

template< unsigned size > static inline void Load(volatile const void* location, void* val, BARRIER_LD ignored);

template<> inline void Load< 1 >(volatile const void* location, void* val, BARRIER_LD ignored)
{
    *static_cast< UINT8* >(val) = *static_cast< volatile const UINT8* >(location);
}

template<> inline void Load< 2 >(volatile const void* location, void* val, BARRIER_LD ignored)
{
    *static_cast< UINT16* >(val) = *static_cast< volatile const UINT16* >(location);
}

template<> inline void Load< 4 >(volatile const void* location, void* val, BARRIER_LD ignored)
{
    *static_cast< UINT32* >(val) = *static_cast< volatile const UINT32* >(location);
}

template<> inline void Load< 8 >(volatile const void* location, void* val, BARRIER_LD ignored)
{
    *static_cast< UINT64* >(val) = *static_cast< volatile const UINT64* >(location);
}

// ---------- Swap ----------

template< unsigned size >
static inline void Swap(volatile void* location, void* oldVal, const void* newVal, BARRIER_SWAP ignored);

template<> inline void Swap< 1 >(volatile void* location, void* oldVal, const void* newVal, BARRIER_SWAP ignored)
{
    ATOMIC_Swap8(location, oldVal, newVal);
}

template<> inline void Swap< 2 >(volatile void* location, void* oldVal, const void* newVal, BARRIER_SWAP ignored)
{
    ATOMIC_Swap16(location, oldVal, newVal);
}

template<> inline void Swap< 4 >(volatile void* location, void* oldVal, const void* newVal, BARRIER_SWAP ignored)
{
    ATOMIC_Swap32(location, oldVal, newVal);
}

template<> inline void Swap< 8 >(volatile void* location, void* oldVal, const void* newVal, BARRIER_SWAP ignored)
{
    ATOMIC_Swap64(location, oldVal, newVal);
}

// ---------- Increment ----------

template< unsigned size >
static inline void Increment(volatile void* location, const void* inc, void* oldVal, BARRIER_CS ignored);

template<> inline void Increment< 1 >(volatile void* location, const void* inc, void* oldVal, BARRIER_CS ignored)
{
    ATOMIC_Increment8(location, inc, oldVal);
}

template<> inline void Increment< 2 >(volatile void* location, const void* inc, void* oldVal, BARRIER_CS ignored)
{
    ATOMIC_Increment16(location, inc, oldVal);
}

template<> inline void Increment< 4 >(volatile void* location, const void* inc, void* oldVal, BARRIER_CS ignored)
{
    ATOMIC_Increment32(location, inc, oldVal);
}

template<> inline void Increment< 8 >(volatile void* location, const void* inc, void* oldVal, BARRIER_CS ignored)
{
    ATOMIC_Increment64(location, inc, oldVal);
}
} // namespace OPS_IMPL

} // namespace ATOMIC
#endif // file guard
