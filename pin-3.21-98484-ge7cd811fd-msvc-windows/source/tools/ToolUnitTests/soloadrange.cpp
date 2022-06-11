/*
 * Copyright (C) 2007-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <string>
#include <linux/limits.h>
#include "pin.H"
using std::endl;
using std::hex;
using std::ofstream;
using std::string;

// On new Linux systems realpath is replaced with a safer version realpath_chk,
// this function is implemented only in GLIBC_2.4 and therefore is not supported
// with Pin-probes runtime. The replacement of realpath is implemented in <stdlib.h>
// This definition replaces the include of stdlib.h and preserve the "old" function.
extern "C" char* realpath(__const char* __restrict __name, char* __restrict __resolved);

ofstream out("soloadrange.out");

BOOL CheckImageContiguous(const char* name, ADDRINT low, ADDRINT high)
{
    FILE* fp = fopen("/proc/self/maps", "r");
    char buff[1024];
    ADDRINT glow = 0, ghigh = 0;
    // Try to make an educated guess about the image low and high address
    // According to file mapping created by the loader.
    // We're also prepared for the case where the last pages of the image
    // are anonymous file mapping.
    while (fgets(buff, 1024, fp) != NULL)
    {
        ADDRINT mapl, maph;
        if (sscanf(buff, "%lx-%lx", (unsigned long*)&mapl, (unsigned long*)&maph) != 2) continue;
        if (ghigh == mapl || strstr(buff, name) != 0)
        {
            if (glow == 0) glow = mapl;
            if (ghigh == 0) ghigh = maph;
            if (mapl < glow) glow = mapl;
            if (maph > ghigh) ghigh = maph;
        }
    }
    fclose(fp);

    if (glow == 0 || ghigh == 0) return TRUE;

    if (low < glow || high > ghigh)
    {
        return FALSE;
    }

    return TRUE;
}

BOOL CheckImageRegions(const char* name, IMG img)
{
    FILE* fp = fopen("/proc/self/maps", "r");
    char buff[1024];
    while (fgets(buff, 1024, fp) != NULL)
    {
        ADDRINT mapl, maph;
        if (strstr(buff, name) != 0)
        {
            if (sscanf(buff, "%lx-%lx", (unsigned long*)&mapl, (unsigned long*)&maph) != 2) continue;

            // For every segment, check if it is contained in a region
            // If not, we missed it...
            //
            BOOL found_region = FALSE;
            for (UINT i = 0; i < IMG_NumRegions(img); ++i)
            {
                found_region = (IMG_RegionLowAddress(img, i) <= mapl) && (IMG_RegionHighAddress(img, i) >= maph);
            }
            if (!found_region)
            {
                fclose(fp);
                return FALSE;
            }
        }
    }
    fclose(fp);

    return TRUE;
}

VOID ImageLoad(IMG img, VOID* v)
{
    string name = IMG_Name(img);

    char realname[PATH_MAX];
    if (realpath(name.c_str(), realname) == NULL) return;

    name                  = string(realname);
    string::size_type pos = name.rfind('/');
    if (pos != string::npos) name = name.substr(pos + 1);

    static BOOL stat = TRUE;
    if (name.find("libc-") != string::npos)
    {
        stat = FALSE;
        return;
    }

    if (stat) return;

    ADDRINT low  = IMG_LowAddress(img);
    ADDRINT high = IMG_HighAddress(img);

    if (IMG_NumRegions(img) == 1)
    {
        if (!CheckImageContiguous(realname, low, high))
        {
            out << "IMG name: " << name << " Low: " << low << " High: " << high << " "
                << "FAILED" << endl;
        }
    }
    else
    {
        if (!CheckImageRegions(realname, img))
        {
            out << "IMG name: " << name << " FAILED" << endl;
        }
    }
}

int main(INT32 argc, CHAR** argv)
{
    PIN_Init(argc, argv);

    out << hex;

    IMG_AddInstrumentFunction(ImageLoad, 0);

    // Never returns
    PIN_StartProgram();

    return 0;
}
