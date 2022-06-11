/*
 * Copyright (C) 2012-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * This tool mimics the behavior of TPSS on Linux by adding probes to various libdl functions.
 * However, in this tool these probes are merely empty wrappers that call the original functions.
 * The objective of the test is to verify that probe generation and insertion don't cause Pin
 * to crash.
 */

#include "pin.H"
#include <iostream>
#include <fstream>
#include <dlfcn.h>
using std::cerr;
using std::endl;
using std::hex;
using std::ios;
using std::ofstream;
using std::string;

typedef char* CHAR_PTR;

typedef void* VOID_PTR;

ofstream OutFile;

/* ===================================================================== */
/* Commandline Switches                                                  */
/* ===================================================================== */

KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "tpss_lin_libdl.txt", "specify tool log file name");

/* ===================================================================== */
/* Utility functions                                                     */
/* ===================================================================== */

// Print help information
INT32 Usage()
{
    cerr << "This tool mimics the behavior of TPSS on Linux by adding probes to various libdl functions." << endl;
    cerr << KNOB_BASE::StringKnobSummary();
    cerr << endl;
    return 1;
}

// Utility function to return the time
string CurrentTime()
{
    char tmpbuf[128];
    time_t thetime = time(NULL);
    ctime_r(&thetime, tmpbuf);
    return tmpbuf;
}

/* ===================================================================== */
/* Function signatures - these functions will be probed                  */
/* ===================================================================== */

VOID_PTR (*fptrdlopen)(__const CHAR_PTR __file, int __mode);

int (*fptrdlclose)(VOID_PTR __handle);

VOID_PTR (*fptrdlsym)(VOID_PTR __handle, __const CHAR_PTR __name);

VOID_PTR (*fptrdlvsym)(VOID_PTR __handle, __const CHAR_PTR __name, __const CHAR_PTR __version);

int (*fptrdladdr)(__const void* __address, Dl_info* __info);

int (*fptrdladdr1)(__const void* __address, Dl_info* __info, void** __extra_info, int __flags);

CHAR_PTR (*fptrdlerror)(void);

/* ===================================================================== */
/* Probes - implementation of the wrapper functions                      */
/* ===================================================================== */

VOID_PTR mydlopen(__const CHAR_PTR __file, int __mode)
{
    OutFile << CurrentTime() << "mydlopen called " << endl;
    OutFile.flush();
    VOID_PTR res = fptrdlopen(__file, __mode);

    return res;
}

int mydlclose(VOID_PTR __handle)
{
    OutFile << CurrentTime() << "mydlclose called " << endl;
    OutFile.flush();
    int res = fptrdlclose(__handle);

    return res;
}

VOID_PTR mydlsym(VOID_PTR __handle, __const CHAR_PTR __name)
{
    OutFile << CurrentTime() << "mydlsym called " << endl;
    OutFile.flush();
    VOID_PTR res = fptrdlsym(__handle, __name);

    return res;
}

VOID_PTR mydlvsym(VOID_PTR __handle, __const CHAR_PTR __name, __const CHAR_PTR __version)
{
    OutFile << CurrentTime() << "mydlvsym called " << endl;
    OutFile.flush();
    VOID_PTR res = fptrdlvsym(__handle, __name, __version);

    return res;
}

int mydladdr(__const void* __address, Dl_info* __info)
{
    OutFile << CurrentTime() << "mydladdr called " << endl;
    OutFile.flush();
    int res = fptrdladdr(__address, __info);

    return res;
}

int mydladdr1(__const void* __address, Dl_info* __info, void** __extra_info, int __flags)
{
    OutFile << CurrentTime() << "mydladdr1 called " << endl;
    OutFile.flush();
    int res = fptrdladdr1(__address, __info, __extra_info, __flags);

    return res;
}

CHAR_PTR mydlerror(void)
{
    OutFile << CurrentTime() << "CHAR_PTR called " << endl;
    OutFile.flush();
    CHAR_PTR res = fptrdlerror();

    return res;
}

/* ===================================================================== */
/* Instrumnetation functions                                             */
/* ===================================================================== */

