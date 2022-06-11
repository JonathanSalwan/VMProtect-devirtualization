/*
 * Copyright (C) 2006-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

// <COMPONENT>: atomic
// <FILE-TYPE>: component public header

#ifndef ATOMIC_LIFO_CTR_HPP
#define ATOMIC_LIFO_CTR_HPP

#include "atomic/config.hpp"
#include "atomic/ops.hpp"
#include "atomic/nullstats.hpp"
#include "atomic/exponential-backoff.hpp"

namespace ATOMIC
{
/*! @brief  Last-in-first-out queue.
 *
 * A non-blocking atomic LIFO queue (stack) of elements.  The client manages the allocation, deallocation, and content
 * of each element in the queue.  This implementation uses an algorithm from R. K. Treiber to solve the "A-B-A problem".
 *
 *  @param ELEMENT      The type of each element in the queue.  This type must include a field named "_next",
 *                       which is of type "ELEMENT * volatile".
 *  @param HEAP         Each element must be associated with a unique numeric "index".  For example, the
 *                       client may store the elements in an array, so the array index could be used as
 *                       an element index.  The index value 0 must be reserved to mean "no element" (like
 *                       a NULL pointer).  The \a HEAP parameter is the type of an object that can be
 *                       used to convert between indices and pointers.  It must define the following
 *                       methods:
 *                                                                                          \code
 *                          ELEMENT *Pointer(WORD index);
 *                          WORD Index(const ELEMENT *) const;
 *                                                                                          \endcode
 *  @param IndexBits    Tells how many bits are reserved in the queue to hold an element index.  Clearly,
 *                       this places an upper limit on the total number of elements that may be added to the
 *                       queue.
 *  @param CounterBits  The queue contains a counter field that is used to maintain atomicity.  The counter
 *                       is incremented each time a thread modifies the head of the queue.  If a thread
 *                       starts to modify the queue and then stalls, it must wake up again before the other
 *                       threads increment the counter 2^CounterBits times (thus, wrapping the counter).
 *                       For this reason, \a CounterBits should be fairly large (typically 32 bits).
 *  @param WORD         An integral type which is at least as large as \a IndexBits + \a CounterBits.
 *                       Typically, this is UINT64.
 *  @param STATS        Type of an object that collects statistics.  See NULLSTATS for a model.
 *
 * @par Example:
 *                                                                                          \code
 *  #include "atomic/lifo-ctr.hpp"
 *
 *  struct MyElement
 *  {
 *      MyElement * volatile _next;
 *      unsigned _myMember;
 *  };
 *
 *  ATOMIC::LIFO_CTR<MyElement, MyHeap, 32, 32, FUND::UINT64> Queue(&Heap);
 *
 *  void Foo(MyElement *el)
 *  {
 *      Queue.Push(el);
 *      el = Queue.Pop();
 *  }
 *                                                                                          \endcode
 */
template< typename ELEMENT, typename HEAP, unsigned int IndexBits, unsigned int CounterBits, typename WORD,
          typename STATS = NULLSTATS >
class /*<UTILITY>*/ LIFO_CTR
{
  public:
    /*!
     * Construct a new (empty) lifo queue.
     *
     *  @param[in] heap     An object which converts between element indices and pointers.
     *  @param[in] stats    The statistics collection object, or NULL if no statistics should be collected.
     */
    LIFO_CTR(HEAP* heap, STATS* stats = 0) : _heap(heap), _stats(stats)
    {
        ATOMIC_CHECK_ASSERT(sizeof(_head) == sizeof(_head._word));

        _head._word = 0;
    }

    /*!
     * Set the statistics collection object.  This method is NOT atomic.
     *
     *  @param[in] stats    The new statistics collection object.
     */
    void SetStatsNonAtomic(STATS* stats) { _stats = stats; }

