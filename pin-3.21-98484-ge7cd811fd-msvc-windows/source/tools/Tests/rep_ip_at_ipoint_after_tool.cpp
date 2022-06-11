/*
 * Copyright (C) 2014-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 *  This file contains a test for correctness of the size of memory read
    for cmp instructions with repeat string operation prefix.
    (As well as a number of other tests for our handling of REPped string
    operations).
 */

#include <cstdio>
#include <cstring>
#ifdef TARGET_LINUX
#include <unistd.h>
#endif
#include "pin.H"
#include "instlib.H"

using std::ostringstream;
/* ===================================================================== */
/* Global Variables */
/* ===================================================================== */

/* ===================================================================== */
/* Commandline Switches */
/* ===================================================================== */

int repAppStarted              = 0;
int expectedInstrumentationNum = 1;

ADDRINT GetInstructionLength(ADDRINT ip)
{
    xed_state_t dstate;
    xed_error_enum_t xed_error;
    xed_decoded_inst_t xedd;
    ostringstream os;
    if (sizeof(ADDRINT) == 4)
        xed_state_init(&dstate, XED_MACHINE_MODE_LEGACY_32, XED_ADDRESS_WIDTH_32b, XED_ADDRESS_WIDTH_32b);
    else
        xed_state_init(&dstate, XED_MACHINE_MODE_LONG_64, XED_ADDRESS_WIDTH_64b, XED_ADDRESS_WIDTH_64b);

    xed_decoded_inst_zero_set_mode(&xedd, &dstate);
    UINT32 len = 15;

    xed_error = xed_decode(&xedd, reinterpret_cast< const UINT8* >(ip), len);
    if (xed_error != XED_ERROR_NONE)
    {
        printf("Failure to decode at %p\n", (char*)ip);
        exit(1);
    }
    return xed_decoded_inst_get_length(&xedd);
}

int numCallsToAfter1WithContext = 0;
static VOID After1WithContext(ADDRINT ip, ADDRINT repInsAddr, CONTEXT* ctxt, CONTEXT* constCtxt, ADDRINT* constRefToIp)
{
    ADDRINT expectedIpAfter;
    numCallsToAfter1WithContext++;
    printf("***After1WithContext#    %d repInsAddr %p ip %p\n", //*constRefToIp %p\n",
           numCallsToAfter1WithContext, (char*)repInsAddr, (char*)ip
           //,(char *)constRefToIp,
           //(char *)(*constRefToIp)
    );
    if (numCallsToAfter1WithContext == 2)
    { // see "Test different string comparison" in rep_ip_at_ipoint_after_app.c
        expectedIpAfter = repInsAddr + GetInstructionLength(repInsAddr);
    }
    else
    {
        expectedIpAfter = repInsAddr;
    }

    if (ip != *constRefToIp)
    {
        printf("Unexpcted diff between ip and *constRefToIp\n");
        exit(1);
    }

    if (PIN_GetContextReg(ctxt, REG_INST_PTR) != expectedIpAfter)
    {
        printf("  After1WithContext Unexpected IP in ctxt %p\n", (char*)PIN_GetContextReg(ctxt, REG_INST_PTR));
        exit(1);
    }

    if (PIN_GetContextReg(constCtxt, REG_INST_PTR) != expectedIpAfter)
    {
        printf("  After1WithContext Unexpected IP in constCtxt %p\n", (char*)PIN_GetContextReg(constCtxt, REG_INST_PTR));
        exit(1);
    }
    if (expectedIpAfter != ip)
    {
        printf("  After1WithContext Unexpected IP from REG_VALUE REG_INST_PTR %p\n", (char*)ip);
        exit(1);
    }
}

