/*
 * Copyright (C) 2013-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#ifndef REGVALUES_H
#define REGVALUES_H

#ifdef TARGET_WINDOWS
#define ASMNAME(name)
#else
#define ASMNAME(name) asm(name)
#endif

const unsigned MAXSIZE       = 64;
const unsigned GPRSIZEMAX    = 8;
const unsigned GPRSIZE       = sizeof(void*);
const unsigned GPR32SIZE     = 4;
const unsigned STSIZE        = 10;
const unsigned STSIZEALIGNED = 16;
const unsigned XMMSIZE       = 16;
const unsigned YMMSIZE       = 32;
const unsigned ZMMSIZE       = 64;
const unsigned OPMASKSIZE    = 8;

const unsigned GPRBITSIZE    = GPRSIZE * 8;
const unsigned STBITSIZE     = STSIZE * 8;
const unsigned XMMBITSIZE    = XMMSIZE * 8;
const unsigned YMMBITSIZE    = YMMSIZE * 8;
const unsigned ZMMBITSIZE    = ZMMSIZE * 8;
const unsigned OPMASKBITSIZE = OPMASKSIZE * 8;

// The fpSaveArea includes the fxsave area (512 bytes), the additional xsave area (320 bytes)
// and a 64 byte padding since the (f)xsave instructions require a 64-byte-aligned address
const unsigned FPSAVEAREASIZE = 896;

extern "C"
{
    extern unsigned char fpSaveArea[FPSAVEAREASIZE] ASMNAME("fpSaveArea");

    // These values will be loaded to registers by the application in the ChangeRegs() function.
    extern const unsigned char gprval[GPRSIZEMAX] ASMNAME("gprval");
    extern const unsigned char* gpr32val ASMNAME("gpr32val");
    extern const unsigned char* gpr16val ASMNAME("gpr16val");
    extern const unsigned char* gprlval ASMNAME("gprlval");
    extern const unsigned char* gprhval ASMNAME("gprhval");
    extern const unsigned char stval[STSIZE] ASMNAME("stval");
    extern const unsigned char xmmval[XMMSIZE] ASMNAME("xmmval");
    extern const unsigned char ymmval[YMMSIZE] ASMNAME("ymmval");
    extern const unsigned char zmmval[ZMMSIZE] ASMNAME("zmmval");
    extern const unsigned char opmaskval[OPMASKSIZE] ASMNAME("opmaskval");

    // These values will be loaded to registers by the tool in the ReplaceChangeRegs() function.
    // All values should be 64-bit aligned.
    extern const unsigned char tgprval[GPRSIZEMAX] ASMNAME("tgprval");
    extern const unsigned char* tgpr32val ASMNAME("tgpr32val");
    extern const unsigned char* tgpr16val ASMNAME("tgpr16val");
    extern const unsigned char* tgprlval ASMNAME("tgprlval");
    extern const unsigned char* tgprhval ASMNAME("tgprhval");
    extern const unsigned char tstval[STSIZEALIGNED] ASMNAME("tstval");
    extern const unsigned char txmmval[XMMSIZE] ASMNAME("txmmval");
    extern const unsigned char tymmval[YMMSIZE] ASMNAME("tymmval");
    extern const unsigned char tzmmval[ZMMSIZE] ASMNAME("tzmmval");
    extern const unsigned char topmaskval[OPMASKSIZE] ASMNAME("topmaskval");

    // These values will be assigned (stored from registers) by the application in the SaveRegsToMem() function.
    extern unsigned char agprval[GPRSIZE] ASMNAME("agprval");
    extern unsigned char astval[STSIZE] ASMNAME("astval");
    extern unsigned char axmmval[XMMSIZE] ASMNAME("axmmval");
    extern unsigned char aymmval[YMMSIZE] ASMNAME("aymmval");
    extern unsigned char azmmval[ZMMSIZE] ASMNAME("azmmval");
    extern unsigned char aopmaskval[OPMASKSIZE] ASMNAME("aopmaskval");
} // extern "C"

#endif // REGVALUES_H
