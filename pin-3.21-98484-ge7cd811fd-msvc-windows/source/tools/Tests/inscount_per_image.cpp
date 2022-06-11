/*
 * Copyright (C) 2013-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

//
// This tool counts instructions instrumented and executed per-image in the process
//

#include <stdio.h>
#include <string.h>
#include "pin.H"
#include <map>
using std::make_pair;
using std::map;
using std::pair;
using std::string;

KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "inscount_per_image.out",
                              "specify file name for inscount_per_image output");

KNOB< BOOL > KnobCountExecutedInstructions(KNOB_MODE_WRITEONCE, "pintool", "count_exceuted_instructions", "1",
                                           "count executed instructions");

const UINT32 MaxNumThreads = 1024;
UINT32 numThreads          = 0;

typedef struct
{
    MemRange memRange;
    long long unsigned int numExecutedPerThread[MaxNumThreads];
    UINT32 numInstrumented;
    char moduleName[1024];
} LoadedModuleInfo;

LoadedModuleInfo unknownModuleInfo;

typedef map< ADDRINT, LoadedModuleInfo, std::less< ADDRINT > > LoadedModuleInfoMap;
LoadedModuleInfoMap loadedModulesInfoMap;

FILE* outFile;

LoadedModuleInfo* AddrInLoadedModules(ADDRINT addr)
{
    // first range whose Base() is above the target
    LoadedModuleInfoMap::iterator iter = loadedModulesInfoMap.upper_bound(addr);
    if (iter == loadedModulesInfoMap.begin())
    { // all ranges are above addr
        return (NULL);
    }
    iter--;
    if (iter->second.memRange.Contains(Addrint2VoidStar(addr)))
    {
        return (&(iter->second));
    }
    return (NULL);
}

// Pin calls this function every time a new img is loaded
// It can instrument the image, but this example does not
// Note that imgs (including shared libraries) are loaded lazily
int numTimesLoaded = 0;
VOID ImageLoad(IMG img, VOID* v)
{
    fprintf(outFile, "Loading %s, Image id = %d   %p %p\n", IMG_Name(img).c_str(), IMG_Id(img), (void*)(IMG_LowAddress(img)),
            (void*)(IMG_HighAddress(img)));
    fflush(outFile);

    // maintain record of loaded modules
    LoadedModuleInfo loadedModuleInfo;
    MemRange memRange((void*)(IMG_LowAddress(img)), (void*)(IMG_HighAddress(img) + 1));
    loadedModuleInfo.memRange = memRange;
    for (UINT32 i = 0; i < MaxNumThreads; i++)
    {
        loadedModuleInfo.numExecutedPerThread[i] = 0;
    }
    loadedModuleInfo.numInstrumented = 0;

    strcpy(loadedModuleInfo.moduleName, IMG_Name(img).c_str());
    const pair< const ADDRINT, LoadedModuleInfo >& newpair = make_pair(IMG_LowAddress(img), loadedModuleInfo);

    loadedModulesInfoMap.insert(newpair);
}

// Pin calls this function every time a new img is unloaded
// You can't instrument an image that is about to be unloaded
VOID ImageUnload(IMG img, VOID* v)
{
    fprintf(outFile, "Unloading %s\n", IMG_Name(img).c_str());
    fflush(outFile);
    // Note - removing the image from the loadedModulesInfoMap is not implemented
}

VOID ExecutedBbl(long long unsigned int* ptrToPerThreadExecutionCounter, UINT32 insCountInBbl, THREADID tid)
{
    ptrToPerThreadExecutionCounter[tid] += insCountInBbl;
}

VOID Trace(TRACE trace, VOID* v)
{
    long long unsigned int* ptrToPerThreadExecutionCounter;
    BBL bblHead = TRACE_BblHead(trace);
    INS ins     = BBL_InsHead(bblHead);

    // first range whose Base() is above the target
    LoadedModuleInfoMap::iterator iter = loadedModulesInfoMap.upper_bound(INS_Address(ins));
    if (iter == loadedModulesInfoMap.begin())
    { // all ranges are above addr
        unknownModuleInfo.numInstrumented += BBL_NumIns(bblHead);
        ptrToPerThreadExecutionCounter = (unknownModuleInfo.numExecutedPerThread);
    }
    else
    {
        iter--;
        if (iter->second.memRange.Contains((void*)(INS_Address(ins))))
        {
            iter->second.numInstrumented += BBL_NumIns(bblHead);
            ptrToPerThreadExecutionCounter = (iter->second.numExecutedPerThread);
        }
        else
        {
            unknownModuleInfo.numInstrumented += BBL_NumIns(bblHead);
            ptrToPerThreadExecutionCounter = (unknownModuleInfo.numExecutedPerThread);
        }
    }
    if (KnobCountExecutedInstructions)
    {
        // Visit every basic block  in the trace
        for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
        {
            BBL_InsertCall(bbl, IPOINT_ANYWHERE, (AFUNPTR)ExecutedBbl, IARG_PTR, ptrToPerThreadExecutionCounter, IARG_UINT32,
                           BBL_NumIns(bbl), IARG_THREAD_ID, IARG_END);
        }
    }
}

// This function is called when the application exits
VOID Fini(INT32 code, VOID* v)
{
    LoadedModuleInfoMap::const_iterator iter = loadedModulesInfoMap.begin();
    if (KnobCountExecutedInstructions)
    {
        long long unsigned int totalExecuted                = 0;
        long long unsigned int totalExecutedInMainExe       = 0;
        long long unsigned int totalExecutedInSysModules    = 0;
        long long unsigned int totalExecutedInNonSysModules = 0;

        long long unsigned int totalInstrumented                = 0;
        long long unsigned int totalInstrumentedInMainExe       = 0;
        long long unsigned int totalInstrumentedInSysModules    = 0;
        long long unsigned int totalInstrumentedInNonSysModules = 0;

        while (iter != loadedModulesInfoMap.end())
        {
            fprintf(outFile, "instrumented %d in module %s\n", iter->second.numInstrumented, iter->second.moduleName);
            long long unsigned int totalExecutedInModule = 0;
            for (UINT32 i = 0; i < numThreads; i++)
            {
                if (iter->second.numExecutedPerThread[i])
                {
                    fprintf(outFile, "  executed %llu instructions in thread# %d\n", iter->second.numExecutedPerThread[i], i);
                    totalExecutedInModule += iter->second.numExecutedPerThread[i];
                }
            }
            fprintf(outFile, "    total executed instructions in module %llu\n", totalExecutedInModule);

            totalExecuted += totalExecutedInModule;
            totalInstrumented += iter->second.numInstrumented;
            if (strcmp(".exe", &(iter->second.moduleName[strlen(iter->second.moduleName) - 4])) == 0)
            {
                ASSERTX(totalExecutedInMainExe == 0);
                totalInstrumentedInMainExe = iter->second.numInstrumented;
                totalExecutedInMainExe     = totalExecutedInModule;
            }
            else if (!strstr(iter->second.moduleName, "C:\\Windows") && !strstr(iter->second.moduleName, "C:\\WINDOWS") &&
                     !strstr(iter->second.moduleName, "C:\\ProgramData") &&
                     !strstr(iter->second.moduleName, "Host Intrusion Prevention"))
            {
                totalExecutedInNonSysModules += totalExecutedInModule;
                totalInstrumentedInNonSysModules += iter->second.numInstrumented;
            }
            else
            {
                totalExecutedInSysModules += totalExecutedInModule;
                totalInstrumentedInSysModules += iter->second.numInstrumented;
            }
            iter++;
        }
        fprintf(outFile, "instrumented %d in module UNKNOWN\n", unknownModuleInfo.numInstrumented);
        long long unsigned int totalExecutedInModule = 0;
        for (UINT32 i = 0; i < numThreads; i++)
        {
            if (unknownModuleInfo.numExecutedPerThread[i])
            {
                fprintf(outFile, "  executed %llu instructions in thread# %d\n", unknownModuleInfo.numExecutedPerThread[i], i);
                totalExecutedInModule += unknownModuleInfo.numExecutedPerThread[i];
            }
        }
        fprintf(outFile, "    total executed instructions in module %llu\n", totalExecutedInModule);
        totalExecuted += totalExecutedInModule;
        totalExecutedInNonSysModules += totalExecutedInModule;
        totalInstrumentedInNonSysModules += unknownModuleInfo.numInstrumented;

        fprintf(outFile, "\ntotal instrumented instructions %llu\n", totalInstrumented);
        fprintf(outFile, "      total instrumented instructions in MainExe        %llu %3.2f\n", totalInstrumentedInMainExe,
                ((float)totalInstrumentedInMainExe * 100) / (float)totalInstrumented);
        fprintf(outFile, "      total instrumented instructions in nonSys modules %llu %3.2f\n", totalInstrumentedInNonSysModules,
                (float)(totalInstrumentedInNonSysModules * 100) / (float)totalInstrumented);
        fprintf(outFile, "      total instrumented instructions in Sys    modules %llu %3.2f\n", totalInstrumentedInSysModules,
                ((float)totalInstrumentedInSysModules * 100) / (float)totalInstrumented);

        fprintf(outFile, "\ntotal executed instructions %llu\n", totalExecuted);
        fprintf(outFile, "      total executed instructions in MainExe        %llu %3.2f\n", totalExecutedInMainExe,
                ((float)totalExecutedInMainExe * 100) / (float)totalExecuted);
        fprintf(outFile, "      total executed instructions in nonSys modules %llu %3.2f\n", totalExecutedInNonSysModules,
                (float)(totalExecutedInNonSysModules * 100) / (float)totalExecuted);
        fprintf(outFile, "      total executed instructions in Sys    modules %llu %3.2f\n", totalExecutedInSysModules,
                ((float)totalExecutedInSysModules * 100) / (float)totalExecuted);
    }
    else
    {
        while (iter != loadedModulesInfoMap.end())
        {
            fprintf(outFile, "instrumented %d instructions in module %s\n", iter->second.numInstrumented,
                    iter->second.moduleName);
            iter++;
        }
        fprintf(outFile, "instrumented %d instructions in module UNKNOWN\n", unknownModuleInfo.numInstrumented);
    }
    fclose(outFile);
}

VOID ThreadStart(THREADID threadid, CONTEXT* ctxt, INT32 flags, VOID* v)
{
    numThreads++;
    ASSERT(numThreads <= MaxNumThreads, "Maximum number of threads exceeded\n");
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */

int main(int argc, char* argv[])
{
    for (UINT32 i = 0; i < MaxNumThreads; i++)
    {
        unknownModuleInfo.numExecutedPerThread[i] = 0;
    }

    // Initialize symbol processing
    PIN_InitSymbols();

    // Initialize pin
    PIN_Init(argc, argv);

    // Initialize the output stream
    outFile = fopen(KnobOutputFile.Value().c_str(), "w");

    // Register ImageLoad to be called when an image is loaded
    IMG_AddInstrumentFunction(ImageLoad, 0);

    // Register ImageUnload to be called when an image is unloaded
    IMG_AddUnloadFunction(ImageUnload, 0);

    // Register Trace to be called to instrument Traces
    TRACE_AddInstrumentFunction(Trace, 0);

    PIN_AddThreadStartFunction(ThreadStart, 0);

    // Register Fini to be called when the application exits
    PIN_AddFiniFunction(Fini, 0);

    // Start the program, never returns
    PIN_StartProgram();

    return 0;
}
