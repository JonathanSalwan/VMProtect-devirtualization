/*
 * Copyright (C) 2011-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * This app checks a bug in the debug_elf cache mechanism. The cache contains images
 * that have previously been searched for debug information. When a pintool calls the
 * PIN_GetSourceLocation API, this cache is searched first (for the nearest address
 * which is greater or equal to the one being searched for). However, unloaded images
 * were never removed from the cache. When the API was used during an image unload
 * callback, if the images at high addresses were removed first, the search could be
 * conducted on images in the cache that had already been removed from Pin's internal
 * data structures, thus causing an assertion. The solution was to remove these images
 * from the cache as well as from the other data structres.
 * This app loads two images and then unloads the higher addressed one first in order
 * to recreate the conditions mentioned above.
 */

#include <dlfcn.h>
#include <iostream>
#include <cstdlib>

using std::cerr;
using std::endl;

void* DLopen(const char* filename)
{
    void* handle = dlopen(filename, RTLD_LAZY);
    if (handle == NULL)
    {
        cerr << "APP ERROR: failed to open " << filename << " - " << dlerror() << endl;
        exit(-1);
    }
    return handle;
}

void* DLsym(void* handle, const char* symname)
{
    dlerror(); // clear errors
    void* sym   = dlsym(handle, symname);
    char* error = dlerror();
    if (error != NULL)
    {
        cerr << "APP ERROR: failed to get address for symbol " << symname << " - " << error << endl;
        exit(-1);
    }
    return sym;
}

void DLcloseBoth(void* first, void* second)
{
    if (dlclose(first) != 0)
    {
        cerr << "APP ERROR: failed to close image at " << first << " - " << dlerror() << endl;
        exit(-1);
    }
    if (dlclose(second) != 0)
    {
        cerr << "APP ERROR: failed to close image at " << second << " - " << dlerror() << endl;
        exit(-1);
    }
}

/*
 * Expected argv arguments:
 * [1] image1
 * [2] image2
 */
int main(int argc, char* argv[])
{
    void* handle1 = DLopen(argv[1]);          // open image1
    void* handle2 = DLopen(argv[2]);          // open image2
    void* dummy1  = DLsym(handle1, "dummy1"); // get address from image1
    void* dummy2  = DLsym(handle2, "dummy2"); // get address from image2

    // unload images - higher addressed image first
    if (dummy1 > dummy2)
    {
        DLcloseBoth(handle1, handle2);
    }
    else
    {
        DLcloseBoth(handle2, handle1);
    }
    return 0;
}
