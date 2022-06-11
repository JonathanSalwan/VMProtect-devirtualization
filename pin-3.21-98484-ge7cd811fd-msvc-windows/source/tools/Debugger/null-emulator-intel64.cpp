/*
 * Copyright (C) 2010-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * This tool tests the API PIN_AddDebuggerRegisterEmulator(), which allows
 * a tool to provide emulated registers to GDB when using application-level debugging.
 * The tool doesn't do any useful emulation because all registers have the identity
 * emulation (e.g. the tool emulates REG_RCX by providing the value of REG_RCX).
 * However, this does demonstrate the usage of the API.
 */

#include <cstring>
#include <iostream>
#include "pin.H"

//
// These registers are "emulated" by the tool.
//
enum EMULATED_REG
{
    EMULATED_REG_RCX,
    EMULATED_REG_RSP,
    EMULATED_REG_ST0,
    EMULATED_REG_FPSW,
    EMULATED_REG_XMM0
};

// This describes the full set of registers that Pin provides to GDB.
// The order and size of each register must match the XML documents
// below.  Entries with REG_NONE are emulated by the tool.  Pin
// provides the values for the other registers.
//
DEBUGGER_REG_DESCRIPTION RegDescs[] = {{REG_RAX, 0, 64},
                                       {REG_RBX, 0, 64},
                                       {REG_NONE, EMULATED_REG_RCX, 64},
                                       {REG_RDX, 0, 64},
                                       {REG_RSI, 0, 64},
                                       {REG_RDI, 0, 64},
                                       {REG_RBP, 0, 64},
                                       {REG_NONE, EMULATED_REG_RSP, 64},
                                       {REG_R8, 0, 64},
                                       {REG_R9, 0, 64},
                                       {REG_R10, 0, 64},
                                       {REG_R11, 0, 64},
                                       {REG_R12, 0, 64},
                                       {REG_R13, 0, 64},
                                       {REG_R14, 0, 64},
                                       {REG_R15, 0, 64},
                                       {REG_INST_PTR, 0, 64},
                                       {REG_RFLAGS, 0, 32},
                                       {REG_SEG_CS, 0, 32},
                                       {REG_SEG_SS, 0, 32},
                                       {REG_SEG_DS, 0, 32},
                                       {REG_SEG_ES, 0, 32},
                                       {REG_SEG_FS, 0, 32},
                                       {REG_SEG_GS, 0, 32},
                                       {REG_NONE, EMULATED_REG_ST0, 80},
                                       {REG_ST1, 0, 80},
                                       {REG_ST2, 0, 80},
                                       {REG_ST3, 0, 80},
                                       {REG_ST4, 0, 80},
                                       {REG_ST5, 0, 80},
                                       {REG_ST6, 0, 80},
                                       {REG_ST7, 0, 80},
                                       {REG_FPCW, 0, 32},
                                       {REG_NONE, EMULATED_REG_FPSW, 32},
                                       {REG_FPTAG, 0, 32},
                                       {REG_FPIP_SEL, 0, 32},
                                       {REG_FPIP_OFF, 0, 32},
                                       {REG_FPDP_SEL, 0, 32},
                                       {REG_FPDP_OFF, 0, 32},
                                       {REG_FPOPCODE, 0, 32},
                                       {REG_NONE, EMULATED_REG_XMM0, 128},
                                       {REG_XMM1, 0, 128},
                                       {REG_XMM2, 0, 128},
                                       {REG_XMM3, 0, 128},
                                       {REG_XMM4, 0, 128},
                                       {REG_XMM5, 0, 128},
                                       {REG_XMM6, 0, 128},
                                       {REG_XMM7, 0, 128},
                                       {REG_XMM8, 0, 128},
                                       {REG_XMM9, 0, 128},
                                       {REG_XMM10, 0, 128},
                                       {REG_XMM11, 0, 128},
                                       {REG_XMM12, 0, 128},
                                       {REG_XMM13, 0, 128},
                                       {REG_XMM14, 0, 128},
                                       {REG_XMM15, 0, 128},
                                       {REG_MXCSR, 0, 32},
                                       {REG_ORIG_RAX, 0, 64}};

unsigned NumRegs = sizeof(RegDescs) / sizeof(RegDescs[0]);

