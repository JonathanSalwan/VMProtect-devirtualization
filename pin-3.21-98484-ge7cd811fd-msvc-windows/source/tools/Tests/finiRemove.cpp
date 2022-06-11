/*
 * Copyright (C) 2010-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 *  Check the PIN_RemoveFiniFunctions interface.
 */

#include "pin_tests_util.H"

VOID BadFini(INT32 code, VOID* v) { TEST(false, "PIN_RemoveFiniFunctions failed"); }

int main(INT32 argc, CHAR** argv)
{
    PIN_Init(argc, argv);

    PIN_AddFiniFunction(BadFini, 0);
    PIN_RemoveFiniFunctions();

    // Never returns
    PIN_StartProgram();

    return 0;
}
