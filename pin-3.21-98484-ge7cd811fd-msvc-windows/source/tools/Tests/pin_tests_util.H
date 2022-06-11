/*
 * Copyright (C) 2010-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 * Useful includes and macros for tests.
 */

#ifndef PIN_TESTS_UTIL_H
#define PIN_TESTS_UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include "pin.H"

using namespace std;

#define TEST(COND, MSG)          \
    do                           \
    {                            \
        if (!(COND))             \
        {                        \
            cerr << MSG << endl; \
            exit(1);             \
        }                        \
    }                            \
    while (0)

#define PRINT_ME_AND_MY_FATHER()                                                           \
    do                                                                                     \
    {                                                                                      \
        cout << "I'm " << PIN_GetTid() << ", my father is " << PIN_GetParentTid() << endl; \
    }                                                                                      \
    while (0)

#endif // PIN_TESTS_UTIL_H
