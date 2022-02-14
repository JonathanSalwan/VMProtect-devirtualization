/*
 * Copyright (C) 2020-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#ifdef PIN_G_UTIL_PH
#error duplicate inclusion of util
#else
#define PIN_G_UTIL_PH
/*! @file
 * This module contains useful utility functions.
 */

/*! @ingroup UTILS
  ctype::isspace alternative (avoids complications from including ctype.h).
*/
extern BOOL CharIsSpace(CHAR c);

/*! @ingroup UTILS
    ctype::toupper alternative (avoids complications from including ctype.h).
*/
extern CHAR CharToUpper(CHAR c);

/*! @ingroup UTILS
  Print pointer.
 */
extern std::string ptrstr(const VOID* val);

/*! @ingroup UTILS
  Convert a ADDRINT into a string using the hex address format.
 */
extern std::string StringFromAddrint(ADDRINT l);

/*! @ingroup UTILS
  Convert a UINT64 into a string using the hex address format.
 */
extern std::string StringFromUint64(UINT64 l);

/*! @ingroup UTILS
  Convert a UINT64 into a dec string. Padding can be specified as well.
 */
extern std::string StringDec(UINT64 l, UINT32 digits, CHAR padding);

/*! @ingroup UTILS
  Convert a INT64 into a dec string. Padding can be specified as well.
 */
extern std::string StringDecSigned(INT64 l, UINT32 digits, CHAR padding);

/*! @ingroup UTILS
  Convert a INT64 into a dec string with 1000 separation . Padding can be specified as well.
 */
extern std::string StringBignum(INT64 l, UINT32 digits, CHAR padding);

/*! @ingroup UTILS
  Add new lines to string to make it fit given line width restrictions.
 */
extern std::string Reformat(const std::string& s, const std::string& prefix, UINT32 min_line, UINT32 max_line);

/*! @ingroup UTILS
  Convert a UINT32 into a hex string. Padding can be specified as well.
 */
extern std::string StringHex32(UINT32 l, UINT32 digits, BOOL prefix_0x);

/*! @ingroup UTILS
  Convert a BOOL into a string.
 */
extern std::string StringBool(BOOL b);

/*! @ingroup UTILS
  Convert a TRI into a string.
 */
extern std::string StringTri(TRI t);

/*! @ingroup UTILS
  Convert a string to an INT32.
 */
extern INT32 Int32FromString(const std::string& s);

/*! @ingroup UTILS
  Convert a string to a UINT32.
 */
extern UINT32 Uint32FromString(const std::string& s);

/*! @ingroup UTILS
  Convert a string to an INT64.
 */
extern INT64 Int64FromString(const std::string& s);

/*! @ingroup UTILS
  Convert a string to a UINT64.
 */
extern UINT64 Uint64FromString(const std::string& s);

/*! @ingroup UTILS
  Convert a string to a FLT64.
 */
extern FLT64 FLT64FromString(const std::string& s);

/*! @ingroup UTILS
 * Convert specified character to the corresponding hexadecimal digit.
 * @return integer in the interval [0, 15] or -1 upon failure
 */
extern INT CharToHexDigit(CHAR c);

/*! @ingroup UTILS
  Convert a string to an ADDRINT.
 */
extern ADDRINT AddrintFromString(const std::string& str);

/*! @ingroup UTILS
  Read a line from file while maintaining a current line count
  skipping over blank and comment lines.
 */
extern std::string ReadLine(std::istream& inputFile, UINT32* lineNum);

/*! @ingroup UTILS
  Create a string with a hexadecimal prefix containing the given hexadecimal integer.
 */
inline std::string StringHex(UINT32 l, UINT32 digits, BOOL prefix_0x = TRUE) { return StringHex32(l, digits, prefix_0x); }

/*! @ingroup UTILS
  Create a string containing the given decimal integer.
 */
inline std::string decstr(INT64 val, UINT32 width = 0) { return StringDecSigned(val, width, ' '); }

/*! @ingroup UTILS
  Create a string containing the given decimal integer.
 */
inline std::string decstr(INT32 val, UINT32 width = 0) { return StringDecSigned(val, width, ' '); }

