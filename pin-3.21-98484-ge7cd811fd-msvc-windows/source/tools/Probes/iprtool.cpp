/*
 * Copyright (C) 2006-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

//
// This tool replaces three routines that have different types
// of ip-relative instructions to ensure that Pin handles this
// properly.
//

#include "pin.H"
#include <iostream>
#include "tool_macros.h"
using std::cerr;
using std::endl;

/* ===================================================================== */
/* Global Variables */
/* ===================================================================== */

static void (*pf_iprel_imm)();
static void (*pf_iprel_reg)(int);
static int (*pf_reg_iprel)();

/* ===================================================================== */
/* Replacement Functions */
/* ===================================================================== */

void IprelImmProbe()
{
    if (pf_iprel_imm)
    {
        (pf_iprel_imm)();
    }
}

void IprelRegProbe(int b)
{
    if (pf_iprel_reg)
    {
        (pf_iprel_reg)(b);
    }
}

int RegIprelProbe()
{
    int a = 0;

    if (pf_reg_iprel)
    {
        a = (pf_reg_iprel)();
    }
    return a;
}

/* ===================================================================== */

// Called every time a new image is loaded
// Look for routines that we want to probe

VOID ImageLoad(IMG img, VOID* v)
{
    RTN iprelImmRtn = RTN_FindByName(img, C_MANGLE("iprel_imm"));
    if (RTN_Valid(iprelImmRtn))
    {
        pf_iprel_imm = (void (*)())RTN_ReplaceProbed(iprelImmRtn, AFUNPTR(IprelImmProbe));
        cerr << "Inserted probe for iprel_imm:" << IMG_Name(img) << endl;
    }

    RTN iprelRegRtn = RTN_FindByName(img, C_MANGLE("iprel_reg"));
    if (RTN_Valid(iprelRegRtn))
    {
        pf_iprel_reg = (void (*)(int))RTN_ReplaceProbed(iprelRegRtn, AFUNPTR(IprelRegProbe));
        cerr << "Inserted probe for iprel_reg:" << IMG_Name(img) << endl;
    }

    RTN regIprelRtn = RTN_FindByName(img, C_MANGLE("reg_iprel"));
    if (RTN_Valid(regIprelRtn))
    {
        pf_reg_iprel = (int (*)())RTN_ReplaceProbed(regIprelRtn, AFUNPTR(RegIprelProbe));
        cerr << "Inserted probe for reg_iprel:" << IMG_Name(img) << endl;
    }
}

/* ===================================================================== */

int main(int argc, CHAR* argv[])
{
    PIN_InitSymbols();

    PIN_Init(argc, argv);

    IMG_AddInstrumentFunction(ImageLoad, 0);

    PIN_StartProgramProbed();

    return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
