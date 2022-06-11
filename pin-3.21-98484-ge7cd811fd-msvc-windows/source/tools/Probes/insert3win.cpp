/*
 * Copyright (C) 2008-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 *  This is an example of probing RtlAllocateHeap
 */

#include "pin.H"
namespace WINDOWS
{
#include <Windows.h>
}
#include <iostream>

using std::cout;
using std::dec;
using std::endl;
using std::flush;
using std::hex;

VOID Before_RtlAllocateHeap0(WINDOWS::PVOID hHeap, WINDOWS::ULONG dwFlags, WINDOWS::SIZE_T dwBytes)
{
    cout << "Before_RtlAllocateHeap0: RtlAllocateHeap( " << hex << (ADDRINT)hHeap << ", " << dwFlags << ", " << dwBytes << ")"
         << dec << endl
         << flush;
}

VOID Before_RtlAllocateHeap1(WINDOWS::PVOID hHeap, WINDOWS::ULONG dwFlags, WINDOWS::SIZE_T dwBytes)
{
    cout << "Before_RtlAllocateHeap1: RtlAllocateHeap( " << hex << (ADDRINT)hHeap << ", " << dwFlags << ", " << dwBytes << ")"
         << dec << endl
         << flush;
}

VOID Before_Free0(WINDOWS::PVOID ptr) { cout << "Before_Free0: Free " << hex << (ADDRINT)ptr << dec << endl << flush; }

VOID Before_Free1(WINDOWS::PVOID ptr) { cout << "Before_Free1: Free " << hex << (ADDRINT)ptr << dec << endl << flush; }

/*
 * process_loaded_image: Called every time when new image is loaded.
 */
static VOID process_loaded_image(IMG image, VOID* value)
{
    if (!IMG_Valid(image)) return;

    RTN allocRtn = RTN_FindByName(image, "RtlAllocateHeap");
    if (RTN_Valid(allocRtn) && RTN_IsSafeForProbedInsertion(allocRtn))
    {
        cout << "RTN_InsertCallProbed on "
             << "RtlAllocateHeap" << endl
             << flush;

        RTN_InsertCallProbed(allocRtn, IPOINT_BEFORE, AFUNPTR(Before_RtlAllocateHeap0), IARG_FUNCARG_ENTRYPOINT_VALUE, 0,
                             IARG_FUNCARG_ENTRYPOINT_VALUE, 1, IARG_FUNCARG_ENTRYPOINT_VALUE, 2, IARG_END);

        RTN_InsertCallProbed(allocRtn, IPOINT_BEFORE, AFUNPTR(Before_RtlAllocateHeap1), IARG_FUNCARG_ENTRYPOINT_VALUE, 0,
                             IARG_FUNCARG_ENTRYPOINT_VALUE, 1, IARG_FUNCARG_ENTRYPOINT_VALUE, 2, IARG_END);
    }

    RTN freeRtn = RTN_FindByName(image, "free");

    if (RTN_Valid(freeRtn) && RTN_IsSafeForProbedInsertion(freeRtn))
    {
        cout << "RTN_InsertCallProbed on "
             << "Free" << endl
             << flush;

        RTN_InsertCallProbed(freeRtn, IPOINT_BEFORE, AFUNPTR(Before_Free0), IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_END);

        RTN_InsertCallProbed(freeRtn, IPOINT_BEFORE, AFUNPTR(Before_Free1), IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_END);
    }
}

int main(int argc, char** argv)
{
    PIN_InitSymbols();
    if (PIN_Init(argc, argv)) return -1;

    IMG_AddInstrumentFunction(process_loaded_image, 0);
    PIN_StartProgramProbed();
}
