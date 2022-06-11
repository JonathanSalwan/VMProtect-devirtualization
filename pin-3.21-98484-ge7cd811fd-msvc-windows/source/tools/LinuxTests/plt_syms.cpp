/*
 * Copyright (C) 2016-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include "pin.H"
#include <iostream>
#include <fstream>
using std::cerr;
using std::cout;
using std::endl;
using std::string;

/* ===================================================================== */

/*!
 * Print out the error message and exit the process.
 */
static void AbortProcess(const string& msg, unsigned long code)
{
    cerr << "Test aborted: " << msg << " with code " << code << endl;
    PIN_WriteErrorMessage(msg.c_str(), 1002, PIN_ERR_FATAL, 0);
    PIN_ExitProcess(1);
}

/*!
 * Called before write@plt.
 */
void Before_Write(VOID* s)
{
    string str((CHAR*)s);
    if ("printing using write\n" != str)
    {
        AbortProcess("Argument for write@plt isn't expected!", 0);
    }
    cout << "calling write@plt" << endl;
}

/* ===================================================================== */
VOID ImageLoad(IMG img, VOID* v)
{
    if (IMG_IsMainExecutable(img))
    {
        PROTO proto_write = PROTO_Allocate(PIN_PARG(ssize_t), CALLINGSTD_DEFAULT, "write@plt", PIN_PARG(INT32), PIN_PARG(VOID*),
                                           PIN_PARG(size_t), PIN_PARG_END());

        RTN rtn = RTN_FindByName(img, "write@plt");
        if (RTN_Valid(rtn))
        {
            if (!RTN_IsSafeForProbedInsertion(rtn))
            {
                AbortProcess("Cannot insert before " + RTN_Name(rtn) + " in " + IMG_Name(img), 0);
            }
            RTN_InsertCallProbed(rtn, IPOINT_BEFORE, AFUNPTR(Before_Write), IARG_PROTOTYPE, proto_write,
                                 IARG_FUNCARG_ENTRYPOINT_VALUE, 1, IARG_END);
            cout << "Inserted probe call before write@plt: " << IMG_Name(img) << endl;
        }

        PROTO_Free(proto_write);
    }
}

/* ===================================================================== */

int main(int argc, CHAR* argv[])
{
    PIN_InitSymbols();
    PIN_Init(argc, argv);

    IMG_AddInstrumentFunction(ImageLoad, NULL);

    PIN_StartProgramProbed();

    return 1;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
