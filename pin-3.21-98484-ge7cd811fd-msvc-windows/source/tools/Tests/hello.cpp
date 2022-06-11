/*
 * Copyright (C) 2004-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <locale.h>
#include "pin.H"

int main(INT32 argc, CHAR** argv)
{
    setlocale(LC_ALL, 0);

    PIN_Init(argc, argv);

    // Never returns
    PIN_StartProgram();

    return 0;
}