int numCallsToAfter1WithoutContext = 0;
static VOID After1WithoutContext(ADDRINT ip, ADDRINT repInsAddr, CONTEXT* constCtxt, ADDRINT* constRefToIp)
{
    ADDRINT expectedIpAfter;
    numCallsToAfter1WithoutContext++;
    printf("***After1WithoutContext# %d repInsAddr %p ip %p\n", numCallsToAfter1WithoutContext, (char*)repInsAddr, (char*)ip);
    if (numCallsToAfter1WithoutContext == 2)
    { // see "Test different string comparison" in rep_ip_at_ipoint_after_app.c
        expectedIpAfter = repInsAddr + GetInstructionLength(repInsAddr);
    }
    else
    {
        expectedIpAfter = repInsAddr;
    }

    if (ip != *constRefToIp)
    {
        printf("Unexpcted diff between ip and *constRefToIp\n");
        exit(1);
    }

    if (PIN_GetContextReg(constCtxt, REG_INST_PTR) != expectedIpAfter)
    {
        printf("  After1WithoutContext Unexpected IP in constCtxt %p expectedIpAfter %p\n",
               (char*)PIN_GetContextReg(constCtxt, REG_INST_PTR), (char*)expectedIpAfter);
        exit(1);
    }
    if (expectedIpAfter != ip)
    {
        printf("  After1WithoutContext Unexpected IP from REG_VALUE REG_INST_PTR %p\n", (char*)ip);
        exit(1);
    }
}

int numCallsToAfter2WithContext = 0;
static VOID After2WithContext(ADDRINT ip, ADDRINT repInsAddr, CONTEXT* ctxt, CONTEXT* constCtxt, ADDRINT* constRefToIp)
{
    ADDRINT expectedIpAfter;
    numCallsToAfter2WithContext++;
    printf("***After2WithContext#    %d repInsAddr %p ip %p\n", numCallsToAfter2WithContext, (char*)repInsAddr, (char*)ip);
    if (numCallsToAfter2WithContext == 9)
    { // see "Test same string comparison" in rep_ip_at_ipoint_after_app.c
        expectedIpAfter = repInsAddr + GetInstructionLength(repInsAddr);
    }
    else
    {
        expectedIpAfter = repInsAddr;
    }

    if (ip != *constRefToIp)
    {
        printf("Unexpcted diff between ip and *constRefToIp\n");
        exit(1);
    }

    if (PIN_GetContextReg(ctxt, REG_INST_PTR) != expectedIpAfter)
    {
        printf("  After2WithContext Unexpected IP in ctxt %p\n", (char*)PIN_GetContextReg(ctxt, REG_INST_PTR));
        exit(1);
    }

    if (PIN_GetContextReg(constCtxt, REG_INST_PTR) != expectedIpAfter)
    {
        printf("  After2WithContext Unexpected IP in constCtxt %p\n", (char*)PIN_GetContextReg(constCtxt, REG_INST_PTR));
        exit(1);
    }
    if (expectedIpAfter != ip)
    {
        printf("  After2WithContext Unexpected IP from REG_VALUE REG_INST_PTR %p\n", (char*)ip);
        exit(1);
    }
}

int numCallsToAfter2WithoutContext = 0;
static VOID After2WithoutContext(ADDRINT ip, ADDRINT repInsAddr, CONTEXT* constCtxt, ADDRINT* constRefToIp)
{
    ADDRINT expectedIpAfter;
    numCallsToAfter2WithoutContext++;
    printf("***After2WithoutContext# %d repInsAddr %p ip %p\n", numCallsToAfter2WithoutContext, (char*)repInsAddr, (char*)ip);
    if (numCallsToAfter2WithoutContext == 9)
    { // see "Test same string comparison" in rep_ip_at_ipoint_after_app.c
        expectedIpAfter = repInsAddr + GetInstructionLength(repInsAddr);
    }
    else
    {
        expectedIpAfter = repInsAddr;
    }

    if (ip != *constRefToIp)
    {
        printf("Unexpcted diff between ip and *constRefToIp\n");
        exit(1);
    }

    if (PIN_GetContextReg(constCtxt, REG_INST_PTR) != expectedIpAfter)
    {
        printf("  After2WithoutContext Unexpected IP in constCtxt %p\n", (char*)PIN_GetContextReg(constCtxt, REG_INST_PTR));
        exit(1);
    }
    if (expectedIpAfter != ip)
    {
        printf("  After2WithoutContext Unexpected IP from REG_VALUE REG_INST_PTR %p\n", (char*)ip);
        exit(1);
    }
}

