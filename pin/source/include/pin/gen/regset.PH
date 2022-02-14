/*
 * Copyright (C) 2003-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#ifdef PIN_G_REGSET_PH
#error duplicate inclusion of regset
#else
#define PIN_G_REGSET_PH
/*! @file
    x
*/
/*! @ingroup REG
 * 
 */
template< UINT32 _min, UINT32 _max > class REGISTER_SET
{
  private:
    static const UINT32 _SET_SIZE  = sizeof(ADDRINT) * 8; // # of bits in each _set element
    static const UINT32 _NUM_SETS  = ((_max - _min) / _SET_SIZE) + 1;
    static const ADDRINT _ONE      = ADDRINT(1);
    static const ADDRINT _ALL_ONES = ~ADDRINT(0);

    ADDRINT _set[_NUM_SETS];

  public:
    REGISTER_SET()
    {
        for (UINT32 k = 0; k < _NUM_SETS; k++)
        {
            _set[k] = 0;
        }
    }

    REGISTER_SET(UINT32 first, UINT32 last)
    {
        for (UINT32 k = 0; k < _NUM_SETS; k++)
        {
            _set[k] = 0;
        }
        for (UINT32 r = first; r <= last; r++)
        {
            Insert(REG(r));
        }
    }

    REG Min() const { return REG(_min); }
    REG Max() const { return REG(_max); }

    BOOL Contains(REG reg_no) const
    {
        const UINT32 i = UINT32(reg_no);
#if defined(__INTEL_COMPILER)
#pragma warning(push)
#pragma warning(disable : 186)
#endif
        ASSERTX(_min <= i && i <= _max);
#if defined(__INTEL_COMPILER)
#pragma warning(pop)
#endif
        const UINT32 set_no = (i - _min) / _SET_SIZE;
        const UINT32 pos_no = (i - _min) % _SET_SIZE;
        return ((_set[set_no] & (_ONE << pos_no)) != 0);
    }

    VOID InsertVector(REG reg_no, UINT32 vector)
    {
        const UINT32 i = UINT32(reg_no);
#if defined(__INTEL_COMPILER)
#pragma warning(push)
#pragma warning(disable : 186)
#endif
        ASSERTX(_min <= i && i <= _max);
#if defined(__INTEL_COMPILER)
#pragma warning(pop)
#endif
        const UINT32 set_no = (i - _min) / _SET_SIZE;
        ASSERTX(0 == (i - _min) % _SET_SIZE);
        _set[set_no] = vector;
    }

    VOID InsertAll()
    {
        for (UINT32 k = 0; k < _NUM_SETS; k++)
        {
            _set[k] = _ALL_ONES;
        }
    }

    VOID Insert(REG reg_no)
    {
        const UINT32 i = UINT32(reg_no);
#if defined(__INTEL_COMPILER)
#pragma warning(push)
#pragma warning(disable : 186)
#endif
        if (KnobSlowAsserts)
        {
            ASSERT(_min <= i && i <= _max, "_min(" + decstr(_min) + ") i(" + decstr(i) + ") _max( " + decstr(_max) + ")\n");
        }
#if defined(__INTEL_COMPILER)
#pragma warning(pop)
#endif
        const UINT32 set_no = (i - _min) / _SET_SIZE;
        const UINT32 pos_no = (i - _min) % _SET_SIZE;

        _set[set_no] |= (_ONE << pos_no);
    }

    VOID Remove(REG reg_no)
    {
        const UINT32 i = UINT32(reg_no);
#if defined(__INTEL_COMPILER)
#pragma warning(push)
#pragma warning(disable : 186)
#endif
        ASSERTX(_min <= i && i <= _max);
#if defined(__INTEL_COMPILER)
#pragma warning(pop)
#endif
        const UINT32 set_no = (i - _min) / _SET_SIZE;
        const UINT32 pos_no = (i - _min) % _SET_SIZE;

        _set[set_no] &= ~(_ONE << pos_no);
    }

    VOID Clear()
    {
        for (UINT32 k = 0; k < _NUM_SETS; k++)
        {
            _set[k] = 0;
        }
    }

    VOID Invert()
    {
        for (UINT32 k = 0; k < _NUM_SETS; k++)
        {
            _set[k] = ~_set[k];
        }
    }

    REG PopNext()
    {
        for (UINT32 i = _min; i <= _max; i++)
        {
            const REG reg = REG(i);
            if (Contains(reg))
            {
                Remove(reg);
                return reg;
            }
        }

        return REG_INVALID();
    }

    REG PopNextFast()
    {
        for (UINT32 s = 0; s < _NUM_SETS; s++)
        {
            if (_set[s] == 0) continue;

            const UINT32 limit = MIN(_max, (s + 1) * _SET_SIZE);

            for (UINT32 i = _min + (s * _SET_SIZE); i <= limit; i++)
            {
                const REG reg = REG(i);
                if (Contains(reg))
                {
                    Remove(reg);
                    return reg;
                }
            }
        }

        return REG_INVALID();
    }

    UINT32 PopCount() const
    {
        UINT32 count = 0;
        for (UINT32 s = 0; s < _NUM_SETS; s++)
        {
            count += BitCount(_set[s]);
        }
        return count;
    }

    BOOL PopCountIsZero() const
    {
        for (UINT32 s = 0; s < _NUM_SETS; s++)
        {
            if (_set[s] != 0) return FALSE;
        }
        return TRUE;
    }

    UINT32 HashKey() const
    {
        UINT32 hashKey = 0;
        for (UINT32 s = 0; s < _NUM_SETS; s++)
        {
            hashKey = HashData(hashKey, _set[s]);
        }
        return hashKey;
    }

    REGISTER_SET operator|(const REGISTER_SET& b) const
    {
        REGISTER_SET c;

        for (UINT32 i = 0; i < _NUM_SETS; i++)
        {
            c._set[i] = _set[i] | b._set[i];
        }
        return c;
    }

    REGISTER_SET& operator|=(const REGISTER_SET& b)
    {
        for (UINT32 i = 0; i < _NUM_SETS; i++)
        {
            _set[i] |= b._set[i];
        }
        return *this;
    }

    int operator==(const REGISTER_SET& b) const
    {
        for (UINT32 i = 0; i < _NUM_SETS; i++)
        {
            if (_set[i] != b._set[i]) return FALSE;
        }
        return TRUE;
    }

    int operator!=(const REGISTER_SET& b) const
    {
        for (UINT32 i = 0; i < _NUM_SETS; i++)
        {
            if (_set[i] != b._set[i]) return TRUE;
        }
        return FALSE;
    }

    REGISTER_SET operator&(const REGISTER_SET& b) const
    {
        REGISTER_SET c;

        for (UINT32 i = 0; i < _NUM_SETS; i++)
        {
            c._set[i] = _set[i] & b._set[i];
        }
        return c;
    }

    REGISTER_SET& operator&=(const REGISTER_SET& b)
    {
        for (UINT32 i = 0; i < _NUM_SETS; i++)
        {
            _set[i] &= b._set[i];
        }
        return *this;
    }

    REGISTER_SET operator-(const REGISTER_SET& b) const
    {
        REGISTER_SET c;

        for (UINT32 i = 0; i < _NUM_SETS; i++)
        {
            c._set[i] = _set[i] & ~b._set[i];
        }
        return c;
    }

    REGISTER_SET& operator-=(const REGISTER_SET& b)
    {
        for (UINT32 i = 0; i < _NUM_SETS; i++)
        {
            _set[i] &= ~b._set[i];
        }
        return *this;
    }

    STATIC inline std::string PrintRange(REG start, REG stop)
    {
        if (start == stop)
        {
            return REG_StringShort(start);
        }
        else
        {
            return REG_StringShort(start) + "-" + REG_StringShort(stop);
        }
    }

    std::string Dump() const
    {
        std::string s("{");
        for (UINT32 i = 0; i < _NUM_SETS; i++)
        {
            if (i != 0) s += ",";
            s += hexstr(_set[i]);
        }
        s += "}";
        return s;
    }

    std::string String() const
    {
        std::string s;

        s += "{";
        BOOL printed_range = FALSE;

        for (UINT32 i = _min; i <= _max; i++)
        {
            const REG reg = REG(i);

            if (Contains(reg))
            {
                if (printed_range) s += ",";

                s += PrintRange(reg, reg);
                printed_range = TRUE;
            }
        }

        s += "}";

        return s;
    }

    std::string StringList() const
    {
        std::string s;

        s += "{";

        BOOL first = TRUE;

        for (UINT32 i = _min; i <= _max; i++)
        {
            const REG reg = REG(i);

            if (Contains(reg))
            {
                if (first)
                {
                    first = FALSE;
                }
                else
                {
                    s += ",";
                }
                s += REG_StringShort(reg);
            }
        }

        s += "}";

        return s;
    }
};

#endif // PIN_G_REGSET_PH
