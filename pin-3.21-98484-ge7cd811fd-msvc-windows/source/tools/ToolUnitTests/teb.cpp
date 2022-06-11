/*
 * Copyright (C) 2007-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include "pin.H"
#include <string>
#include <iostream>

namespace WINDOWS
{
#include <windows.h>
}

using std::cout;
using std::endl;
using std::vector;
//=======================================================================
// This is a tool that instruments the GetLastError() function and changes
// TEB.LastErrorValue. This change should not be seen in the application.
//
// Also this tool uses very big automatic variables in order to verify that
// pin handles stack bounds in TEB correctly.
//=======================================================================

// Address of the GetLastError API.
ADDRINT pfnGetLastError = 0;

typedef UINT64 COUNTER;
const UINT32 MAX_INDEX = 8000;
const UINT32 VECT_SIZE = 4;

struct CSTATS
{
    CSTATS() { memset(counters, 0, sizeof(COUNTER) * MAX_INDEX); }
    COUNTER counters[MAX_INDEX];
};

// very big static object initialized before main()
vector< CSTATS > MyGlobalVect(VECT_SIZE);

//=======================================================================
//
VOID InGetLastError()
{
    static BOOL first = TRUE;
    if (first)
    {
        cout << "In GetLastError" << endl;
        // test very big variables on stack
        first = FALSE;
        vector< CSTATS > myVect(VECT_SIZE);
        myVect[0].counters[0] = 1;
        MyGlobalVect          = myVect;

        CSTATS myStat;
        myStat.counters[0] = 2;
        MyGlobalVect[1]    = myStat;
    }
    // Change TEB.LastErrorValue to something different from 777
    WINDOWS::SetLastError(999);
}

//=======================================================================
// This function is called for every instruction and instruments the
// GetLastError() function
VOID Instruction(INS ins, VOID* v)
{
    if (INS_Address(ins) == pfnGetLastError)
    {
        INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(InGetLastError), IARG_END);
    }
}

//=======================================================================
int main(int argc, CHAR* argv[])
{
    pfnGetLastError = (ADDRINT)WINDOWS::GetProcAddress(WINDOWS::GetModuleHandle("kernel32.dll"), "GetLastError");
    PIN_Init(argc, argv);
    INS_AddInstrumentFunction(Instruction, 0);
    PIN_StartProgram();

    return 0;
}
