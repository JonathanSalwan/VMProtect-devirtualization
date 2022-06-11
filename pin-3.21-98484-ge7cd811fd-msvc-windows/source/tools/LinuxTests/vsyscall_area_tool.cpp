/*
 * Copyright (C) 2019-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*!
 *  Starting from Linux kernel 5.3, vsyscall area is execute only (XO). Therefore PIN cannot fetch and/or instrument it.
 *  Pin needs to ran it "natively" from VM.
 *
 *  If the kernel version is below 5.3 this tool checks that vsyscall area is instrumented as usual.
 *
 *  If the kernel version is higher or equal to 5.3 this tool checks the following:
 *  - vsyscall area is not instrumented
 *  - verify that registered syscall callback are called before/after jump to vsyscall area
 *  - verify calling order - IPOINT_BEFORE and IPOINT_TAKEN_BRANCH instrumentation should be called before callbacks of vsyscall
 *  - verify that IPOINT_AFTER instrumentation is not allowed when branching to vsyscall area
 *
 */

#include <iostream>
#include <fstream>
#include <linux/unistd.h>
#include "pin.H"
using std::cerr;
using std::cout;
using std::endl;
using std::ofstream;
using std::string;

// Global variables

// A knob for defining the file with list of loaded images
KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "vsyscall_area_tool.log", "log file for the tool");

ofstream outFile; // The tool's output file for printing the loaded images.

BOOL vsyscallAreaUsed    = false; // True if vsyscall area is being fetched and instrumented
BOOL entryCallbackCalled = false; // True if vsyscall entry callback has been called
BOOL exitCallbackCalled  = false; // True if vsyscall exit callback has been called
BOOL isTargetInVsyscall  = false; // True if target address is in vsyscall area
UINT32 vsyscall_number   = 0;     // Should be VSYSCALL_NR
ADDRINT currentIP        = 0;     // current application IP

BOOL IsAdreessInVsyscallArea(ADDRINT target) { return ((target >= 0xffffffffff600000) && (target < 0xffffffffff601000)); }

/*!
 * Kernel release type enumerations (like major and minor)
 */
typedef enum
{
    KERNEL_RELEASE_MAJOR,
    KERNEL_RELEASE_MINOR
} KERNEL_RELEASE_KIND;

/*
 * Return kernel release major or minor number based on the specified kind enum.
 * 0 returned upon KERNEL_RELEASE_MAJOR means some error has occurred.
 */
static UINT32 GetKernelRelease(KERNEL_RELEASE_KIND kind)
{
    static BOOL first                = TRUE;
    static UINT32 kernelReleaseMajor = 0;
    static UINT32 kernelReleaseMinor = 0;
    char* ptr;

    if ((kind < KERNEL_RELEASE_MAJOR) || (kind > KERNEL_RELEASE_MINOR))
    {
        ASSERTX(0);
    }

    if (!first)
    {
        if (kind == KERNEL_RELEASE_MAJOR)
            return kernelReleaseMajor;
        else if (kind == KERNEL_RELEASE_MINOR)
            return kernelReleaseMinor;
    }

    char buf[128];
    OS_RETURN_CODE ret = OS_GetKernelRelease(buf, sizeof(buf));
    ASSERTX(OS_RETURN_CODE_IS_SUCCESS(ret));
    ptr = buf;

    char* dot = strchr(ptr, '.');
    if (dot)
    {
        *dot               = '\0';
        kernelReleaseMajor = atoi(ptr);
    }
    else
    {
        return 0;
    }

    ptr = dot + 1;
    dot = strchr(ptr, '.');
    if (dot)
    {
        *dot               = '\0';
        kernelReleaseMinor = atoi(ptr);
    }

    first = FALSE;

    if (kind == KERNEL_RELEASE_MAJOR)
        return kernelReleaseMajor;
    else if (kind == KERNEL_RELEASE_MINOR)
        return kernelReleaseMinor;

    // Shouldn't get here.
    ASSERTX(0);
    return 0;
}

