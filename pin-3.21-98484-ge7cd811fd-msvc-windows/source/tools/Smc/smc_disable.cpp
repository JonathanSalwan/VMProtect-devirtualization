/*
 * Copyright (C) 2004-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

//
//  This tool tests the functionality of PIN_DisableSmcSupport
//

#include <cstdio>
#include "pin.H"

/* ================================================================== */
/* Global Variables                                                   */
/* ================================================================== */

/* ===================================================================== */
/* Commandline Switches */
/* ===================================================================== */

/* ================================================================== */

/* ================================================================== */
/*
 Initialize and begin program execution under the control of Pin
*/
int main(INT32 argc, CHAR** argv)
{
    if (PIN_Init(argc, argv)) return 1;

    PIN_SetSmcSupport(SMC_DISABLE);

    PIN_StartProgram(); // Never returns

    return 0;
}
