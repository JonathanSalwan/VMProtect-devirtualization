/*
 * Copyright (C) 2006-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include "pin.H"
#include <fstream>

#define READ 1
#define WRITE 2
FILE* outfile;

extern "C" unsigned int xmmInitVals[];
unsigned int xmmInitVals[64];
extern "C" int SetXmmScratchesFun();

/*
static void change_xmm_scratch_regs(unsigned int * xmm_reg)
{
#ifdef TARGET_IA32
	// on ia32 all xmm regs are scratch
#if defined(PIN_GNU_COMPATIBLE)
    asm volatile("movdqu %0, %%xmm0" :  : "m" (xmm_reg) : "%xmm0"  );
    asm volatile("movdqu %0, %%xmm1" :  : "m" (xmm_reg) : "%xmm1"  );
    asm volatile("movdqu %0, %%xmm2" :  : "m" (xmm_reg) : "%xmm2"  );
    asm volatile("movdqu %0, %%xmm3" :  : "m" (xmm_reg) : "%xmm3"  );
	asm volatile("movdqu %0, %%xmm4" :  : "m" (xmm_reg) : "%xmm4"  );
    asm volatile("movdqu %0, %%xmm5" :  : "m" (xmm_reg) : "%xmm5"  );
	asm volatile("movdqu %0, %%xmm6" :  : "m" (xmm_reg) : "%xmm6"  );
	asm volatile("movdqu %0, %%xmm7" :  : "m" (xmm_reg) : "%xmm7"  );

#else
    __asm 
    {
        movdqu xmm0, xmm_reg;
        movdqu xmm1, xmm_reg;
        movdqu xmm2, xmm_reg;
        movdqu xmm3, xmm_reg;
		movdqu xmm4, xmm_reg;
		movdqu xmm5, xmm_reg;
		movdqu xmm6, xmm_reg;
		movdqu xmm7, xmm_reg;
    }
#endif
#else
#if defined(PIN_GNU_COMPATIBLE)
	// on linux Intel64 all the xmm registers are scratch
    asm volatile("movdqu %0, %%xmm0" :  : "m" (xmm_reg) : "%xmm0"  );
    asm volatile("movdqu %0, %%xmm1" :  : "m" (xmm_reg) : "%xmm1"  );
    asm volatile("movdqu %0, %%xmm2" :  : "m" (xmm_reg) : "%xmm2"  );
    asm volatile("movdqu %0, %%xmm3" :  : "m" (xmm_reg) : "%xmm3"  );
	asm volatile("movdqu %0, %%xmm4" :  : "m" (xmm_reg) : "%xmm4"  );
    asm volatile("movdqu %0, %%xmm5" :  : "m" (xmm_reg) : "%xmm5"  );
	asm volatile("movdqu %0, %%xmm6" :  : "m" (xmm_reg) : "%xmm6"  );
	asm volatile("movdqu %0, %%xmm7" :  : "m" (xmm_reg) : "%xmm7"  );
	asm volatile("movdqu %0, %%xmm8" :  : "m" (xmm_reg) : "%xmm8"  );
	asm volatile("movdqu %0, %%xmm9" :  : "m" (xmm_reg) : "%xmm9"  );
	asm volatile("movdqu %0, %%xmm10" :  : "m" (xmm_reg) : "%xmm10"  );
	asm volatile("movdqu %0, %%xmm10" :  : "m" (xmm_reg) : "%xmm11"  );
	asm volatile("movdqu %0, %%xmm12" :  : "m" (xmm_reg) : "%xmm12"  );
	asm volatile("movdqu %0, %%xmm13" :  : "m" (xmm_reg) : "%xmm13"  );
	asm volatile("movdqu %0, %%xmm14" :  : "m" (xmm_reg) : "%xmm14"  );
	asm volatile("movdqu %0, %%xmm15" :  : "m" (xmm_reg) : "%xmm15"  );
#else
	// on Win64 xmm0 thru xmm5 are scratch
	SetXmmScratchesFun();
#endif
#endif
}
*/

VOID TestXmm(ADDRINT ip, UINT8* reg_val, UINT8* reg_val1, unsigned int operation, unsigned int xmm_reg_index)
{
    // unsigned int dummy_values[4] = {1, 2, 3, 4};
    // change_xmm_scratch_regs(dummy_values);

    fprintf(outfile, "ip %p xmm%d = ", (void*)(ip), xmm_reg_index);
    for (int i = 0; i < 16; i++)
    {
        fprintf(outfile, "0x%x ", reg_val[i]);
    }
    fprintf(outfile, "operation %s\n", (operation == READ ? "read" : "write"));
    fprintf(outfile, "    xmm1 = ");
    for (int i = 0; i < 16; i++)
    {
        fprintf(outfile, "0x%x ", reg_val1[i]);
    }
    fprintf(outfile, "\n");
}

VOID Instruction(INS ins, VOID* v)
{
    for (REG reg = REG_XMM_BASE; reg <= REG_XMM_LAST; reg = static_cast< REG >((static_cast< INT32 >(reg) + 1)))
    {
        if (INS_RegRContain(ins, reg))
        {
            INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(TestXmm), IARG_INST_PTR, IARG_REG_REFERENCE, reg, IARG_REG_REFERENCE,
                           REG_XMM1, IARG_ADDRINT, READ, IARG_ADDRINT, (reg - REG_XMM_BASE), IARG_END);
            fprintf(outfile, "Instrumented read  on ins %p %s\n", (void*)(INS_Address(ins)), INS_Disassemble(ins).c_str());
            fflush(outfile);
        }

        if (INS_RegWContain(ins, reg))
        {
            INS_InsertCall(ins, IPOINT_AFTER, AFUNPTR(TestXmm), IARG_INST_PTR, IARG_REG_REFERENCE, reg, IARG_REG_REFERENCE,
                           REG_XMM1, IARG_ADDRINT, WRITE, IARG_ADDRINT, (reg - REG_XMM_BASE), IARG_END);
            fprintf(outfile, "Instrumented write on ins %p %s\n", (void*)(INS_Address(ins)), INS_Disassemble(ins).c_str());
            fflush(outfile);
        }
    }
}

int main(int argc, char* argv[])
{
    // initialize memory area used to set values in xmm regss
    for (int i = 0; i < 64; i++)
    {
        xmmInitVals[i] = 0xdeadbeef;
    }

    PIN_Init(argc, argv);

    outfile = fopen("xmmt.out", "w");
    INS_AddInstrumentFunction(Instruction, 0);

    // Never returns
    PIN_StartProgram();

    return 0;
}
