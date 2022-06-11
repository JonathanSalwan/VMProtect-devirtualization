/*
 * Copyright (C) 2013-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include "pin.H"
#include <map>
using std::make_pair;
using std::map;
using std::pair;

typedef map< ADDRINT, MemRange, std::less< ADDRINT > > LoadedImagesMap;
LoadedImagesMap loadedImagesMap;

UINT32 numTimesImgFoundInInstrumentation    = 0;
UINT32 numTimesImgNotFoundInInstrumentation = 0;

long long unsigned int numTimesImgFoundInAnalysis    = 0;
long long unsigned int numTimesImgNotFoundInAnalysis = 0;

static BOOL IsAddrInLoadedImg(ADDRINT ip)
{
    LoadedImagesMap::iterator iter = loadedImagesMap.upper_bound(ip);
    if (iter == loadedImagesMap.begin())
    { // all ranges are above addr
        return FALSE;
    }
    iter--;
    if (iter->second.Contains(Addrint2VoidStar(ip)))
    {
        return TRUE;
    }
    return FALSE;
}

static VOID AnalysisFindImg(ADDRINT ip)
{
    if (!IsAddrInLoadedImg(ip)) return;
    PIN_LockClient();
    IMG img = IMG_FindByAddress(ip);
    PIN_UnlockClient();
    if (IMG_Valid(img))
    {
        numTimesImgFoundInAnalysis++;
    }
    else
    {
        numTimesImgNotFoundInAnalysis++;
    }
}

static VOID Instruction(INS ins, VOID* v)
{
    if (!IsAddrInLoadedImg(INS_Address(ins))) return;

    IMG img = IMG_FindByAddress(INS_Address(ins));

    if (IMG_Valid(img))
    {
        numTimesImgFoundInInstrumentation++;
    }
    else
    {
        numTimesImgNotFoundInInstrumentation++;
    }
    INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)AnalysisFindImg, IARG_INST_PTR, IARG_END);
}

VOID ImageLoad(IMG img, VOID* v)
{
    MemRange memRange((void*)(IMG_LowAddress(img)), (void*)(IMG_HighAddress(img) + 1));
    const pair< const ADDRINT, MemRange >& newpair = make_pair(IMG_LowAddress(img), memRange);
    loadedImagesMap.insert(newpair);

    printf("loaded image %s lowAddr %p highAddr %p loadOffset %p\n", IMG_Name(img).c_str(), (void*)IMG_LowAddress(img),
           (void*)IMG_HighAddress(img), (void*)IMG_LoadOffset(img));
}

VOID ImageUnload(IMG img, VOID* v)
{
    loadedImagesMap.erase(IMG_LowAddress(img));
    printf("Unloaded image %s lowAddr %p highAddr %p loadOffset %p\n", IMG_Name(img).c_str(), (void*)IMG_LowAddress(img),
           (void*)IMG_HighAddress(img), (void*)IMG_LoadOffset(img));
}

VOID Fini(INT32 code, VOID* v)
{
    printf("numTimesImgFoundInInstrumentation %d numTimesImgNotFoundInInstrumentation %d\n", numTimesImgFoundInInstrumentation,
           numTimesImgNotFoundInInstrumentation);
    printf("numTimesImgFoundInAnalysis %llu numTimesImgNotFoundInAnalysis %llu\n", numTimesImgFoundInAnalysis,
           numTimesImgNotFoundInAnalysis);
    ASSERTX((numTimesImgFoundInAnalysis) > (numTimesImgNotFoundInAnalysis));
    ASSERTX((numTimesImgFoundInInstrumentation) > (numTimesImgNotFoundInInstrumentation));
}

int main(int argc, char* argv[])
{
    PIN_InitSymbols();
    PIN_Init(argc, argv);

    INS_AddInstrumentFunction(Instruction, 0);
    IMG_AddInstrumentFunction(ImageLoad, 0);
    IMG_AddUnloadFunction(ImageUnload, 0);
    // Register Fini to be called when the application exits
    PIN_AddFiniFunction(Fini, 0);
    // Start the program, never returns
    PIN_StartProgram();

    return 0;
}
