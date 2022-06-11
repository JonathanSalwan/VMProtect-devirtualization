/*
 * Copyright (C) 2008-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

// <COMPONENT>: util
// <FILE-TYPE>: component public header

#ifndef UTIL_SCOPED_ARRAY_HPP
#define UTIL_SCOPED_ARRAY_HPP

#include <stddef.h>
#include <assert.h>

namespace UTIL
{
/*!
 * A simple smart pointer, inspired by boost::scoped_array.  Pointers managed by
 * SCOPED_ARRAY cannot be shared, and there is no reference counting overhead.  A
 * scoped pointer is simply deleted at the end of the enclosing scope.  Since
 * these pointers can't be shared, SCOPED_ARRAY disallows assignment from one
 * SCOPED_ARRAY to another.
 */
template< typename T > class /*<UTILITY>*/ SCOPED_ARRAY
{
  public:
    /*!
     * Create a smart pointer wrapper for \a p.
     *
     *  @param[in] p     A pointer from "new[]", or NULL.
     */
    explicit SCOPED_ARRAY(T* p = 0) : _ptr(p) {}

    /*!
     * The destructor automatically calls delete on the pointer.
     */
    ~SCOPED_ARRAY()
    {
        delete[] _ptr; // Note, delete of NULL is defined to do nothing.
    }

    /*!
     * Deletes the underlying pointer, then assigns a new pointer.
     *
     *  @param[in] p    A pointer from "new[]", or NULL.
     */
    void Reset(T* p = 0)
    {
        delete[] _ptr;
        _ptr = p;
    }

    /*!
     * Index into the underlying array, which must not be NULL.
     *
     *  @param[in] i    Index into the array.
     *
     * @return  A reference to the array element indexed by \a i.
     */
    T& operator[](std::ptrdiff_t i) const
    {
        assert(_ptr != 0);
        assert(i >= 0);
        return _ptr[i];
    }

    /*!
     * @return The underlying pointer.
     */
    T* Get() const { return _ptr; }

    /*!
     * Conversion to bool.
     *
     * @return  TRUE if the underlying pointer is not NULL.
     */
    operator bool() const { return (_ptr != 0); }

    /*!
     * @return  TRUE if the underlying pointer is NULL.
     */
    bool operator!() const { return (_ptr == 0); }

  private:
    T* _ptr;

    SCOPED_ARRAY(SCOPED_ARRAY const&);
    SCOPED_ARRAY& operator=(SCOPED_ARRAY const&);
};

} // namespace UTIL
#endif // file guard
