/*
 * Copyright (C) 2004-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 *  This file contains utility routines for cache/memory PIN tools
 */

#ifndef PIN_UTIL_H
#define PIN_UTIL_H

#include <iomanip>
#include <iostream>
#include <fstream>
#include <sstream>

#define KILO 1024
#define MEGA (KILO * KILO)
#define GIGA (KILO * MEGA)

extern std::string StringInt(UINT64 val, UINT32 width = 0, CHAR padding = ' ')
{
    std::ostringstream ostr;
    ostr.setf(std::ios::fixed, std::ios::floatfield);
    ostr.fill(padding);
    ostr << std::setw(width) << val;
    return ostr.str();
}

extern std::string StringHex(UINT64 val, UINT32 width = 0, CHAR padding = ' ')
{
    std::ostringstream ostr;
    ostr.setf(std::ios::fixed, std::ios::floatfield);
    ostr.fill(padding);
    ostr << std::setw(width) << std::hex << "0x" << val;
    return ostr.str();
}

extern std::string StringString(std::string val, UINT32 width = 0, CHAR padding = ' ')
{
    std::ostringstream ostr;
    ostr.setf(std::ios::fixed, std::ios::floatfield);
    ostr.fill(padding);
    ostr << std::setw(width) << val;
    return ostr.str();
}

#endif // PIN_UTIL_H