int numCallsToAfter3WithContext = 0;
static VOID After3WithContext(ADDRINT ip, ADDRINT repInsAddr, CONTEXT* ctxt, CONTEXT* constCtxt, ADDRINT* constRefToIp)
{
    ADDRINT expectedIpAfter;
    numCallsToAfter3WithContext++;
    printf("***After3WithContext#    %d repInsAddr %p ip %p\n", numCallsToAfter3WithContext, (char*)repInsAddr, (char*)ip);
    if (numCallsToAfter3WithContext == 1)
    { // see "Test same string comparison, but with no count" in rep_ip_at_ipoint_after_app.c
        expectedIpAfter = repInsAddr + GetInstructionLength(repInsAddr);
    }
    else
    {
        expectedIpAfter = repInsAddr;
    }

    if (ip != *constRefToIp)
    {
        printf("Unexpcted diff between ip and *constRefToIp\n");
        exit(1);
    }

    if (PIN_GetContextReg(ctxt, REG_INST_PTR) != expectedIpAfter)
    {
        printf("  After3WithContext Unexpected IP in ctxt %p\n", (char*)PIN_GetContextReg(ctxt, REG_INST_PTR));
        exit(1);
    }

    if (PIN_GetContextReg(constCtxt, REG_INST_PTR) != expectedIpAfter)
    {
        printf("  After3WithContext Unexpected IP in constCtxt %p\n", (char*)PIN_GetContextReg(constCtxt, REG_INST_PTR));
        exit(1);
    }
    if (expectedIpAfter != ip)
    {
        printf("  After3WithContext Unexpected IP from REG_VALUE REG_INST_PTR %p\n", (char*)ip);
        exit(1);
    }
}

int numCallsToAfter3WithoutContext = 0;
static VOID After3WithoutContext(ADDRINT ip, ADDRINT repInsAddr, CONTEXT* constCtxt, ADDRINT* constRefToIp)
{
    ADDRINT expectedIpAfter;
    numCallsToAfter3WithoutContext++;
    printf("***After3WithoutContext# %d repInsAddr %p ip %p\n", numCallsToAfter3WithoutContext, (char*)repInsAddr, (char*)ip);
    if (numCallsToAfter3WithoutContext == 1)
    { // see "Test same string comparison, but with no count" in rep_ip_at_ipoint_after_app.c
        expectedIpAfter = repInsAddr + GetInstructionLength(repInsAddr);
    }
    else
    {
        expectedIpAfter = repInsAddr;
    }

    if (ip != *constRefToIp)
    {
        printf("Unexpcted diff between ip and *constRefToIp\n");
        exit(1);
    }

    if (PIN_GetContextReg(constCtxt, REG_INST_PTR) != expectedIpAfter)
    {
        printf("  After3WithoutContext Unexpected IP in constCtxt %p\n", (char*)PIN_GetContextReg(constCtxt, REG_INST_PTR));
        exit(1);
    }
    if (expectedIpAfter != ip)
    {
        printf("  After3WithoutContext Unexpected IP from REG_VALUE REG_INST_PTR %p\n", (char*)ip);
        exit(1);
    }
}

int numCallsToAfter4WithContext = 0;
static VOID After4WithContext(ADDRINT ip, ADDRINT repInsAddr, CONTEXT* ctxt, CONTEXT* constCtxt, ADDRINT* constRefToIp)
{
    ADDRINT expectedIpAfter;
    numCallsToAfter4WithContext++;
    printf("***After4WithContext#    %d repInsAddr %p ip %p\n", numCallsToAfter4WithContext, (char*)repInsAddr, (char*)ip);
    if (numCallsToAfter4WithContext == 6)
    { // see "Test same string comparison, but with no count" in rep_ip_at_ipoint_after_app.c
        expectedIpAfter = repInsAddr + GetInstructionLength(repInsAddr);
    }
    else
    {
        expectedIpAfter = repInsAddr;
    }

    if (ip != *constRefToIp)
    {
        printf("Unexpcted diff between ip and *constRefToIp\n");
        exit(1);
    }

    if (PIN_GetContextReg(ctxt, REG_INST_PTR) != expectedIpAfter)
    {
        printf("  After4WithContext Unexpected IP in ctxt %p\n", (char*)PIN_GetContextReg(ctxt, REG_INST_PTR));
        exit(1);
    }

    if (PIN_GetContextReg(constCtxt, REG_INST_PTR) != expectedIpAfter)
    {
        printf("  After4WithContext Unexpected IP in constCtxt %p\n", (char*)PIN_GetContextReg(constCtxt, REG_INST_PTR));
        exit(1);
    }
    if (expectedIpAfter != ip)
    {
        printf("  After4WithContext Unexpected IP from REG_VALUE REG_INST_PTR %p\n", (char*)ip);
        exit(1);
    }
}