// These XML documents describe the registers to GDB.  See the GDB
// Manual appendix titled "Target Descriptions" for more information.
//
const char FeatureDocumentTop[] = "<?xml version=\"1.0\"?>\n"
                                  "<!-- Copyright (C) 2010 Free Software Foundation, Inc.\n"
                                  "\n"
                                  "     Copying and distribution of this file, with or without modification,\n"
                                  "     are permitted in any medium without royalty provided the copyright\n"
                                  "     notice and this notice are preserved.  -->\n"
                                  "\n"
                                  "<!-- x86-64 - Includes Linux-only special \"register\".  -->\n"
                                  "\n"
                                  "<!DOCTYPE target SYSTEM \"gdb-target.dtd\">\n"
                                  "<target>\n"
                                  "  <architecture>i386:x86-64</architecture>\n"
                                  "  <osabi>GNU/Linux</osabi>\n"
                                  "  <xi:include href=\"64bit-core.xml\"/>\n"
                                  "  <xi:include href=\"64bit-sse.xml\"/>\n"
                                  "  <xi:include href=\"64bit-linux.xml\"/>\n"
                                  "</target>\n";

const char FeatureDocumentCore[] = "<?xml version=\"1.0\"?>\n"
                                   "<!-- Copyright (C) 2010 Free Software Foundation, Inc.\n"
                                   "\n"
                                   "     Copying and distribution of this file, with or without modification,\n"
                                   "     are permitted in any medium without royalty provided the copyright\n"
                                   "     notice and this notice are preserved.  -->\n"
                                   "\n"
                                   "<!DOCTYPE feature SYSTEM \"gdb-target.dtd\">\n"
                                   "<feature name=\"org.gnu.gdb.i386.core\">\n"
                                   "  <flags id=\"i386_eflags\" size=\"4\">\n"
                                   "    <field name=\"CF\" start=\"0\" end=\"0\"/>\n"
                                   "    <field name=\"\" start=\"1\" end=\"1\"/>\n"
                                   "    <field name=\"PF\" start=\"2\" end=\"2\"/>\n"
                                   "    <field name=\"AF\" start=\"4\" end=\"4\"/>\n"
                                   "    <field name=\"ZF\" start=\"6\" end=\"6\"/>\n"
                                   "    <field name=\"SF\" start=\"7\" end=\"7\"/>\n"
                                   "    <field name=\"TF\" start=\"8\" end=\"8\"/>\n"
                                   "    <field name=\"IF\" start=\"9\" end=\"9\"/>\n"
                                   "    <field name=\"DF\" start=\"10\" end=\"10\"/>\n"
                                   "    <field name=\"OF\" start=\"11\" end=\"11\"/>\n"
                                   "    <field name=\"NT\" start=\"14\" end=\"14\"/>\n"
                                   "    <field name=\"RF\" start=\"16\" end=\"16\"/>\n"
                                   "    <field name=\"VM\" start=\"17\" end=\"17\"/>\n"
                                   "    <field name=\"AC\" start=\"18\" end=\"18\"/>\n"
                                   "    <field name=\"VIF\" start=\"19\" end=\"19\"/>\n"
                                   "    <field name=\"VIP\" start=\"20\" end=\"20\"/>\n"
                                   "    <field name=\"ID\" start=\"21\" end=\"21\"/>\n"
                                   "  </flags>\n"
                                   "\n"
                                   "  <reg name=\"rax\" bitsize=\"64\" type=\"int64\"/>\n"
                                   "  <reg name=\"rbx\" bitsize=\"64\" type=\"int64\"/>\n"
                                   "  <reg name=\"rcx\" bitsize=\"64\" type=\"int64\"/>\n"
                                   "  <reg name=\"rdx\" bitsize=\"64\" type=\"int64\"/>\n"
                                   "  <reg name=\"rsi\" bitsize=\"64\" type=\"int64\"/>\n"
                                   "  <reg name=\"rdi\" bitsize=\"64\" type=\"int64\"/>\n"
                                   "  <reg name=\"rbp\" bitsize=\"64\" type=\"data_ptr\"/>\n"
                                   "  <reg name=\"rsp\" bitsize=\"64\" type=\"data_ptr\"/>\n"
                                   "  <reg name=\"r8\" bitsize=\"64\" type=\"int64\"/>\n"
                                   "  <reg name=\"r9\" bitsize=\"64\" type=\"int64\"/>\n"
                                   "  <reg name=\"r10\" bitsize=\"64\" type=\"int64\"/>\n"
                                   "  <reg name=\"r11\" bitsize=\"64\" type=\"int64\"/>\n"
                                   "  <reg name=\"r12\" bitsize=\"64\" type=\"int64\"/>\n"
                                   "  <reg name=\"r13\" bitsize=\"64\" type=\"int64\"/>\n"
                                   "  <reg name=\"r14\" bitsize=\"64\" type=\"int64\"/>\n"
                                   "  <reg name=\"r15\" bitsize=\"64\" type=\"int64\"/>\n"
                                   "\n"
                                   "  <reg name=\"rip\" bitsize=\"64\" type=\"code_ptr\"/>\n"
                                   "  <reg name=\"eflags\" bitsize=\"32\" type=\"i386_eflags\"/>\n"
                                   "  <reg name=\"cs\" bitsize=\"32\" type=\"int32\"/>\n"
                                   "  <reg name=\"ss\" bitsize=\"32\" type=\"int32\"/>\n"
                                   "  <reg name=\"ds\" bitsize=\"32\" type=\"int32\"/>\n"
                                   "  <reg name=\"es\" bitsize=\"32\" type=\"int32\"/>\n"
                                   "  <reg name=\"fs\" bitsize=\"32\" type=\"int32\"/>\n"
                                   "  <reg name=\"gs\" bitsize=\"32\" type=\"int32\"/>\n"
                                   "\n"
                                   "  <reg name=\"st0\" bitsize=\"80\" type=\"i387_ext\"/>\n"
                                   "  <reg name=\"st1\" bitsize=\"80\" type=\"i387_ext\"/>\n"
                                   "  <reg name=\"st2\" bitsize=\"80\" type=\"i387_ext\"/>\n"
                                   "  <reg name=\"st3\" bitsize=\"80\" type=\"i387_ext\"/>\n"
                                   "  <reg name=\"st4\" bitsize=\"80\" type=\"i387_ext\"/>\n"
                                   "  <reg name=\"st5\" bitsize=\"80\" type=\"i387_ext\"/>\n"
                                   "  <reg name=\"st6\" bitsize=\"80\" type=\"i387_ext\"/>\n"
                                   "  <reg name=\"st7\" bitsize=\"80\" type=\"i387_ext\"/>\n"
                                   "\n"
                                   "  <reg name=\"fctrl\" bitsize=\"32\" type=\"int\" group=\"float\"/>\n"
                                   "  <reg name=\"fstat\" bitsize=\"32\" type=\"int\" group=\"float\"/>\n"
                                   "  <reg name=\"ftag\" bitsize=\"32\" type=\"int\" group=\"float\"/>\n"
                                   "  <reg name=\"fiseg\" bitsize=\"32\" type=\"int\" group=\"float\"/>\n"
                                   "  <reg name=\"fioff\" bitsize=\"32\" type=\"int\" group=\"float\"/>\n"
                                   "  <reg name=\"foseg\" bitsize=\"32\" type=\"int\" group=\"float\"/>\n"
                                   "  <reg name=\"fooff\" bitsize=\"32\" type=\"int\" group=\"float\"/>\n"
                                   "  <reg name=\"fop\" bitsize=\"32\" type=\"int\" group=\"float\"/>\n"
                                   "</feature>\n";

