/*
 * Copyright (C) 2021-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdint.h>
#include <string>
#include <sstream>

enum elementFormat
{
    formatHex,
    formatDec,
    formatDouble
};

// This function returns a string represeting the content of a list of elements in memory.
//  [in] addr           : The base address.
//  [in] elementSize    : The number of bytes in each element. Elements are delimited by a space.
//  [in] numElements    : The number of elements to read.
//  [in] format         : The print format of each element (decimal/hex/double).
//  [in] minWidth       : The minimal number of characters to print for each element (padded with zeros for hex format)
inline std::string bytes2str(const char* addr, int elementSize, int numElements, elementFormat format, int minWidth = 1)
{
    std::stringstream ss;
    char* p = (char*)addr;
    for (int i = 0; i < numElements; i++)
    {
        uint64_t value = 0;
        int width      = 0;
        switch (elementSize)
        {
            case 1:
            {
                uint8_t* i = (uint8_t*)(p);
                value      = (uint64_t)(*i);
                width      = 2;
            }
            break;
            case 2:
            {
                uint16_t* i = (uint16_t*)(p);
                value       = (uint64_t)(*i);
                width       = 4;
            }
            break;
            case 4:
            {
                uint32_t* i = (uint32_t*)(p);
                value       = (uint64_t)(*i);
                width       = 8;
            }
            break;
            case 8:
            {
                uint64_t* i = (uint64_t*)(p);
                value       = (uint64_t)(*i);
                width       = 16;
            }
            break;
            default:
                assert(0);
                break;
        }
        width = std::max(width, minWidth);
        switch (format)
        {
            case formatHex:
                ss << std::setw(width) << std::setfill('0') << std::hex << value << " ";
                break;
            case formatDec:
                ss << std::setw(width) << std::dec << value << " ";
                break;
            case formatDouble:
            {
                double d = (double)value;
                ss << std::setw(width) << d << " ";
            }
            break;
            default:;
        }
        p += elementSize;
    }
    return ss.str();
}