int numCallsToAfter4WithoutContext = 0;
static VOID After4WithoutContext(ADDRINT ip, ADDRINT repInsAddr, CONTEXT* constCtxt, ADDRINT* constRefToIp)
{
    ADDRINT expectedIpAfter;
    numCallsToAfter4WithoutContext++;
    printf("***After4WithoutContext# %d repInsAddr %p ip %p\n", numCallsToAfter4WithoutContext, (char*)repInsAddr, (char*)ip);
    if (numCallsToAfter4WithoutContext == 6)
    { // see "Test same string comparison, but with no count" in rep_ip_at_ipoint_after_app.c
        expectedIpAfter = repInsAddr + GetInstructionLength(repInsAddr);
    }
    else
    {
        expectedIpAfter = repInsAddr;
    }

    if (ip != *constRefToIp)
    {
        printf("Unexpcted diff between ip and *constRefToIp\n");
        exit(1);
    }

    if (PIN_GetContextReg(constCtxt, REG_INST_PTR) != expectedIpAfter)
    {
        printf("  After4WithoutContext Unexpected IP in constCtxt %p\n", (char*)PIN_GetContextReg(constCtxt, REG_INST_PTR));
        exit(1);
    }
    if (expectedIpAfter != ip)
    {
        printf("  After4WithoutContext Unexpected IP from REG_VALUE REG_INST_PTR %p\n", (char*)ip);
        exit(1);
    }
}

int numCallsToAfter5WithContext = 0;
static VOID After5WithContext(ADDRINT ip, ADDRINT repInsAddr, CONTEXT* ctxt, CONTEXT* constCtxt, ADDRINT* constRefToIp)
{
    ADDRINT expectedIpAfter;
    numCallsToAfter5WithContext++;
    printf("***After5WithContext#    %d repInsAddr %p ip %p\n", numCallsToAfter5WithContext, (char*)repInsAddr, (char*)ip);
    if (numCallsToAfter5WithContext == 7)
    { // see "Test different comparison repne" in rep_ip_at_ipoint_after_app.c
        expectedIpAfter = repInsAddr + GetInstructionLength(repInsAddr);
    }
    else
    {
        expectedIpAfter = repInsAddr;
    }

    if (ip != *constRefToIp)
    {
        printf("Unexpcted diff between ip and *constRefToIp\n");
        exit(1);
    }

    if (PIN_GetContextReg(ctxt, REG_INST_PTR) != expectedIpAfter)
    {
        printf("  After5WithContext Unexpected IP in ctxt %p\n", (char*)PIN_GetContextReg(ctxt, REG_INST_PTR));
        exit(1);
    }

    if (PIN_GetContextReg(constCtxt, REG_INST_PTR) != expectedIpAfter)
    {
        printf("  After5WithContext Unexpected IP in constCtxt %p\n", (char*)PIN_GetContextReg(constCtxt, REG_INST_PTR));
        exit(1);
    }
    if (expectedIpAfter != ip)
    {
        printf("  After5WithContext Unexpected IP from REG_VALUE REG_INST_PTR %p\n", (char*)ip);
        exit(1);
    }
}

int numCallsToAfter5WithoutContext = 0;
static VOID After5WithoutContext(ADDRINT ip, ADDRINT repInsAddr, CONTEXT* constCtxt, ADDRINT* constRefToIp)
{
    ADDRINT expectedIpAfter;
    numCallsToAfter5WithoutContext++;
    printf("***After5WithoutContext# %d repInsAddr %p ip %p\n", numCallsToAfter5WithoutContext, (char*)repInsAddr, (char*)ip);
    if (numCallsToAfter5WithoutContext == 7)
    { // see "Test different comparison repne, but with no count" in rep_ip_at_ipoint_after_app.c
        expectedIpAfter = repInsAddr + GetInstructionLength(repInsAddr);
    }
    else
    {
        expectedIpAfter = repInsAddr;
    }

    if (ip != *constRefToIp)
    {
        printf("Unexpcted diff between ip and *constRefToIp\n");
        exit(1);
    }

    if (PIN_GetContextReg(constCtxt, REG_INST_PTR) != expectedIpAfter)
    {
        printf("  After5WithoutContext Unexpected IP in constCtxt %p\n", (char*)PIN_GetContextReg(constCtxt, REG_INST_PTR));
        exit(1);
    }
    if (expectedIpAfter != ip)
    {
        printf("  After5WithoutContext Unexpected IP from REG_VALUE REG_INST_PTR %p\n", (char*)ip);
        exit(1);
    }
}

