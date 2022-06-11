/*
 * Copyright (C) 2006-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

// <COMPONENT>: atomic
// <FILE-TYPE>: component public header

#ifndef ATOMIC_IDSET_HPP
#define ATOMIC_IDSET_HPP

#include "atomic/ops.hpp"
#include "atomic/nullstats.hpp"

namespace ATOMIC
{
/*! @brief  Maintains a set of unique IDs.
 *
 * A non-blocking utility that maintains a set of small integral IDs.  Clients
 * can request a unique ID and release an ID when it is no longer needed.  The
 * IDSET reuses IDs that have been released to ensure that the ID numbers remain
 * small.
 *
 *  @param MaxID    The IDSET allows IDs in the inclusive range [1, MaxID]
 *  @param STATS    Type of an object that collects statistics.  See NULLSTATS for a model.
 *
 * @par Example:
 *                                                                                          \code
 *  #include "atomic/idset.hpp"
 *
 *  ATOMIC::IDSET<31> IdGenerator;
 *
 *  void Foo()
 *  {
 *      UINT32 id = IdGenerator.GetID();
 *      IdGenerator.ReleaseID(id);
 *  }
 *                                                                                          \endcode
 */
template< UINT32 MaxID, typename STATS = NULLSTATS > class /*<UTILITY>*/ IDSET
{
  public:
    /*!
     * Construct a new IDSET.  This method is NOT atomic.
     *
     *  @param[in] stats    The new statistics collection object.
     */
    IDSET(STATS* stats = 0) : _stats(stats)
    {
        for (UINT32 i = 0; i < _numElements; i++)
            _bits[i] = 0;

        // If MaxID is not an even multiple of the number of bits in a UINT32,
        // the _bits[] will contain some "extra" bits.  Permanently reserve these
        // extra bit positions so GetID() never returns an ID greater than MaxID.
        //
        const UINT32 MaxIDMod32 = MaxID % 32;
        if (MaxIDMod32) _bits[_numElements - 1] = ((1 << ((32 - MaxIDMod32) % 32)) - 1) << MaxIDMod32;
    }

    /*!
     * Set the statistics collection object.  This method is NOT atomic.
     *
     *  @param[in] stats    The new statistics collection object.
     */
    void SetStatsNonAtomic(STATS* stats) { _stats = stats; }

    /*!
     * Request a new ID that is not currently in use.
     *
     * @return  Returns an ID in the range [1, MaxID] or 0 if there are no
     *           unused IDs.
     */
    UINT32 GetID()
    {
        EXPONENTIAL_BACKOFF< STATS > backoff(1, _stats);

        for (UINT32 i = 0; i < _numElements; i++)
        {
            UINT32 val = OPS::Load(&_bits[i]);

            while (val != 0xffffffff)
            {
                UINT32 bit = 0;
                for (UINT32 tval = val; tval & 1; tval >>= 1)
                    bit++;

                UINT32 newval = val | (1 << bit);
                if (OPS::CompareAndDidSwap(&_bits[i], val, newval)) return i * sizeof(UINT32) * 8 + bit + 1;

                backoff.Delay();
                val = OPS::Load(&_bits[i]);
            }
        }

        return 0;
    }

    /*!
     * Release an ID, making it available for reuse.
     *
     *  @param[in] id   The ID, which must be in the range [1,MaxID].
     */
    void ReleaseID(UINT32 id)
    {
        id--;
        UINT32 i   = id >> 5;
        UINT32 bit = 1 << (id & 0x1f);

        UINT32 val;
        UINT32 newval;
        EXPONENTIAL_BACKOFF< STATS > backoff(1, _stats);

        do
        {
            backoff.Delay();
            val    = OPS::Load(&_bits[i]);
            newval = val & ~bit;
        }
        while (!OPS::CompareAndDidSwap(&_bits[i], val, newval));
    }

    /*!
    * Return true for an id is in use, false otherwise
    *
    *  @param[in] id   The ID, which must be in the range [1,MaxID].
    */
    bool IsIDInUse(UINT32 id)
    {
        id--;
        UINT32 i   = id >> 5;
        UINT32 bit = 1 << (id & 0x1f);
        UINT32 val = OPS::Load(&_bits[i]);
        return ((val & bit) != 0);
    }

  private:
    static const UINT32 _numElements = (MaxID + 8 * sizeof(UINT32) - 1) / (8 * sizeof(UINT32));
    volatile UINT32 _bits[_numElements];

    STATS* _stats; // Object which collects statistics, or NULL
};

} // namespace ATOMIC
#endif // file guard
