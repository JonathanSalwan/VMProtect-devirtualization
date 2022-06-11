/*
 * Copyright (C) 2008-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

// <COMPONENT>: util
// <FILE-TYPE>: component public header

#ifndef UTIL_NUMBERBITS_HPP
#define UTIL_NUMBERBITS_HPP

namespace UTIL
{
/*! @brief  Compute the number of bits required to hold a constant integer value.
 *
 * This can be used to compute the number of bits required to hold an integer (up to a
 * 32-bit integer).  Usage is like this:
 *
 *                                                                                  \code
 * int nbits = NUMBER_BITS<54>::count;  // number of bits required to hold "54"
 *                                                                                  \endcode
 *
 * Note, the value computed is a compile-time constant, so it can be used wherever
 * a constant is allowed:
 *                                                                                  \code
 * struct FOO
 * {
 *    int a : NUMBER_BITS<54>::count;   // define a bit field large enough to hold "54"
 * };
 *                                                                                  \endcode
 */
template< UINT32 num > struct /*<UTILITY>*/ NUMBER_BITS
{
    static const UINT32 count =
        ((num ? 1 : 0) + ((num >> 1) ? 1 : 0) + ((num >> 2) ? 1 : 0) + ((num >> 3) ? 1 : 0) + ((num >> 4) ? 1 : 0) +
         ((num >> 5) ? 1 : 0) + ((num >> 6) ? 1 : 0) + ((num >> 7) ? 1 : 0) + ((num >> 8) ? 1 : 0) + ((num >> 9) ? 1 : 0) +
         ((num >> 10) ? 1 : 0) + ((num >> 11) ? 1 : 0) + ((num >> 12) ? 1 : 0) + ((num >> 13) ? 1 : 0) + ((num >> 14) ? 1 : 0) +
         ((num >> 15) ? 1 : 0) + ((num >> 16) ? 1 : 0) + ((num >> 17) ? 1 : 0) + ((num >> 18) ? 1 : 0) + ((num >> 19) ? 1 : 0) +
         ((num >> 20) ? 1 : 0) + ((num >> 21) ? 1 : 0) + ((num >> 22) ? 1 : 0) + ((num >> 23) ? 1 : 0) + ((num >> 24) ? 1 : 0) +
         ((num >> 25) ? 1 : 0) + ((num >> 26) ? 1 : 0) + ((num >> 27) ? 1 : 0) + ((num >> 28) ? 1 : 0) + ((num >> 29) ? 1 : 0) +
         ((num >> 30) ? 1 : 0) + ((num >> 31) ? 1 : 0));
};

} // namespace UTIL
#endif // file guard
