/*
 * Copyright (C) 2016-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 *  This tool check that vdso image can be instrumented (see code comments for more details).
 *  The tool should be used with the l_vdso_image_app application.
 *
 *  See below KnobCheckVsyscallAreaNotUsed documentation for another mode this tool is checking.
 */

#include <iostream>
#include <fstream>
#include <linux/unistd.h>
#include "pin.H"
using std::cerr;
using std::endl;
using std::ofstream;
using std::string;

// Global variables

// A knob for defining the file with list of loaded images
KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "l_vdso_image.log", "log file for the tool");

ofstream outFile; // The tool's output file for printing the loaded images.
BOOL beforeTimeOfDayCalled        = false;
BOOL unload_vsdo                  = false;
BOOL vdsoUsed                     = false; // True if one of the functions below are fetched
ADDRINT vdsoGetTimeOfDayAddress   = 0;     // Address of __vdso_gettimeofday function
ADDRINT kernelVsyscallAddress     = 0;     // Address of __kernel_vsyscall function
ADDRINT vdsoClockGetTime64Address = 0;     // Address of __vdso_clock_gettime64 function
BOOL vsyscallAreaUsed             = false; // True if vsyscall area is being fetched and instrumented

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

/* ===================================================================== */
/* Analysis routines                                                     */
/* ===================================================================== */

VOID timeOfDayBefore()
{
    outFile << "Before __vdso_gettimeofday" << endl;
    beforeTimeOfDayCalled = true;
}

VOID clockGetTime64Before()
{
    outFile << "Before __vdso_clock_gettime64" << endl;
    beforeTimeOfDayCalled = true;
}

VOID kernelVSyscallRtnBefore(ADDRINT l_eax)
{
    if (l_eax == __NR_gettimeofday)
    {
        outFile << "Before __kernel_vsyscall with EAX equal to __NR_gettimeofday" << endl;
        beforeTimeOfDayCalled = true;
    }
}

VOID Trace(TRACE trace, VOID* v)
{
    ADDRINT traceAddress     = TRACE_Address(trace);
    ADDRINT traceLastAddress = traceAddress + TRACE_Size(trace) - 1;

    if (((vdsoGetTimeOfDayAddress >= traceAddress) && (vdsoGetTimeOfDayAddress <= traceLastAddress)) ||
        ((kernelVsyscallAddress >= traceAddress) && (kernelVsyscallAddress <= traceLastAddress)) ||
        ((vdsoClockGetTime64Address >= traceAddress) && (vdsoClockGetTime64Address <= traceLastAddress)))

    {
        outFile << "vdso used" << endl;
        vdsoUsed = true;
    }

#if defined(TARGET_IA32E)
    if ((traceAddress >= 0xffffffffff600000) && (traceAddress < 0xffffffffff601000))
    {
        outFile << "Reached ins of vsyscall area, adderss = 0x" << std::hex << traceAddress << endl;
        vsyscallAreaUsed = true;
    }
#endif
}

/* ===================================================================== */
/* Instrumentation routines                                              */
/* ===================================================================== */

static VOID ImageLoad(IMG img, VOID* v)
{
    if (!IMG_IsVDSO(img))
    {
        return;
    }
    outFile << IMG_Name(img) << endl;

    RTN getTimeOfDayRtn   = RTN_FindByName(img, "__vdso_gettimeofday");
    RTN clockGetTime64Rtn = RTN_FindByName(img, "__vdso_clock_gettime64");
    RTN kernelVSyscallRtn = RTN_FindByName(img, "__kernel_vsyscall");
    if (!RTN_Valid(getTimeOfDayRtn) && !RTN_Valid(kernelVSyscallRtn) && !RTN_Valid(clockGetTime64Rtn))
    {
        cerr << "TOOL ERROR: Unable to find the __vdso_gettimeofday or __kernel_vsyscall functions in the application." << endl;
        PIN_ExitProcess(1);
    }

    // Different Linux OS's (as well as 32/64 bit) have different ways of calling VDSO gettimeofday service
    // (no easy switch). So just catching any of them will satisfy the test.
    if (RTN_Valid(getTimeOfDayRtn))
    {
        vdsoGetTimeOfDayAddress = RTN_Address(getTimeOfDayRtn);

        // Instrumenting __vdso_gettimeofday() will satisfy the test
        RTN_Open(getTimeOfDayRtn);
        RTN_InsertCall(getTimeOfDayRtn, IPOINT_BEFORE, (AFUNPTR)timeOfDayBefore, IARG_END);
        RTN_Close(getTimeOfDayRtn);
    }
    if (RTN_Valid(kernelVSyscallRtn))
    {
        kernelVsyscallAddress = RTN_Address(kernelVSyscallRtn);

        // Instrumenting __kernel_vsyscall() with EAX equal to __NR_gettimeofday will satisfy the test
        RTN_Open(kernelVSyscallRtn);
        RTN_InsertCall(kernelVSyscallRtn, IPOINT_BEFORE, (AFUNPTR)kernelVSyscallRtnBefore, IARG_REG_VALUE, REG_EAX, IARG_END);
        RTN_Close(kernelVSyscallRtn);
    }
    if (RTN_Valid(clockGetTime64Rtn))
    {
        vdsoClockGetTime64Address = RTN_Address(clockGetTime64Rtn);

        // Instrumenting __vdso_clock_gettime64
        RTN_Open(clockGetTime64Rtn);
        RTN_InsertCall(clockGetTime64Rtn, IPOINT_BEFORE, (AFUNPTR)timeOfDayBefore, IARG_END);
        RTN_Close(clockGetTime64Rtn);
    }
}

static VOID ImageUnload(IMG img, VOID* v)
{
    if (IMG_IsVDSO(img))
    {
        unload_vsdo = true;
    }
}

static VOID Fini(INT32 code, VOID* v)
{
    ASSERT(unload_vsdo, "Error, VDSO wasn't unloaded");
    ASSERT(!vdsoUsed || beforeTimeOfDayCalled, "Error, VDSO gettimeofday service was not instrumented "
                                               "(__vdso_gettimeofday() or __kernel_vsyscall with __NR_gettimeofday)");
    // sanity check: A situation where VDSO was not used but the analysis was called shouldn't happen
    ASSERTX(!(!vdsoUsed && beforeTimeOfDayCalled));

    const UINT32 kerenReleseMajor = GetKernelRelease(KERNEL_RELEASE_MAJOR);
    const UINT32 kerenReleseMinor = GetKernelRelease(KERNEL_RELEASE_MINOR);

    if ((kerenReleseMajor >= 5) && (kerenReleseMinor >= 3))
    {
        // Starting from Linux kernel 5.3, vsyscall area is execute only (XO). Therefore PIN cannot fetch and/or instrument it.
        // on Linux distributions greater than  kernel 5.3, if Pin doesn't handle correctly indirect branches to vsyscall area
        // it will crash before getting to this assert. So this assert should never occur on these distributions.
        ASSERT(vsyscallAreaUsed == false,
               "Starting from Linux kernel XYZ, vsyscall area is execute only (XO). Therefore PIN cannot "
               "fetch and/or instrument it.\n");
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

    IMG_AddInstrumentFunction(ImageLoad, 0);
    IMG_AddUnloadFunction(ImageUnload, 0);
    TRACE_AddInstrumentFunction(Trace, 0);
    PIN_AddFiniFunction(Fini, 0);

    // Start the program.
    PIN_StartProgram(); // never returns

    return 1; // return error value
}
