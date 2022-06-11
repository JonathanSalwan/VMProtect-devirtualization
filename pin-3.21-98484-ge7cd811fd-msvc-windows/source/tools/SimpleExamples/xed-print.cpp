/*
 * Copyright (C) 2004-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 * This file exemplifies XED usage on IA-32 and Intel(R) 64 architectures.
 */

#include "pin.H"
extern "C"
{
#include "xed-interface.h"
}
#include <iostream>
#include <iomanip>
#include <fstream>
using std::cerr;
using std::endl;
using std::hex;

/* ===================================================================== */
/* Global Variables */
/* ===================================================================== */
std::ofstream* out = 0;

/* ===================================================================== */
/* Commandline Switches */
/* ===================================================================== */

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 Usage()
{
    cerr << "This tool prints IA-32 and Intel(R) 64 instructions" << endl;
    cerr << KNOB_BASE::StringKnobSummary();
    cerr << endl;
    return -1;
}

/* ===================================================================== */

VOID use_xed(ADDRINT pc)
{
#if defined(TARGET_IA32E)
    static const xed_state_t dstate = {XED_MACHINE_MODE_LONG_64, XED_ADDRESS_WIDTH_64b};
#else
    static const xed_state_t dstate = {XED_MACHINE_MODE_LEGACY_32, XED_ADDRESS_WIDTH_32b};
#endif
    xed_decoded_inst_t xedd;
    xed_decoded_inst_zero_set_mode(&xedd, &dstate);

    //Pass in the proper length: 15 is the max. But if you do not want to
    //cross pages, you can pass less than 15 bytes, of course, the
    //instruction might not decode if not enough bytes are provided.
    const unsigned int max_inst_len = 15;

    xed_error_enum_t xed_code = xed_decode(&xedd, reinterpret_cast< UINT8* >(pc), max_inst_len);
    BOOL xed_ok               = (xed_code == XED_ERROR_NONE);
    if (xed_ok)
    {
        *out << hex << std::setw(8) << pc << " ";
        char buf[2048];

        // set the runtime adddress for disassembly
        xed_uint64_t runtime_address = static_cast< xed_uint64_t >(pc);

        xed_format_context(XED_SYNTAX_INTEL, &xedd, buf, 2048, runtime_address, 0, 0);
        *out << buf << endl;
    }
}

/* ===================================================================== */

VOID Instruction(INS ins, VOID* v) { INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)use_xed, IARG_INST_PTR, IARG_END); }

/* ===================================================================== */

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */

int main(int argc, char* argv[])
{
    out = new std::ofstream("xed-print.out");
    if (PIN_Init(argc, argv)) return Usage();
    INS_AddInstrumentFunction(Instruction, 0);
    PIN_StartProgram(); // Never returns
    return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
