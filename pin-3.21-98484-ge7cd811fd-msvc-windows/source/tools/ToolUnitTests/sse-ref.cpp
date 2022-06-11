/*
 * Copyright (C) 2005-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <iostream>
#include <iomanip>
#include "pin.H"

UINT64 icount = 0;

#if defined(PIN_MS_COMPATIBLE)
extern "C" void mmx_save(char* buf);
extern "C" void mmx_restore(char* buf);
#endif

VOID mmx_arg(PIN_REGISTER* r, UINT32 opnd_indx, UINT32 regno)
{
    char buffer[512 + 16];
    char* aligned_bufp = reinterpret_cast< char* >(((reinterpret_cast< ADDRINT >(buffer) + 16) >> 4) << 4);
#if defined(PIN_GNU_COMPATIBLE)
    asm("fxsave %0" : "=m"(*aligned_bufp));
#else
    mmx_save(aligned_bufp);
#endif

#if defined(DEBUG_SSE_REF)
    cout << "MMX" << regno << " operand_index: " << opnd_indx << " ";
    cout << setw(10) << r->dword[0] << " ";
    cout << setw(10) << r->dword[1] << " ";
    cout << endl;
#endif
    // increment the destination...
    if (opnd_indx == 0)
    {
        r->dword[0]++;
    }
#if defined(PIN_GNU_COMPATIBLE)
    asm volatile("fxrstor %0" ::"m"(*aligned_bufp));
#else
    mmx_restore(aligned_bufp);
#endif

    icount++;
}

VOID xmm_arg(PIN_REGISTER* r, UINT32 opnd_indx, UINT32 regno)
{
#if defined(DEBUG_SSE_REF)
    cout << "XMM" << regno << " operand_index: " << opnd_indx << " ";
    for (unsigned int i = 0; i < MAX_DWORDS_PER_PIN_REG; i++)
    {
        cout << setw(10) << r->dword[i] << " ";
    }
    cout << endl;
#endif
    // increment the destination...
    if (opnd_indx == 0)
    {
        r->dword[0]++;
    }
    icount++;
}

CONTEXT* gctxtx;
VOID TestConstContext(CONTEXT* ctxt) { gctxtx = ctxt; }

int dummy = 2;
VOID BeforeTestConstContext()
{
    // so it won't be inlined
    if (dummy > 0)
    {
        dummy = dummy * dummy * dummy;
    }
    dummy += dummy;
}

VOID Img(IMG img, VOID* v)
{
    if (IMG_IsMainExecutable(img))
    {
        for (SEC sec = IMG_SecHead(img); SEC_Valid(sec); sec = SEC_Next(sec))
        {
            for (RTN rtn = SEC_RtnHead(sec); RTN_Valid(rtn); rtn = RTN_Next(rtn))
            {
                RTN_Open(rtn);
                for (INS ins = RTN_InsHead(rtn); INS_Valid(ins); ins = INS_Next(ins))
                {
                    xed_iclass_enum_t iclass = static_cast< xed_iclass_enum_t >(INS_Opcode(ins));
                    if (iclass == XED_ICLASS_MOVQ || iclass == XED_ICLASS_MOVDQU)
                    {
                        // const unsigned int opnd_count =  INS_OperandCount(ins);
                        unsigned int i = 0;
                        // for(unsigned int i=0; i < opnd_count;i++)
                        {
                            if (INS_OperandIsReg(ins, i))
                            {
                                REG r = INS_OperandReg(ins, i);
                                if (REG_is_mm(r))
                                {
                                    // BeforeTestConstContext causes X87 state to be spilled
                                    // before the instruction that accesses the mmx reg
                                    INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)BeforeTestConstContext, IARG_END);
                                    // TestConstContext requests the X87 context in order to verify
                                    // the Pin can find it after the instruction that accesses the mmx reg
                                    // when the X87 state is spilled
                                    INS_InsertCall(ins, IPOINT_AFTER, (AFUNPTR)TestConstContext, IARG_CONST_CONTEXT, IARG_END);

                                    INS_InsertCall(ins, IPOINT_AFTER, (AFUNPTR)mmx_arg, IARG_REG_REFERENCE, r, IARG_UINT32, i,
                                                   IARG_UINT32, (r - REG_MM_BASE),

                                                   IARG_END);
                                }
                                if (REG_is_xmm(r))
                                {
                                    INS_InsertCall(ins, IPOINT_AFTER, (AFUNPTR)xmm_arg, IARG_REG_REFERENCE, r, IARG_UINT32, i,
                                                   IARG_UINT32, (r - REG_XMM_BASE), IARG_END);
                                }
                            }
                        }
                    }
                }
                RTN_Close(rtn);
            }
        }
    }
}

VOID Fini(INT32 code, VOID* v)
{
    // Don't output icount as part of the reference output
    // because the dynamic loader may also use xmm insts.

    //std::cout << "Count: " << icount << endl;
}

int main(int argc, char* argv[])
{
    PIN_Init(argc, argv);
    PIN_InitSymbols();

    IMG_AddInstrumentFunction(Img, 0);
    PIN_AddFiniFunction(Fini, 0);

    // Never returns
    PIN_StartProgram();

    return 0;
}