const UINT32 kernelReleaseMajor = GetKernelRelease(KERNEL_RELEASE_MAJOR);
const UINT32 kernelReleaseMinor = GetKernelRelease(KERNEL_RELEASE_MINOR);

/* ===================================================================== */
/* Analysis routines                                                     */
/* ===================================================================== */

// This function is called before every instruction is executed
VOID InsAnalysisBefore(ADDRINT iaddr, ADDRINT target, BOOL taken)
{
    if (IsAdreessInVsyscallArea(target))
    {
        outFile << std::hex << "instruction  in address = 0x" << iaddr << " branch to vsyscall area at address = 0x" << target
                << ", taken = " << taken << endl;
    }
}

// This function is called on the taken edge of control-flow instruction
VOID InsAnalysisBranchTaken(ADDRINT iaddr, ADDRINT target)
{
    if (IsAdreessInVsyscallArea(target))
    {
        currentIP          = iaddr;
        isTargetInVsyscall = true;
        return;
    }
}

// This function will be called on the fall-through path of instruction.
// Specifically in this test, this analysis routine should not be called nor instrumented
// since we are jumping to vsyscall area which is execute-only.
VOID InsAnalysisAfter(ADDRINT target)
{
    if (IsAdreessInVsyscallArea(target))
    {
        ASSERT(0, "IPOINT_AFTER analysis call has been called after indirect branch to vsyscall");
    }
}

// Pin calls this function every time a new instruction is encountered
VOID Instruction(INS ins, VOID* v)
{
    ADDRINT insAddress = INS_Address(ins);

    if (INS_IsControlFlow(ins))
    {
        // record taken branch targets
        INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(InsAnalysisBefore), IARG_INST_PTR, IARG_BRANCH_TARGET_ADDR, IARG_BRANCH_TAKEN,
                       IARG_END);

        if ((kernelReleaseMajor > 5) || ((kernelReleaseMajor == 5) && (kernelReleaseMinor >= 3)))
        {
            INS_InsertCall(ins, IPOINT_TAKEN_BRANCH, AFUNPTR(InsAnalysisBranchTaken), IARG_INST_PTR, IARG_BRANCH_TARGET_ADDR,
                           IARG_END);

            if (INS_IsValidForIpointAfter(ins))
            {
                INS_InsertCall(ins, IPOINT_AFTER, AFUNPTR(InsAnalysisAfter), IARG_BRANCH_TARGET_ADDR, IARG_END);
            }
        }
    }

    if (IsAdreessInVsyscallArea(insAddress))
    {
        outFile << "Reached ins of vsyscall area, adderss = 0x" << std::hex << insAddress << endl;
        vsyscallAreaUsed = true;
    }
}

// Print vsyscall number and arguments
VOID vSyscallBefore(ADDRINT ip, ADDRINT num, ADDRINT arg0, ADDRINT arg1, SYSCALL_STANDARD scStd)
{
    static UINT32 callback_counter = 0;

    // logging prints
    std::cout << "\n----- Callback Entry #" << ++callback_counter << " -----" << endl;
    std::cout << "IP              : " << std::hex << "0x" << (unsigned long)ip << endl;
    std::cout << "vsyscall number : " << std::dec << (int)num << endl;
    std::cout << "Arg 0           : 0x" << std::hex << (unsigned long)arg0 << endl;
    std::cout << "Arg 1           : 0x" << std::hex << (unsigned long)arg1 << endl;
}

