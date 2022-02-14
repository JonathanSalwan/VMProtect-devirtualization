/*
 * Copyright (C) 2017-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#ifndef REGION_UTILS_H
#define REGION_UTILS_H

#include <sstream>
#include <string.h>

using namespace std;
namespace CONTROLLER
{
class REGION_UTILS
{
  public:
    static UINT32 StringToUINT32(string& s, const char* name, UINT32 base = 10)
    {
        char* end    = 0;
        INT32 retval = strtoul(s.c_str(), &end, base);
        ASSERT((*end == 0), "ERROR reading " + name + " from " + s);
        ASSERT((retval >= 0), name + " (" + s + ") must be positive ");
        return (UINT32)retval;
    }

    static UINT64 StringToUINT64(string& s, const char* name, UINT32 base = 10)
    {
        char* end    = 0;
        INT64 retval = strtoull(s.c_str(), &end, base);
        ASSERT((*end == 0), "ERROR reading " + name + " from " + s);
        ASSERT((retval >= 0), name + " (" + s + ") must be positive ");
        return (UINT64)retval;
    }

    static ADDRINT StringToADDRINT(string& s, const char* name)
    {
#if defined(TARGET_IA32)
        return StringToUINT32(s, name, 16);
#else
        return StringToUINT64(s, name, 16);
#endif
    }

    static double StringToDouble(string& s, const char* name)
    {
        char* end     = 0;
        double retval = strtod(s.c_str(), &end);
        ASSERT((*end == 0), "ERROR reading " + name + " from " + s);
        return retval;
    }

    // Convert weight to string for setting region name
    static string WeightToString(UINT32 weightX100000)
    {
        CHAR weight_array[8]; // xxx.xxx
        string prefix = (weightX100000 == 100000) ? "1" : "0";

        // WeightTimesHundredThousand naming
        ASSERTX(weightX100000 < 10000000);
        sprintf(weight_array, "%07d", weightX100000);

        for (UINT32 d = 1; d < 7; d++)
        {
            weight_array[d] = weight_array[d + 1];
        }
        weight_array[0] = '-';
        weight_array[7] = 0;
        return prefix + string(weight_array);
    }
};
} // namespace CONTROLLER
#endif