/*! @ingroup UTILS
  Create a string containing the given decimal integer.
 */
inline std::string decstr(INT16 val, UINT32 width = 0) { return StringDecSigned(val, width, ' '); }

/*! @ingroup UTILS
  Create a string containing the given decimal integer.
 */
inline std::string decstr(UINT64 val, UINT32 width = 0) { return StringDec(val, width, ' '); }

/*! @ingroup UTILS
  Create a string containing the given decimal integer.
 */
inline std::string decstr(UINT32 val, UINT32 width = 0) { return StringDec(val, width, ' '); }

/*! @ingroup UTILS
  Create a string containing the given decimal integer.
 */
inline std::string decstr(UINT16 val, UINT32 width = 0) { return StringDec(val, width, ' '); }

/*! @ingroup UTILS
  Create a string with a hexadecimal prefix containing the given hexadecimal integer.
 */
inline std::string hexstr(INT64 val, UINT32 width = 0)
{
    std::string ostr;
#if defined(_MSC_VER) && _MSC_VER >= 1400
    ostr = StringHex(INT32((val >> 16) >> 16), width);
#else
    ostr = StringHex(INT32(val >> 32), width);
#endif
    ostr += StringHex(UINT32(val), 8, FALSE);

    return ostr;
}

/*! @ingroup UTILS
  Create a string with a hexadecimal prefix containing the given hexadecimal integer.
 */
inline std::string hexstr(INT32 val, UINT32 width = 0) { return StringHex(INT32(val), width); }

/*! @ingroup UTILS
  Create a string with a hexadecimal prefix containing the given hexadecimal integer.
 */
inline std::string hexstr(INT16 val, UINT32 width = 0) { return StringHex(INT32(val), width); }

/*! @ingroup UTILS
  Create a string with a hexadecimal prefix containing the given hexadecimal integer.
 */
inline std::string hexstr(UINT64 val, UINT32 width = 0)
{
    std::string ostr;
#if defined(_MSC_VER) && _MSC_VER >= 1400
    ostr = StringHex(UINT32((val >> 16) >> 16), width);
#else
    ostr = StringHex(UINT32(val >> 32), width);
#endif
    ostr += StringHex(UINT32(val), 8, FALSE);

    return ostr;
}

/*! @ingroup UTILS
  Create a string with a hexadecimal prefix containing the given hexadecimal integer.
 */
inline std::string hexstr(VOID* p, UINT32 width = 0)
{
#if defined(HOST_IA32E)
    UINT64 val = reinterpret_cast< UINT64 >(p);
    return hexstr(val, width);
#else
    UINT32 val = reinterpret_cast< UINT32 >(p);
    return StringHex(val, width);
#endif
}

/*! @ingroup UTILS
  Create a string with a hexadecimal prefix containing the given hexadecimal integer.
 */
inline std::string hexstr(const VOID* p, UINT32 width = 0)
{
#if defined(HOST_IA32E)
    UINT64 val = reinterpret_cast< UINT64 >(p);
    return hexstr(val, width);
#else
    UINT32 val = reinterpret_cast< UINT32 >(p);
    return StringHex(val, width);
#endif
}

/*! @ingroup UTILS
  Create a string with a hexadecimal prefix containing the given hexadecimal integer.
 */
inline std::string hexstr(UINT32 val, UINT32 width = 0) { return StringHex(UINT32(val), width); }

/*! @ingroup UTILS
  Create a string with a hexadecimal prefix containing the given hexadecimal integer.
 */
inline std::string hexstr(UINT16 val, UINT32 width = 0) { return StringHex(UINT32(val), width); }

/*! @ingroup UTILS
  Left justify string.
 */
inline std::string ljstr(const std::string& s, UINT32 width, CHAR padding = ' ')
{
    std::string ostr(width, padding);
    ostr.replace(0, s.length(), s);
    return ostr;
}

/*! @ingroup UTILS
  Decimal-to-string conversion object.
 */
struct DECSTR
{
    DECSTR(UINT32 width = 0) : _w(width) {}
    template< typename T > std::string operator()(const T& t) const { return decstr(t, _w); }
    UINT32 _w;
};

