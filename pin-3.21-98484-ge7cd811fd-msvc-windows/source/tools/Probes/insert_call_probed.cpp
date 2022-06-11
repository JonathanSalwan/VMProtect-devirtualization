/*
 * Copyright (C) 2010-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
  This file test the PIN_InsertCallProbed feature.
  The test application adds the special "annotations".
  The test tool read the annotation and insert call to the notification function.
  Since we want the notification prints to be interleaved with the application output,
  we use the same "write function" from the application.
 */

#include "pin.H"
#include <stdlib.h>
#include <stdio.h>
#include "tool_macros.h"

/* ===================================================================== */
/* Global Variables */
/* ===================================================================== */

struct ANNOTATION
{
    ADDRINT addr;
    ADDRINT value;
    ANNOTATION() : addr(0), value(0) {}
};

// Function to write to the output stream
void (*writeFun)(char*) = 0;

/* ===================================================================== */

INT32 Usage()
{
    fprintf(stderr, "This pin tool tests the PIN_InsertCallProbed API.\n");
    fprintf(stderr, "%s\n", KNOB_BASE::StringKnobSummary().c_str());
    return -1;
}

void Notification(ADDRINT val)
{
    char buff[80];

    if (!writeFun)
    {
        fprintf(stderr, "Write Function was not initialized ...\n");
        exit(1);
    }

    sprintf(buff, "Notification value: %p", Addrint2VoidStar(val));
    writeFun(buff);
}

/* ===================================================================== */
// Called every time a new image is loaded
// Look for routines that we want to probe
VOID ImageLoad(IMG img, VOID* v)
{
    const ANNOTATION* ann = 0;
    USIZE num             = 0;

    printf("Processing %s\n", IMG_Name(img).c_str());

    for (SEC sec = IMG_SecHead(img); SEC_Valid(sec); sec = SEC_Next(sec))
    {
        if (SEC_Name(sec) == "MyAnnot")
        {
            ann = reinterpret_cast< const ANNOTATION* >(SEC_Data(sec));
            num = SEC_Size(sec) / sizeof(ANNOTATION);
        }
    }

    if (ann)
    {
        printf("Found annotations: \n");
        for (UINT32 i = 0; i < num; i++)
        {
            ADDRINT addr = ann[i].addr + IMG_LoadOffset(img);
            ADDRINT val  = ann[i].value;
            printf("\t%p %p\t", Addrint2VoidStar(addr), Addrint2VoidStar(val));
            if (PIN_IsSafeForProbedInsertion(addr))
            {
                PIN_InsertCallProbed(addr, AFUNPTR(Notification), IARG_ADDRINT, val, IARG_END);
                printf(" - OK\n");
            }
            else
            {
                printf(" - Failed\n");
            }
        }

        // Set the write line function, from the image of the annotations (i.e. the main executable).
        RTN writeRtn = RTN_FindByName(img, C_MANGLE("write_line"));
        if (RTN_Valid(writeRtn))
        {
            writeFun = (void (*)(char*))RTN_Funptr(writeRtn);
        }
    }

    printf("Completed %s\n", IMG_Name(img).c_str());
}

/* ===================================================================== */

int main(int argc, CHAR* argv[])
{
    PIN_InitSymbols();

    if (PIN_Init(argc, argv))
    {
        return Usage();
    }

    IMG_AddInstrumentFunction(ImageLoad, 0);

    PIN_StartProgramProbed();

    return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
