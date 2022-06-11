/*
 * Copyright (C) 2005-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

// <COMPONENT>: atomic
// <FILE-TYPE>: component public header

#ifndef ATOMIC_FIXED_MULTIMAP_HPP
#define ATOMIC_FIXED_MULTIMAP_HPP

#include "atomic/config.hpp"
#include "atomic/ops.hpp"
#include "atomic/exponential-backoff.hpp"
#include "atomic/nullstats.hpp"

namespace ATOMIC
{
/*! @brief  Associative map with pre-allocated elements.
 *
 * A map container that is thread safe and safe to use from signal handlers.
 * It uses compare-and-swap operations to maintain atomicity rather than locking.
 * This ensures that operations are safe even if a signal interrupts an
 * operation and the signal handler accesses the same map.  The map also
 * statically allocates all of its data, so operations on the map will never
 * attempt to dynamically allocate memory.
 *
 *  @param KEY          The type of the key which is used to index the map.
 *                       The OPS operations (Load, Store, CompareAndSwap) must
 *                       be supported for this data type.
 *  @param OBJECT       Type of the object which is associated with each key.  There
 *                       are no restrictions on this data type.
 *  @param InvalidKey1  The client must provide two "invalid" key values, which it
 *                       promises to never use when inserting into the map.
 *  @param InvalidKey2  The second "invalid" key value.
 *  @param Capacity     Maximum number of objects that the map can hold.
 *  @param STATS        Type of an object that collects statistics.  See NULLSTATS for a model.
 *
 * @par Example:
 *                                                                                          \code
 *  #include "atomic/fixed-multimap.hpp"
 *
 *  struct MyElement
 *  {
 *      char name[256];
 *  };
 *
 *  ATOMIC::FIXED_MULTIMAP<int, MyElement, -1, -2, 128> Map;
 *
 *  void Foo()
 *  {
 *      MyElement newEl;
 *      Map.Add(1, newEl);              // Adds a copy of 'newEl' associated with key '1'
 *      MyElement *el = Map.Find(1);
 *  }
 *                                                                                          \endcode
 */
template< typename KEY, typename OBJECT, KEY InvalidKey1, KEY InvalidKey2, unsigned int Capacity, typename STATS = NULLSTATS >
class /*<UTILITY>*/ FIXED_MULTIMAP
{
  public:
    /*!
     * Construct a new (empty) map.  This method is NOT atomic.
     *
     *  @param[in] stats    The new statistics collection object.
     */
    FIXED_MULTIMAP(STATS* stats = 0) : _highWaterMark(0), _freeLocationHint(0), _stats(stats)
    {
        for (UINT32 i = 0; i < Capacity; i++)
            _map[i] = KeyAvailable;
    }

    /*!
     * Set the statistics collection object.  This method is NOT atomic.
     *
     *  @param[in] stats    The new statistics collection object.
     */
    void SetStatsNonAtomic(STATS* stats) { _stats = stats; }

    /*!
     * Remove all elements from the map.  This method is NOT atomic.
     */
    void ClearNonAtomic()
    {
        _highWaterMark    = 0;
        _freeLocationHint = 0;

        for (UINT32 i = 0; i < Capacity; i++)
            _map[i] = KeyAvailable;
    }

    /*!
     * Add a new key and object to the map.  A new entry is added even if there is
     * already an entry with this key.
     *
     *  @param[in] key      The key value for the new element.
     *  @param[in] userObj  The object associated with the key.  The contents of
     *                       \a userObj are copied into the map.
     *
     * @return  Returns a pointer to the object which the map contains.  The client
     *           can only safely dereference this pointer if it can guarantee that
     *           no other client has removed this element.
     */
    OBJECT* Add(KEY key, const OBJECT& userObj)
    {
        ATOMIC_CHECK_ASSERT(key != KeyAvailable && key != KeyReserved);

        UINT32 highWater = OPS::Load(&_highWaterMark);
        UINT32 freeHint  = OPS::Load(&_freeLocationHint);

        for (UINT32 i = freeHint; i < highWater; i++)
        {
            if (OPS::Load(&_map[i]) == KeyAvailable && AddAt(i, key, userObj)) return &_objects[i];
        }
        for (UINT32 i = 0; i < Capacity; i++)
        {
            if (OPS::Load(&_map[i]) == KeyAvailable && AddAt(i, key, userObj)) return &_objects[i];
        }
        return 0;
    }

