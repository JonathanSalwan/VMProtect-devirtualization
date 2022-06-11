/*
 * Copyright (C) 2007-2021 Intel Corporation.
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
#include <stdio.h>

FILE* fp;

typedef WINDOWS::LPVOID(__stdcall* HeapAllocType)(WINDOWS::HANDLE hHeap, WINDOWS::DWORD dwFlags, WINDOWS::DWORD dwBytes);

// the calling convention of ReplacementFunc should be the default calling convention.
WINDOWS::LPVOID /*WINAPI*/ ReplacementFunc(HeapAllocType originalHeapAlloc, WINDOWS::HANDLE hHeap, WINDOWS::DWORD dwFlags,
                                           WINDOWS::DWORD dwBytes, CONTEXT* pPinContext, ADDRINT returnIp)

{
    fprintf(fp, "Inside Probe ReplacementFunc\n");
    WINDOWS::LPVOID ptr = NULL;
    ptr                 = originalHeapAlloc(hHeap, dwFlags, dwBytes);

    return ptr;
}

void InsertProbe(IMG img, char* funcName)
{
    /*
    printf ("Image %s\n", IMG_Name(img).c_str());

    for (SEC sec = IMG_SecHead(img); SEC_Valid(sec); sec = SEC_Next(sec))
    {     
        for (RTN rtn = SEC_RtnHead(sec); RTN_Valid(rtn); rtn = RTN_Next(rtn))
        {
            printf ("  Rtn: %s  %s\n", RTN_Name(rtn).c_str(), funcName);
            if (strstr( RTN_Name(rtn).c_str(), funcName))
            {
                printf ("    found\n");
                
            }
        } 
    }
    */
    RTN allocRtn = RTN_FindByName(img, funcName);
    if (RTN_Valid(allocRtn) && RTN_IsSafeForProbedReplacement(allocRtn))
    {
        fprintf(fp, "RTN_ReplaceSignatureProbed on %s\n", funcName);
        PROTO protoHeapAlloc = PROTO_Allocate(PIN_PARG(void*), CALLINGSTD_STDCALL, "protoHeapAlloc", PIN_PARG(WINDOWS::HANDLE),
                                              PIN_PARG(WINDOWS::DWORD), PIN_PARG(WINDOWS::DWORD), PIN_PARG_END());

        RTN_ReplaceSignatureProbed(allocRtn, AFUNPTR(ReplacementFunc), IARG_PROTOTYPE, protoHeapAlloc, IARG_ORIG_FUNCPTR,
                                   IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_FUNCARG_ENTRYPOINT_VALUE, 1,
                                   IARG_FUNCARG_ENTRYPOINT_VALUE, 2, IARG_CONTEXT, IARG_RETURN_IP, IARG_END);
        PROTO_Free(protoHeapAlloc);
    }
}

/*
 * process_loaded_image: Called every time when new image is loaded.
 */
static VOID process_loaded_image(IMG image, VOID* value)
{
    if (!IMG_Valid(image)) return;

    InsertProbe(image, "RtlAllocateHeap");
}

int main(int argc, char** argv)
{
    PIN_InitSymbols();
    if (PIN_Init(argc, argv)) return -1;

    fp = fopen("probeheapalloc.outfile", "w");
    IMG_AddInstrumentFunction(process_loaded_image, 0);
    PIN_StartProgramProbed();
}
