/*
 * Copyright (C) 2004-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include "pin.H"
#include <iostream>

UINT64 icount = 0;

static VOID PrintContext(CONTEXT* ctxt)
{
#if defined(TARGET_IA32) || defined(TARGET_IA32E)
    cout << "gax:   " << ctxt->_gax << endl;
    cout << "gbx:   " << ctxt->_gbx << endl;
    cout << "gcx:   " << ctxt->_gcx << endl;
    cout << "gdx:   " << ctxt->_gdx << endl;
    cout << "gsi:   " << ctxt->_gsi << endl;
    cout << "gdi:   " << ctxt->_gdi << endl;
    cout << "gbp:   " << ctxt->_gbp << endl;
    cout << "sp:    " << ctxt->_stack_ptr << endl;

#if defined(TARGET_IA32E)
    cout << "r8:    " << ctxt->_r8 << endl;
    cout << "r9:    " << ctxt->_r9 << endl;
    cout << "r10:   " << ctxt->_r10 << endl;
    cout << "r11:   " << ctxt->_r11 << endl;
    cout << "r12:   " << ctxt->_r12 << endl;
    cout << "r13:   " << ctxt->_r13 << endl;
    cout << "r14:   " << ctxt->_r14 << endl;
    cout << "r15:   " << ctxt->_r15 << endl;
#endif

    cout << "ss:    " << ctxt->_ss << endl;
    cout << "cs:    " << ctxt->_cs << endl;
    cout << "ds:    " << ctxt->_ds << endl;
    cout << "es:    " << ctxt->_es << endl;
    cout << "fs:    " << ctxt->_fs << endl;
    cout << "gs:    " << ctxt->_gs << endl;
    cout << "gflags:" << ctxt->_gflags << endl;

    cout << "mxcsr: " << ctxt->_fxsave._mxcsr << endl;

#endif
}

VOID ShowContext(VOID* ip, VOID* handle, ADDRINT gax)
{
    CONTEXT ctxt;

    // Capture the context. This must be done first before some floating point
    // registers have been overwritten
    PIN_MakeContext(handle, &ctxt);

    static bool first = false;

    if (first)
    {
        cout << "ip:    " << ip << endl;

        PrintContext(&ctxt);

        cout << endl;
    }

#if defined(TARGET_IA32) || defined(TARGET_IA32E)
    ASSERTX(gax == ctxt._gax);
#endif
}

VOID Trace(TRACE tr, VOID* v)
{
    TRACE_InsertCall(tr, IPOINT_BEFORE, AFUNPTR(ShowContext), IARG_INST_PTR, IARG_CONTEXT, IARG_REG_VALUE, REG_GAX, IARG_END);
}

int main(int argc, char* argv[])
{
    cout << hex;
    cout.setf(ios::showbase);

    PIN_Init(argc, argv);

    TRACE_AddInstrumentFunction(Trace, 0);

    // Never returns
    PIN_StartProgram();

    return 0;
}
