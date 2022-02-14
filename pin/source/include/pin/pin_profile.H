/*
 * Copyright (C) 2004-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 *  This file contains support for stats counters on sparse input ranges.
 */

#ifndef PIN_PROFILE_H
#define PIN_PROFILE_H

#include <map>
#include <vector>
#include <cassert>

/*!
 *  Class to map arbitrary sequences of sparse input values to
 *  a range of compact indices [0..N],
 *  such that the same input value always produces the same index.
 */
template< class KEY, class INDEX > class COMPRESSOR
{
  protected:
    typedef std::pair< const KEY, INDEX > PAIR;
    typedef std::map< KEY, INDEX > MAP;

    MAP _map;
    INDEX _nextIndex;
    std::string _keyName;

  public:
    // constructors/destructors
    COMPRESSOR() { _nextIndex = 0; }

    // accessors
    std::string StringLong() const
    {
        std::string os;

        os += "COMPRESSOR BEGIN\n";
        os += "# " + _nextIndex.str() + " counters\n";
        os += "# " + _keyName + ": index\n";
        for (typename MAP::const_iterator it = _map.begin(); it != _map.end(); it++)
        {
            os += it->first.str() + ": " + decstr(it->second, 12) + "\n";
        }
        os += "COMPRESSOR END\n";

        return os;
    }

    // modifiers
    VOID SetKeyName(const std::string& keyName) { _keyName = keyName; }

    INDEX Map(KEY key)
    {
        typename MAP::const_iterator it = _map.find(key);

        if (it != _map.end())
        {
            // key found: return index
            return it->second;
        }
        else
        {
            // key not yet present: insert and return new index
            const PAIR p(key, _nextIndex);
            _map.insert(p);

            return _nextIndex++;
        }
    }
};

/*!
 *  Class to provide a counter for each compresses index. Counters are
 *  accessed similar to standard library classes with array syntax [] for
 *  unchecked accesses, and with at() for range-checked accesses. The
 *  array of counters is auto-extending as necessary and is guaranteed to
 *  contain as many entries as have been mapped.
 */

template< class KEY, class INDEX, class COUNTER > class COMPRESSOR_COUNTER : public COMPRESSOR< KEY, INDEX >
{
  private:
    typedef std::vector< COUNTER > VECTOR;
    static const UINT32 defaultInitCounterSize = 8 * 1024;

    VECTOR _counters;
    std::string _counterName;
    COUNTER _threshold;

  public:
    // constructors/destructors
    COMPRESSOR_COUNTER(UINT32 initCounterSize = defaultInitCounterSize) : COMPRESSOR< KEY, INDEX >(), _counters(initCounterSize)
    {}

    // accessors
    std::string StringLong() const
    {
        std::string os;

        INDEX num_counters = 0;

        for (typename COMPRESSOR< KEY, INDEX >::MAP::const_iterator it = this->_map.begin(); it != this->_map.end(); it++)
        {
            const COUNTER& counter = _counters[it->second];

            if (_threshold <= counter) num_counters++;
        }

        os += "NumItems " + decstr(num_counters) + "\n";
        os += "DATA:START\n";
        os += "#  counters\n";
        os += "# " + this->_keyName + ": " + _counterName + "\n";

        for (typename COMPRESSOR< KEY, INDEX >::MAP::const_iterator it = this->_map.begin(); it != this->_map.end(); it++)
        {
            const COUNTER& counter = _counters[it->second];
            if (_threshold <= counter)
            {
                os += hexstr(it->first, 8) + ": " + counter.str() + "\n";
            }
        }
        os += "DATA:END\n";

        return os;
    }

    // modifiers
    VOID SetCounterName(const std::string& counterName) { _counterName = counterName; }

    VOID SetThreshold(const COUNTER& threshold) { _threshold = threshold; }

    INDEX Map(KEY key)
    {
        // use compressor to map
        const INDEX Idx = COMPRESSOR< KEY, INDEX >::Map(key);

        // ... and check if need to add more counters
        if (Idx + 1 >= _counters.capacity())
        {
            _counters.reserve(2 * _counters.capacity());
        }

        return Idx;
    }

    const COUNTER& operator[](INDEX index) const { return _counters[index]; }
    COUNTER& operator[](INDEX index) { return _counters[index]; }

    const COUNTER& at(INDEX index) const { return _counters.at(index); }
    COUNTER& at(INDEX index) { return _counters.at(index); }
};

/*!
 *  Class to provide an array of counters for use with COMPRESSOR_COUNTER
 *  if more than a single counter is required.
 *  Counters are accessed similar to standard library classes with array
 *  syntax [] for unchecked accesses, and with at() for range-checked
 *  accesses. The array of counters is auto-extending as necessary and is
 *  guaranteed to contain as many entries as have been mapped.
 */
template< class NUMTYPE, UINT32 NUM_COUNTERS > class COUNTER_ARRAY
{
  private:
    NUMTYPE _counters[NUM_COUNTERS];

  public:
    // accessors
    std::string str() const
    {
        std::string os;

        for (UINT32 i = 0; i < NUM_COUNTERS; i++)
        {
            if (i != 0) os += " ";
            os += decstr(_counters[i], 12);
        }

        return os;
    }

    // allow compare to 0
    bool operator==(const COUNTER_ARRAY& x) const
    {
        for (UINT32 i = 0; i < NUM_COUNTERS; i++)
        {
            if (_counters[i] != x._counters[i]) return false;
        }

        return true;
    }
    bool operator!=(const COUNTER_ARRAY& x) const { return !operator==(x); }

    bool operator<=(const COUNTER_ARRAY& x) const
    {
        for (UINT32 i = 0; i < NUM_COUNTERS; i++)
        {
            if (_counters[i] > x._counters[i]) return false;
        }

        return true;
    }

    // modifiers
    const NUMTYPE& operator[](UINT32 index) const { return _counters[index]; }
    NUMTYPE& operator[](UINT32 index) { return _counters[index]; }

    const NUMTYPE& at(UINT32 index) const
    {
        assert(index < NUM_COUNTERS);
        return _counters[index];
    }

    NUMTYPE& at(UINT32 index)
    {
        assert(index < NUM_COUNTERS);
        return _counters[index];
    }
};

#define PROFILE(n) COMPRESSOR_COUNTER< ADDRINT, UINT32, COUNTER_ARRAY< UINT32, n > >

#endif // PIN_PROFILE_H