/*! @ingroup UTILS
  Hexadecimal-to-string conversion object.
 */
struct HEXSTR
{
    HEXSTR(UINT32 width = 0) : _w(width) {}
    template< typename T > std::string operator()(const T& t) const { return hexstr(t, _w); }
    UINT32 _w;
};

/*! @ingroup UTILS
  popcount/bitcount code using the usual trick
 */
extern UINT32 BitCount(ADDRINT val);

/*! @ingroup UTILS
    Convert ADDRINT to "void *"
 */
inline VOID* Addrint2VoidStar(ADDRINT addr)
{
#if defined(HOST_IA32E) && defined(TARGET_IA32)
    ASSERT(false, "Should not be called in cross environment\n");
    return 0;
#else
    return reinterpret_cast< VOID* >(addr);
#endif
}

/*! @ingroup UTILS
  Convert "void *" to ADDRINT.
 */
inline ADDRINT VoidStar2Addrint(const VOID* addr)
{
#if defined(HOST_IA32E) && defined(TARGET_IA32)
    ASSERT(false, "Should not be called in cross environment\n");
    return 0;
#else
    return reinterpret_cast< ADDRINT >(addr);
#endif
}

/*! @ingroup UTILS
  Convert "void *" to ADDRINT
 */
inline ADDRINT VoidStar2Addrint(VOID* addr)
{
#if defined(HOST_IA32E) && defined(TARGET_IA32)
    ASSERT(false, "Should not be called in cross environment\n");
    return 0;
#else
    return reinterpret_cast< ADDRINT >(addr);
#endif
}

/*! @ingroup UTILS
 * Round integer of type <T> up to given alignment.
 */
template< typename T > T RoundUp(T value, size_t alignment)
{
    if (alignment == 0)
    {
        return value;
    }

    value += alignment - 1;
    value /= alignment;
    value *= alignment;

    return value;
}

/*! @ingroup UTILS
 * Specialization of the RoundUp function for pointer type
 */
template< typename T > T* RoundUp(T* ptr, size_t alignment)
{
    return (reinterpret_cast< T* >(RoundUp(reinterpret_cast< ADDRINT >(ptr), alignment)));
}

/*! @ingroup UTILS
 * Round integer of type <T> down to given alignment.
 */
template< typename T > T RoundDown(T value, size_t alignment)
{
    if (alignment == 0)
    {
        return value;
    }

    value /= alignment;
    value *= alignment;

    return value;
}

/*! @ingroup UTILS
 * Specialization of the RoundDown function for pointer type
 */
template< typename T > T* RoundDown(T* ptr, size_t alignment)
{
    return (reinterpret_cast< T* >(RoundDown(reinterpret_cast< ADDRINT >(ptr), alignment)));
}

/*! @ingroup UTILS
    Get the address of the page that addr is in
 */
extern ADDRINT GetPageOfAddr(ADDRINT addr);

#if !defined(TARGET_WINDOWS)

/*! @ingroup UTILS
   Concatenate two strings with a path delimiter
 */
extern std::string Joinpath(std::string s1, std::string s2);

/*! @ingroup UTILS
   Create a temporary file name
 */
extern CHAR* CreateTmpFileName(const CHAR* fnameTemplate, const UINT32 fnameTemplateSize);

#endif

/*! @ingroup UTILS
  @return current stack pointer
*/
extern const VOID* GetSp();

/*! @ingroup UTILS
  Return offset, in bytes, of <ptr1> from <ptr2>
 */
inline size_t PtrDiff(const VOID* ptr1, const VOID* ptr2)
{
    return (reinterpret_cast< const INT8* >(ptr1) - reinterpret_cast< const INT8* >(ptr2));
}

/*! @ingroup UTILS
  Return pointer whose offset, in bytes, from <ptr> is <offset>
 */
inline VOID* PtrAtOffset(VOID* ptr, size_t offset) { return (reinterpret_cast< INT8* >(ptr) + offset); }

/*! @ingroup UTILS
  Return const pointer whose offset, in bytes, from <ptr> is <offset>
 */
