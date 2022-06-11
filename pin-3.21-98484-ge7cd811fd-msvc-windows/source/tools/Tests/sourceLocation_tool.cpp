/*
 * Copyright (C) 2011-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * This tool checks a bug in the debug_elf cache mechanism encountered when invoking
 * the PIN_GetSourceLocation API.
 * See sourceLocation_app.cpp for detailed explanations.
 */

#include "pin.H"

VOID onImageUnload(IMG img, VOID* data) { PIN_GetSourceLocation(IMG_LowAddress(img), NULL, NULL, NULL); }

int main(int argc, char** argv)
{
    PIN_InitSymbols();

    if (!PIN_Init(argc, argv))
    {
        IMG_AddUnloadFunction(onImageUnload, 0);

        PIN_StartProgram();
    }
    return (1);
}
