/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

//
// Check that IARG_EXECUTING works on IA32, by counting the number of
// cmovs and the number executed in the test code.
//
#include "pin.H"
#include <stdlib.h>
#include <iostream>
#include <fstream>
using std::cerr;
using std::endl;
using std::ofstream;
using std::string;

static ofstream out;

KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "checkexecuting.out", "Output file");
KNOB< BOOL > KnobVerbose(KNOB_MODE_WRITEONCE, "pintool", "v", "0", "Verbose");

UINT64 executed = 0;
UINT64 total    = 0;
int enabled     = 0;

static struct
{
    const char* name;
    UINT32 opcode;
} opInfo[] = {
    {"cmovb", XED_ICLASS_CMOVB},
    {"cmovbe", XED_ICLASS_CMOVBE},
    {"cmovl", XED_ICLASS_CMOVL},
    {"cmovle", XED_ICLASS_CMOVLE},
    {"cmovnb", XED_ICLASS_CMOVNB},
    {"cmovnbe", XED_ICLASS_CMOVNBE},
    {"cmovnl", XED_ICLASS_CMOVNL},
    {"cmovnle", XED_ICLASS_CMOVNLE},
    {"cmovno", XED_ICLASS_CMOVNO},
    {"cmovnp", XED_ICLASS_CMOVNP},
    {"cmovns", XED_ICLASS_CMOVNS},
    {"cmovnz", XED_ICLASS_CMOVNZ},
    {"cmovo", XED_ICLASS_CMOVO},
    {"cmovp", XED_ICLASS_CMOVP},
    {"cmovs", XED_ICLASS_CMOVS},
    {"cmovz", XED_ICLASS_CMOVZ},
    // String ops, which are predicated if REPped
    {"lodsb", XED_ICLASS_LODSB},
    {"lodsw", XED_ICLASS_LODSW},
    {"lodsd", XED_ICLASS_LODSD},
    {"lodsq", XED_ICLASS_LODSQ},
    {"movsb", XED_ICLASS_MOVSB},
    {"movsw", XED_ICLASS_MOVSW},
    {"movsd", XED_ICLASS_MOVSD},
    {"movsq", XED_ICLASS_MOVSQ},
    {"scasb", XED_ICLASS_SCASB},
    {"scasw", XED_ICLASS_SCASW},
    {"scasd", XED_ICLASS_SCASD},
    {"scasq", XED_ICLASS_SCASQ},
    {"cmpsb", XED_ICLASS_CMPSB},
    {"cmpsw", XED_ICLASS_CMPSW},
    {"cmpsd", XED_ICLASS_CMPSD},
    {"cmpsq", XED_ICLASS_CMPSQ},
    {"stosb", XED_ICLASS_STOSB},
    {"stosw", XED_ICLASS_STOSW},
    {"stosd", XED_ICLASS_STOSD},
    {"stosq", XED_ICLASS_STOSQ},
};
#define NumOps (sizeof(opInfo) / sizeof(opInfo[0]))

static struct opCS
{
    int total;
    int executed;
    int viaPredicated;
    int viaIfPredicated;
    int viaThenPredicated;
    int viaIfThenPredicated;
} opCounts[NumOps];

INT32 Usage()
{
    cerr << "This pin tool counts predicated instructions selected by the\n"
            "following filter options\n"
            "\n";

    cerr << KNOB_BASE::StringKnobSummary() << endl;
    return -1;
}

static int opcodeToIndex(UINT32 opcode)
{
    for (UINT32 i = 0; i < NumOps; i++)
    {
        if (opInfo[i].opcode == opcode) return i;
    }

    ASSERT(false, "Bad opcode " + xed_iclass_enum_t2str(xed_iclass_enum_t(opcode)) + "\n");
    return -1;
}

VOID doCount(UINT32 executing, UINT32 idx)
{
    if (enabled)
    {
        opCounts[idx].total++;
        opCounts[idx].executed += executing;

        total++;
        executed += executing;
    }
}

VOID doCountPredicated(UINT32 idx)
{
    if (enabled)
    {
        opCounts[idx].viaPredicated++;
    }
}

VOID doCountIfPredicated(UINT32 idx)
{
    if (enabled)
    {
        opCounts[idx].viaIfPredicated++;
    }
}

VOID doCountThenPredicated(UINT32 idx)
{
    if (enabled)
    {
        opCounts[idx].viaThenPredicated++;
    }
}

VOID doCountIfThenPredicated(UINT32 idx)
{
    if (enabled)
    {
        opCounts[idx].viaIfThenPredicated++;
    }
}

ADDRINT trueFunction() { return 1; }

ADDRINT falseFunction() { return 0; }

VOID shouldntBeCalled() { out << "***shouldntBeCalled has been called\n"; }

