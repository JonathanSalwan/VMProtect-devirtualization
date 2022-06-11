/*
 * Copyright (C) 2014-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include "xstateBVUtils.h"

const unsigned int testRegSize[TEST_REG_CLASS_SIZE] = {
    /* TEST_REG_CLASS_X87 */ 2,  // fcw
    /* TEST_REG_CLASS_SSE */ 16, // xmm3
    /* TEST_REG_CLASS_AVX */ 32, // ymm3
};

const unsigned int testRegLocation[TEST_REG_CLASS_SIZE] = {
    /* TEST_REG_CLASS_X87 */ 0,   // fcw
    /* TEST_REG_CLASS_SSE */ 208, // xmm3
    /* TEST_REG_CLASS_AVX */ 624, // ymm3 (upper 128 bits)
};

// Masks for checking if a single state component in the XSTATE_BV state-component bitmaps is set.
const unsigned char xstateBvMasks[TEST_REG_CLASS_SIZE] = {
    0x01, // bit 0 in the XSTATE_BV save mask - controls the X87 registers
    0x02, // bit 1 in the XSTATE_BV save mask - controls the SSE registers
    0x04  // bit 2 in the XSTATE_BV save mask - controls the AVX registers
};

const string componentStrings[TEST_REG_CLASS_SIZE] = {"x87", "SSE", "AVX"};

static const unsigned char fpcwval[] = {0x4e, 0x1f};
static const unsigned char xmm3val[] = {0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
                                        0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc};
static const unsigned char ymm3val[] = {0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd,
                                        0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd,
                                        0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd};

const unsigned char* toolRegisterValues[TEST_REG_CLASS_SIZE] = {fpcwval, xmm3val, ymm3val};

TEST_REG_CLASS GetTestReg(const string& arg)
{
    for (unsigned int i = 0; i < TEST_REG_CLASS_SIZE; ++i)
    {
        if (componentStrings[i] == arg) return (TEST_REG_CLASS)i;
    }
    return TEST_REG_CLASS_INVALID;
}
