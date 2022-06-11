/*
 * Copyright (C) 2008-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

// <COMPONENT>: atomic
// <FILE-TYPE>: component public header

#ifndef ATOMIC_HPP
#define ATOMIC_HPP

/*! @mainpage ATOMIC library
 *
 * The ATOMIC library provides a variety of non-blocking atomic utility routines, such as thread safe queues
 * and associative maps.  These utilities use hardware primitives such as compare-and-swap to maintain
 * atomicity, not locks, which makes them safe even when used in asynchronous interrupt handlers.
 *
 * All utilities have a C++ template interface, somewhat reminiscent of the STL.  As a result, they should
 * be easy to customize for use with your own data structures.
 *
 * Queues:
 *  - @ref ATOMIC::LIFO_CTR             "LIFO_CTR - Last-in-first-out queue"
 *  - @ref ATOMIC::LIFO_PTR             "LIFO_PTR - Last-in-first-out queue"
 *  - @ref ATOMIC::FIXED_LIFO           "FIXED_LIFO - Last-in-first-out queue with pre-allocated elements"
 *
 * Associative maps and sets:
 *  - @ref ATOMIC::FIXED_MULTIMAP       "FIXED_MULTIMAP - Associative map with pre-allocated elements"
 *  - @ref ATOMIC::FIXED_MULTISET       "FIXED_MULTISET - Unordered set of data with pre-allocated elements"
 *
 * Fundamental operations, utilities:
 *  - @ref ATOMIC::OPS                  "OPS - Fundamental atomic operations"
 *  - @ref ATOMIC::EXPONENTIAL_BACKOFF  "EXPONENTIAL_BACKOFF - Helper object for exponential delays"
 *  - @ref ATOMIC::IDSET                "IDSET - Maintains a set of unique IDs"
 *  - @ref ATOMIC::NULLSTATS            "NULLSTATS - Model for a statistics-gathering object"
 *
 * Configuration:
 *  - @ref CONFIG
 */

#include "atomic/ops.hpp"
#include "atomic/lifo-ctr.hpp"
#include "atomic/lifo-ptr.hpp"
#include "atomic/fixed-lifo.hpp"
#include "atomic/fixed-multimap.hpp"
#include "atomic/fixed-multiset.hpp"
#include "atomic/idset.hpp"
#include "atomic/exponential-backoff.hpp"

#endif // file guard
