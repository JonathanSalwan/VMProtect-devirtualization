/*
 * Copyright (C) 2012-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include "pin.H"
#include <iostream>
#include <stdlib.h>
#include <string>
using std::cerr;
using std::endl;

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */
INT32 Usage()
{
    cerr << "This tool checks that the launcher setups the environment correctly. \n"
            "\n";
    cerr << KNOB_BASE::StringKnobSummary();
    cerr << endl;
    return -1;
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */
int main(int argc, char* argv[])
{
    if (PIN_Init(argc, argv))
    {
        return Usage();
    }

#if defined(TARGET_LINUX)
    // Check that LD_LIBRARY_PATH contains the required libraries to the VM.
    //
    char* ld_library_path = getenv("LD_LIBRARY_PATH");
    if (ld_library_path)
    {
        std::string ld_path(ld_library_path);
        // Adding ':' after each string because they all new to be before the old values in the
        // LD_LIBRARY_PATH.
        //
    }
#elif defined(TARGET_MAC)
    char* dyld_library_path = getenv("DYLD_LIBRARY_PATH");
    // Check that DYLD_LIBRARY_PATH is set and contains the runtime libraries.
    //
    if (NULL == dyld_library_path)
    {
        std::cout << "Failed in tool! DYLD_LIBRARY_PATH not set" << std::endl;
        exit(1);
    }
    std::string ld_path(dyld_library_path);
    bool not_found_32_xed = ld_path.find("/extras/xed-ia32/lib:") == std::string::npos;
    bool not_found_64_xed = ld_path.find("/extras/xed-intel64/lib:") == std::string::npos;
    bool not_found_32_ext = ld_path.find("/ia32/lib-ext:") == std::string::npos;
    bool not_found_64_ext = ld_path.find("/intel64/lib-ext:") == std::string::npos;
    if (not_found_32_xed || not_found_64_xed || not_found_32_ext || not_found_64_ext)
    {
        std::cout << "Failed! DYLD_LIBRARY_PATH = " << dyld_library_path << std::endl;
        exit(1);
    }
#ifdef PIN_CRT
    // Check that DYLD_LIBRARY_PATH contains the PIN CRT libraries.
    //
    bool not_found_32_pincrt = ld_path.find("/ia32/runtime/pincrt:") == std::string::npos;
    bool not_found_64_pincrt = ld_path.find("/intel64/runtime/pincrt:") == std::string::npos;
    if (not_found_32_pincrt || not_found_64_pincrt)
    {
        std::cout << "Failed! DYLD_LIBRARY_PATH = " << dyld_library_path << std::endl;
        exit(1);
    }
#endif // PIN_CRT
#endif // TARGET_MAC

    std::cout << "Tool success!" << std::endl;

    // Never returns
    PIN_StartProgram();

    return 0;
}
/* ===================================================================== */
/* eof */
/* ===================================================================== */