int numCallsToAfter6WithContext = 0;
static VOID After6WithContext(ADDRINT ip, ADDRINT repInsAddr, CONTEXT* ctxt, CONTEXT* constCtxt, ADDRINT* constRefToIp)
{
    ADDRINT expectedIpAfter;
    numCallsToAfter6WithContext++;
    printf("***After6WithContext#    %d repInsAddr %p ip %p\n", numCallsToAfter6WithContext, (char*)repInsAddr, (char*)ip);
    if (numCallsToAfter6WithContext == 11)
    { // see "Test scasb" in rep_ip_at_ipoint_after_app.c
        expectedIpAfter = repInsAddr + GetInstructionLength(repInsAddr);
    }
    else
    {
        expectedIpAfter = repInsAddr;
    }

    if (ip != *constRefToIp)
    {
        printf("Unexpcted diff between ip and *constRefToIp\n");
        exit(1);
    }

    if (PIN_GetContextReg(ctxt, REG_INST_PTR) != expectedIpAfter)
    {
        printf("  After6WithContext Unexpected IP in ctxt %p\n", (char*)PIN_GetContextReg(ctxt, REG_INST_PTR));
        exit(1);
    }

    if (PIN_GetContextReg(constCtxt, REG_INST_PTR) != expectedIpAfter)
    {
        printf("  After6WithContext Unexpected IP in constCtxt %p\n", (char*)PIN_GetContextReg(constCtxt, REG_INST_PTR));
        exit(1);
    }
    if (expectedIpAfter != ip)
    {
        printf("  After6WithContext Unexpected IP from REG_VALUE REG_INST_PTR %p\n", (char*)ip);
        exit(1);
    }
}

int numCallsToAfter6WithoutContext = 0;
static VOID After6WithoutContext(ADDRINT ip, ADDRINT repInsAddr, CONTEXT* constCtxt, ADDRINT* constRefToIp)
{
    ADDRINT expectedIpAfter;
    numCallsToAfter6WithoutContext++;
    printf("***After6WithoutContext# %d repInsAddr %p ip %p\n", numCallsToAfter6WithoutContext, (char*)repInsAddr, (char*)ip);
    if (numCallsToAfter6WithoutContext == 11)
    { // see "Test scasb" in rep_ip_at_ipoint_after_app.c
        expectedIpAfter = repInsAddr + GetInstructionLength(repInsAddr);
    }
    else
    {
        expectedIpAfter = repInsAddr;
    }

    if (ip != *constRefToIp)
    {
        printf("Unexpcted diff between ip and *constRefToIp\n");
        exit(1);
    }

    if (PIN_GetContextReg(constCtxt, REG_INST_PTR) != expectedIpAfter)
    {
        printf("  After6WithoutContext Unexpected IP in constCtxt %p\n", (char*)PIN_GetContextReg(constCtxt, REG_INST_PTR));
        exit(1);
    }
    if (expectedIpAfter != ip)
    {
        printf("  After6WithoutContext Unexpected IP from REG_VALUE REG_INST_PTR %p\n", (char*)ip);
        exit(1);
    }
}

int numCallsToAfter7WithContext = 0;
static VOID After7WithContext(ADDRINT ip, ADDRINT repInsAddr, CONTEXT* ctxt, CONTEXT* constCtxt, ADDRINT* constRefToIp)
{
    ADDRINT expectedIpAfter;
    numCallsToAfter7WithContext++;
    printf("***After7WithContext#    %d repInsAddr %p ip %p\n", numCallsToAfter7WithContext, (char*)repInsAddr, (char*)ip);
    if (numCallsToAfter7WithContext == 8)
    { // see "Test scasb" in rep_ip_at_ipoint_after_app.c
        expectedIpAfter = repInsAddr + GetInstructionLength(repInsAddr);
    }
    else
    {
        expectedIpAfter = repInsAddr;
    }

    if (ip != *constRefToIp)
    {
        printf("Unexpcted diff between ip and *constRefToIp\n");
        exit(1);
    }

    if (PIN_GetContextReg(ctxt, REG_INST_PTR) != expectedIpAfter)
    {
        printf("  After7WithContext Unexpected IP in ctxt %p\n", (char*)PIN_GetContextReg(ctxt, REG_INST_PTR));
        exit(1);
    }

    if (PIN_GetContextReg(constCtxt, REG_INST_PTR) != expectedIpAfter)
    {
        printf("  After7WithContext Unexpected IP in constCtxt %p\n", (char*)PIN_GetContextReg(constCtxt, REG_INST_PTR));
        exit(1);
    }
    if (expectedIpAfter != ip)
    {
        printf("  After7WithContext Unexpected IP from REG_VALUE REG_INST_PTR %p\n", (char*)ip);
        exit(1);
    }
}

