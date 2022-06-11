/*
 * Copyright (C) 2010-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdlib.h>
#include <stdint.h>

#include <iostream>
#include <fstream>
#include <map>
#include <vector>

#include "pin.H"
using std::cerr;
using std::cout;
using std::endl;
using std::flush;
using std::hex;
using std::left;
using std::ofstream;
using std::showbase;

ofstream outfile;

bool FindAddressInImage(IMG img, ADDRINT addr)
{
    for (UINT32 i = 0; i < IMG_NumRegions(img); i++)
    {
        if ((addr >= IMG_RegionLowAddress(img, i)) && (addr <= IMG_RegionHighAddress(img, i)))
        {
            return true;
        }
    }
    return false;
}

void InstImage(IMG img, void* v)
{
    outfile << "-----------" << endl << "Image name = " << IMG_Name(img) << endl << flush;
    ADDRINT mappedStart = IMG_StartAddress(img);
    ADDRINT mappedEnd   = mappedStart + IMG_SizeMapped(img);

    outfile << hex << showbase;
    outfile << "mapped start " << mappedStart << " mapped end " << mappedEnd << endl;

    for (UINT32 i = 0; i < IMG_NumRegions(img); i++)
    {
        outfile << "Region #" << i << ": low addr " << IMG_RegionLowAddress(img, i) << " high address "
                << IMG_RegionHighAddress(img, i) << endl;
    }

    for (SEC sec = IMG_SecHead(img); SEC_Valid(sec); sec = SEC_Next(sec))
    {
        ADDRINT secAddr = SEC_Address(sec);
        if (!SEC_Size(sec) && (secAddr == (IMG_HighAddress(img) + 1)))
        {
            continue;
        }
        if (secAddr && SEC_Mapped(sec))
        {
            if (!FindAddressInImage(img, secAddr))
            {
                cout << "ERROR: Image " << IMG_Name(img) << "Section " << SEC_Name(sec) << " address wrong ptr: " << secAddr
                     << endl;
                PIN_ExitProcess(-1);
            }
        }
        outfile << "Section \"";
        outfile.width(30);
        outfile << left << SEC_Name(sec) << "\"";
        outfile.width(0);
        outfile << " addr ptr " << secAddr << endl;
    }
}

/*!
 *  Print out help message.
 */
INT32 Usage()
{
    cerr << "Pintool Usage. " << endl;
    cerr << KNOB_BASE::StringKnobSummary() << endl;

    return -1;
}

int main(int argc, char** argv)
{
    PIN_InitSymbols();

    if (PIN_Init(argc, argv))
    {
        return Usage();
    }

    outfile.open("sectest.out");

    IMG_AddInstrumentFunction(InstImage, 0);

    PIN_StartProgramProbed();
    return 0;
}