inline const VOID* PtrAtOffset(const VOID* ptr, size_t offset) { return (reinterpret_cast< const INT8* >(ptr) + offset); }

/*! @ingroup UTILS
  Return pointer of type <T> whose offset, in bytes, from <ptr> is <offset>
 */
template< typename T > T* PtrAtOffset(VOID* ptr, size_t offset) { return (reinterpret_cast< T* >(PtrAtOffset(ptr, offset))); }

/*! @ingroup UTILS
  Return const pointer of type <T> whose offset, in bytes, from <ptr> is <offset>
 */
template< typename T > const T* PtrAtOffset(const VOID* ptr, size_t offset)
{
    return (reinterpret_cast< const T* >(PtrAtOffset(ptr, offset)));
}

/*! @ingroup UTILS
 * Class that represents a memory range - interval of addresses in the virtual
 * address space.
 * @note   The range that spans the entire address space can not be represented
 *         by this class. Whenever a function of this class should construct such
 *         range (logically), an empty range is returned instead.
 */
class MemRange
{
  public:
    //======= Constructors

    //Default constructor
    MemRange() : m_base(NULL), m_size(0) {}
    //Construct range with a given base address and size
    MemRange(VOID* base, size_t size) : m_base(base), m_size(size) {}
    MemRange(ADDRINT base, size_t size) : m_base(Addrint2VoidStar(base)), m_size(size) {}
    //Construct range with a given base and end addresses.
    //End address of a range is the address of the first byte following the range
    MemRange(VOID* base, VOID* end) : m_base(base), m_size(PtrDiff(end, base)) {}

    //Default copy constructor and assignment operator

    //======= operators == and !=
    BOOL operator==(const MemRange& range) const { return ((m_base == range.m_base) && (m_size == range.m_size)); }
    BOOL operator!=(const MemRange& range) const { return (!(*this == range)); }

    //======= Accessors

    //Get/set base address of the range.
    VOID* Base() const { return m_base; }
    MemRange& Base(VOID* base)
    {
        m_base = base;
        return *this;
    }

    //Get/set size, in bytes, of the range.
    size_t Size() const { return m_size; }
    MemRange& Size(size_t size)
    {
        m_size = size;
        return *this;
    }

    //Get end address of the range.
    VOID* End() const { return PtrAtOffset(m_base, m_size); }

    //Get the last address of the non-empty range.
    VOID* Last() const { return PtrAtOffset(m_base, m_size - 1); }

    /*
    * Check to see whether this range is empty
    *
    * @return  true, if this range is empty, otherwise - false
    */
    BOOL IsEmpty() const { return (m_size == 0); }

    /*
    * Check to see whether this range contains the specified memory address
    *
    * @param[in] addr       memory address, potentially contained in this range
    *
    * @return   true, if this range contains the specified address,
    *           otherwise - false
    */
    BOOL Contains(const VOID* addr) const { return (PtrDiff(addr, m_base) < m_size); }

    /*
    * Check to see whether this range contains the specified memory range
    *
    * @param[in] range      memory range, potentially contained in this range
    *
    * @return   true, if this range contains the specified range,
    *           otherwise - false.
    */
    BOOL Contains(const MemRange& range) const { return (Contains(range.m_base) && !range.Contains(End())); }

    /* Check to see whether this range intersects with the specified memory range
    *
    * @param[in] range      memory range, potentially intersecting with this range
    *
    * @return   true, if this range intersects with the specified range
    *           otherwise - false
    */
    BOOL Intersects(const MemRange& range) const { return (Contains(range.m_base) || range.Contains(m_base)); }

    /* Check to see whether this range is adjacent to the specified memory range
    *
    * @param[in] range      memory range, potentially adjacent to this range
    *
    * @return   true, if this range is adjacent to the specified range
    *           otherwise - false
    */
    BOOL AdjacentTo(const MemRange& range) const
    {
        return (((range.m_base == End()) && (range.m_base != 0)) || ((m_base == range.End()) && (m_base != 0)));
    }

    /* Check to see whether the current stack pointer belongs to this range
    *
    * @return   true, if current stack pointer belongs to this range
    *           otherwise - false
    */
    BOOL IsCurrentStack() const { return (Contains(GetSp())); }

