/*
 * Copyright (C) 2015-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 *  This application should be used with the images_on_attach_tool tool.
 *  See documentation in the tool for the test details.
 */

#include <iostream>
#include <cstdlib>
#include <dlfcn.h>
#include <unistd.h>

using std::cerr;
using std::cout;
using std::endl;

extern "C"
{
    void DoRelease(volatile bool* doRelease)
    {
        // Do nothing
    }
} // extern "C"

static void WaitForAttach()
{
    const unsigned int timeout  = 300;
    unsigned int releaseCounter = 0;
    volatile bool released      = false;
    while (!released)
    {
        if (timeout == releaseCounter)
        {
            cerr << "APP ERROR: Timeout reached and the tool did not release the application." << endl;
            exit(1);
        }
        ++releaseCounter;
        DoRelease(&released);
        sleep(1);
    }
}

static void LoadAdditionalLibraries(const char* usrlib)
{
    const void* libutil = dlopen("libutil.so.1", RTLD_LAZY);
    if (NULL == libutil)
    {
        cerr << "APP ERROR: Failed to load libutil.so.1" << endl;
        exit(1);
    }
    const void* usrlibptr = dlopen(usrlib, RTLD_LAZY);
    if (NULL == usrlibptr)
    {
        cerr << "APP ERROR: Failed to load " << usrlib << endl;
        exit(1);
    }
}

/*
 * Expected arguments:
 *
 * [1] - Shared object to load dynamically
 */
int main(int argc, char* argv[])
{
    // Check the number of parameters.
    if (2 != argc)
    {
        cerr << "Usage: " << argv[0] << " <shared object to load>" << endl;
        return 1;
    }

    // Wait for the tool to attach to the application.
    WaitForAttach();

    // Pin is attached, now load two more shared objects.
    LoadAdditionalLibraries(argv[1]);

    // Done.
    cout << "APP: Application completed successfully." << endl;
    return 0;
}