const char FeatureDocumentLinux[] = "<?xml version=\"1.0\"?>\n"
                                    "<!-- Copyright (C) 2010 Free Software Foundation, Inc.\n"
                                    "\n"
                                    "     Copying and distribution of this file, with or without modification,\n"
                                    "     are permitted in any medium without royalty provided the copyright\n"
                                    "     notice and this notice are preserved.  -->\n"
                                    "\n"
                                    "<!DOCTYPE feature SYSTEM \"gdb-target.dtd\">\n"
                                    "<feature name=\"org.gnu.gdb.i386.linux\">\n"
                                    "  <reg name=\"orig_rax\" bitsize=\"64\" type=\"int\" regnum=\"57\"/>\n"
                                    "</feature>\n";

const char FeatureDocumentSSE[] = "<?xml version=\"1.0\"?>\n"
                                  "<!-- Copyright (C) 2010 Free Software Foundation, Inc.\n"
                                  "\n"
                                  "     Copying and distribution of this file, with or without modification,\n"
                                  "     are permitted in any medium without royalty provided the copyright\n"
                                  "     notice and this notice are preserved.  -->\n"
                                  "\n"
                                  "<!DOCTYPE feature SYSTEM \"gdb-target.dtd\">\n"
                                  "<feature name=\"org.gnu.gdb.i386.sse\">\n"
                                  "  <vector id=\"v4f\" type=\"ieee_single\" count=\"4\"/>\n"
                                  "  <vector id=\"v2d\" type=\"ieee_double\" count=\"2\"/>\n"
                                  "  <vector id=\"v16i8\" type=\"int8\" count=\"16\"/>\n"
                                  "  <vector id=\"v8i16\" type=\"int16\" count=\"8\"/>\n"
                                  "  <vector id=\"v4i32\" type=\"int32\" count=\"4\"/>\n"
                                  "  <vector id=\"v2i64\" type=\"int64\" count=\"2\"/>\n"
                                  "  <union id=\"vec128\">\n"
                                  "    <field name=\"v4_float\" type=\"v4f\"/>\n"
                                  "    <field name=\"v2_double\" type=\"v2d\"/>\n"
                                  "    <field name=\"v16_int8\" type=\"v16i8\"/>\n"
                                  "    <field name=\"v8_int16\" type=\"v8i16\"/>\n"
                                  "    <field name=\"v4_int32\" type=\"v4i32\"/>\n"
                                  "    <field name=\"v2_int64\" type=\"v2i64\"/>\n"
                                  "    <field name=\"uint128\" type=\"uint128\"/>\n"
                                  "  </union>\n"
                                  "  <flags id=\"i386_mxcsr\" size=\"4\">\n"
                                  "    <field name=\"IE\" start=\"0\" end=\"0\"/>\n"
                                  "    <field name=\"DE\" start=\"1\" end=\"1\"/>\n"
                                  "    <field name=\"ZE\" start=\"2\" end=\"2\"/>\n"
                                  "    <field name=\"OE\" start=\"3\" end=\"3\"/>\n"
                                  "    <field name=\"UE\" start=\"4\" end=\"4\"/>\n"
                                  "    <field name=\"PE\" start=\"5\" end=\"5\"/>\n"
                                  "    <field name=\"DAZ\" start=\"6\" end=\"6\"/>\n"
                                  "    <field name=\"IM\" start=\"7\" end=\"7\"/>\n"
                                  "    <field name=\"DM\" start=\"8\" end=\"8\"/>\n"
                                  "    <field name=\"ZM\" start=\"9\" end=\"9\"/>\n"
                                  "    <field name=\"OM\" start=\"10\" end=\"10\"/>\n"
                                  "    <field name=\"UM\" start=\"11\" end=\"11\"/>\n"
                                  "    <field name=\"PM\" start=\"12\" end=\"12\"/>\n"
                                  "    <field name=\"FZ\" start=\"15\" end=\"15\"/>\n"
                                  "  </flags>\n"
                                  "\n"
                                  "  <reg name=\"xmm0\" bitsize=\"128\" type=\"vec128\" regnum=\"40\"/>\n"
                                  "  <reg name=\"xmm1\" bitsize=\"128\" type=\"vec128\"/>\n"
                                  "  <reg name=\"xmm2\" bitsize=\"128\" type=\"vec128\"/>\n"
                                  "  <reg name=\"xmm3\" bitsize=\"128\" type=\"vec128\"/>\n"
                                  "  <reg name=\"xmm4\" bitsize=\"128\" type=\"vec128\"/>\n"
                                  "  <reg name=\"xmm5\" bitsize=\"128\" type=\"vec128\"/>\n"
                                  "  <reg name=\"xmm6\" bitsize=\"128\" type=\"vec128\"/>\n"
                                  "  <reg name=\"xmm7\" bitsize=\"128\" type=\"vec128\"/>\n"
                                  "  <reg name=\"xmm8\" bitsize=\"128\" type=\"vec128\"/>\n"
                                  "  <reg name=\"xmm9\" bitsize=\"128\" type=\"vec128\"/>\n"
                                  "  <reg name=\"xmm10\" bitsize=\"128\" type=\"vec128\"/>\n"
                                  "  <reg name=\"xmm11\" bitsize=\"128\" type=\"vec128\"/>\n"
                                  "  <reg name=\"xmm12\" bitsize=\"128\" type=\"vec128\"/>\n"
                                  "  <reg name=\"xmm13\" bitsize=\"128\" type=\"vec128\"/>\n"
                                  "  <reg name=\"xmm14\" bitsize=\"128\" type=\"vec128\"/>\n"
                                  "  <reg name=\"xmm15\" bitsize=\"128\" type=\"vec128\"/>\n"
                                  "\n"
                                  "  <reg name=\"mxcsr\" bitsize=\"32\" type=\"i386_mxcsr\" group=\"vector\"/>\n"
                                  "</feature>\n";

