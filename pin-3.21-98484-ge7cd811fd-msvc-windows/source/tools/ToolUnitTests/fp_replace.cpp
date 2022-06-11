/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

// Encapsulating all floating point operations within the Pintool
// replacement function inside "fxsave; emms" and "fxrstor" causes
// a seg fault.
//
// Robert reproduced this problem on vs-lin64-3. The problem is
// that the stack pointer is not properly aligned in the replacement
// routine and you get a segv trying to save an xmm to memory.
//
// (gdb) x/i$pc
// 0x2a96429676:	movaps XMMWORD PTR [rax-0x7f],xmm0
// (gdb) p/x $rax-0x7f
// $2 = 0x2a9816b8b8
//
// At the entry point, it should be 8 mod 16, but it is 0 mod 16.

#include <iostream>
#include <stdio.h>
#include "pin.H"
using std::cerr;
using std::endl;

void my_print(int x)
{
#if defined(TARGET_IA32) || defined(TARGET_IA32E)
    static char buffer[2048];
    static char* aligned_bufp = reinterpret_cast< char* >(((reinterpret_cast< ADDRINT >(buffer) + 16) >> 4) << 4);

#if defined(PIN_GNU_COMPATIBLE)
    cerr << "Pin GNU compatible" << endl;
    asm volatile("fxsave %0\n\t"
                 "emms"
                 : "=m"(*aligned_bufp));
#else
    __asm {
            push eax
            
            mov  eax, aligned_bufp
            fxsave [eax]
            
            pop eax
          }
#endif
#endif

    cerr << "my_print: " << x << endl;
    double y = x * 0.33445;
    cerr << "Done initializing y" << endl;
    cerr << y << endl;
    cerr << "Done with my_print" << endl;

#if defined(TARGET_IA32) || defined(TARGET_IA32E)
#if defined(PIN_GNU_COMPATIBLE)
    asm volatile("fxrstor %0" ::"m"(*aligned_bufp));
#else
    __asm {
            push eax
            
            mov  eax, aligned_bufp
            fxrstor [eax]
            
            pop eax
          }
#endif
#endif
}

VOID ImageLoad(IMG img, VOID* v)
{
    RTN rtn;

    rtn = RTN_FindByName(img, "print");
    if (RTN_Valid(rtn))
    {
        PROTO proto = PROTO_Allocate(PIN_PARG(void), CALLINGSTD_DEFAULT, "print", PIN_PARG(int), PIN_PARG_END());
        RTN_ReplaceSignature(rtn, (AFUNPTR)my_print, IARG_PROTOTYPE, proto, IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_END);
    }
}

int main(int argc, char* argv[])
{
    PIN_InitSymbols();
    PIN_Init(argc, argv);

    IMG_AddInstrumentFunction(ImageLoad, 0);

    PIN_StartProgram();
    return 0;
}
