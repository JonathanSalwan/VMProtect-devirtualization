/*
 * Copyright (C) 2007-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

// leave.cpp -- testing  the leave instruction's memory address
#include <iostream>
#include <iomanip>
#include <fstream>
#include "pin.H"
extern "C"
{
#include "xed-interface.h"
    using std::cerr;
    using std::endl;
    using std::string;
}

class simple_test_t
{
  public:
    simple_test_t() : knob_output_file(KNOB_MODE_WRITEONCE, "pintool", "o", "leave.out", "specify profile file name") { out = 0; }

    ~simple_test_t()
    {
        if (out) out->close();
    }

    std::ofstream* out;
    KNOB< string > knob_output_file;

    void activate()
    {
        string filename = knob_output_file.Value();
        out             = new std::ofstream(filename.c_str());

        PIN_AddThreadStartFunction(reinterpret_cast< THREAD_START_CALLBACK >(thread_begin), this);
        PIN_AddThreadFiniFunction(reinterpret_cast< THREAD_FINI_CALLBACK >(thread_end), this);

        TRACE_AddInstrumentFunction(reinterpret_cast< TRACE_INSTRUMENT_CALLBACK >(instrument_trace), this);
        *out << "tool activated" << endl;
    }

    static void thread_begin(THREADID tid, CONTEXT* ctxt, INT32 flags, simple_test_t* pthis) {}

    static void thread_end(THREADID tid, const CONTEXT* ctxt, INT32 code, simple_test_t* pthis) {}

    static void print_memop(ADDRINT memea, ADDRINT memsize, ADDRINT pc, THREADID tid, simple_test_t* pthis)
    {
        *pthis->out << std::hex << std::setw(sizeof(ADDRINT) * 2) << pc << " TID: " << std::setw(2) << tid
                    << " LEAVE memea: " << std::setw(sizeof(ADDRINT) * 2) << memea << " length " << memsize << std::endl;
    }

    void instrument_instruction(INS ins)
    {
        *out << "INSTRUMENT: " << std::setw(16) << std::hex << INS_Address(ins) << std::dec << " " << INS_Disassemble(ins)
             << std::endl;

        INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(print_memop), IARG_MEMORYREAD_EA, IARG_MEMORYREAD_SIZE, IARG_INST_PTR,
                       IARG_THREAD_ID, IARG_PTR, this, IARG_END);
    }

    static bool check_for_leave(INS ins, simple_test_t* pthis)
    {
        xed_iclass_enum_t iclass = static_cast< xed_iclass_enum_t >(INS_Opcode(ins));
        if (iclass == XED_ICLASS_LEAVE) return true;
        return false;
    }

    static void instrument_trace(TRACE trace, simple_test_t* pthis)
    {
        for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
            for (INS ins = BBL_InsHead(bbl); INS_Valid(ins); ins = INS_Next(ins))
                if (check_for_leave(ins, pthis)) pthis->instrument_instruction(ins);
    }

}; // class

int usage()
{
    cerr << "Usage: ..." << endl;
    cerr << KNOB_BASE::StringKnobSummary() << endl;
    return 1;
}

int main(int argc, char* argv[])
{
    static simple_test_t t; // must be before usage...

    PIN_InitSymbols();
    if (PIN_Init(argc, argv)) return usage();

    t.activate();

    // Never returns
    PIN_StartProgram();

    return 0;
}