static VOID GetReg(unsigned, THREADID, CONTEXT*, VOID*, VOID*);
static VOID SetReg(unsigned, THREADID, CONTEXT*, const VOID*, VOID*);
static USIZE GetDoc(const std::string&, USIZE, void*, VOID*);
static void PrintEmulated();

int main(int argc, char* argv[])
{
    PIN_Init(argc, argv);

    PIN_AddDebuggerRegisterEmulator(NumRegs, RegDescs, GetReg, SetReg, GetDoc, 0);

    PIN_StartProgram();
    return 0;
}

/*
 * Pin calls this function to get the value of an emulated register.
 */
static VOID GetReg(unsigned toolRegId, THREADID tid, CONTEXT* ctxt, VOID* data, VOID*)
{
    PrintEmulated();

    switch (toolRegId)
    {
        case EMULATED_REG_RCX:
        {
            ADDRINT* val = static_cast< ADDRINT* >(data);
            *val         = PIN_GetContextReg(ctxt, REG_RCX);
            break;
        }
        case EMULATED_REG_RSP:
        {
            ADDRINT* val = static_cast< ADDRINT* >(data);
            *val         = PIN_GetContextReg(ctxt, REG_RSP);
            break;
        }
        case EMULATED_REG_FPSW:
        {
            UINT32* val = static_cast< UINT32* >(data);
            *val        = static_cast< UINT32 >(PIN_GetContextReg(ctxt, REG_FPSW));
            break;
        }
        case EMULATED_REG_ST0:
        {
            FPSTATE fpstate;
            PIN_GetContextFPState(ctxt, &fpstate);
            std::memcpy(data, &fpstate.fxsave_legacy._sts[0], 10);
            break;
        }
        case EMULATED_REG_XMM0:
        {
            FPSTATE fpstate;
            PIN_GetContextFPState(ctxt, &fpstate);
            std::memcpy(data, &fpstate.fxsave_legacy._xmms[0], 16);
            break;
        }
        default:
        {
            ASSERTX(0);
            break;
        }
    }
}

