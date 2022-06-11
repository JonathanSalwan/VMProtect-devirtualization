/*
 * Copyright (C) 2006-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <cstdio>
#include <cstring>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <cstdlib>
#include <map>
#include "pin.H"
#include "instlib.H"
using std::iostream;
using std::ostringstream;

std::map< ADDRINT, std::string > disAssemblyMap;

static ADDRINT dl_debug_state_Addr    = 0;
static ADDRINT dl_debug_state_AddrEnd = 0;
static BOOL justFoundDlDebugState     = FALSE;

static char nibble_to_ascii_hex(UINT8 i)
{
    if (i < 10) return i + '0';
    if (i < 16) return i - 10 + 'A';
    return '?';
}

static void print_hex_line(char* buf, const UINT8* array, const int length)
{
    int n = length;
    int i = 0;
    if (length == 0) n = XED_MAX_INSTRUCTION_BYTES;
    for (i = 0; i < n; i++)
    {
        buf[2 * i + 0] = nibble_to_ascii_hex(array[i] >> 4);
        buf[2 * i + 1] = nibble_to_ascii_hex(array[i] & 0xF);
    }
    buf[2 * i] = 0;
}

static string disassemble(UINT64 start, UINT64 stop)
{
    UINT64 pc = start;
    xed_state_t dstate;
    xed_syntax_enum_t syntax = XED_SYNTAX_INTEL;
    xed_error_enum_t xed_error;
    xed_decoded_inst_t xedd;
    ostringstream os;
    if (sizeof(ADDRINT) == 4)
        xed_state_init(&dstate, XED_MACHINE_MODE_LEGACY_32, XED_ADDRESS_WIDTH_32b, XED_ADDRESS_WIDTH_32b);
    else
        xed_state_init(&dstate, XED_MACHINE_MODE_LONG_64, XED_ADDRESS_WIDTH_64b, XED_ADDRESS_WIDTH_64b);

    /*while( pc < stop )*/ {
        xed_decoded_inst_zero_set_mode(&xedd, &dstate);
        UINT32 len = 15;
        if (stop - pc < 15) len = stop - pc;

        xed_error              = xed_decode(&xedd, reinterpret_cast< const UINT8* >(pc), len);
        bool okay              = (xed_error == XED_ERROR_NONE);
        iostream::fmtflags fmt = os.flags();
        os << std::setfill('0') << "XDIS " << std::hex << std::setw(sizeof(ADDRINT) * 2) << pc << std::dec << ": "
           << std::setfill(' ') << std::setw(4);

        if (okay)
        {
            char buffer[200];
            unsigned int dec_len, sp;

            os << xed_extension_enum_t2str(xed_decoded_inst_get_extension(&xedd));
            dec_len = xed_decoded_inst_get_length(&xedd);
            print_hex_line(buffer, reinterpret_cast< UINT8* >(pc), dec_len);
            os << " " << buffer;
            for (sp = dec_len; sp < 12; sp++) // pad out the instruction bytes
                os << "  ";
            os << " ";
            memset(buffer, 0, 200);
            int dis_okay = xed_format_context(syntax, &xedd, buffer, 200, pc, 0, 0);
            if (dis_okay)
                os << buffer << endl;
            else
                os << "Error disasassembling pc 0x" << std::hex << pc << std::dec << endl;
            pc += dec_len;
        }
        else
        { // print the byte and keep going.
            UINT8 memval = *reinterpret_cast< UINT8* >(pc);
            os << "???? " // no extension
               << std::hex << std::setw(2) << std::setfill('0') << static_cast< UINT32 >(memval) << std::endl;
            pc += 1;
        }
        os.flags(fmt);
    }
    return os.str();
}

VOID ImageLoad(IMG img, VOID* v)
{
    printf("ImageLoad %s\n", IMG_Name(img).c_str());
    for (SEC sec = IMG_SecHead(img); SEC_Valid(sec); sec = SEC_Next(sec))
    {
        for (RTN rtn = SEC_RtnHead(sec); RTN_Valid(rtn); rtn = RTN_Next(rtn))
        {
            if (strcmp(RTN_Name(rtn).c_str(), "_dl_debug_state") == 0)
            {
                printf("  RTN %s at %p\n", RTN_Name(rtn).c_str(), reinterpret_cast< void* >(RTN_Address(rtn)));
                printf("    ** found _dl_debug_state\n");
                dl_debug_state_Addr   = RTN_Address(rtn);
                justFoundDlDebugState = TRUE;
            }
            else if (justFoundDlDebugState)
            {
                printf("  RTN %s at %p\n", RTN_Name(rtn).c_str(), reinterpret_cast< void* >(RTN_Address(rtn)));
                dl_debug_state_AddrEnd = RTN_Address(rtn);
                justFoundDlDebugState  = FALSE;
                printf("    ** _dl_debug_state from %p to %p\n", reinterpret_cast< void* >(dl_debug_state_Addr),
                       reinterpret_cast< void* >(dl_debug_state_AddrEnd));
            }
        }
    }
}

static VOID AtBranch(ADDRINT ip, ADDRINT target, BOOL taken)
{
    if (target >= dl_debug_state_Addr && target < dl_debug_state_AddrEnd)
    {
        string s = disassemble((ip), (ip) + 15);

        printf("  branch %s   branches to range of interest %p  taken %d\n", s.c_str(), reinterpret_cast< void* >(target), taken);
        fflush(stdout);
    }
    if (ip >= dl_debug_state_Addr && ip < dl_debug_state_AddrEnd)
    {
        string s = disassemble((ip), (ip) + 15);
        printf("  instruction in range of interest executed %s\n", s.c_str());
        fflush(stdout);
    }
}

static VOID AtNonBranch(ADDRINT ip)
{
    if (ip >= dl_debug_state_Addr && ip < dl_debug_state_AddrEnd)
    {
        string s = disassemble((ip), (ip) + 15);
        printf("  instruction in range of interest executed %s\n", s.c_str());
        fflush(stdout);
    }
}

static VOID Instruction(INS ins, VOID* v)
{
    //disAssemblyMap[INS_Address(ins)] = INS_Disassemble(ins);
    if (INS_IsControlFlow(ins))
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)AtBranch, IARG_INST_PTR, IARG_BRANCH_TARGET_ADDR, IARG_BRANCH_TAKEN,
                       IARG_END);
    else
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)AtNonBranch, IARG_INST_PTR, IARG_END);
}

int main(INT32 argc, CHAR** argv)
{
    PIN_Init(argc, argv);
    PIN_InitSymbols();

    INS_AddInstrumentFunction(Instruction, 0);
    IMG_AddInstrumentFunction(ImageLoad, 0);

    PIN_StartProgram();
    return 0;
}