int numCallsToAfter7WithoutContext = 0;
static VOID After7WithoutContext(ADDRINT ip, ADDRINT repInsAddr, CONTEXT* constCtxt, ADDRINT* constRefToIp)
{
    ADDRINT expectedIpAfter;
    numCallsToAfter7WithoutContext++;
    printf("***After7WithoutContext# %d repInsAddr %p ip %p\n", numCallsToAfter7WithoutContext, (char*)repInsAddr, (char*)ip);
    if (numCallsToAfter7WithoutContext == 8)
    { // see "Test scasb" in rep_ip_at_ipoint_after_app.c
        expectedIpAfter = repInsAddr + GetInstructionLength(repInsAddr);
    }
    else
    {
        expectedIpAfter = repInsAddr;
    }

    if (ip != *constRefToIp)
    {
        printf("Unexpcted diff between ip and *constRefToIp\n");
        exit(1);
    }

    if (PIN_GetContextReg(constCtxt, REG_INST_PTR) != expectedIpAfter)
    {
        printf("  After7WithoutContext Unexpected IP in constCtxt %p\n", (char*)PIN_GetContextReg(constCtxt, REG_INST_PTR));
        exit(1);
    }
    if (expectedIpAfter != ip)
    {
        printf("  After7WithoutContext Unexpected IP from REG_VALUE REG_INST_PTR %p\n", (char*)ip);
        exit(1);
    }
}

static void DoInstrumentation1(INS ins)
{
    if (XED_ICLASS_CMPSB != INS_Opcode(ins))
    {
        printf("Unexpected first rep instruction\n");
        exit(1);
    }
    if (!INS_RepPrefix(ins))
    {
        printf("Unexpected first rep type\n");
        exit(1);
    }

    INS_InsertCall(ins, IPOINT_AFTER, (AFUNPTR)After1WithoutContext, IARG_REG_VALUE, REG_INST_PTR, IARG_INST_PTR,
                   IARG_CONST_CONTEXT, // has CONST_CONTEXT but not CONTEXT
                   IARG_REG_CONST_REFERENCE, REG_INST_PTR, IARG_END);
    INS_InsertCall(ins, IPOINT_AFTER, (AFUNPTR)After1WithContext, IARG_REG_VALUE, REG_INST_PTR, IARG_INST_PTR, IARG_CONTEXT,
                   IARG_CONST_CONTEXT, IARG_REG_CONST_REFERENCE, REG_INST_PTR, IARG_END);
}

static void DoInstrumentation2(INS ins)
{
    if (XED_ICLASS_CMPSB != INS_Opcode(ins))
    {
        printf("Unexpected second rep instruction\n");
        exit(1);
    }
    if (!INS_RepPrefix(ins))
    {
        printf("Unexpected second rep type\n");
        exit(1);
    }

    INS_InsertCall(ins, IPOINT_AFTER, (AFUNPTR)After2WithContext, IARG_REG_VALUE, REG_INST_PTR, IARG_INST_PTR, IARG_CONTEXT,
                   IARG_CONST_CONTEXT, IARG_REG_CONST_REFERENCE, REG_INST_PTR, IARG_END);

    INS_InsertCall(ins, IPOINT_AFTER, (AFUNPTR)After2WithoutContext, IARG_REG_VALUE, REG_INST_PTR, IARG_INST_PTR,
                   IARG_CONST_CONTEXT, // has CONST_CONTEXT but not CONTEXT
                   IARG_REG_CONST_REFERENCE, REG_INST_PTR, IARG_END);
}

