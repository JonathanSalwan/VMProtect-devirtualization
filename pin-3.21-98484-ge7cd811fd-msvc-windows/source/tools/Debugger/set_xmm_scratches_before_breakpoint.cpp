/*
 * Copyright (C) 2012-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include "pin.H"
#include <stdio.h>

KNOB< BOOL > KnobUseIargConstContext(KNOB_MODE_WRITEONCE, "pintool", "const_context", "0", "use IARG_CONST_CONTEXT");
KNOB< std::string > KnobOutputFileName(KNOB_MODE_WRITEONCE, "pintool", "output_filename",
                                       "set_xmm_scratches_before_breakpoint.out", "Name output file.");

FILE* fp;

bool instrumentedIpAfterMovdqa = FALSE;
bool instrumentedMovdqa        = FALSE;
bool setApplicationBreakpoint  = FALSE;
bool IsFirstBreakpoint         = TRUE;
ADDRINT ipAfterMovdqa          = 0;

unsigned int xmmInitVals[64];

extern "C" int SetXmmScratchesFun(unsigned int* values);

// Insert a call to an analysis routine that sets the scratch xmm registers, the call is inserted just after the
// movdqa instruction of DoXmm (see xmm-asm-*.s)
static VOID InstrumentRoutine(RTN rtn, VOID*)
{
    if (RTN_Name(rtn) == "DoXmm")
    {
        RTN_Open(rtn);
        for (INS ins = RTN_InsHead(rtn); INS_Valid(ins); ins = INS_Next(ins))
        {
            if (INS_Opcode(ins) == XED_ICLASS_MOVDQA)
            {
                fprintf(fp, "instrumenting ins %p %s\n", (void*)INS_Address(ins), INS_Disassemble(ins).c_str());
                instrumentedMovdqa = TRUE;
                INS_InsertCall(ins, IPOINT_AFTER, (AFUNPTR)SetXmmScratchesFun, IARG_PTR, xmmInitVals, IARG_END);
                ipAfterMovdqa = INS_Address(INS_Next(ins));
                fprintf(fp, "ipAfterMovdqa ins %p  %s\n", (void*)ipAfterMovdqa, INS_Disassemble(INS_Next(ins)).c_str());
                fflush(fp);
            }
        }
        RTN_Close(rtn);
    }
}

static void DoBreakpoint(CONTEXT* ctxt, THREADID tid)
{
    if (!IsFirstBreakpoint)
    {
        return;
    }

    IsFirstBreakpoint = FALSE;
    fprintf(fp, "DoBreakpoint\n");
    fflush(fp);
    setApplicationBreakpoint = TRUE;
    PIN_ApplicationBreakpoint(ctxt, tid, 0, "The tool wants to stop");
}

VOID Instruction(INS ins, void* v)
{
    if (INS_Address(ins) == ipAfterMovdqa)
    {
        instrumentedIpAfterMovdqa = TRUE;
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)DoBreakpoint, (KnobUseIargConstContext) ? IARG_CONST_CONTEXT : IARG_CONTEXT,
                       IARG_THREAD_ID, IARG_END);
        if (KnobUseIargConstContext)
        {
            fprintf(fp, "const_context\n");
            fflush(fp);
        }
        else
        {
            fprintf(fp, "regular_context\n");
            fflush(fp);
        }
    }
}

static void OnExit(INT32, VOID*)
{
    if (!instrumentedMovdqa)
    {
        fprintf(fp, "***Error tool did not instrument the movdqa instruction of DoXmm\n");
        fflush(fp);
        PIN_ExitProcess(1);
    }
    else
    {
        fprintf(fp, "instrumented the movdqa instruction of DoXmm\n");
        fflush(fp);
    }
    if (!instrumentedIpAfterMovdqa)
    {
        fprintf(fp, "***Error tool did not instrument the ret instruction after the movdqa instruction of DoXmm\n");
        fflush(fp);
    }
    else
    {
        fprintf(fp, "instrumented the ret instruction after the movdqa instruction of DoXmm\n");
        fflush(fp);
    }

    if (!setApplicationBreakpoint)
    {
        fprintf(fp,
                "***Error tool did not setApplicationBreakpoint at the ret instruction after the movdqa instruction of DoXmm\n");
        fflush(fp);
    }
    else
    {
        fprintf(fp, "did setApplicationBreakpoint at the ret instruction after the movdqa instruction of DoXmm\n");
        fflush(fp);
    }
}

// argc, argv are the entire command line, including pin -t <toolname> -- ...
int main(int argc, char* argv[])
{
    // initialize memory area used to set values in ymm regs
    for (int i = 0; i < 64; i++)
    {
        xmmInitVals[i] = 0xdeadbeef;
    }

    PIN_InitSymbols();

    // Initialize pin
    PIN_Init(argc, argv);

    printf("filename %s\n", KnobOutputFileName.Value().c_str());
    fp = fopen(KnobOutputFileName.Value().c_str(), "w");

    // Register Instruction to be called to instrument the movdqa instruction of DoXmm
    RTN_AddInstrumentFunction(InstrumentRoutine, 0);

    INS_AddInstrumentFunction(Instruction, 0);

    PIN_AddFiniFunction(OnExit, 0);

    // Start the program, never returns
    PIN_StartProgram();

    return 0;
}
