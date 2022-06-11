/*
 * Copyright (C) 2008-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

// <COMPONENT>: atomic
// <FILE-TYPE>: component public header

#ifndef ATOMIC_EXPONENTIAL_BACKOFF_HPP
#define ATOMIC_EXPONENTIAL_BACKOFF_HPP

#include "util.hpp"
#include "atomic/nullstats.hpp"
#include "atomic/private/backoff-impl.hpp"

namespace ATOMIC
{
/*! @brief  Helper object for exponential delays.
 *
 * A helper object that implements an exponential backoff algorithm.  This is most often
 * used inside a compare-swap loop to prevent thrashing when there is high contention.
 *
 *  @param STATS    Type of an object that collects statistics.  See NULLSTATS for a model.
 *
 * @par Example:
 *                                                                                          \code
 *  #include "atomic/exponential-backoff.hpp"
 *  #include "atomic/ops.hpp"
 *
 *  void Foo()
 *  {
 *      ATOMIC::EXPONENTIAL_BACKOFF<> backoff;
 *      do {
 *          backoff.Delay();
 *          oldVal = ....
 *          newVal = ....
 *      } while (!ATOMIC::OPS::CompareAndDidSwap(&val, oldVal, newVal));
 *  }
 *                                                                                          \endcode
 */
template< typename STATS = NULLSTATS > class /*<UTILITY>*/ EXPONENTIAL_BACKOFF
{
  public:
    /*!
     * @param[in] freeIterations    Number of times through loop before Delay() does anything.
     * @param[in] stats             Object to keep track of statistics, or NULL
     */
    EXPONENTIAL_BACKOFF(UINT32 freeIterations = 1, STATS* stats = 0)
        : _freeIterations(freeIterations), _iteration(0), _stats(stats)
    {}

    ~EXPONENTIAL_BACKOFF()
    {
        if (_stats && _iteration > _freeIterations) _stats->Backoff(_iteration - _freeIterations);
    }

    /*!
     * Reset the object to the first "iteration".
     */
    void Reset()
    {
        if (_stats && _iteration > _freeIterations) _stats->Backoff(_iteration - _freeIterations);
        _iteration = 0;
    }

    /*!
     * Delay for a short period of time and advance to the next "iteration".  The delay
     * time typically grows longer for each successive iteration.
     */
    void Delay()
    {
        if (_iteration++ < _freeIterations) return;

        UINT32 fixed  = 1 << (_iteration - 1 - _freeIterations);
        UINT32 mask   = fixed - 1;
        UINT32 random = (reinterpret_cast< PTRINT >(&random) >> 4) & mask;
        UINT32 delay  = fixed + random;

        ATOMIC_SpinDelay(delay);
    }

    /*!
     * @return  The number of times Delay() has been called since the last Reset().
     */
    UINT32 GetIterationCount() { return _iteration; }

  private:
    const UINT32 _freeIterations; // number "free" iterations before we start to delay
    UINT32 _iteration;            // current iteration
    STATS* _stats;                // points to object which collects statistics, or NULL
};

} // namespace ATOMIC
#endif // file guard
