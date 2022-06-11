/*
 * Copyright (C) 2012-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

//
// This tool prints a trace of image load and unload events
//

#include "pin.H"
#include <stdio.h>

namespace WINDOWS
{
#include <windows.h>
}

WINDOWS::PIMAGE_DATA_DIRECTORY GetExportDirectory(ADDRINT base)
{
    if (base == 0)
    {
        return 0;
    }
    WINDOWS::PIMAGE_DOS_HEADER pDos = reinterpret_cast< WINDOWS::PIMAGE_DOS_HEADER >(base);

    // Returns FALSE when not DOS MZ header
    if (pDos->e_magic != IMAGE_DOS_SIGNATURE)
    {
        return 0;
    }

    const WINDOWS::PIMAGE_NT_HEADERS pHeaders =
        reinterpret_cast< WINDOWS::PIMAGE_NT_HEADERS >(reinterpret_cast< WINDOWS::ULONG_PTR >(pDos) + pDos->e_lfanew);

    // check that this is PE/COFF image
    if (pHeaders->Signature != IMAGE_NT_SIGNATURE)
    {
        return 0;
    }

    WINDOWS::PIMAGE_DATA_DIRECTORY pExpDirEntry = &pHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
    if ((pExpDirEntry->VirtualAddress == 0) || (pExpDirEntry->Size == 0))
    {
        return NULL; // No export directory
    }

    return pExpDirEntry;
}

// Pin calls this function every time a new img is loaded
VOID ImageLoad(IMG img, VOID* v)
{
    if (!IMG_IsMainExecutable(img)) return;

    printf("%s loaded\n", IMG_Name(img).c_str());
    fflush(stdout);

    ADDRINT imageBase                      = IMG_LowAddress(img);
    WINDOWS::PIMAGE_DATA_DIRECTORY pExpDir = GetExportDirectory(imageBase);
    if ((pExpDir == 0) || (pExpDir->Size == 0))
    {
        // Failure: Executable image lacks export directory.
        printf("ERROR: No export directory in executable image\n");
        fflush(stdout);
        exit(3);
    }
    ADDRINT exportBase = imageBase + pExpDir->VirtualAddress;

    // First check that bytes in export directory range do not belong to a RTN
    for (ADDRINT addr = exportBase; addr < exportBase + pExpDir->Size; ++addr)
    {
        if (RTN_FindByAddress(addr) != RTN_Invalid())
        {
            // Test failure. Byte in export directory belongs to a RTN.
            printf("ERROR: Data from export directory included in RTN\n");
            fflush(stdout);
            exit(1);
        }
    }

    // Second check RTN size. RTN range should not overlap with export directory range.
    for (SEC sec = IMG_SecHead(img); sec != SEC_Invalid(); sec = SEC_Next(sec))
    {
        for (RTN rtn = SEC_RtnHead(sec); rtn != RTN_Invalid(); rtn = RTN_Next(rtn))
        {
            if (((RTN_Address(rtn) <= exportBase) && (RTN_Address(rtn) + RTN_Size(rtn) > exportBase)) ||
                ((RTN_Address(rtn) > exportBase) && (exportBase + pExpDir->Size > RTN_Address(rtn))))
            {
                // Test failure. RTN overlaps with export directory.
                printf("ERROR: RTN overlaps with export directory\n");
                fflush(stdout);
                exit(2);
            }
        }
    }

    return;
}

// argc, argv are the entire command line, including pin -t <toolname> -- ...
int main(int argc, char* argv[])
{
    // Initialize symbol processing
    PIN_InitSymbols();

    // Initialize pin
    if (PIN_Init(argc, argv) != 0)
    {
        return 1;
    }

    // Register ImageLoad to be called when an image is loaded
    IMG_AddInstrumentFunction(ImageLoad, 0);

    // Start the program, never returns
    PIN_StartProgram();

    return 0;
}
