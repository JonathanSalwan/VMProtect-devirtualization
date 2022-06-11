/*
 * Copyright (C) 2008-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

// <COMPONENT>: util
// <FILE-TYPE>: component public header

#ifndef UTIL_STRINGS_HPP
#define UTIL_STRINGS_HPP

#include <sstream>
#include <iomanip>
#include <cctype>

namespace UTIL
{
/*!
 * Convert an integral value to a decimal string.
 *
 *  @param[in] val      Integral value.
 *  @param[in] width    If not zero, "0"'s are prefixed if the
 *                       number has less than \a width digits.
 *
 * @return  String representation of \a val as a decimal integer.
 */
template< typename T > std::string GetDecString(T val, int width = 0)
{
    std::ostringstream s;
    s << std::dec << std::setfill('0') << std::setw(width) << val;
    return s.str();
}

/*!
 * Convert a pointer value to a decimal string.
 *
 *  @param[in] val      Pointer value.
 *  @param[in] width    If not zero, "0"'s are prefixed if the
 *                       number has less than \a width digits.
 *
 * @return  String representation of \a val as a decimal integer.
 */
template< typename T > std::string GetDecString(T* val, int width = 0)
{
    return GetDecString(reinterpret_cast< uintptr_t >(val), width);
}

/*!
 * Convert an integral value to a decimal string.
 *
 *  @param[in] val      Integral value.
 *  @param[in] width    If not zero, "0"'s are prefixed if the
 *                       number has less than \a width digits.
 *
 * @return  String representation of \a val as a decimal integer.
 */
template< typename T > std::wstring GetDecStringW(T val, int width = 0)
{
    std::wostringstream s;
    s << std::dec << std::setfill(L'0') << std::setw(width) << val;
    return s.str();
}

/*!
 * Convert a pointer value to a decimal string.
 *
 *  @param[in] val      Pointer value.
 *  @param[in] width    If not zero, "0"'s are prefixed if the
 *                       number has less than \a width digits.
 *
 * @return  String representation of \a val as a decimal integer.
 */
template< typename T > std::wstring GetDecStringW(T* val, int width = 0)
{
    return GetDecStringW(reinterpret_cast< uintptr_t >(val), width);
}

/*!
 * Convert an integral value to a hex string.
 *
 *  @param[in] val          Integral value.
 *  @param[in] showBase     If TRUE, the string is prefix with "0x".
 *  @param[in] width        If not zero, "0"'s are prefixed if the
 *                           number has less than \a width digits.
 *
 * @return  String representation of \a val as a hex string.
 */
template< typename T > std::string GetHexString(T val, bool showBase = true, int width = 0)
{
    std::ostringstream s;
    if (showBase) s << "0x";
    s << std::hex << std::noshowbase << std::setfill('0') << std::setw(width) << val;
    return s.str();
}

/*!
 * Convert a pointer value to a hex string.
 *
 *  @param[in] val          Pointer value.
 *  @param[in] showBase     If TRUE, the string is prefix with "0x".
 *  @param[in] width        If not zero, "0"'s are prefixed if the
 *                           number has less than \a width digits.
 *
 * @return  String representation of \a val as a hex string.
 */
template< typename T > std::string GetHexString(T* val, bool showBase = true, int width = 0)
{
    return GetHexString(reinterpret_cast< uintptr_t >(val), showBase, width);
}

/*!
 * Convert an integral value to a hex string.
 *
 *  @param[in] val          Integral value.
 *  @param[in] showBase     If TRUE, the string is prefix with "0x".
 *  @param[in] width        If not zero, "0"'s are prefixed if the
 *                           number has less than \a width digits.
 *
 * @return  String representation of \a val as a hex string.
 */
template< typename T > std::wstring GetHexStringW(T val, bool showBase = true, int width = 0)
{
    std::wostringstream s;
    if (showBase) s << L"0x";
    s << std::hex << std::noshowbase << std::setfill(L'0') << std::setw(width) << val;
    return s.str();
}

