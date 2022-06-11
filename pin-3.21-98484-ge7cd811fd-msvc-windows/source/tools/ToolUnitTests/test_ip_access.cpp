/*
 * Copyright (C) 2008-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <assert.h>
#include <stdio.h>
#include "pin.H"

ADDRINT globalIpAtReadBefore                        = 0;
ADDRINT globalEaReadBefore                          = 0;
ADDRINT globalRegInstPtrAtReadBefore                = 0;
ADDRINT globalIpOfReadRecordedAtInstrumentationTime = 0;
ADDRINT globalRegInstPtrAtReadAfter                 = 0;
int globalReadInsSize                               = 0;

ADDRINT globalIpAtWriteBefore                        = 0;
ADDRINT globalEaWriteBefore                          = 0;
ADDRINT globalRegInstPtrAtWriteBefore                = 0;
ADDRINT globalIpOfWriteRecordedAtInstrumentationTime = 0;
ADDRINT globalRegInstPtrAtWriteAfter                 = 0;
int globalWriteInsSize                               = 0;

BOOL instrumentedReadFromIpWithNoOffset  = FALSE;
BOOL instrumentedWriteFromIpWithNoOffset = FALSE;

static void IpReadBefore(ADDRINT ip, ADDRINT ea, ADDRINT rip)
{
    globalIpAtReadBefore         = ip;
    globalEaReadBefore           = ea;
    globalRegInstPtrAtReadBefore = rip;
}

static void IpWriteBefore(ADDRINT ip, ADDRINT ea, ADDRINT rip)
{
    globalIpAtWriteBefore         = ip;
    globalEaWriteBefore           = ea;
    globalRegInstPtrAtWriteBefore = rip;
}

static void IpReadAfter(ADDRINT rip) { globalRegInstPtrAtReadAfter = rip; }

static void IpWriteAfter(ADDRINT rip) { globalRegInstPtrAtWriteAfter = rip; }

VOID Instruction(INS ins, VOID* v)
{
    if (INS_IsMemoryRead(ins) && !instrumentedReadFromIpWithNoOffset)
    {
        BOOL readsFromIpWithNoOffset = FALSE;
        for (UINT32 i = 0; i < INS_OperandCount(ins); i++)
        {
            if (!INS_OperandIsMemory(ins, i)) continue;

            if (INS_OperandMemoryBaseReg(ins, i) == REG_INST_PTR && INS_OperandMemoryDisplacement(ins, i) == 0)
            {
                readsFromIpWithNoOffset = TRUE;
                break;
            }
        }
        if (!readsFromIpWithNoOffset)
        {
            return;
        }
        instrumentedReadFromIpWithNoOffset = TRUE; // only instrument one of these
        printf("Instrumenting [ip] read   %p   %s\n", INS_Address(ins), INS_Disassemble(ins).c_str());
        fflush(stdout);
        globalIpOfReadRecordedAtInstrumentationTime = INS_Address(ins);
        globalReadInsSize                           = INS_Size(ins);
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)IpReadBefore, IARG_INST_PTR, IARG_MEMORYREAD_EA, IARG_REG_VALUE, REG_INST_PTR,
                       IARG_END);
        INS_InsertCall(ins, IPOINT_AFTER, (AFUNPTR)IpReadAfter, IARG_REG_VALUE, REG_INST_PTR, IARG_END);
    }
    else if (INS_IsMemoryWrite(ins) && !instrumentedWriteFromIpWithNoOffset)
    {
        /*
        const xed_decoded_inst_t* xedd = INS_XedDec(ins);
        
        xed_reg_enum_t breg1 = xed_decoded_inst_get_base_reg(xedd,0);
        if (breg1== XED_REG_RIP) 
        {
            readsFromIpWithNoOffset = TRUE;
        }
        */
        BOOL writesFromIpWithNoOffset = FALSE;
        for (UINT32 i = 0; i < INS_OperandCount(ins); i++)
        {
            if (!INS_OperandIsMemory(ins, i)) continue;

            if (INS_OperandMemoryBaseReg(ins, i) == REG_INST_PTR && INS_OperandMemoryDisplacement(ins, i) == 0)
            {
                writesFromIpWithNoOffset = TRUE;
                break;
            }
        }
        if (!writesFromIpWithNoOffset)
        {
            return;
        }
        instrumentedReadFromIpWithNoOffset = TRUE; // only instrument one of these
        printf("Instrumenting [ip] write  %p   %s\n", INS_Address(ins), INS_Disassemble(ins).c_str());
        fflush(stdout);
        globalIpOfWriteRecordedAtInstrumentationTime = INS_Address(ins);
        globalWriteInsSize                           = INS_Size(ins);
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)IpWriteBefore, IARG_INST_PTR, IARG_MEMORYWRITE_EA, IARG_REG_VALUE,
                       REG_INST_PTR, IARG_END);
        INS_InsertCall(ins, IPOINT_AFTER, (AFUNPTR)IpWriteAfter, IARG_REG_VALUE, REG_INST_PTR, IARG_END);
    }
}