static void DoInstrumentation3(INS ins)
{
    if (XED_ICLASS_CMPSB != INS_Opcode(ins))
    {
        printf("Unexpected third rep instruction\n");
        exit(1);
    }
    if (!INS_RepPrefix(ins))
    {
        printf("Unexpected third rep type\n");
        exit(1);
    }

    INS_InsertCall(ins, IPOINT_AFTER, (AFUNPTR)After3WithContext, IARG_REG_VALUE, REG_INST_PTR, IARG_INST_PTR, IARG_CONTEXT,
                   IARG_CONST_CONTEXT, IARG_REG_CONST_REFERENCE, REG_INST_PTR, IARG_END);
    INS_InsertCall(ins, IPOINT_AFTER, (AFUNPTR)After3WithoutContext, IARG_REG_VALUE, REG_INST_PTR, IARG_INST_PTR,
                   IARG_CONST_CONTEXT, // has CONST_CONTEXT but not CONTEXT
                   IARG_REG_CONST_REFERENCE, REG_INST_PTR, IARG_END);
}

static void DoInstrumentation4(INS ins)
{
    if (XED_ICLASS_CMPSB != INS_Opcode(ins))
    {
        printf("Unexpected fourth rep instruction\n");
        exit(1);
    }
    if (!INS_RepPrefix(ins))
    {
        printf("Unexpected fourth rep type\n");
        exit(1);
    }

    INS_InsertCall(ins, IPOINT_AFTER, (AFUNPTR)After4WithContext, IARG_REG_VALUE, REG_INST_PTR, IARG_INST_PTR, IARG_CONTEXT,
                   IARG_CONST_CONTEXT, IARG_REG_CONST_REFERENCE, REG_INST_PTR, IARG_END);
    INS_InsertCall(ins, IPOINT_AFTER, (AFUNPTR)After4WithoutContext, IARG_REG_VALUE, REG_INST_PTR, IARG_INST_PTR,
                   IARG_CONST_CONTEXT, // has CONST_CONTEXT but not CONTEXT
                   IARG_REG_CONST_REFERENCE, REG_INST_PTR, IARG_END);
}

static void DoInstrumentation5(INS ins)
{
    if (XED_ICLASS_CMPSB != INS_Opcode(ins))
    {
        printf("Unexpected fifth rep instruction\n");
        exit(1);
    }
    if (!INS_RepnePrefix(ins))
    {
        printf("Unexpected fifth rep type\n");
        exit(1);
    }

    INS_InsertCall(ins, IPOINT_AFTER, (AFUNPTR)After5WithContext, IARG_REG_VALUE, REG_INST_PTR, IARG_INST_PTR, IARG_CONTEXT,
                   IARG_CONST_CONTEXT, IARG_REG_CONST_REFERENCE, REG_INST_PTR, IARG_END);
    INS_InsertCall(ins, IPOINT_AFTER, (AFUNPTR)After5WithoutContext, IARG_REG_VALUE, REG_INST_PTR, IARG_INST_PTR,
                   IARG_CONST_CONTEXT, // has CONST_CONTEXT but not CONTEXT
                   IARG_REG_CONST_REFERENCE, REG_INST_PTR, IARG_END);
}

static void DoInstrumentation6(INS ins)
{
    if (XED_ICLASS_SCASB != INS_Opcode(ins))
    {
        printf("Unexpected sixth rep instruction\n");
        exit(1);
    }
    if (!INS_RepPrefix(ins))
    {
        printf("Unexpected sixth rep type\n");
        exit(1);
    }

    INS_InsertCall(ins, IPOINT_AFTER, (AFUNPTR)After6WithContext, IARG_REG_VALUE, REG_INST_PTR, IARG_INST_PTR, IARG_CONTEXT,
                   IARG_CONST_CONTEXT, IARG_REG_CONST_REFERENCE, REG_INST_PTR, IARG_END);
    INS_InsertCall(ins, IPOINT_AFTER, (AFUNPTR)After6WithoutContext, IARG_REG_VALUE, REG_INST_PTR, IARG_INST_PTR,
                   IARG_CONST_CONTEXT, // has CONST_CONTEXT but not CONTEXT
                   IARG_REG_CONST_REFERENCE, REG_INST_PTR, IARG_END);
}