/*!
 * Convert a pointer value to a hex string.
 *
 *  @param[in] val          Pointer value.
 *  @param[in] showBase     If TRUE, the string is prefix with "0x".
 *  @param[in] width        If not zero, "0"'s are prefixed if the
 *                           number has less than \a width digits.
 *
 * @return  String representation of \a val as a hex string.
 */
template< typename T > std::wstring GetHexStringW(T* val, bool showBase = true, int width = 0)
{
    return GetHexStringW(reinterpret_cast< uintptr_t >(val), showBase, width);
}

/*!
* This function is similar to strtoul(), with two differences: 1) the type of
* the result is a template argument instead of hard-coded to "long", 2) the input
* is specified as two STL-style iterators into a sequence of items that are
* convertible to "char".
*
* Parse an unsigned integral value from a sequence of char's, or other items that
* are convertible to char.  If the sequence starts with whitespace characters, it
* is skipped before any parsing is done.
*
* The \a base parameter specifies the radix to use when parsing the number and may
* be any value from 0 to 37.  Each digit in the parsed number must be one of the
* characters '0' - '9 or 'a' - 'z' depending on the value of \a base.  When \a base
* is greater than 10, digits may include the letters 'a' - 'z' in either upper or
* lower case.
*
* When \a base is zero, the radix is determined by the prefix of the input, after
* the initial whitespace has been ignored.  If the prefix is "0x", the radix is 16.
* If the prefix is "0", the radix is 8.  Otherwise, the radix is 10.  When \a base
* is 16, the prefix "0x" is also allowed.
*
* This function fails if the input sequence (after ignoring initial whitespace)
* does not start with a valid number.  The function also fails if the parsed number
* would overflow its type or if \a base is out or range.
*
*  @param[in] start    Input iterator to the start of the input sequence.
*  @param[in] end      Input iterator to the end of the input sequence.
*  @param[in] base     Tells the radix to use when parsing the value, which must
*                       be less than 37.
*  @param[out] val     On success, receives the parsed value.
*
* @return  On success, an iterator that points to the first unparsed position in
*           the sequence.  On failure, this function returns \a start.
*/
template< typename InputIterator, typename T >
InputIterator ParseUnsigned(InputIterator first, InputIterator last, unsigned base, T* val)
{
    // Make sure the radix is in range.
    //
    if (base > 37) return first;

    // Skip initial whitespace.
    //
    InputIterator it = first;
    while (it != last && std::isspace(*it))
        it++;
    if (it == last) return first;
    InputIterator itAfterWs = it;

    // See if there is a leading zero (e.g. the prefix is "0x" or "0").
    //
    bool zeroPending = false;
    if (*it == '0')
    {
        zeroPending = true;
        it++;
        if (it == last)
        {
            *val = 0;
            return it;
        }
    }

    // Check for a "0x" prefix if the radix is 0 or 16.
    //
    if ((base == 16 || base == 0) && zeroPending && (*it == 'x' || *it == 'X'))
    {
        InputIterator it2 = it++;
        if (it == last || !std::isxdigit(*it))
        {
            // If the prefix is "0x" with NO subsequent digits, parse the
            // value zero and stop parsing before the "x".
            //
            *val = 0;
            return it2;
        }
        base = 16;
    }

    if (base == 0) base = (zeroPending) ? 8 : 10;

    // Parse each digit.
    //
    T myVal = 0;
    while (it != last)
    {
        char c = std::tolower(*it);
        unsigned digit;
        if (c >= '0' && c <= '9')
            digit = c - '0';
        else if (c >= 'a' && c <= 'z')
            digit = 10 + c - 'a';
        else
            break;
        if (digit >= base) break;
        T newVal = myVal * base + digit;

        // Check for overflow.
        //
        if (((newVal - digit) / base) != myVal) return first;
        myVal = newVal;
        it++;
    }

    // We must parse at least one digit for success.
    //
    if (it == itAfterWs) return first;
    *val = myVal;
    return it;
}

} // namespace UTIL
#endif // file guard
