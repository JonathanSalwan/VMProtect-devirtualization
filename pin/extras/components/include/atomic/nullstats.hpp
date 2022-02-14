/*
 * Copyright (C) 2008-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

// <COMPONENT>: atomic
// <FILE-TYPE>: component public header

#ifndef ATOMIC_NULLSTATS_HPP
#define ATOMIC_NULLSTATS_HPP

namespace ATOMIC
{
/*! @brief  Model for a statistics-gathering object.
 *
 * A dummy type to use when you don't want to keep track of statistics on atomic operations.
 * To actually gather statistics, implement your own class with the same methods.
 */
class /*<UTILITY>*/ NULLSTATS
{
  public:
    /*!
     * This is called at the end of each compare-and-swap loop, and whenever the
     * EXPONENTIAL_BACKOFF object's Reset() method is called.
     *
     *  @param[in] iterations   The number of failed CAS iterations, each requiring an
     *                           exponential backoff delay.
     */
    void Backoff(UINT32 iterations) {}
};

} // namespace ATOMIC
#endif // file guard
