/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
  This tool validates ability to set probe if base relocation is present in probed bytes.
  Such probe is not allowed in relocated image where fixups are not yet resolved.
  The test application first loads a DLL at preferred base address and then its copy
  which is certainly relocated by system loader, so rebase condition is guaranteed.
  The tool limits validation of Pin functionality only to case where fixups are not yet resolved.
  To detect unresolved fixups the tool checks reference to known address.
*/

#include <stdio.h>
#include <string>

#include "pin.H"

namespace WIND
{
#include <windows.h>
}

using std::string;

/*
 * Return preferred image base taken from field OptionalHeader.ImageBase
 * in NT optional header of loaded image.
 * NOTE: OS may set value in the field to actual base address of the loaded image.
 * Pin reasonably assumes that it may only happen AFTER the OS loader relocated the image
 * and applied base relocations.
 * @param[in]  moduleBase    Actual base address of loaded image.
 */
ADDRINT GetModulePreferredBase(ADDRINT moduleBase)
{
    if (moduleBase == 0)
    {
        return 0;
    }
    WIND::PIMAGE_DOS_HEADER pDos = reinterpret_cast< WIND::PIMAGE_DOS_HEADER >(moduleBase);

    // Returns FALSE when not DOS MZ header
    if (pDos->e_magic != IMAGE_DOS_SIGNATURE)
    {
        return 0;
    }

    const WIND::PIMAGE_NT_HEADERS pHeaders =
        reinterpret_cast< WIND::PIMAGE_NT_HEADERS >(reinterpret_cast< WIND::ULONG_PTR >(pDos) + pDos->e_lfanew);

    // check that this is PE/COFF image
    if (pHeaders->Signature != IMAGE_NT_SIGNATURE)
    {
        return 0;
    }

    return pHeaders->OptionalHeader.ImageBase;
}

/*
 * Return TRUE if baseName matches tail of imageName. Comparison is case-insensitive.
 * @param[in]  imageName  image file name in either form with extension
 * @param[in]  baseName   image base name with extension (e.g. kernel32.dll)
 */
static BOOL CmpBaseImageName(const string& imageName, const string& baseName)
{
    if (imageName.size() >= baseName.size())
    {
        return _stricmp(imageName.c_str() + imageName.size() - baseName.size(), baseName.c_str()) == 0;
    }
    return FALSE;
}

static VOID on_module_loading(IMG img, VOID* data)
{
    // Image rebase detection.
    // Only mismatch between actual and preferred base address is considered as detectable rebase.
    BOOL rebase_detected = (IMG_LowAddress(img) != GetModulePreferredBase(IMG_LowAddress(img)));

    RTN routine = RTN_FindByName(img, "baserel_in_probe");
    if (!RTN_Valid(routine))
    {
        routine = RTN_FindByName(img, "_baserel_in_probe");
    }

    // This same function is exported by the main executable and by 2 loaded DLLs.
    // Main executable is built with base address 0, so it will be relocated.
    // DLLs are exact copies, differ only by file name.
    // The first DLL is loaded at its preferred base address 0x10000000 and the second is relocated.
    if (RTN_Valid(routine))
    {
        // Fixup is located at offset 3 from function entry point.
        // Value of the fixup is address of the function entry point.
        ADDRINT* fixup_addr = reinterpret_cast< ADDRINT* >(RTN_Address(routine) + 3);

        if (rebase_detected)
        {
            // This is apparent image relocation event.
            // In general we expect Pin to reject probe with base relocation,
            // since Pin considers worst scenario (fixups were not yet resolved).
            // Check that probe is not allowed due to unresolved fixup.
            if (!RTN_IsSafeForProbedInsertion(routine) && !RTN_IsSafeForProbedReplacement(routine))
            {
                printf("fixup is handled properly, probe refused\n");
            }
            else if (*fixup_addr == RTN_Address(routine))
            {
                // If Pin indeed allowed probe (for images loaded before Pin attach, excluding main exe),
                // ensure that fixups have been already applied.
                printf("fixup is handled properly, probe enabled as fixups are considered applied\n");
            }
            else
            {
                printf("ERROR: probe was enabled while fixups were not yet applied in relocated image\n");
            }
        }
        else
        {
            if (RTN_IsSafeForProbedInsertion(routine) && RTN_IsSafeForProbedReplacement(routine))
            {
                if (*fixup_addr != RTN_Address(routine))
                {
                    // Unexpected situaltion. OS reports fixups were applied while actually it is not yet done.
                    printf("ERROR: Image fared relocated. Probe was enabled while fixups were not yet applied\n");
                }
                else
                {
                    printf("fixup is handled properly, probe enabled\n");
                }
            }
            else
            {
                printf("ERROR: probe was unexpectedly refused for non-relocated image or image with applied fixups\n");
            }
        }
        fflush(stdout);
    }

    if (CmpBaseImageName(IMG_Name(img), "kernel32.dll"))
    {
        routine = RTN_FindByName(img, "DuplicateHandle");
        if (!RTN_Valid(routine))
        {
            routine = RTN_FindByName(img, "_DuplicateHandle@28");
        }
        if (RTN_Valid(routine))
        {
            // DuplicateHandle in kernel32.dll contains base relocation in first 5 bytes of code.
            // Anyway Pin should allow probe regardless of kernel32.dll relocation.
            // kernel32.dll is always loaded in memory prior to Pin attach and thus it is assumed
            // base relocation were already applied by OS prior to invocation of this callback.
            // Check that probe is allowed.
            if (RTN_IsSafeForProbedInsertion(routine) && RTN_IsSafeForProbedReplacement(routine))
            {
                printf("fixup is handled properly, probe enabled\n");
            }
            // This extra check is done to demonstrate that Pin may reject this probe due to
            // other reasons (like wrong assumptions in Pin's naive static code discovery algorithm
            // regarding branch targets in probed bytes).
            // NOTE: Due to lack of internal info we do not consider this reject as test failure.
            else if (!RTN_IsSafeForProbedInsertion(routine) && !RTN_IsSafeForProbedReplacement(routine))
            {
                if (rebase_detected)
                {
                    printf("fixup is handled properly, probe LIKELY refused due to reason not related to image relocation\n");
                }
                else
                {
                    printf("fixup is handled properly, probe refused due to reason not related to image relocation\n");
                }
            }
            else
            {
                printf("ERROR: probe of DuplicateHandle is handled improperly\n");
            }
            fflush(stdout);
        }
    }
    // The tool is expected to print the message 4 times.
}

int main(int argc, char** argv)
{
    PIN_InitSymbols();

    if (!PIN_Init(argc, argv))
    {
        IMG_AddInstrumentFunction(on_module_loading, 0);

        PIN_StartProgramProbed();
    }

    exit(1);
}
