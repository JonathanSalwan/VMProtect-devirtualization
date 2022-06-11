/*
 * Copyright (C) 2010-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 *  Check how many instructions use EAX when it contains 0, using xed APIs.
 *  The purpose of this test is to check INS_XedExactMapToPinReg and INS_XedExactMapFromPinReg
 *  APIs.
 */

#include <iostream>
#include <assert.h>
#include "pin.H"

extern "C"
{
#include "xed-interface.h"
}

using std::cout;
using std::endl;
using std::string;

UINT32 eaxIsZeroCount = 0;

VOID CountIfZero(INT32 eaxVal)
{
    if (!eaxVal) eaxIsZeroCount++;
}

VOID InstrumentOperand(INS ins, xed_decoded_inst_t const* const xedd, xed_inst_t const* const xedi, unsigned int operand_index)
{
    const xed_operand_t* operand          = xed_inst_operand(xedi, operand_index);
    const xed_operand_enum_t operand_name = xed_operand_name(operand);

    if (xed_operand_is_register(operand_name))
    {
        xed_reg_enum_t xedreg          = xed_decoded_inst_get_reg(xedd, operand_name);
        xed_reg_class_enum_t reg_class = xed_reg_class(xedreg);
        if (reg_class == XED_REG_CLASS_GPR)
        {
            xed_reg_enum_t fullXedreg = xed_get_largest_enclosing_register(xedreg);
            xed_reg_enum_t fullXedregTarget;
#if defined(TARGET_IA32)
            fullXedregTarget = static_cast< xed_reg_enum_t >(fullXedreg - XED_REG_GPR64_FIRST + XED_REG_GPR32_FIRST);
#else
            fullXedregTarget = fullXedreg;
#endif
            REG pinreg = INS_XedExactMapToPinReg(fullXedregTarget);
            //check INS_XedExactMapFromPinReg
            xed_reg_enum_t xed_reg2 = INS_XedExactMapFromPinReg(pinreg);
            ASSERTX(xed_reg2 == fullXedregTarget);

            if (pinreg == REG_EAX) INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)CountIfZero, IARG_REG_VALUE, REG_EAX, IARG_END);
        }
    }
}

VOID Ins(INS ins, VOID* v)
{
    xed_decoded_inst_t const* const xedd = INS_XedDec(ins);
    const xed_inst_t* xedi               = xed_decoded_inst_inst(xedd);
    const unsigned int operand_count     = xed_inst_noperands(xedi);
    for (unsigned int i = 0; i < operand_count; i++)
        InstrumentOperand(ins, xedd, xedi, i);
}

VOID Fini(INT32 code, VOID* v) { cout << "eaxIsZeroCount = " << eaxIsZeroCount << endl; }

int main(INT32 argc, CHAR** argv)
{
    PIN_InitSymbols();
    PIN_Init(argc, argv);

    INS_AddInstrumentFunction(Ins, 0);
    PIN_AddFiniFunction(Fini, 0);

    // Never returns
    PIN_StartProgram();

    return 0;
}
