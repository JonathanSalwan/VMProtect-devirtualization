/*
 * Copyright (C) 2004-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 *  Print a detailed disassemble data on each IMG, loops run forward.
 */

#include <iostream>
#include <fstream>
#include <iomanip>
#include "pin.H"
using std::dec;
using std::endl;
using std::hex;
using std::ofstream;
using std::showbase;
using std::string;

ofstream* out = 0;

KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "imageload.out", "specify output file name");

KNOB< BOOL > KnobImageOnly(KNOB_MODE_WRITEONCE, "pintool", "l", "0", "List the loaded images");

VOID ImageLoad(IMG img, VOID* v)
{
    *out << "Tool loading " << IMG_Name(img) << " at " << IMG_LoadOffset(img) << endl;
    if (KnobImageOnly) return;

    for (SEC sec = IMG_SecHead(img); SEC_Valid(sec); sec = SEC_Next(sec))
    {
        *out << "  sec " << SEC_Name(sec) << " " << SEC_Address(sec) << ":" << SEC_Size(sec) << endl;
        for (RTN rtn = SEC_RtnHead(sec); RTN_Valid(rtn); rtn = RTN_Next(rtn))
        {
            *out << "    rtn " << RTN_Name(rtn) << " " << RTN_Address(rtn) << ":" << RTN_Size(rtn) << endl;

            RTN_Open(rtn);

            for (INS ins = RTN_InsHead(rtn); INS_Valid(ins); ins = INS_Next(ins))
            {
                // Just print first and last
                if (!INS_Valid(INS_Prev(ins)) || !INS_Valid(INS_Next(ins)))
                {
                    *out << "      " << INS_Address(ins);

#if 0
                    *out << " " << INS_Disassemble(ins) << " read:";
                    
                    for (UINT32 i = 0; i < INS_MaxNumRRegs(ins); i++)
                    {
                        *out << " " << REG_StringShort(INS_RegR(ins, i));
                    }
                    *out << " writes:";
                    for (UINT32 i = 0; i < INS_MaxNumWRegs(ins); i++)
                    {
                        *out << " " << REG_StringShort(INS_RegW(ins, i));
                    }
#endif

                    *out << endl;
                }
            }

            RTN_Close(rtn);
        }
    }
}

VOID ImageUnload(IMG img, VOID* v) { *out << "Tool unloading " << IMG_Name(img) << " at " << IMG_LoadOffset(img) << endl; }

VOID Trace(TRACE trace, VOID* v)
{
    INS head = BBL_InsHead(TRACE_BblHead(trace));

    INT32 line;
    INT32 column;
    string file;

    PIN_GetSourceLocation(INS_Address(head), &column, &line, &file);
    if (file != "")
    {
        *out << file << ":" << dec << line << ":" << column << " " << hex;
    }

    RTN rtn = RTN_FindByAddress(INS_Address(head));

    if (RTN_Valid(rtn))
    {
        IMG img = SEC_Img(RTN_Sec(rtn));

        if (IMG_Valid(img))
        {
            *out << IMG_Name(img) << ":" << RTN_Name(rtn) << "+" << INS_Address(head) - RTN_Address(rtn) << " "
                 << INS_Disassemble(head) << endl;
        }
    }
}

int main(INT32 argc, CHAR** argv)
{
    PIN_InitSymbols();
    PIN_Init(argc, argv);

    out = new ofstream(KnobOutputFile.Value().c_str());
    *out << hex << showbase;

    IMG_AddInstrumentFunction(ImageLoad, 0);
    IMG_AddUnloadFunction(ImageUnload, 0);

    if (!KnobImageOnly) TRACE_AddInstrumentFunction(Trace, 0);

    // Never returns
    PIN_StartProgram();

    return 0;
}
