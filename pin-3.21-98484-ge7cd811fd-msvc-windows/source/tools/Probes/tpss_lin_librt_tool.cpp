/*
 * Copyright (C) 2012-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * This tool mimics the behavior of TPSS on Linux by adding probes to various librt functions.
 * However, in this tool these probes are merely empty wrappers that call the original functions.
 * The objective of the test is to verify that probe generation and insertion don't cause Pin
 * to crash.
 *
 * This file is part of the tpss_lin_librt tool and compiles against the tool's libc (e.g. PIN CRT).
 * This file implements all the intrumenting logic thus requires PIN headers.
 */
#include "pin.H"
#include <iostream>
#include <fstream>
using std::cerr;
using std::endl;
using std::hex;
using std::ios;
using std::ofstream;
using std::string;

ofstream OutFile;

/* ===================================================================== */
/* Pointers to the original functions that we probe                      */
/* ===================================================================== */
extern AFUNPTR fptrclock_nanosleep;
extern AFUNPTR fptrmq_close;
extern AFUNPTR fptrmq_open;
extern AFUNPTR fptrmq_receive;
extern AFUNPTR fptrmq_timedreceive;
extern AFUNPTR fptrmq_send;
extern AFUNPTR fptrmq_timedsend;

/* ===================================================================== */
/* Replacement functions implemented in the other part of the tool       */
/* ===================================================================== */
extern "C"
{
    void myclock_nanosleep();
    void mymq_close();
    void mymq_open();
    void mymq_receive();
    void mymq_timedreceive();
    void mymq_send();
    void mymq_timedsend();
}

/* ===================================================================== */
/* Commandline Switches                                                  */
/* ===================================================================== */

KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "tpss_lin_librt.txt", "specify tool log file name");

/* ===================================================================== */
/* Utility functions                                                     */
/* ===================================================================== */

// Print help information
INT32 Usage()
{
    cerr << "This tool mimics the behavior of TPSS on Linux by adding probes to various librt functions." << endl;
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

extern "C" void printFunctionCalled(const char* funcName)
{
    OutFile << CurrentTime() << funcName << " called " << endl;
    OutFile.flush();
}

/* ===================================================================== */
/* Instrumnetation functions                                             */
/* ===================================================================== */

// Image load callback - inserts the probes.
void ImgLoad(IMG img, void* v)
{
    // Called every time a new image is loaded

    if ((IMG_Name(img).find("librt.so") != string::npos) || (IMG_Name(img).find("LIBRT.SO") != string::npos) ||
        (IMG_Name(img).find("LIBRT.so") != string::npos))
    {
        RTN rtnclock_nanosleep = RTN_FindByName(img, "clock_nanosleep");
        if (RTN_Valid(rtnclock_nanosleep) && RTN_IsSafeForProbedReplacement(rtnclock_nanosleep))
        {
            OutFile << CurrentTime() << "Inserting probe for clock_nanosleep at " << RTN_Address(rtnclock_nanosleep) << endl;
            OutFile.flush();
            AFUNPTR fptr        = (RTN_ReplaceProbed(rtnclock_nanosleep, AFUNPTR(myclock_nanosleep)));
            fptrclock_nanosleep = fptr;
        }

        RTN rtnmq_close = RTN_FindByName(img, "mq_close");
        if (RTN_Valid(rtnmq_close) && RTN_IsSafeForProbedReplacement(rtnmq_close))
        {
            OutFile << CurrentTime() << "Inserting probe for mq_close at " << RTN_Address(rtnmq_close) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnmq_close, AFUNPTR(mymq_close)));
            fptrmq_close = fptr;
        }

        RTN rtnmq_open = RTN_FindByName(img, "mq_open");
        if (RTN_Valid(rtnmq_open) && RTN_IsSafeForProbedReplacement(rtnmq_open))
        {
            OutFile << CurrentTime() << "Inserting probe for mq_open at " << RTN_Address(rtnmq_open) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnmq_open, AFUNPTR(mymq_open)));
            fptrmq_open  = fptr;
        }

        RTN rtnmq_receive = RTN_FindByName(img, "mq_receive");
        if (RTN_Valid(rtnmq_receive) && RTN_IsSafeForProbedReplacement(rtnmq_receive))
        {
            OutFile << CurrentTime() << "Inserting probe for mq_receive at " << RTN_Address(rtnmq_receive) << endl;
            OutFile.flush();
            AFUNPTR fptr   = (RTN_ReplaceProbed(rtnmq_receive, AFUNPTR(mymq_receive)));
            fptrmq_receive = fptr;
        }

        RTN rtnmq_timedreceive = RTN_FindByName(img, "mq_timedreceive");
        if (RTN_Valid(rtnmq_timedreceive) && RTN_IsSafeForProbedReplacement(rtnmq_timedreceive))
        {
            OutFile << CurrentTime() << "Inserting probe for mq_timedreceive at " << RTN_Address(rtnmq_timedreceive) << endl;
            OutFile.flush();
            AFUNPTR fptr        = (RTN_ReplaceProbed(rtnmq_timedreceive, AFUNPTR(mymq_timedreceive)));
            fptrmq_timedreceive = fptr;
        }

        RTN rtnmq_send = RTN_FindByName(img, "mq_send");
        if (RTN_Valid(rtnmq_send) && RTN_IsSafeForProbedReplacement(rtnmq_send))
        {
            OutFile << CurrentTime() << "Inserting probe for mq_send at " << RTN_Address(rtnmq_send) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnmq_send, AFUNPTR(mymq_send)));
            fptrmq_send  = fptr;
        }

        RTN rtnmq_timedsend = RTN_FindByName(img, "mq_timedsend");
        if (RTN_Valid(rtnmq_timedsend) && RTN_IsSafeForProbedReplacement(rtnmq_timedsend))
        {
            OutFile << CurrentTime() << "Inserting probe for mq_timedsend at " << RTN_Address(rtnmq_timedsend) << endl;
            OutFile.flush();
            AFUNPTR fptr     = (RTN_ReplaceProbed(rtnmq_timedsend, AFUNPTR(mymq_timedsend)));
            fptrmq_timedsend = fptr;
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
