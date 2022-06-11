/*
 * Copyright (C) 2008-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
*/

#include "pin.H"
namespace WND
{
#include <windows.h>
}
#include <iostream>
#include <fstream>
#include <string>

using std::cerr;
using std::endl;
using std::hex;
using std::ios;
using std::ofstream;
using std::string;
/* ===================================================================== */
/* Global Variables */
/* ===================================================================== */

ofstream TraceFile;

typedef WND::BOOL(WINAPI* CREATE_PROCESSA_FUNC)(WND::LPCTSTR, WND::LPTSTR, WND::LPSECURITY_ATTRIBUTES, WND::LPSECURITY_ATTRIBUTES,
                                                WND::BOOL, WND::DWORD, WND::LPVOID, WND::LPCTSTR, WND::LPSTARTUPINFOA,
                                                WND::LPPROCESS_INFORMATION);

typedef WND::BOOL(WINAPI* CREATE_PROCESSW_FUNC)(WND::LPCWSTR, WND::LPWSTR, WND::LPSECURITY_ATTRIBUTES, WND::LPSECURITY_ATTRIBUTES,
                                                WND::BOOL, WND::DWORD, WND::LPVOID, WND::LPCWSTR, WND::LPSTARTUPINFOW,
                                                WND::LPPROCESS_INFORMATION);

typedef WND::BOOL(WINAPI* CREATE_PROCESS_AS_USERA_FUNC)(WND::HANDLE, WND::LPCTSTR, WND::LPTSTR, WND::LPSECURITY_ATTRIBUTES,
                                                        WND::LPSECURITY_ATTRIBUTES, WND::BOOL, WND::DWORD, WND::LPVOID,
                                                        WND::LPCTSTR, WND::LPSTARTUPINFOA, WND::LPPROCESS_INFORMATION);

typedef WND::BOOL(WINAPI* CREATE_PROCESS_AS_USERW_FUNC)(WND::HANDLE, WND::LPCWSTR, WND::LPWSTR, WND::LPSECURITY_ATTRIBUTES,
                                                        WND::LPSECURITY_ATTRIBUTES, WND::BOOL, WND::DWORD, WND::LPVOID,
                                                        WND::LPCWSTR, WND::LPSTARTUPINFOW, WND::LPPROCESS_INFORMATION);
/*
 * A data about function replacement: 
 *   - name
 *   - pointer to original function
 *   - pointer to a wrapper
 */
struct FUNC_REPLACE
{
    FUNC_REPLACE(const string& name, VOID* origF, VOID* wrapper) : _funcName(name), _orgFuncPtr(origF), _funcWrapper(wrapper) {}
    FUNC_REPLACE() {}
    string _funcName;
    VOID* _orgFuncPtr;
    VOID* _funcWrapper;
};

enum FUNC_IDX
{
    FUNC_CreateProcessA,
    FUNC_CreateProcessW,
    FUNC_CreateProcessAsUserA,
    FUNC_CreateProcessAsUserW,
    FUNC_Last
};

class FUNC_REPLACE_CONTAINER
{
  public:
    VOID* OriginalPtr(FUNC_IDX idx) { return _array[idx]._orgFuncPtr; }
    VOID* Wrapper(FUNC_IDX idx) { return _array[idx]._funcWrapper; }
    string Name(FUNC_IDX idx) { return _array[idx]._funcName; }
    VOID SetOriginalPtr(FUNC_IDX idx, VOID* fptr) { _array[idx]._orgFuncPtr = fptr; }
    UINT32 Size() { return FUNC_Last; }

    FUNC_REPLACE_CONTAINER();

  private:
    FUNC_REPLACE _array[FUNC_Last];
};

FUNC_REPLACE_CONTAINER funcList;

/* ===================================================================== */
/* Commandline Switches */
/* ===================================================================== */

KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "probe_on_probe2.outfile", "specify file name");

/* ===================================================================== */

INT32 Usage()
{
    cerr << "This pin tool tests probe replacement.\n"
            "\n";
    cerr << KNOB_BASE::StringKnobSummary();
    cerr << endl;
    return -1;
}
/*
 * A test wrapper for CreateProcessA()
 */
