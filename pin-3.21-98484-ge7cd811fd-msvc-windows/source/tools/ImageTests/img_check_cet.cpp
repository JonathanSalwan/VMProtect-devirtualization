/*
 * Copyright (C) 2018-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

//
// This tool validates Pin handling CET enabled image.
// The tool gets and image (-m) to check and additional 2 optional CET properties to check (-i , -s):
// -i (IBT) checks if indirect branch target property was set.
// -s (SHSTK) checks if shadow stack property was enabled.
// If one of the option is missing it is expected that this property is NOT set in the image. Thus:
// No -i -s implies to check if both properties are NOT set.
// -i alone implies to check that only IBT was enabled.
// -s alone implies to check that only SHSTK was enabled.
// -i -s implies to check that both properties are enabled.
// In a note in Mantis 4620 one can find instructions how to build IBT/SHSTK enabled imaged on CET enabled system.
// To build the tool one can simply use the following:
//      make obj-intel64/img_check_cet.so
// A template to run the tool (in the makefile.ruules):
// $(PIN) -t $(OBJDIR)img_check_cet$(PINTOOL_SUFFIX) [-i|-s] -m <my app> -- <any app>
// One can give <any app> as the tool exits from its main after analyzing the image provided.

#include "pin.H"
#include <stdio.h>
#include <iostream>
#include <fstream>

using std::cerr;
using std::endl;
using std::string;

KNOB< string > KnobImageName(KNOB_MODE_WRITEONCE, "pintool", "m", "", "image name to process");

KNOB< BOOL > KnobIbt(KNOB_MODE_WRITEONCE, "pintool", "i", "0", "verify indirect branch tracking");

KNOB< BOOL > KnobShstk(KNOB_MODE_WRITEONCE, "pintool", "s", "0", "verify shadow stack");

// argc, argv are the entire command line, including pin -t <toolname> -- ...
int main(int argc, char* argv[])
{
    // Initialize symbol processing
    PIN_InitSymbols();

    // Initialize pin
    if (PIN_Init(argc, argv) != 0)
    {
        PIN_ExitProcess(1);
    }

    if (KnobImageName.Value() == "")
    {
        cerr << "image name must be specified" << endl;
        PIN_ExitProcess(1);
    }

    // Test IMG_Open
    IMG img = IMG_Open(KnobImageName.Value());
    if (!IMG_Valid(img))
    {
        cerr << "could not open image " + KnobImageName.Value();
        PIN_ExitProcess(1);
    }

    if (KnobIbt && !IMG_HasProperty(img, IMG_PROPERTY_IBT_ENABLED))
    {
        cerr << "image is not IBT enabled" << endl;
        PIN_ExitProcess(1);
    }
    if (KnobShstk && !IMG_HasProperty(img, IMG_PROPERTY_SHSTK_ENABLED))
    {
        cerr << "image is not SHSTK enabled" << endl;
        PIN_ExitProcess(1);
    }
    if (!KnobIbt && IMG_HasProperty(img, IMG_PROPERTY_IBT_ENABLED))
    {
        cerr << "image is IBT enabled" << endl;
        PIN_ExitProcess(1);
    }
    if (!KnobShstk && IMG_HasProperty(img, IMG_PROPERTY_SHSTK_ENABLED))
    {
        cerr << "image is SHSTK enabled" << endl;
        PIN_ExitProcess(1);
    }

    IMG_Close(img);
    PIN_ExitProcess(0);
}