VOID PrintFinalInfo(INT32 code, VOID* v)
{
    printf("globalIpAtReadBefore %p globalEaReadBefore %p globalRegInstPtrAtReadBefore %p "
           "globalIpOfReadRecordedAtInstrumentationTime %p globalRegInstPtrAtReadAfter %p globalReadInsSize %d\n",
           globalIpAtReadBefore, globalEaReadBefore, globalRegInstPtrAtReadBefore, globalIpOfReadRecordedAtInstrumentationTime,
           globalRegInstPtrAtReadAfter, globalReadInsSize);
    printf("globalIpAtWriteBefore %p globalEaWriteBefore %p globalRegInstPtrAtWriteBefore %p "
           "globalIpOfWriteRecordedAtInstrumentationTime %p globalRegInstPtrAtWriteAfter %p globalWriteInsSize %d\n",
           globalIpAtWriteBefore, globalEaWriteBefore, globalRegInstPtrAtWriteBefore,
           globalIpOfWriteRecordedAtInstrumentationTime, globalRegInstPtrAtWriteAfter, globalWriteInsSize);
    BOOL hadError = FALSE;
    if (globalIpAtReadBefore == 0 || globalEaReadBefore == 0 || globalRegInstPtrAtReadBefore == 0 ||
        globalIpOfReadRecordedAtInstrumentationTime == 0 || globalReadInsSize == 0)
    {
        printf("Error on handling read from [REG_INST_PTR] appears to not have been instrumented\n");
        hadError = TRUE;
    }
    if (globalIpAtWriteBefore == 0 || globalEaWriteBefore == 0 || globalRegInstPtrAtWriteBefore == 0 ||
        globalIpOfWriteRecordedAtInstrumentationTime == 0 || globalWriteInsSize == 0)
    {
        printf("Error on handling write to [REG_INST_PTR] appears to not have been instrumented\n");
        hadError = TRUE;
    }
    if (globalIpAtReadBefore != globalRegInstPtrAtReadBefore ||
        globalIpAtReadBefore != globalIpOfReadRecordedAtInstrumentationTime)
    {
        printf("Error on handling read from [REG_INST_PTR] appears that the rip value received is not correct\n");
        hadError = TRUE;
    }
    if (globalIpAtWriteBefore != globalRegInstPtrAtWriteBefore ||
        globalIpAtWriteBefore != globalIpOfWriteRecordedAtInstrumentationTime)
    {
        printf("Error on handling write to [REG_INST_PTR] appears that the rip value received is not correct\n");
        hadError = TRUE;
    }
    if (globalEaReadBefore != globalIpAtReadBefore + globalReadInsSize)
    {
        printf("Error on handling read from [REG_INST_PTR] appears that the effective address value received is not correct\n");
        hadError = TRUE;
    }
    if (globalEaWriteBefore != globalIpAtWriteBefore + globalWriteInsSize)
    {
        printf("Error on handling write to [REG_INST_PTR] appears that the effective address value received is not correct\n");
        hadError = TRUE;
    }
    if (globalRegInstPtrAtReadAfter != globalIpAtReadBefore + globalReadInsSize)
    {
        printf(
            "Error on handling read from [REG_INST_PTR] appears that REG_INST_PTR received on the IPOINT_AFTER is not correct\n");
        hadError = TRUE;
    }
    if (globalRegInstPtrAtWriteAfter != globalIpAtWriteBefore + globalWriteInsSize)
    {
        printf(
            "Error on handling write to [REG_INST_PTR] appears that REG_INST_PTR received on the IPOINT_AFTER is not correct\n");
        hadError = TRUE;
    }
    if (!hadError)
    {
        printf("SUCCESS\n");
    }
    fflush(stdout);
}

int main(int argc, char* argv[])
{
    PIN_Init(argc, argv);

    INS_AddInstrumentFunction(Instruction, 0);

    // Register a routine that gets called when the program ends
    PIN_AddFiniFunction(PrintFinalInfo, 0);

    PIN_StartProgram();

    return 0;
}
