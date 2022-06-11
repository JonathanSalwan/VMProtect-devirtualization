/*
 * Copyright (C) 2006-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

// <COMPONENT>: atomic
// <FILE-TYPE>: component public header

#ifndef ATOMIC_LIFO_PTR_HPP
#define ATOMIC_LIFO_PTR_HPP

#include "atomic/config.hpp"
#include "atomic/ops.hpp"
#include "atomic/idset.hpp"
#include "atomic/exponential-backoff.hpp"
#include "atomic/nullstats.hpp"

namespace ATOMIC
{
/*! @brief  Last-in-first-out queue.
 *
 * A non-blocking atomic LIFO queue of elements.  The client manages the allocation, deallocation, and content
 * of each element in the queue.
 *
 * The algorithm assumes that the low order bits of the element pointers are guaranteed to be zero (i.e. the
 * elements are aligned on some known boundary), and it uses these bits internally to solve the "A-B-A problem".
 *
 *  @param ELEMENT      The type of each element in the queue.  This type must include a field named "_next",
 *                       which is of type "ELEMENT * volatile".
 *  @param LowBits      The number of low-order bits in the element pointers that are guaranteed to be zero.
 *                       More bits allow more threads to simltaneously access the list, but this also increases
 *                       the amount of memory alocated by the list.
 *  @param STATS        Type of an object that collects statistics.  See NULLSTATS for a model.
 *
 * @par Example:
 *                                                                                          \code
 *  #include "atomic/lifo-ptr.hpp"
 *
 *  struct MyElement
 *  {
 *      MyElement * volatile _next;
 *      unsigned _myMember;
 *  };
 *
 *  ATOMIC::LIFO_PTR<MyElement, 5> Queue;   // Assumes all MyElement's are 32-byte aligned (low 5 bits zero).
 *
 *  void Foo(MyElement *el)
 *  {
 *      Queue.Push(el);
 *      el = Queue.Pop();
 *  }
 *                                                                                          \endcode
 */