    /*
    * Round base/end address of the range down/up according to specified alignment
    *
    * @param[in] alignment      alignment value. Must be power of two.
    *
    * @return  reference to aligned range.
    */
    MemRange& Align(UINT32 alignment)
    {
        VOID* end = RoundUp(End(), alignment);
        m_base    = RoundDown(m_base, alignment);
        m_size    = PtrDiff(end, m_base);
        return *this;
    }

    /*
    * Round both base and end address of the range up according to specified alignment.
    * The size of the updated range is at least size the range before update.
    * @param[in] alignment      alignment value. Must be power of two.
    *  return  reference to aligned range
    */
    MemRange& AlignUp(UINT32 alignment)
    {
        m_base = RoundUp(m_base, alignment);
        m_size = RoundUp(m_size, alignment);
        return *this;
    }

    /*
    * Round both base and end address of the range down according to specified alignment.
    * The size of the updated range is at least size the range before update.
    * @param[in] alignment      alignment value. Must be power of two.
    * @return  reference to aligned range
    */
    MemRange& AlignDown(UINT32 alignment)
    {
        VOID* end = RoundDown(End(), alignment);
        m_size    = RoundUp(m_size, alignment);
        m_base    = PtrAtOffset(end, 0 - m_size);
        return *this;
    }

    /*
    * Shift both base and end address of the range
    * @param[in] offset         shift value.
    * @return  reference to shifted range
    */
    MemRange& Shift(size_t offset)
    {
        m_base = PtrAtOffset(m_base, offset);
        return *this;
    }

    /*
    * Shift base address of the range without changing the end address
    * If the base is shifted to be after the end address, then the region size
    * will be set to zero
    * @param[in] offset         shift value.
    * @return  reference to shifted range
    */
    MemRange& ShiftBase(size_t offset)
    {
        VOID* end = End();
        m_base    = PtrAtOffset(m_base, offset);
        if (end < m_base)
        {
            m_size = 0;
        }
        else
        {
            m_size = PtrDiff(end, m_base);
        }
        return *this;
    }

    /*
    * Shift end address of the range without changing the base address
    * @param[in] offset         shift value.
    * @return  reference to shifted range
    */
    MemRange& ShiftEnd(size_t offset)
    {
        m_size += offset;
        return *this;
    }

    /*
    * Extend boundaries of this range to include the specified range.
    * The new range is the minimal range that contains both ranges.
    * @param[in] range      memory range, to be combined with this range
    * @return  reference to updated range
    */
    MemRange& Combine(const MemRange& range)
    {
        size_t size;
        if (range.m_base >= m_base)
        {
            size = PtrDiff(range.End(), m_base);
        }
        else
        {
            size  = PtrDiff(End(), range.m_base);
            *this = range;
        }
        if (size > m_size)
        {
            m_size = size;
        }
        return *this;
    }

    /*
    * Decompose this range into three ranges (each of them may be empty):
    * @param[out]  pIntersection - intersection of <this> range with specified <range>
    * @param[out]  pLowDiff      - part of <this> range, that contains all addresses
    *                              lower than any address in the specified <range>
    * @param[out]  pHighDiff     - part of <this> range, that contains all addresses
    *                              higher than any address in the specified <range>
    * @param[in]   range         - range to be intersected with and subtracted from
    *                              <this> range
    * All three output paramters are optional - the range is not returned if the
    * corresponding parameter is NULL.
    */
    VOID Decompose(const MemRange& range, MemRange* pIntersection, MemRange* pLowDiff, MemRange* pHighDiff) const
    {
        VOID* thisEnd  = End();
        VOID* otherEnd = range.End();

        VOID* intersectionBase;
        VOID* intersectionEnd;

        if (Contains(range.m_base))
        {
            // intersecting ranges
            intersectionBase = range.m_base;
            intersectionEnd  = (range.Contains(thisEnd) ? thisEnd : otherEnd);
        }
        else if (range.Contains(m_base))
        {
            // intersecting ranges
            intersectionBase = m_base;
            intersectionEnd  = (Contains(otherEnd) ? otherEnd : thisEnd);
        }
        else if (m_base > range.m_base)
        {
            // intersection is empty; <this> is higher than <range>
            intersectionBase = intersectionEnd = m_base;
        }
        else
        {
            // intersection is empty; <this> is lower than <range>
            intersectionBase = intersectionEnd = thisEnd;
        }

        if (pIntersection != 0)
        {
            *pIntersection = MemRange(intersectionBase, intersectionEnd);
        }
        if (pLowDiff != 0)
        {
            *pLowDiff = MemRange(m_base, intersectionBase);
        }
        if (pHighDiff != 0)
        {
            *pHighDiff = MemRange(intersectionEnd, thisEnd);
        }
    }