/*
 * Pin calls this function to set the value of an emulated register.
 */
static VOID SetReg(unsigned toolRegId, THREADID tid, CONTEXT* ctxt, const VOID* data, VOID*)
{
    PrintEmulated();

    switch (toolRegId)
    {
        case EMULATED_REG_RCX:
        {
            const ADDRINT* val = static_cast< const ADDRINT* >(data);
            PIN_SetContextReg(ctxt, REG_RCX, *val);
            break;
        }
        case EMULATED_REG_RSP:
        {
            const ADDRINT* val = static_cast< const ADDRINT* >(data);
            PIN_SetContextReg(ctxt, REG_RSP, *val);
            break;
        }
        case EMULATED_REG_FPSW:
        {
            const UINT32* val = static_cast< const UINT32* >(data);
            PIN_SetContextReg(ctxt, REG_FPSW, static_cast< ADDRINT >(*val));
            break;
        }
        case EMULATED_REG_ST0:
        {
            FPSTATE fpstate;
            PIN_GetContextFPState(ctxt, &fpstate);
            std::memcpy(&fpstate.fxsave_legacy._sts[0], data, 10);
            PIN_SetContextFPState(ctxt, &fpstate);
            break;
        }
        case EMULATED_REG_XMM0:
        {
            FPSTATE fpstate;
            PIN_GetContextFPState(ctxt, &fpstate);
            std::memcpy(&fpstate.fxsave_legacy._xmms[0], data, 16);
            PIN_SetContextFPState(ctxt, &fpstate);
            break;
        }
        default:
        {
            ASSERTX(0);
            break;
        }
    }
}

/*
 * Pin calls this function to get the content of the XML documents above.
 */
static USIZE GetDoc(const std::string& name, USIZE size, void* buf, VOID*)
{
    const char* content = 0;
    size_t docSize      = 0;

    if (name == "target.xml")
    {
        content = FeatureDocumentTop;
        docSize = sizeof(FeatureDocumentTop) - 1;
    }
    else if (name == "64bit-core.xml")
    {
        content = FeatureDocumentCore;
        docSize = sizeof(FeatureDocumentCore) - 1;
    }
    else if (name == "64bit-linux.xml")
    {
        content = FeatureDocumentLinux;
        docSize = sizeof(FeatureDocumentLinux) - 1;
    }
    else if (name == "64bit-sse.xml")
    {
        content = FeatureDocumentSSE;
        docSize = sizeof(FeatureDocumentSSE) - 1;
    }
    else
    {
        return 0; // Unknown document
    }

    if (docSize <= size) std::memcpy(buf, content, docSize);
    return docSize;
}

static void PrintEmulated()
{
    static bool printed = false;

    if (!printed)
    {
        std::cerr << "Tool is emulating registers" << std::endl;
        printed = true;
    }
}