template< typename ELEMENT, unsigned int LowBits, typename STATS = NULLSTATS > class /*<UTILITY>*/ LIFO_PTR
{
  public:
    /*!
     * Construct a new (empty) lifo queue.
     *
     *  @param[in] stats    The statistics collection object, or NULL if no statistics should be collected.
     */
    LIFO_PTR(STATS* stats = 0) : _idGenerator(stats), _stats(stats) { _head = 0; }

    /*!
     * Set the statistics collection object.  This method is NOT atomic.
     *
     *  @param[in] stats    The new statistics collection object.
     */
    void SetStatsNonAtomic(STATS* stats)
    {
        _idGenerator.SetStatsNonAtomic(stats);
        _stats = stats;
    }

    /*!
     * Push an element onto the head of the lifo queue.
     *
     *  @param[in] element  The element to push.
     */
    void Push(ELEMENT* element)
    {
        // Validate that the required low-order bits are zero.
        //
        PTRINT intElement = reinterpret_cast< PTRINT >(element);
        ATOMIC_CHECK_ASSERT(((intElement >> LowBits) << LowBits) == intElement);

        PTRINT oldHead;
        PTRINT newHead;
        EXPONENTIAL_BACKOFF< STATS > backoff(1, _stats);
        do
        {
            backoff.Delay();

            oldHead        = OPS::Load(&_head);
            element->_next = reinterpret_cast< ELEMENT* >((oldHead >> LowBits) << LowBits); // clear any previous "owner"
            newHead        = intElement;

            // BARRIER_CS_PREV below ensures that all processors will see the write to _next
            // before the element is inserted into the queue.
        }
        while (!OPS::CompareAndDidSwap(&_head, oldHead, newHead, BARRIER_CS_PREV));
    }

    /*!
     * Push a list of elements onto the head of the lifo queue.
     *
     *  @param[in] listHead     A list of ELEMENTs linked through their _next pointers.  The
     *                           last element's _next pointer must be NULL.
     *  @param[in] listTail     The last element in the list.
     */
    void PushList(ELEMENT* listHead, ELEMENT* listTail)
    {
        ATOMIC_CHECK_ASSERTSLOW(listTail && CheckList(listHead, listTail));

        PTRINT oldHead;
        PTRINT newHead;
        EXPONENTIAL_BACKOFF< STATS > backoff(1, _stats);

        do
        {
            backoff.Delay();

            oldHead         = OPS::Load(&_head);
            listTail->_next = reinterpret_cast< ELEMENT* >((oldHead >> LowBits) << LowBits); // clear any previous "owner"
            newHead         = reinterpret_cast< PTRINT >(listHead);

            // BARRIER_CS_PREV below ensures that all processors will see the write to _next
            // before the element is inserted into the queue.
        }
        while (!OPS::CompareAndDidSwap(&_head, oldHead, newHead, BARRIER_CS_PREV));
    }

    /*!
     * Pop an element off the head of the lifo queue.
     *
     * This method may fail (return NULL) if there are too many simultaneous callers to the Pop()
     * method.  The \a isEmpty parameter can be used to distinguish this failure from an empty
     * queue.  When Pop() fails due to high contention, the caller may want to wait and call Pop()
     * again in hopes that some other thread returns from its call to Pop().  The maximum number
     * of simultaneous callers to Pop() is (2^LowBits - 1) where \e LowBits is the template parameter
     * to LIFO_PTR.
     *
     *  @param[out] isEmpty     If Pop() returns NULL and \a isEmpty is not NULL, the \a isEmpty
     *                           parameter is written with TRUE if the Pop() fails because the
     *                           queue is empty or with FALSE if the Pop() fails because of
     *                           contention.
     *
     * @return  The popped element on success.  The return value is NULL if the queue is empty
     *           or if there are too many simultaneous callers to Pop().
     */
    ELEMENT* PopInternal(bool* isEmpty = 0)
    {
        // Get a unique ID for the calling thread.  We need this to avoid an "A-B-A" problem below.
        // This might fail (return zero) if there are too many threads simultaneously calling Pop().
        //
        UINT32 myID;
        if (!(myID = _idGenerator.GetID()))
        {
            if (isEmpty) *isEmpty = false;
            return 0;
        }

        PTRINT oldHead;
        PTRINT midHead;
        PTRINT newHead;
        ELEMENT* oldHeadPtr;
        EXPONENTIAL_BACKOFF< STATS > backoff(1, _stats);
        do
        {
            PTRINT oldHeadBare;

            // Store our unique ID in the low-order bits of the head pointer.  This avoids the "A-B-A"
            // problem below.  It's possible that this will overwrite someone else's unique ID, but that's OK.
            do
            {
                backoff.Delay();
                oldHead = OPS::Load(&_head);
                if (!oldHead)
                {
                    _idGenerator.ReleaseID(myID);
                    if (isEmpty) *isEmpty = true;
                    return 0;
                }

                oldHeadBare = (oldHead >> LowBits) << LowBits;
                midHead     = oldHeadBare | myID;
            }
            while (!OPS::CompareAndDidSwap(&_head, oldHead, midHead));

            // Read through the head pointer to get the second element on the list, which will be the
            // new head if the Pop() succeeds.
            //
            // Note, the unique ID avoids an "A-B-A" problem here.  It's possible that another thread
            // will pop off the head, change the list, and then push the same head back on.  If we just
            // did CAS to check that (oldHead == _head), the check would incorrectly succeed.  The presence
            // of the unique ID in the head ensures that the CAS below will fail if another thread changes
            // head and then pushes it back on.
            //
            oldHeadPtr = reinterpret_cast< ELEMENT* >(oldHeadBare);
            newHead    = reinterpret_cast< PTRINT >(oldHeadPtr->_next);
        }
        while (!OPS::CompareAndDidSwap(&_head, midHead, newHead, BARRIER_CS_NEXT));

        // BARRIER_CS_NEXT above ensures that all processors see that the element is removed from
        // the queue before the consumer uses the element.

        _idGenerator.ReleaseID(myID);
        return oldHeadPtr;
    }

    /*!
     * Pop an element off the head of the lifo queue.
     *
     * This method may fail (return NULL) if there are too many simultaneous callers to the Pop()
     * method.  The \a isEmpty parameter can be used to distinguish this failure from an empty
     * queue.  When Pop() fails due to high contention, the caller may want to wait and call Pop()
     * again in hopes that some other thread returns from its call to Pop().  The maximum number
     * of simultaneous callers to Pop() is (2^LowBits - 1) where \e LowBits is the template parameter
     * to LIFO_PTR.
     *
     * The \a maxRetries parameter can be used to automatically retry the "pop" operation if there
     * are too many simultaneous callers.  The default is zero, which means that no retries are
     * attempted if the pop fails when there are too many simultaneous callers.  If the \a maxRetries
     * parameter is greater than zero, an exponential backoff is inserted between each "pop" attempt.
     *
     * Note that even a single thread can exceed the simultaneous caller limit.  For example, a Unix
     * signal could interrupt one Pop() call and the signal handler could reentrantly call into Pop()
     * again.  This is legal, but each interrupted Pop() call counts towards the simultaneous caller
     * limit.  As a result, it could be dangerous to specify a very high value for \a maxRetries if a
     * single thread could reentrantly call into Pop() many times.
     *
     *  @param[out] isEmpty     If Pop() returns NULL and \a isEmpty is not NULL, the \a isEmpty
     *                           parameter is written with TRUE if the Pop() fails because the
     *                           queue is empty or with FALSE if the Pop() fails because of
     *                           contention.
     *  @param[in] maxRetries   Tells the number of times to retry the "pop" operation if there
     *                           are too many simultaneous callers.
     *
     * @return  The popped element on success.  The return value is NULL if the queue is empty
     *           or if there are too many simultaneous callers to Pop().
     */
    ELEMENT* Pop(bool* isEmpty = 0, unsigned maxRetries = 0)
    {
        bool isEmptyLocal;
        bool* isEmptyPtr = isEmpty ? isEmpty : &(isEmptyLocal);
        ELEMENT* element = PopInternal(isEmptyPtr);
        if (element)
        {
            return (element);
        }
        EXPONENTIAL_BACKOFF< STATS > backoff(1, _stats);
        unsigned int numTried = 0;
        while (!element && !(*isEmptyPtr) && (numTried < maxRetries))
        {
            numTried++;
            backoff.Delay();
            element = PopInternal(isEmptyPtr);
            if (element)
            {
                return (element);
            }
        }
        return (0);
    }

    /*!
     * @return  Returns the first element on the queue, or NULL if it is empty.
     */
    ELEMENT* Head()
    {
        PTRINT head = OPS::Load(&_head);
        head        = (head >> LowBits) << LowBits;
        return reinterpret_cast< ELEMENT* >(head);
    }

    /*!
     * @return  Returns the first element on the queue, or NULL if it is empty.
     */
    const ELEMENT* Head() const { return const_cast< LIFO_PTR* >(this)->Head(); }

    /*!
     * Atomically clears the lifo queue and returns a pointer to the previous contents.
     *
     * @return  Returns a pointer to a linked list with the previous elements in
     *           in the queue, or NULL if the queue was already empty.
     */
    ELEMENT* Clear()
    {
        PTRINT oldHead;
        EXPONENTIAL_BACKOFF< STATS > backoff(1, _stats);

        do
        {
            backoff.Delay();
            oldHead = OPS::Load(&_head);
        }
        while (!OPS::CompareAndDidSwap< PTRINT >(&_head, oldHead, 0, BARRIER_CS_NEXT));

        // BARRIER_CS_NEXT above ensures that all processors see that the elements are
        // removed from the list before the caller starts changing them.

        oldHead = (oldHead >> LowBits) << LowBits;
        return reinterpret_cast< ELEMENT* >(oldHead);
    }

    /*!
     * Set the contents of the lifo queue to a singly-linked list of elements.  This method
     * is NOT atomic.
     *
     *  @param[in] list     A list of ELEMENTs linked through their _next pointers.  The
     *                       last element's _next pointer must be NULL.
     */
    void AssignNonAtomic(ELEMENT* list)
    {
        ATOMIC_CHECK_ASSERTSLOW(CheckList(list, 0));

        _head = reinterpret_cast< PTRINT >(list);
    }

  private:
    /*
     * Validate an input list that will be pushed onto the fifo.  The input list is assumed
     * to be private to the calling thread.
     *
     *  @param[in] head     The head of the list.
     *  @param[in] tail     If not NULL, the tail of the list.
     *
     * @return  Return TRUE if all elements have their LowBits clear.
     */
    bool CheckList(ELEMENT* head, ELEMENT* tail)
    {
        ELEMENT* last = 0;
        for (ELEMENT* el = head; el; el = el->_next)
        {
            PTRINT intEl = reinterpret_cast< PTRINT >(el);
            if (((intEl >> LowBits) << LowBits) != intEl) return false;
            last = el;
        }

        if (tail && tail != last) return false;
        return true;
    }

  private:
    volatile PTRINT _head; // The head of the list

    // We use the low-order bits of _head to hold a unique ID (see Pop() method).  This object
    // allows us to generate small, unique IDs that will fit in the low-order bits.
    //
    static const UINT32 MaxID = (1 << LowBits) - 1;
    IDSET< MaxID, STATS > _idGenerator;

    STATS* _stats; // Object which collects statistics, or NULL
};

} // namespace ATOMIC
#endif // file guard
