/*
 * Copyright (C) 2018-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/**
 * @file
 * An app which set XMM registers, calls DoNothing() which does nothing and then check that the XMM state haven't changed.
 * A Pin tool that is being used together with this application probe DoNothing() and calls another function from that
 * analysis function which scrambles XMMS registers (not the native application XMM registers). After that Pin_Detach() is being
 * called from the same analysis function.
 * Since analysis function shouldn't effect the state of the application then this function shouldn't change the native XMM
 * registers when going to native after detach
 */
#include <stdio.h>
#include <stdint.h>
//#include <unistd.h>
#include "tool_macros.h"

//16 32byte registers
unsigned char ymmInitVals[512];
unsigned char ymmSaveVals[512];

typedef union /*<POD>*/
{
    uint64_t _64[2]; ///< Vector of 2 64-bit elements.
} XMM_REG1;

XMM_REG1 _xmmInit[8];
XMM_REG1 _xmmAfter[8];

extern void CleanXmms() ASMNAME("CleanXmms");
extern void SaveXmms(XMM_REG1* xmms) ASMNAME("SaveXmms");
extern void DoNothing() ASMNAME("DoNothing");

// argc, argv are the entire command line, including pin -t <toolname> -- ...
int main(int argc, char* argv[])
{
    int hadError = 0;
    int i;

    CleanXmms();
    SaveXmms(_xmmInit);
    // Probing DoNothing() and not the second SaveXmms() on purpose: It's a good point to scramble the registers and to
    // do the detach. If we'll probe the second SaveXmms() then the test case will not be test correctly since the XMMS will
    // taken from the spill area and issue will be masked.
    DoNothing();
    SaveXmms(_xmmAfter);

    for (i = 0; i < 8; i++)
    {
        if ((_xmmInit[i]._64[0] != _xmmAfter[i]._64[0]) || (_xmmInit[i]._64[1] != _xmmAfter[i]._64[1]))
        {
            hadError = 1;
            //printf ("***Error \n");
            printf("***Error _xmmInit[%d](expected) = 0x%llx 0x%llx :  _xmmAfter[%d](actual) 0x%llx 0x%llx\n", i,
                   _xmmInit[i]._64[0], _xmmInit[i]._64[1], i, _xmmAfter[i]._64[0], _xmmAfter[i]._64[1]);
        }
        else
        {
            printf("XMM[%d] = 0x%llx 0x%llx\n", i, _xmmAfter[i]._64[0], _xmmAfter[i]._64[1]);
        }
    }

    if (!hadError)
    {
        printf("SUCCESS\n");
        return (0);
    }
    return -1;
}
