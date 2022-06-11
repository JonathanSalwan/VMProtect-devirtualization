/*
 * Copyright (C) 2008-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 *  pin tool combined from multi-DLLs (main_dll, dynamic_secondary_dll, static_secondary_dll). 
 */

#include <link.h>
#include <dlfcn.h>

#include <iostream>
#include <fstream>

using std::endl;
using std::hex;
using std::ofstream;
using std::showbase;

ofstream outfile;

// This function gets info of an image loaded by Pin loader.
// Invoked by dl_iterate_phdr()
int dl_iterate_callback(struct dl_phdr_info* info, size_t size, VOID* data)
{
    // Increment module counter.
    ++(*reinterpret_cast< int* >(data));
    return 0;
}

extern "C" __declspec(dllexport) int Init2(bool enumerate, const char* out_filename)
{
    int nModules = 0;
    if (enumerate)
    {
        // Enumerate DLLs currently loaded by Pin loader.
        dl_iterate_phdr(dl_iterate_callback, &nModules);
    }
    outfile.open(out_filename);
    outfile << hex << showbase;

    return nModules;
}

extern "C" __declspec(dllexport) void BeforeBBL2(void* ip) { outfile << "Before BBL, ip " << ip << endl; }

extern "C" __declspec(dllexport) void Fini2() { outfile.close(); }

// Define main - will never be called
// can be avoided by removing /EXPORT:main from link flags
int main(int argc, char* argv[]) { return 0; }
