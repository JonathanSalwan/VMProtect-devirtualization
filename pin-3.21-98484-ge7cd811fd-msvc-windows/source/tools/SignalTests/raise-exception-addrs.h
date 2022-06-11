/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#ifndef RAISE_EXCEPTION_ADDRS_H
#define RAISE_EXCEPTION_ADDRS_H

/*
 * Labels for various instructions that raise exceptions.
 */
typedef struct
{
    char* _unmappedRead;
    char* _unmappedReadAddr;
    char* _unmappedWrite;
    char* _unmappedWriteAddr;
    char* _inaccessibleRead;
    char* _inaccessibleReadAddr;
    char* _inaccessibleWrite;
    char* _inaccessibleWriteAddr;
    char* _misalignedRead;
    char* _misalignedWrite;
    char* _illegalInstruction;
    char* _privilegedInstruction;
    char* _integerDivideByZero;
    char* _integerOverflowTrap;
    char* _boundTrap;
    char* _x87DivideByZero;
    char* _x87Overflow;
    char* _x87Underflow;
    char* _x87Precision;
    char* _x87InvalidOperation;
    char* _x87DenormalizedOperand;
    char* _x87StackUnderflow;
    char* _x87StackOverflow;
    char* _x87MultipleExceptions;
    char* _simdDivideByZero;
    char* _simdOverflow;
    char* _simdUnderflow;
    char* _simdPrecision;
    char* _simdInvalidOperation;
    char* _simdDenormalizedOperand;
    char* _simdMultipleExceptions;
    char* _breakpointTrap;
} RAISE_EXCEPTION_ADDRS;

#endif