VOID vSyscallEntryCallback(THREADID tid, CONTEXT* ctxt, SYSCALL_STANDARD scStd, VOID* arg)
{
    vsyscall_number = (UINT32)PIN_GetSyscallNumber(ctxt, scStd);
    if (isTargetInVsyscall || vsyscall_number == VSYSCALL_NR || scStd == SYSCALL_STANDARD_IA32E_LINUX_VSYSCALL)
    {
        // verify that we're inside vsyscall area
        ASSERT(isTargetInVsyscall == true, "callback called outside of vsyscall area.");

        // verify correct vsyscall number
        ASSERT(vsyscall_number == VSYSCALL_NR, "vsyscall number is invalid.");

        // verify correct calling standard
        ASSERT(scStd == SYSCALL_STANDARD_IA32E_LINUX_VSYSCALL,
               "branching to vsyscall area with calling standard different than SYSCALL_STANDARD_IA32E_LINUX_VSYSCALL.");

        ADDRINT jumpInstructionIP = PIN_GetContextReg(ctxt, REG_INST_PTR);
        ASSERT(jumpInstructionIP == currentIP, "jump IP to vsyscall area is not valid.");

        entryCallbackCalled = true;
        vSyscallBefore(PIN_GetContextReg(ctxt, REG_INST_PTR), vsyscall_number, PIN_GetSyscallArgument(ctxt, scStd, 0),
                       PIN_GetSyscallArgument(ctxt, scStd, 1), scStd);
    }
}

VOID vSyscallExitCallback(THREADID tid, CONTEXT* ctxt, SYSCALL_STANDARD scStd, VOID* arg)
{
    if (isTargetInVsyscall)
    {
        exitCallbackCalled = true;

        // Invalidates Target
        isTargetInVsyscall = false;

        ADDRINT retval = PIN_GetSyscallReturn(ctxt, SYSCALL_STANDARD_IA32E_LINUX_VSYSCALL);
        std::cout << "Tool vsyscall return value : " << retval << std::endl;
    }
}

/* ===================================================================== */
/* Instrumentation routines                                              */
/* ===================================================================== */

static VOID Fini(INT32 code, VOID* v)
{
    if ((kernelReleaseMajor > 5) || ((kernelReleaseMajor == 5) && (kernelReleaseMinor >= 3)))
    {
        // Starting from Linux kernel 5.3, vsyscall area is execute only (XO). Therefore PIN cannot fetch and/or instrument it.
        // on Linux distributions greater than  kernel 5.3, if Pin doesn't handle correctly indirect branches to vsyscall area
        // it will crash before getting to this assert. So this assert should never occur on these distributions.
        ASSERT(vsyscallAreaUsed == false,
               "Starting from Linux kernel 5.3, vsyscall area is execute only (XO). Therefore PIN cannot "
               "fetch and/or instrument it.\n");
        ASSERT(entryCallbackCalled == true, "vsyscall entry callback has not called.");
        ASSERT(exitCallbackCalled == true, "vsyscall exit callback has not called.");
    }
    else
    {
        ASSERT(vsyscallAreaUsed == true, "Pin didn't instrument vsyscall on Linux kernel lower than 5.3\n");
    }
    outFile.close();
}

int main(INT32 argc, CHAR* argv[])
{
    // Initialization.
    PIN_InitSymbols();
    PIN_Init(argc, argv);

    // Open the tool's output file for printing the loaded images.
    outFile.open(KnobOutputFile.Value().c_str());
    if (!outFile.is_open() || outFile.fail())
    {
        cerr << "TOOL ERROR: Unable to open the output file." << endl;
        PIN_ExitProcess(1);
    }

    INS_AddInstrumentFunction(Instruction, 0);

    // in case of kernel > 5.3 we want to test vsyscall callback mechanism
    if ((kernelReleaseMajor > 5) || ((kernelReleaseMajor == 5) && (kernelReleaseMinor >= 3)))
    {
        PIN_AddSyscallEntryFunction(vSyscallEntryCallback, 0);
        PIN_AddSyscallExitFunction(vSyscallExitCallback, 0);
    }

    PIN_AddFiniFunction(Fini, 0);

    // Start the program.
    PIN_StartProgram(); // never returns

    return 1; // return error value
}