// Image load callback - inserts the probes.
void ImgLoad(IMG img, void* v)
{
    // Called every time a new image is loaded

    if ((IMG_Name(img).find("libdl.so") != string::npos) || (IMG_Name(img).find("LIBDL.SO") != string::npos) ||
        (IMG_Name(img).find("LIBDL.so") != string::npos))
    {
        RTN rtndlclose = RTN_FindByName(img, "dlclose");
        if (RTN_Valid(rtndlclose) && RTN_IsSafeForProbedReplacement(rtndlclose))
        {
            OutFile << CurrentTime() << "Inserting probe for dlclose at " << RTN_Address(rtndlclose) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtndlclose, AFUNPTR(mydlclose)));
            fptrdlclose  = (int (*)(VOID_PTR))fptr;
        }

        RTN rtndlopen = RTN_FindByName(img, "dlopen");
        if (RTN_Valid(rtndlopen) && RTN_IsSafeForProbedReplacement(rtndlopen))
        {
            OutFile << CurrentTime() << "Inserting probe for dlopen at " << RTN_Address(rtndlopen) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtndlopen, AFUNPTR(mydlopen)));
            fptrdlopen   = (VOID_PTR(*)(__const CHAR_PTR, int))fptr;
        }

        RTN rtndlsym = RTN_FindByName(img, "dlsym");
        if (RTN_Valid(rtndlsym) && RTN_IsSafeForProbedReplacement(rtndlsym))
        {
            OutFile << CurrentTime() << "Inserting probe for dlsym at " << RTN_Address(rtndlsym) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtndlsym, AFUNPTR(mydlsym)));
            fptrdlsym    = (VOID_PTR(*)(VOID_PTR, __const CHAR_PTR))fptr;
        }

        RTN rtndlvsym = RTN_FindByName(img, "dlvsym");
        if (RTN_Valid(rtndlvsym) && RTN_IsSafeForProbedReplacement(rtndlvsym))
        {
            OutFile << CurrentTime() << "Inserting probe for dlvsym at " << RTN_Address(rtndlvsym) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtndlvsym, AFUNPTR(mydlvsym)));
            fptrdlvsym   = (VOID_PTR(*)(VOID_PTR, __const CHAR_PTR, __const CHAR_PTR))fptr;
        }

        RTN rtndladdr = RTN_FindByName(img, "dladdr");
        if (RTN_Valid(rtndladdr) && RTN_IsSafeForProbedReplacement(rtndladdr))
        {
            OutFile << CurrentTime() << "Inserting probe for dladdr at " << RTN_Address(rtndladdr) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtndladdr, AFUNPTR(mydladdr)));
            fptrdladdr   = (int (*)(__const void*, Dl_info*))fptr;
        }

        RTN rtndladdr1 = RTN_FindByName(img, "dladdr1");
        if (RTN_Valid(rtndladdr) && RTN_IsSafeForProbedReplacement(rtndladdr1))
        {
            OutFile << CurrentTime() << "Inserting probe for dladdr1 at " << RTN_Address(rtndladdr1) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtndladdr, AFUNPTR(mydladdr1)));
            fptrdladdr1  = (int (*)(__const void*, Dl_info*, void**, int))fptr;
        }

        RTN rtndlerror = RTN_FindByName(img, "dlerror");
        if (RTN_Valid(rtndlerror) && RTN_IsSafeForProbedReplacement(rtndlerror))
        {
            OutFile << CurrentTime() << "Inserting probe for dlerror at " << RTN_Address(rtndlerror) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtndlerror, AFUNPTR(mydlerror)));
            fptrdlerror  = (CHAR_PTR(*)(void))fptr;
        }
    }
    // finished instrumentation
}

/* ===================================================================== */
/* Main function                                                         */
/* ===================================================================== */

int main(int argc, char* argv[])
{
    // Initialize Pin
    PIN_InitSymbols();
    if (PIN_Init(argc, argv))
    {
        return Usage();
    }

    OutFile.open(KnobOutputFile.Value().c_str());
    OutFile << hex;
    OutFile.setf(ios::showbase);
    OutFile << CurrentTime() << "started!" << endl;
    OutFile.flush();

    // Register the instrumentation callback
    IMG_AddInstrumentFunction(ImgLoad, 0);

    // Start the application
    PIN_StartProgramProbed(); // never returns

    return 0;
}