    /*!
     * Attempt to find an element in the map that has the given key.  If the map
     * contains more than one element with this key, one of them is chosen arbitrarily.
     *
     * This method is guaranteed to find an element if the map contains a match at
     * the start of the find operation and that matching element remains in the map
     * until the find completes.  If the matching element is inserted or removed during
     * the find operation, this method is not guaranteed to find it.
     *
     *  @param[in] key  The key to search for.
     *
     * @return  Returns a pointer to the object associated with this key, or NULL if
     *           no such element is found.  The client can only safely dereference
     *           this pointer if it can guarantee that no other client has removed it.
     */
    OBJECT* Find(KEY key)
    {
        ATOMIC_CHECK_ASSERT(key != KeyAvailable && key != KeyReserved);

        UINT32 highWater = OPS::Load(&_highWaterMark);
        for (UINT32 i = 0; i < highWater; i++)
        {
            // This BARRIER_LD_NEXT works in conjunction with the other barrier marked (A).
            // They ensure that the contents of _object[i] are visible on this processor,
            // even if they were written by another.
            //
            if (OPS::Load(&_map[i], BARRIER_LD_NEXT) == key) return &_objects[i];
        }
        return 0;
    }

    /*!
     * Attempt to find an element in the map for which a predicate returns TRUE.
     * If the map contains more than one matching element, one of them is chosen arbitrarily.
     *
     * This method is guaranteed to find an element if the map contains a match at
     * the start of the find operation and that matching element remains in the map
     * until the find completes.  If the matching element is inserted or removed during
     * the find operation, this method is not guaranteed to find it.
     *
     *  @param[in] pred     An STL-like predicate functor.  A key is passed as the predicate's
     *                       only argument.  It returns TRUE if there is a match.
     *
     * @return  Returns a pointer to the object associated with this key, or NULL if
     *           no such element is found.  The client can only safely dereference
     *           this pointer if it can guarantee that no other client has removed it.
     */
    template< typename PRED > OBJECT* FindIf(PRED pred)
    {
        UINT32 highWater = OPS::Load(&_highWaterMark);
        for (UINT32 i = 0; i < highWater; i++)
        {
            // This BARRIER_LD_NEXT works in conjunction with the other barrier marked (A).
            // They ensure that the contents of _object[i] are visible on this processor,
            // even if they were written by another.
            //
            KEY key = OPS::Load(&_map[i], BARRIER_LD_NEXT);
            if (key != KeyAvailable && key != KeyReserved)
            {
                if (pred(key)) return &_objects[i];
            }
        }
        return 0;
    }

    /*!
     * Attempt to remove an element in the map that has the given key.  If the map
     * contains more than one element with this key, one of them is chosen arbitrarily.
     *
     * This method is guaranteed to remove an element if the map contains a match at
     * the start of the remove operation and that matching element is not removed by
     * another client during this operation.  If a matching element is inserted during
     * this operation, it may or may not be removed.
     *
     *  @param[in] key  The key to search for.
     */
    void Remove(KEY key)
    {
        ATOMIC_CHECK_ASSERT(key != KeyAvailable && key != KeyReserved);

        UINT32 highWater = OPS::Load(&_highWaterMark);
        for (UINT32 i = 0; i < highWater; i++)
        {
            if (OPS::Load(&_map[i]) == key)
            {
                RemoveAt(i, key);
                return;
            }
        }
    }

    /*!
     * Remove all the elements from the map for which a predicate function returns
     * TRUE.
     *
     * This method is guaranteed to remove an element if the map contains a match at
     * the start of the remove operation and that matching element is not removed by
     * another client during this operation.  If a matching element is inserted during
     * this operation, it may or may not be removed.
     *
     *  @param[in] pred     An STL-like predicate functor.  A key is passed as the predicate's
     *                       only argument.  If it returns TRUE, that element is removed.
     */
    template< typename PRED > void RemoveIf(PRED pred)
    {
        UINT32 highWater = OPS::Load(&_highWaterMark);
        for (UINT32 i = 0; i < highWater; i++)
        {
            KEY key = OPS::Load(&_map[i]);
            if (key != KeyAvailable && key != KeyReserved)
            {
                if (pred(key)) RemoveAt(i, key);
            }
        }
    }

    /*!
     * Execute a function once for each element in the map.  The function is guaranteed
     * to be called only for elements that exist at the start of the ForEach call and are
     * not removed during the ForEach call.
     *
     *  @param[in] func     An binary functor which is executed once for each element
     *                       in the map.  It is called like:
     *                                                                                  \code
     *                          void func(KEY key, OBJECT *obj)
     *                                                                                  \endcode
     *                       The client can only safely dereference \e obj if it can
     *                       guarantee that no other client has deleted it.
     */
    template< typename BINARY > void ForEach(BINARY func)
    {
        UINT32 highWater = OPS::Load(&_highWaterMark);
        for (UINT32 i = 0; i < highWater; i++)
        {
            // This BARRIER_LD_NEXT works in conjunction with the other barrier marked (A).
            // They ensure that the contents of _object[i] are visible on this processor,
            // even if they were written by another.
            //
            KEY key = OPS::Load(&_map[i], BARRIER_LD_NEXT);
            if (key != KeyAvailable && key != KeyReserved) func(key, &_objects[i]);
        }
    }

