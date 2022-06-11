/*
 * Copyright (C) 2010-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 This tool tests that analysis routines called thru non-inlined bridges receive the
 correct value of the flags register, even when the flags are deat to the application.
 It is used in conjuction with flags_at_analysis_app.
In particular the  SetOfFlag_asm function that is part of the flags_at_analysis_app is
instrumented:

 SetOfFlag_asm PROC
    xor eax, eax
    inc eax          //al now contains 1
    pushfd
    popfd            //this is a marker
    cmp al, 081H     //causes OF to be set
    xor ecx, ecx     //analysis routine eads flags just before this instruction - verifies the OF flag is set
    ret
SetOfFlag_asm ENDP
*/

#include <cstdio>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <cstdlib>

#ifdef TARGET_LINUX
#include <unistd.h>
#include <syscall.h>
#include <errno.h>
#ifdef TARGET_IA32E
#include <asm/prctl.h>
#include <sys/prctl.h>
#endif // TARGET_IA32E
#endif // TARGET_LINUX

#include "pin.H"
#include "instlib.H"

// windows.h must be included after pin.H
#ifdef TARGET_WINDOWS
namespace WIND
{
#include <windows.h>
}
#endif // TARGET_WINDOWS

BOOL x = TRUE;

BOOL IfReturnTrue(ADDRINT ip) { return x; }

BOOL haveThenCheck = FALSE;

VOID ThenFunc(ADDRINT flagsVal)
{
    printf("flags at ThenFunc %x\n", flagsVal);
    if ((flagsVal & 0x800) == 0)
    {
        printf("ThenFunc expected OF flag to be set\n");
        exit(-1);
    }
    haveThenCheck = TRUE;
}

BOOL haveAnalysisCheck = FALSE;

VOID AnalysisFunc(ADDRINT flagsVal)
{
    printf("flags at AnalysisFunc %x\n", flagsVal);
    if ((flagsVal & 0x800) == 0)
    {
        printf("AnalysisFunc expected OF flag to be set\n");
        exit(-1);
    }
    haveAnalysisCheck = TRUE;
}

INT32 Usage()
{
    cerr << "This is a test of  flag values at analysis functions"
            "\n";

    cerr << endl;

    return -1;
}

VOID Image(IMG img, VOID* v)
{
    if (strstr(IMG_Name(img).c_str(), "flags_at_analysis_app") == NULL)
    {
        return;
    }
    for (SEC sec = IMG_SecHead(img); SEC_Valid(sec); sec = SEC_Next(sec))
    {
        for (RTN rtn = SEC_RtnHead(sec); RTN_Valid(rtn); rtn = RTN_Next(rtn))
        {
            // Prepare for processing of RTN, an  RTN is not broken up into BBLs,
            // it is merely a sequence of INSs
            RTN_Open(rtn);

            for (INS ins = RTN_InsHead(rtn); INS_Valid(ins); ins = INS_Next(ins))
            {
                if (INS_Opcode(ins) == XED_ICLASS_POPF || INS_Opcode(ins) == XED_ICLASS_POPFD ||
                    INS_Opcode(ins) == XED_ICLASS_POPFQ)
                { // popf is the marker
                    printf("found popf in rtn %s\n", RTN_Name(rtn).c_str());
                    if (!INS_Valid(INS_Next(ins)) || !INS_Valid(INS_Next(INS_Next(ins))))
                    {
                        printf("wrong popf marker found\n");
                        exit(-1);
                    }

                    printf("next ins should be cmp al, 0x81   it is   %s\n", INS_Disassemble(INS_Next(ins)).c_str());
                    printf("next ins should be xor ecx, ecx   it is   %s\n", INS_Disassemble(INS_Next(INS_Next(ins))).c_str());

                    // Insert analysis calls to read the value of the flags register just after the cmp al, 0x81 - the OF flag should be set
                    INS_InsertIfCall(INS_Next(INS_Next(ins)), IPOINT_BEFORE, (AFUNPTR)IfReturnTrue, IARG_INST_PTR, IARG_END);
                    INS_InsertThenCall(INS_Next(INS_Next(ins)), IPOINT_BEFORE, (AFUNPTR)ThenFunc, IARG_REG_VALUE, REG_GFLAGS,
                                       IARG_END);
                    INS_InsertCall(INS_Next(INS_Next(ins)), IPOINT_BEFORE, (AFUNPTR)AnalysisFunc, IARG_REG_VALUE, REG_GFLAGS,
                                   IARG_END);
                }
            }

            // to preserve space, release data associated with RTN after we have processed it
            RTN_Close(rtn);
        }
    }
}

VOID Fini(INT32 code, VOID* v)
{
    if (!haveThenCheck)
    {
        printf("then check was not carried out\n");
        exit(-1);
    }
    if (!haveAnalysisCheck)
    {
        printf("analysis check was not carried out\n");
        exit(-1);
    }
}

int main(int argc, char* argv[])
{
    if (PIN_Init(argc, argv))
    {
        return Usage();
    }

    IMG_AddInstrumentFunction(Image, 0);
    PIN_AddFiniFunction(Fini, 0);

    // Never returns
    PIN_StartProgram();

    return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
