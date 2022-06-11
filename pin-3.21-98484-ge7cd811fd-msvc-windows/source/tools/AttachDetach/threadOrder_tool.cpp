/*
 * Copyright (C) 2011-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include "pin.H"
#include <cstdio>
#include <unistd.h>
#include <sched.h>
#include <cassert>

FILE* master    = NULL;
FILE* generated = NULL;
OS_THREAD_ID mainTid;

VOID onImageLoad(IMG img, VOID* v)
{
    if (IMG_IsMainExecutable(img))
    {
        fprintf(generated, "1\n");
    }
}

// This is done under the Pin client lock so there is no race on the global FILE* generated.
VOID onAppStart(VOID* v) { fprintf(generated, "2\n"); }

// This is done under the Pin client lock so there is no race on the global variables.
VOID onThreadAttach(VOID* sigset, VOID* v)
{
    OS_THREAD_ID tid = PIN_GetTid(); // The master thread's tid is the same as the pid of the entire process.
    if (tid == mainTid)
    { // This is the master thread which should be first.
        fprintf(generated, "3\n");
    }
    else
    { // This is the second thread which should be last, therefore close the file and exit.
        // If this happens before the master thread then there is a problem and the test should fail.
        // This failure will be discovered while comparing the two files.
        fprintf(generated, "4\n");
        fclose(generated);
        PIN_ExitProcess(0);
    }
}

VOID buildMasterFile()
{
    master = fopen("threadOrder_master.out", "w");
    assert(master != NULL);
    fprintf(master, "1\n2\n3\n4\n");
    fclose(master);
}

int main(int argc, char** argv)
{
    if (!PIN_Init(argc, argv))
    {
        PIN_InitSymbols();
        mainTid = PIN_GetTid();

        buildMasterFile();
        generated = fopen("threadOrder_generated.out", "w");
        assert(generated != NULL);

        IMG_AddInstrumentFunction(onImageLoad, 0);
        PIN_AddThreadAttachProbedFunction(onThreadAttach, 0);
        PIN_AddApplicationStartFunction(onAppStart, 0);

        PIN_StartProgramProbed();
    }

    return (1);
}