    /*!
     * Push an element onto the head of the lifo queue.
     *
     *  @param[in] element  The element to push.
     */
    void Push(ELEMENT* element)
    {
        HEAD oldHead;
        HEAD newHead;
        EXPONENTIAL_BACKOFF< STATS > backoff(1, _stats);
        do
        {
            backoff.Delay();

            oldHead._word             = OPS::Load(&_head._word);
            element->_next            = _heap->Pointer(oldHead._fields._iElement);
            newHead._fields._iElement = _heap->Index(element);
            newHead._fields._counter  = oldHead._fields._counter + 1;

            // BARRIER_CS_PREV below ensures that all processors will see the write to _next
            // before the element is inserted into the queue.
        }
        while (!OPS::CompareAndDidSwap(&_head._word, oldHead._word, newHead._word, BARRIER_CS_PREV));
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
        HEAD oldHead;
        HEAD newHead;
        EXPONENTIAL_BACKOFF< STATS > backoff(1, _stats);
        do
        {
            backoff.Delay();

            oldHead._word             = OPS::Load(&_head._word);
            listTail->_next           = _heap->Pointer(oldHead._fields._iElement);
            newHead._fields._iElement = _heap->Index(listHead);
            newHead._fields._counter  = oldHead._fields._counter + 1;

            // BARRIER_CS_PREV below ensures that all processors will see the write to _next
            // before the element is inserted into the queue.
        }
        while (!OPS::CompareAndDidSwap(&_head._word, oldHead._word, newHead._word, BARRIER_CS_PREV));
    }

    /*!
     * Pop an element off the head of the lifo queue.
     *
     *  @param[out] isEmpty     If Pop() returns NULL and \a isEmpty is not NULL, the
     *                           \a isEmpty parameter is set to TRUE.  This parameter
     *                           is not particularly useful, but it is provided for
     *                           symetry with LIFO_PTR::Pop().
     *  @param[in] maxRetries   This parameter is ignored, but it is provided for
     *                           symetry with LIFO_PTR::Pop().
     *
     * @return  Returns the popped element on success, or NULL if the queue is empty.
     */
    ELEMENT* Pop(bool* isEmpty = 0, unsigned maxRetries = 0)
    {
        HEAD oldHead;
        HEAD newHead;
        ELEMENT* element;
        EXPONENTIAL_BACKOFF< STATS > backoff(1, _stats);

        do
        {
            backoff.Delay();

            oldHead._word = OPS::Load(&_head._word);
            if (oldHead._fields._iElement == 0)
            {
                if (isEmpty) *isEmpty = true;
                return 0;
            }

            element                   = _heap->Pointer(oldHead._fields._iElement);
            newHead._fields._iElement = _heap->Index(element->_next);
            newHead._fields._counter  = oldHead._fields._counter + 1;
        }
        while (!OPS::CompareAndDidSwap(&_head._word, oldHead._word, newHead._word, BARRIER_CS_NEXT));

        // BARRIER_CS_NEXT above ensures that all processors see that the element is removed from
        // the queue before the consumer uses the element.

        return element;
    }

    /*!
     * @return  Returns the first element on the queue, or NULL if it is empty.
     */
    ELEMENT* Head()
    {
        HEAD head;
        head._word = OPS::Load(&_head._word);
        return _heap->Pointer(head._fields._iElement);
    }

    /*!
     * @return  Returns the first element on the queue, or NULL if it is empty.
     */
    const ELEMENT* Head() const { return const_cast< LIFO_CTR* >(this)->Head(); }

    /*!
     * Atomically clears the lifo queue and returns a pointer to the previous contents.
     *
     * @return  Returns a pointer to a linked list with the previous elements in
     *           in the queue, or NULL if the queue was already empty.
     */
    ELEMENT* Clear()
    {
        HEAD oldHead;
        HEAD newHead;
        EXPONENTIAL_BACKOFF< STATS > backoff(1, _stats);

        newHead._fields._iElement = 0;
        do
        {
            backoff.Delay();
            oldHead._word            = OPS::Load(&_head._word);
            newHead._fields._counter = oldHead._fields._counter + 1;
        }
        while (!OPS::CompareAndDidSwap(&_head._word, oldHead._word, newHead._word, BARRIER_CS_NEXT));

        // BARRIER_CS_NEXT above ensures that all processors see that the elements are
        // removed from the list before the caller starts changing them.

        return _heap->Pointer(oldHead._fields._iElement);
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
        HEAD newHead;
        newHead._word             = 0;
        newHead._fields._iElement = _heap->Index(list);
        _head._word               = newHead._word;
    }

  private:
    // This is the head of the lifo queue.
    //
    // NOTE: This is defined as a union with _word to avoid a compiler bug.
    //       See the SVN commit log r3543 for more details.
    //
    union HEAD
    {
        WORD _word;
        struct
        {
            WORD _iElement : IndexBits;  // Index of first ELEMENT in queue
            WORD _counter : CounterBits; // Modification counter (solves A-B-A problem)
        } _fields;
    };
    volatile HEAD _head;

    HEAP* _heap;   // Heap containing objects which are in the queue
    STATS* _stats; // Object which collects statistics, or NULL
};

} // namespace ATOMIC
#endif // file guard