static void DoInstrumentation7(INS ins)
{
    if (XED_ICLASS_MOVSB != INS_Opcode(ins))
    {
        printf("Unexpected seventh rep instruction\n");
        exit(1);
    }
    if (!INS_RepPrefix(ins))
    {
        printf("Unexpected seventh rep type\n");
        exit(1);
    }

    INS_InsertCall(ins, IPOINT_AFTER, (AFUNPTR)After7WithContext, IARG_REG_VALUE, REG_INST_PTR, IARG_INST_PTR, IARG_CONTEXT,
                   IARG_CONST_CONTEXT, IARG_REG_CONST_REFERENCE, REG_INST_PTR, IARG_END);
    INS_InsertCall(ins, IPOINT_AFTER, (AFUNPTR)After7WithoutContext, IARG_REG_VALUE, REG_INST_PTR, IARG_INST_PTR,
                   IARG_CONST_CONTEXT, // has CONST_CONTEXT but not CONTEXT
                   IARG_REG_CONST_REFERENCE, REG_INST_PTR, IARG_END);
}

int numInstrumentedRepInss = 0;
ADDRINT repAddress         = 0;
static VOID InstrumentInstruction(INS ins, VOID* v)
{
    if (!repAppStarted) return;
    if (INS_HasRealRep(ins))
    {
        if (INS_Address(ins) != repAddress)
        {
            numInstrumentedRepInss++;
            repAddress = INS_Address(ins);
        }
        printf("Instrument rep# %d  at %p: %s\n", numInstrumentedRepInss, (char*)INS_Address(ins), INS_Disassemble(ins).c_str());
        switch (numInstrumentedRepInss)
        {
            case 1:
                DoInstrumentation1(ins);
                break;
            case 2:
                DoInstrumentation2(ins);
                break;
            case 3:
                DoInstrumentation3(ins);
                break;
            case 4:
                DoInstrumentation4(ins);
                break;
            case 5:
                DoInstrumentation5(ins);
                break;
            case 6:
                DoInstrumentation6(ins);
                break;
            case 7:
                DoInstrumentation7(ins);
                break;
            default:
                break;
        }
    }
}

/* ===================================================================== */

VOID Fini(INT32 code, VOID* v)
{
    if (0 == numCallsToAfter1WithContext || 0 == numCallsToAfter1WithoutContext || 0 == numCallsToAfter2WithContext ||
        0 == numCallsToAfter2WithoutContext || 0 == numCallsToAfter3WithContext || 0 == numCallsToAfter3WithoutContext ||
        0 == numCallsToAfter4WithContext || 0 == numCallsToAfter4WithoutContext || 0 == numCallsToAfter5WithContext ||
        0 == numCallsToAfter5WithoutContext || 0 == numCallsToAfter6WithContext || 0 == numCallsToAfter6WithoutContext ||
        0 == numCallsToAfter7WithContext || 0 == numCallsToAfter7WithoutContext)
    {
        printf("***Error not all instrumentation was executed\n");
    }
}

VOID ToolRepAppAtIpointAfterStarted()
{
    repAppStarted = 1;
    printf("ToolRepAppAtIpointAfterStarted\n");
}

/* ===================================================================== */
VOID Image(IMG img, void* dummy)
{
    RTN rtn;
    RTN rtn1 = RTN_FindByName(img, "_RepAppAtIpointAfterStarted");
    RTN rtn2 = RTN_FindByName(img, "RepAppAtIpointAfterStarted");
    if (RTN_Valid(rtn1))
    {
        rtn = rtn1;
    }
    else
    {
        rtn = rtn2;
    }
    if (RTN_Valid(rtn))
    {
        printf("Found %s\n", RTN_Name(rtn).c_str());

        RTN_Open(rtn);

        RTN_InsertCall(rtn, IPOINT_AFTER, AFUNPTR(ToolRepAppAtIpointAfterStarted), IARG_END);

        RTN_Close(rtn);
    }
}

int main(int argc, char* argv[])
{
    PIN_InitSymbols();
    if (PIN_Init(argc, argv))
    {
        return 1;
    }

    INS_AddInstrumentFunction(InstrumentInstruction, 0);
    IMG_AddInstrumentFunction(Image, 0);
    PIN_AddFiniFunction(Fini, 0);

    // Never returns
    PIN_StartProgram();

    return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
