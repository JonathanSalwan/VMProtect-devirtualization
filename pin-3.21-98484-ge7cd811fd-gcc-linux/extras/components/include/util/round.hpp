/*
 * Copyright (C) 2008-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

// <COMPONENT>: util
// <FILE-TYPE>: component public header

#ifndef UTIL_ROUND_HPP
#define UTIL_ROUND_HPP

namespace UTIL
{
/*!
 * Rounds an integer value down.
 *
 *  @param[in] val      Integral value.
 *  @param[in] align    The \a val is rounded down to this alignment (need not be a power of 2).
 *
 * @return  The rounded value.
 */
template< typename T > T RoundDown(T val, size_t align)
{
    size_t mod = val % align;
    val -= mod;
    return val;
}

/*!
 * Rounds an integer value up.
 *
 *  @param[in] val      Integral value.
 *  @param[in] align    The \a val is rounded up to this alignment (need not be a power of 2).
 *
 * @return  The rounded value.
 */
template< typename T > T RoundUp(T val, size_t align)
{
    size_t mod = val % align;
    if (mod) val += (align - mod);
    return val;
}

/*!
 * Rounds a pointer down.
 *
 *  @param[in] ptr      A pointer.
 *  @param[in] align    The \a ptr is rounded down to this alignment (need not be a power of 2).
 *
 * @return  The rounded pointer value.
 */
template< typename T > T* RoundDown(T* ptr, size_t align)
{
    PTRINT val = reinterpret_cast< PTRINT >(ptr);
    size_t mod = val % align;
    val -= mod;
    return reinterpret_cast< T* >(val);
}

/*!
 * Rounds a pointer up.
 *
 *  @param[in] ptr      A pointer.
 *  @param[in] align    The \a ptr is rounded up to this alignment (need not be a power of 2).
 *
 * @return  The rounded pointer value.
 */
template< typename T > T* RoundUp(T* ptr, size_t align)
{
    PTRINT val = reinterpret_cast< PTRINT >(ptr);
    size_t mod = val % align;
    if (mod) val += (align - mod);
    return reinterpret_cast< T* >(val);
}

/*!
 * Tells difference between two pointers.
 *
 *  @param[in] ptr1     Usually the pointer to higher memory.
 *  @param[in] ptr2     Usually the pointer to lower memory.
 *
 * @return  The difference in bytes from \a ptr2 to \a ptr1.
 */
inline size_t PtrDiff(const void* ptr1, const void* ptr2)
{
    return static_cast< const INT8* >(ptr1) - static_cast< const INT8* >(ptr2);
}

/*!
 * Compute a pointer that is an offset from a base pointer.
 *
 *  @param[in] ptr      The base pointer.
 *  @param[in] offset   Offset in bytes.
 *
 * @return  A new pointer that is \a offset bytes from \a ptr.
 */
inline void* PtrAtOffset(void* ptr, size_t offset) { return static_cast< INT8* >(ptr) + offset; }

/*!
 * Compute a pointer that is an offset from a base pointer.
 *
 *  @param[in] ptr      The base pointer.
 *  @param[in] offset   Offset in bytes.
 *
 * @return  A new pointer that is \a offset bytes from \a ptr.
 */
inline const void* PtrAtOffset(const void* ptr, size_t offset) { return static_cast< const INT8* >(ptr) + offset; }

/*!
 * Return pointer of type <T> whose offset, in bytes, from <ptr> is <offset>
 */
template< typename T > T* PtrAtOffset(void* ptr, size_t offset) { return static_cast< T* >(PtrAtOffset(ptr, offset)); }

/*!
  Return const pointer of type <T> whose offset, in bytes, from <ptr> is <offset>
 */
template< typename T > const T* PtrAtOffset(const void* ptr, size_t offset)
{
    return static_cast< const T* >(PtrAtOffset(ptr, offset));
}

} // namespace UTIL
#endif // file guard