  private:
    /*
     * Attempt to add a new element at the given location.  Return TRUE if it could
     * be added there, FALSE if not.
     */
    bool AddAt(UINT32 index, KEY key, const OBJECT& userObj)
    {
        // If this location is available, mark it as reserved.
        //
        // The BARRIER_CS_NEXT here works in conjunction with the other barrier marked (B).
        // This ensures that the read of _map[index] is made before the read of
        // _highWaterMark below.  Otherwise, we might read a stale version of _highWaterMark
        // and not realize that _highWaterMark needs to be updated for this new element.
        // (See reference mark (C) below.)
        //
        if (!OPS::CompareAndDidSwap(&_map[index], KeyAvailable, KeyReserved, BARRIER_CS_NEXT)) return false;

        // Now that the position is reserved, we can safely write to it without
        // anyone else using it.
        //
        // The BARRIER_ST_PREV here works in conjunction with the other barrier marked (A).
        // They ensure that the write of _object[index] is visible on other processors
        // by the time the _map[index] is made valid.
        //
        _objects[index] = userObj;
        OPS::Store(&_map[index], key, BARRIER_ST_PREV);

        // Make sure the high water mark stays above all the valid entries.
        // If we're storing at the free location hint, we just bump the hint,
        // assuming that the next location is more likely to be free than this
        // one.
        //
        UINT32 highWater;
        EXPONENTIAL_BACKOFF< STATS > backoff(1, _stats);
        do
        {
            backoff.Delay();

            // Reference mark (C).  This is the read of _highWaterMark that is synchronized
            // with barrier (B).  If this read were to happen before the CompareAndDidSwap on
            // _map[index] above, we might get a stale value of _highWaterMark.  This is
            // prevented by barrier (B).
            //
            highWater = OPS::Load(&_highWaterMark);
            if (index < highWater) break;
        }
        while (!OPS::CompareAndDidSwap(&_highWaterMark, highWater, index + 1));

        OPS::CompareAndSwap(&_freeLocationHint, index, index + 1);
        return true;
    }

    /*
     * If the given element contains the given key, remove it.
     */
    void RemoveAt(UINT32 index, KEY key)
    {
        // Unless someone else removes this element first, mark the location as reserved.
        //
        if (!OPS::CompareAndDidSwap(&_map[index], key, KeyReserved)) return;

        do
        {
            // This location is now a good place to insert at in the future.
            //
            OPS::Store(&_freeLocationHint, index);

            // If this location is at the end of the valid portion of the map, we can lower the
            // high water mark.  Note, the position is current marked as "reserved", so no other
            // client will try to use it while we contemplate lowering the high water mark.
            //
            // The BARRIER_CS_NEXT here works in conjunction with the other barrier marked (B).
            // This ensures that the write to _highWaterMark is made visible on other processors
            // before _map[index] is marked available.
            //
            UINT32 highWater = OPS::Load(&_highWaterMark);
            if (index != highWater - 1 || !OPS::CompareAndDidSwap(&_highWaterMark, highWater, index, BARRIER_CS_NEXT))
            {
                OPS::Store(&_map[index], KeyAvailable);
                break;
            }

            // Make this location available for other clients.
            //
            OPS::Store(&_map[index], KeyAvailable);

            // If the next position below is also available, keep iterating in order to
            // reduce the high water mark even further.
            //
            if (index == 0) break;
            index--;
        }
        while (OPS::CompareAndDidSwap(&_map[index], KeyAvailable, KeyReserved));
    }

  private:
    static const KEY KeyAvailable = InvalidKey1; // Entry is available to hold a map
    static const KEY KeyReserved  = InvalidKey2; // Entry is being updated, not available but not valid either

    // These arrays are the map.  Keys in _map correspond to objects in _objects.
    //
    KEY _map[Capacity];
    OBJECT _objects[Capacity];

    // This is an index into the map.  All entries at this location and above are invalid.
    //
    volatile UINT32 _highWaterMark;

    // This is a hint to a location in the map that is probably available.
    //
    volatile UINT32 _freeLocationHint;

    STATS* _stats; // Object which collects statistics, or NULL
};

} // namespace ATOMIC
#endif // file guard
