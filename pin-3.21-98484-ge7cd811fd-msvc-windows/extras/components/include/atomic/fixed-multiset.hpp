/*
 * Copyright (C) 2008-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

// <COMPONENT>: atomic
// <FILE-TYPE>: component public header

#ifndef ATOMIC_FIXED_MULTISET_HPP
#define ATOMIC_FIXED_MULTISET_HPP

#include "atomic/fixed-multimap.hpp"
#include "atomic/nullstats.hpp"

namespace ATOMIC
{
/*! @brief  Unordered set of data with pre-allocated elements.
 *
 * A set container that is thread safe and safe to use from signal handlers.
 * It uses compare-and-swap operations to maintain atomicity rather than locking.
 * This ensures that operations are safe even if a signal interrupts an
 * operation and the signal handler accesses the same set.  The set also
 * statically allocates all of its data, so operations on the set will never
 * attempt to dynamically allocate memory.
 *
 *  @param KEY          The type of the data element contained in the set.  The
 *                       OPS operations (Load, Store, CompareAndSwap) must
 *                       be supported for this data type.
 *  @param InvalidKey1  The client must provide two "invalid" key values, which it
 *                       promises to never use when inserting into the set.
 *  @param InvalidKey2  The second "invalid" key value.
 *  @param Capacity     Maximum number of elements that the set can hold.
 *  @param STATS        Type of an object that collects statistics.  See NULLSTATS for a model.
 *
 * @par Example:
 *                                                                                          \code
 *  #include "atomic/fixed-multiset.hpp"
 *
 *  ATOMIC::FIXED_MULTISET<int, -1, -2, 128> Set;
 *
 *  void Foo()
 *  {
 *      Set.Add(1);
 *      bool found = Set.Find(1);
 *  }
 *                                                                                          \endcode
 */
template< typename KEY, KEY InvalidKey1, KEY InvalidKey2, unsigned int Capacity, typename STATS = NULLSTATS >
class /*<UTILITY>*/ FIXED_MULTISET
{
  public:
    /*!
     * Construct a new (empty) set.  This method is NOT atomic.
     *
     *  @param[in] stats    The new statistics collection object.
     */
    FIXED_MULTISET(STATS* stats = 0) : _map(stats) {}

    /*!
     * Set the statistics collection object.  This method is NOT atomic.
     *
     *  @param[in] stats    The new statistics collection object.
     */
    void SetStatsNonAtomic(STATS* stats) { _map.SetStatsNonAtomic(stats); }

    /*!
     * Remove all elements from the set.  This method is NOT atomic.
     */
    void ClearNonAtomic() { _map.ClearNonAtomic(); }

    /*!
     * Add a new element to the set, even if the element already exists in the set.
     *
     *  @param[in] key      The element to add.
     */
    void Add(KEY key) { _map.Add(key, EMPTY()); }

    /*!
     * Try to find an element in the set.
     *
     * This method is guaranteed to find an element if the set contains a match at
     * the start of the find operation and that matching element remains in the set
     * until the find completes.  If the matching element is inserted or removed during
     * the find operation, this method is not guaranteed to find it.
     *
     *  @param[in] key      The element to search for.
     *
     * @return  TRUE if the element exists.
     */
    bool Find(KEY key) { return (_map.Find(key) != 0); }

    /*!
     * Attempt to find an element in the set for which a predicate returns TRUE.
     *
     * This method is guaranteed to find an element if the set contains a match at
     * the start of the find operation and that matching element remains in the set
     * until the find completes.  If the matching element is inserted or removed during
     * the find operation, this method is not guaranteed to find it.
     *
     *  @param[in] pred     An STL-like predicate functor.  A key is passed as the predicate's
     *                       only argument.  It returns TRUE if there is a match.
     *
     * @return  TRUE if the element exists.
     */
    template< typename PRED > bool FindIf(PRED pred) { return (_map.FindIf(pred) != 0); }

    /*!
     * Attempt to remove an element from the set.
     *
     * This method is guaranteed to remove an element if the set contains a match at
     * the start of the remove operation and that matching element is not removed by
     * another client during this operation.  If a matching element is inserted during
     * this operation, it may or may not be removed.
     *
     *  @param[in] key  The key to search for.
     */
    void Remove(KEY key) { _map.Remove(key); }

    /*!
     * Remove all the elements from the set for which a predicate function returns
     * TRUE.
     *
     * This method is guaranteed to remove an element if the set contains a match at
     * the start of the remove operation and that matching element is not removed by
     * another client during this operation.  If a matching element is inserted during
     * this operation, it may or may not be removed.
     *
     *  @param[in] pred     An STL-like predicate functor.  A key is passed as the predicate's
     *                       only argument.  If it returns TRUE, that element is removed.
     */
    template< typename PRED > void RemoveIf(PRED pred) { _map.RemoveIf(pred); }

  private:
    struct EMPTY
    {
    };
    FIXED_MULTIMAP< KEY, EMPTY, InvalidKey1, InvalidKey2, Capacity, STATS > _map;
};

} // namespace ATOMIC
#endif // file guard