VOID toggleEnabled()
{
    if (enabled)
        enabled = 0;
    else
        enabled = 1;
}

VOID InstructionTrace(TRACE trace, INS ins)
{
    if (INS_Opcode(ins) == XED_ICLASS_FNOP)
    {
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)toggleEnabled, IARG_END);
        INS_Delete(ins);
        return;
    }

    //    INS_InsertPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)docount, IARG_END);
    if (INS_Category(ins) == XED_CATEGORY_CMOV || INS_HasRealRep(ins))
    {
        UINT32 opIdx = opcodeToIndex(INS_Opcode(ins));

        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)doCount, IARG_EXECUTING, IARG_UINT32, opIdx, IARG_END);
        INS_InsertPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)doCountPredicated, IARG_UINT32, opIdx, IARG_END);

        // Each of the if/then predicated cases (with an IF which is always true,
        // so the results should be the same as before).
        INS_InsertIfPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)trueFunction, IARG_END);
        INS_InsertThenCall(ins, IPOINT_BEFORE, (AFUNPTR)doCountIfPredicated, IARG_UINT32, opIdx, IARG_END);

        INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)trueFunction, IARG_END);
        INS_InsertThenPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)doCountThenPredicated, IARG_UINT32, opIdx, IARG_END);

        INS_InsertIfPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)trueFunction, IARG_END);
        INS_InsertThenPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)doCountIfThenPredicated, IARG_UINT32, opIdx, IARG_END);

        // Then each of the if/then cases with a false IF condition, to check that
        // the function is not called, and we really are testing it.
        INS_InsertIfPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)falseFunction, IARG_END);
        INS_InsertThenCall(ins, IPOINT_BEFORE, (AFUNPTR)shouldntBeCalled, IARG_END);

        INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)falseFunction, IARG_END);
        INS_InsertThenPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)shouldntBeCalled, IARG_END);

        INS_InsertIfPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)falseFunction, IARG_END);
        INS_InsertThenPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)shouldntBeCalled, IARG_END);
    }
}

VOID Trace(TRACE trace, VOID* val)
{
    // Images besides the executable may have extra cmovs/other insts that disrupt the reference so ignore them.
    if (!RTN_Valid(TRACE_Rtn(trace)) ||
        (IMG_Valid(SEC_Img(RTN_Sec(TRACE_Rtn(trace)))) && !IMG_IsMainExecutable(SEC_Img(RTN_Sec(TRACE_Rtn(trace))))))
        return;

    for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
    {
        for (INS ins = BBL_InsHead(bbl); INS_Valid(ins); ins = INS_Next(ins))
        {
            InstructionTrace(trace, ins);
        }
    }
}

VOID Fini(INT32 code, VOID* v)
{
    out << "Total : " << total << " Executed : " << executed << endl;
    for (UINT32 i = 0; i < NumOps; i++)
    {
        opCS* counts = &opCounts[i];

        if (counts->total == 0) continue;

        out << opInfo[i].name << " :  " << counts->total << " " << counts->executed << endl;
        // Check that the predicated calls all gave the same results as the executed call.
        // We check externally that the executed call agrees with that the code actually did.
        if (KnobVerbose)
        {
            out << "   via predicated       " << counts->viaPredicated << endl;
            out << "   via ifPredicated     " << counts->viaIfPredicated << endl;
            out << "   via thenPredicated   " << counts->viaThenPredicated << endl;
            out << "   via ifThenPredicated " << counts->viaIfThenPredicated << endl;
        }

        if (counts->executed != counts->viaPredicated)
        {
            out << "***Error : executed gave " << counts->executed << " predicated gave " << counts->viaPredicated << endl;
            exit(1);
        }

        if (counts->executed != counts->viaIfPredicated)
        {
            out << "***Error : executed gave " << counts->executed << " if predicated gave " << counts->viaIfPredicated << endl;
            exit(1);
        }

        if (counts->executed != counts->viaThenPredicated)
        {
            out << "***Error : executed gave " << counts->executed << " then predicated gave " << counts->viaThenPredicated
                << endl;
            exit(1);
        }

        if (counts->executed != counts->viaIfThenPredicated)
        {
            out << "***Error : executed gave " << counts->executed << " ifthen predicated gave " << counts->viaIfThenPredicated
                << endl;
            exit(1);
        }
    }
}

// argc, argv are the entire command line, including pin -t <toolname> -- ...
int main(int argc, char* argv[])
{
    if (PIN_Init(argc, argv))
    {
        return Usage();
    }

    string filename = KnobOutputFile.Value();

    // Do this before we activate controllers
    out.open(filename.c_str());

    TRACE_AddInstrumentFunction(Trace, 0);
    PIN_AddFiniFunction(Fini, 0);

    // Start the program, never returns
    PIN_StartProgram();

    return 0;
}
