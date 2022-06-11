/*
 * Copyright (C) 2008-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

// <COMPONENT>: atomic
// <FILE-TYPE>: component private header

#ifndef ATOMIC_PRIVATE_IA32_OPS_IMPL_IA32_ASM_HPP
#define ATOMIC_PRIVATE_IA32_OPS_IMPL_IA32_ASM_HPP

extern "C" void ATOMIC_CompareAndSwap8(volatile void* location, const void* oldVal, void* newVal);
extern "C" void ATOMIC_CompareAndSwap16(volatile void* location, const void* oldVal, void* newVal);
extern "C" void ATOMIC_CompareAndSwap32(volatile void* location, const void* oldVal, void* newVal);
extern "C" void ATOMIC_CompareAndSwap64(volatile void* location, const void* oldVal, void* newVal);
extern "C" void ATOMIC_Swap8(volatile void* location, void* oldVal, const void* newVal);
extern "C" void ATOMIC_Swap16(volatile void* location, void* oldVal, const void* newVal);
extern "C" void ATOMIC_Swap32(volatile void* location, void* oldVal, const void* newVal);
extern "C" void ATOMIC_Copy64(volatile const void* from, volatile void* to);
extern "C" void ATOMIC_Increment8(volatile void* location, const void* inc, void* oldVal);
extern "C" void ATOMIC_Increment16(volatile void* location, const void* inc, void* oldVal);
extern "C" void ATOMIC_Increment32(volatile void* location, const void* inc, void* oldVal);

#endif // file guard
