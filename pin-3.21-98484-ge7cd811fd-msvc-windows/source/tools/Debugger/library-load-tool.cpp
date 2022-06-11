/*
 * Copyright (C) 2014-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * This tool tests GDB's svr4 libraries extension.
 * One of the tests involves making GDB think that libc.so was loaded at
 * base address 0xd00dead and then check in GDB output that it really
 * shows that libc.so was loaded there.
 */
#include <pin.H>
#include <iostream>
#include "link.h"
#include "stdlib.h"
#include "string.h"
using std::string;

/* =====================================================================
 * Called upon bad command line argument
 * ===================================================================== */
INT32 Usage()
{
    std::cerr << "This pin tool tests GDB svr4 libraries extension" << std::endl;

    std::cerr << KNOB_BASE::StringKnobSummary();

    std::cerr << std::endl;

    return -1;
}

VOID Image(IMG img, VOID*)
{
    LINUX_LOADER_IMAGE_INFO* oldLi = (LINUX_LOADER_IMAGE_INFO*)IMG_GetLoaderInfo(img);
    if (oldLi != NULL)
    {
        // Make sure IMG_LowAddress is the same as the loader base address
        ASSERT(IMG_LoadOffset(img) == oldLi->l_addr, hexstr(IMG_LoadOffset(img)) + " != " + hexstr(oldLi->l_addr));
    }
    if (IMG_Name(img).find("libc.so") != string::npos)
    {
        struct link_map* lm = (struct link_map*)malloc(sizeof(struct link_map));
        string name_copy    = IMG_Name(img);
        // Base address as appear in ELF file before relocating the image
        ADDRINT suggestedBaseAddress = IMG_LowAddress(img) - oldLi->l_addr;
        LINUX_LOADER_IMAGE_INFO li;
        memset(lm, 0, sizeof(*lm));
        // Make libc.so appear to be loaded at 0xd00dead
        // GDB should see libc.so loaded at that address
        lm->l_addr = 0xd00dead - suggestedBaseAddress;
        lm->l_ld   = (ElfW(Dyn)*)(oldLi->l_ld - IMG_LowAddress(img) + 0xd00dead);
        lm->l_name = (char*)name_copy.c_str();
        lm->l_next = ((struct link_map*)oldLi->lm)->l_next;
        lm->l_prev = ((struct link_map*)oldLi->lm)->l_prev;

        li.lm     = (ADDRINT)lm;
        li.l_ld   = (ADDRINT)lm->l_ld;
        li.l_addr = lm->l_addr;
        li.name   = lm->l_name;
        IMG_SetLoaderInfo(img, &li);
        ASSERTX(IMG_LoadOffset(img) == li.l_addr);
    }
}

/* =====================================================================
 * Entry point for the tool
 * ===================================================================== */
int main(int argc, char* argv[])
{
    if (PIN_Init(argc, argv))
    {
        return Usage();
    }
    IMG_AddInstrumentFunction(Image, NULL);

    // Never returns
    PIN_StartProgram();
    return 0;
}
/* ===================================================================== */
/* eof */
/* ===================================================================== */
