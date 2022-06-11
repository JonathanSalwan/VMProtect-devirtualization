/*
 * Copyright (C) 2014-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#ifndef XSTATE_BV_UTILS_H
#define XSTATE_BV_UTILS_H

#include <string>

using std::string;

/////////////////////
// TYPE DEFINITIONS
/////////////////////

enum TEST_REG_CLASS
{
    TEST_REG_CLASS_X87 = 0,
    TEST_REG_CLASS_SSE,
    TEST_REG_CLASS_AVX,
    TEST_REG_CLASS_SIZE,
    TEST_REG_CLASS_INVALID = TEST_REG_CLASS_SIZE
};

extern "C"
{
    /////////////////////
    // GLOBAL VARIABLES
    /////////////////////

    extern const unsigned int testRegSize[TEST_REG_CLASS_SIZE];
    extern const unsigned int testRegLocation[TEST_REG_CLASS_SIZE];
    extern const unsigned char xstateBvMasks[TEST_REG_CLASS_SIZE];
    extern const string componentStrings[TEST_REG_CLASS_SIZE];
    extern const unsigned char* toolRegisterValues[TEST_REG_CLASS_SIZE];

    /////////////////////
    // UTILITY FUNCTIONS
    /////////////////////

    extern TEST_REG_CLASS GetTestReg(const string& arg);

} // extern "C"

#endif // XSTATE_BV_UTILS_H