WND::BOOL WINAPI WrapCreateProcessA(WND::LPCTSTR lpApplicationName, WND::LPTSTR lpCommandLine,
                                    WND::LPSECURITY_ATTRIBUTES lpProcessAttributes, WND::LPSECURITY_ATTRIBUTES lpThreadAttributes,
                                    WND::BOOL bInheritHandles, WND::DWORD dwCreationFlags, WND::LPVOID lpEnvironment,
                                    WND::LPCTSTR lpCurrentDirectory, WND::LPSTARTUPINFOA lpStartupInfo,
                                    WND::LPPROCESS_INFORMATION lpProcessInformation)
{
    TraceFile << "WrapCreateProcessA before" << endl;
    VOID* fptr = funcList.OriginalPtr(FUNC_CreateProcessA);
    BOOL ret   = (*((CREATE_PROCESSA_FUNC)fptr))(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes,
                                               bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo,
                                               lpProcessInformation);
    TraceFile << "WrapCreateProcessA after " << endl;
    return ret;
}

/*
 * A test wrapper for CreateProcessW()
 */
WND::BOOL WINAPI WrapCreateProcessW(WND::LPCWSTR lpApplicationName, WND::LPWSTR lpCommandLine,
                                    WND::LPSECURITY_ATTRIBUTES lpProcessAttributes, WND::LPSECURITY_ATTRIBUTES lpThreadAttributes,
                                    WND::BOOL bInheritHandles, WND::DWORD dwCreationFlags, WND::LPVOID lpEnvironment,
                                    WND::LPCWSTR lpCurrentDirectory, WND::LPSTARTUPINFOW lpStartupInfo,
                                    WND::LPPROCESS_INFORMATION lpProcessInformation)
{
    TraceFile << "WrapCreateProcessW before" << endl;
    CREATE_PROCESSW_FUNC fptr = (CREATE_PROCESSW_FUNC)funcList.OriginalPtr(FUNC_CreateProcessW);
    BOOL ret = (*fptr)(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles,
                       dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
    TraceFile << "WrapCreateProcessW after " << endl;
    return ret;
}

/*
 * A test wrapper for WrapCreateProcessAsUserA()
 */

WND::BOOL WINAPI WrapCreateProcessAsUserA(WND::HANDLE hToken, WND::LPCTSTR lpApplicationName, WND::LPTSTR lpCommandLine,
                                          WND::LPSECURITY_ATTRIBUTES lpProcessAttributes,
                                          WND::LPSECURITY_ATTRIBUTES lpThreadAttributes, WND::BOOL bInheritHandles,
                                          WND::DWORD dwCreationFlags, WND::LPVOID lpEnvironment, WND::LPCTSTR lpCurrentDirectory,
                                          WND::LPSTARTUPINFOA lpStartupInfo, WND::LPPROCESS_INFORMATION lpProcessInformation)
{
    TraceFile << "CreateProcessAsUserA before" << endl;
    BOOL ret = (*(CREATE_PROCESS_AS_USERA_FUNC)funcList.OriginalPtr(FUNC_CreateProcessAsUserA))(
        hToken, lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags,
        lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
    TraceFile << "CreateProcessAsUserA after " << endl;
    return ret;
}

/*
 * A test wrapper for WrapCreateProcessAsUserW()
 */
WND::BOOL WINAPI WrapCreateProcessAsUserW(WND::HANDLE hToken, WND::LPCWSTR lpApplicationName, WND::LPWSTR lpCommandLine,
                                          WND::LPSECURITY_ATTRIBUTES lpProcessAttributes,
                                          WND::LPSECURITY_ATTRIBUTES lpThreadAttributes, WND::BOOL bInheritHandles,
                                          WND::DWORD dwCreationFlags, WND::LPVOID lpEnvironment, WND::LPCWSTR lpCurrentDirectory,
                                          WND::LPSTARTUPINFOW lpStartupInfo, WND::LPPROCESS_INFORMATION lpProcessInformation)
{
    TraceFile << "CreateProcessAsUserW before" << endl;
    BOOL ret = (*(CREATE_PROCESS_AS_USERW_FUNC)funcList.OriginalPtr(FUNC_CreateProcessAsUserW))(
        hToken, lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags,
        lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
    TraceFile << "CreateProcessAsUserW after " << endl;
    return ret;
}

FUNC_REPLACE_CONTAINER::FUNC_REPLACE_CONTAINER()
{
#define INIT_FENTRY(F) _array[FUNC_##F] = FUNC_REPLACE(string(#F), (VOID*)0, (VOID*)Wrap##F)
    INIT_FENTRY(CreateProcessA);
    INIT_FENTRY(CreateProcessW);
    INIT_FENTRY(CreateProcessAsUserA);
    INIT_FENTRY(CreateProcessAsUserW);
}

/* Find a routine that should be replaced and 
 * check whether the probed replacement is possible
 */
BOOL FindAndCheckRtn(IMG img, string rtnName, RTN& rtn)
{
    rtn = RTN_FindByName(img, rtnName.c_str());
    if (!RTN_Valid(rtn)) rtn = RTN_FindByName(img, (string("_") + rtnName).c_str());

    if (RTN_Valid(rtn))
    {
        if (!RTN_IsSafeForProbedReplacement(rtn))
        {
            TraceFile << "Cannot replace " << RTN_Name(rtn) << " in " << IMG_Name(img) << endl;
            exit(1);
        }
        return TRUE;
    }
    return FALSE;
}

BOOL ReplaceProbed(IMG img, FUNC_IDX idx)
{
    RTN rtn;
    if (!FindAndCheckRtn(img, funcList.Name(idx), rtn)) return FALSE;

    VOID* orgPtr = (VOID*)RTN_ReplaceProbed(rtn, AFUNPTR(funcList.Wrapper(idx)));
    funcList.SetOriginalPtr(idx, orgPtr);

    // TraceFile << "Inserted probe for " << funcList.Name(idx) << endl;
    return TRUE;
}

/*
 * Return TRUE if baseName matches tail of imageName. Comparison is case-insensitive.
 * @param[in]  imageName  image file name in either form with extension
 * @param[in]  baseName   image base name with extension (e.g. kernel32.dll)
 */
static BOOL CmpBaseImageName(const string& imageName, const string& baseName)
{
    if (imageName.size() >= baseName.size())
    {
        return _stricmp(imageName.c_str() + imageName.size() - baseName.size(), baseName.c_str()) == 0;
    }
    return FALSE;
}

/* ===================================================================== */
// Called every time a new image is loaded
// Look for routines that we want to probe
VOID ImageLoad(IMG img, VOID* v)
{
    // Skip all images, but kernel32.dll
    if (!CmpBaseImageName(IMG_Name(img), "kernel32.dll"))
    {
        return;
    }

    for (UINT32 i = 0; i < funcList.Size(); i++)
    {
        ReplaceProbed(img, (FUNC_IDX)i);
    }
}

/* Ensure that Pin wrapper still works */
BOOL FollowChild(CHILD_PROCESS cProcess, VOID* userData)
{
    INT appArgc;
    CHAR const* const* appArgv;

    CHILD_PROCESS_GetCommandLine(cProcess, &appArgc, &appArgv);

    TraceFile << "Command line : " << endl;
    for (int i = 0; i < appArgc; i++)
    {
        TraceFile << appArgv[i] << " ";
    }
    TraceFile << endl;
    TraceFile << "Pin Wrapper works" << endl;
    return FALSE;
}

/* ===================================================================== */

int main(int argc, CHAR* argv[])
{
    PIN_InitSymbols();

    if (PIN_Init(argc, argv))
    {
        return Usage();
    }

    TraceFile.open(KnobOutputFile.Value().c_str());
    TraceFile << hex;
    TraceFile.setf(ios::showbase);

    IMG_AddInstrumentFunction(ImageLoad, 0);

    PIN_AddFollowChildProcessFunction(FollowChild, 0);

    PIN_StartProgramProbed();

    return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