  private:
    VOID* m_base;
    size_t m_size;
};

/*! @ingroup UTILS
 * Get the page that contains the specified address
 * @param[in] addr       address inside the requested page
 * @return one-page range that contains the specified address
 */
extern MemRange MemPageRange(ADDRINT addr);

/*! @ingroup UTILS
 * Get the page that contains the specified address
 * @param[in] addr       address inside the requested page
 * @return one-page range that contains the specified address
 */
extern MemRange MemPageRange(const VOID* addr);

/*! @ingroup UTILS
 *  Template POD structure that optionally keeps a value of type <T>.
 *  @tparam  T   the value type
 */
template< typename T > struct OPTIONAL_VALUE
{
    BOOL m_hasValue; ///< TRUE, if a value has been assigned to this object
    T m_value;       ///< The value assigned to this object

    /*!
     * Assign the specified value to this object.
     * @param[in]   value  the new value to be assigned to this object
     */
    VOID Set(const T& value)
    {
        m_hasValue = TRUE;
        m_value    = value;
    }

    /*!
     * Assign a value to this object or remove the previous assignment.
     * @param[in]   pValue  pointer to the new value to be assigned to this object or
     *                      NULL to remove the previous assignment
     */
    VOID Set(const T* pValue)
    {
        if (pValue != 0)
        {
            Set(*pValue);
        }
        else
        {
            Reset();
        }
    }

    /*!
     * Remove the previous value assignment, if any.
     */
    VOID Reset() { m_hasValue = FALSE; }

    /*!
     * Get the value of this object, if previously assigned.
     * @param[out] pValue   optional pointer to variable that receives the value assigned to
     *                      this object, if any
     * @return TRUE if a value has been assigned to this object.
     */
    BOOL Get(T* pValue) const
    {
        if (m_hasValue && (pValue != 0))
        {
            *pValue = m_value;
        }
        return m_hasValue;
    }

    /*!
     * @return TRUE if a value has been assigned to this object.
     */
    BOOL HasValue() const { return m_hasValue; }

    /*!
     * @return reference to the value assigned to this object.
     * The caller must ensure that a value has already been assigned to this object.
     */
    T& Value() { return m_value; }
    const T& Value() const { return m_value; }

    /*!
     * Get a pointer to the value of this object, if the value has been assigned.
     * @return pointer to the value kept in this object or NULL if no value has been
     *         assigned to this object.
     */
    T* ValuePtr() { return ((m_hasValue) ? &m_value : 0); }
    const T* ValuePtr() const { return ((m_hasValue) ? &m_value : 0); }
};

/*! @ingroup UTILS
  Convert a FLT64 into a string. Padding can be specified as well.
 */
extern std::string StringFlt(FLT64 val, UINT32 precision, UINT32 width);

/*! @ingroup UTILS
  Create a string containing the given floating point number.
 */
inline std::string fltstr(FLT64 val, UINT32 prec = 0, UINT32 width = 0) { return StringFlt(val, prec, width); }

/*! @ingroup UTILS
  Floating-to-string conversion object.
 */
struct FLTSTR
{
    FLTSTR(UINT32 prec = 0, UINT32 width = 0) : _p(prec), _w(width) {}
    template< typename T > std::string operator()(const T& t) const { return fltstr(t, _p, _w); }
    UINT32 _p;
    UINT32 _w;
};

#endif // PIN_G_UTIL_PH
