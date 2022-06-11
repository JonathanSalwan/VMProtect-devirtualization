/*
 * Copyright (C) 2012-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 *  This tool verifies that the registered image unload callbacks are being called
 *  before the image was unloaded.
 */

#include <iostream>
#include <cstdio>
#include <cstring>
#include <sys/resource.h>
#include <sys/types.h>
#include <unistd.h>
#include "pin.H"
using std::cout;
using std::endl;
using std::string;

const string SO_NAME = "one.so";

const int BUFF_SIZE = 1024;

VOID ImageUnload(IMG img, VOID* v)
{
    size_t found1 = IMG_Name(img).find(SO_NAME);
    if (found1 != string::npos)
    {
        // verify that "one.so" is in the mapped memory regions of the process.
        string mapFile = string("/proc/") + decstr(getpid()) + "/maps";
        FILE* fp       = fopen(mapFile.c_str(), "r");
        char buff[BUFF_SIZE];

        while (fgets(buff, BUFF_SIZE, fp) != NULL)
        {
            if (strstr(buff, SO_NAME.c_str()) != 0)
            {
                cout << "one.so is mapped in the memory." << endl;
            }
        }

        fclose(fp);
    }
}

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

static INT32 Usage()
{
    PIN_ERROR(" This tool verifies that the registered image unload callback is called when an image has been unloaded\n");
    return -1;
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */

int main(int argc, char* argv[])
{
    // Initialize symbol processing
    PIN_InitSymbols();

    // Initialize pin
    if (PIN_Init(argc, argv)) return Usage();

    // Register ImageUnload to be called when an image is unloaded
    IMG_AddUnloadFunction(ImageUnload, 0);

    // Start the program, never returns
    PIN_StartProgram();

    return 0;
}
