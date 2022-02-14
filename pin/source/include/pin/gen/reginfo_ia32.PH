/*
 * Copyright (C) 2013-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#ifdef PIN_G_REGINFO_IA32_PH
#error duplicate inclusion of reginfo_ia32
#else
#define PIN_G_REGINFO_IA32_PH
/*! @file
  This file contains REG primitives
 */
/*! @ingroup REG
  register widths
*/
enum REGWIDTH
{
    REGWIDTH_8  = 0,
    REGWIDTH_16 = 1,
    REGWIDTH_32 = 2,
    REGWIDTH_64 = 3,
    REGWIDTH_80,
    REGWIDTH_128,
    REGWIDTH_256,
    REGWIDTH_512,
    REGWIDTH_TILE,
    REGWIDTH_REG_X87,
    REGWIDTH_INVALID,
#if defined(TARGET_IA32)
    REGWIDTH_NATIVE = REGWIDTH_32
#else
    REGWIDTH_NATIVE = REGWIDTH_64
#endif
};

/*! @ingroup REG
 * Enumeration of register classes. Each register belongs to one and only one class.
 */
enum REG_CLASS
{
    REG_CLASS_NONE = 0,
    REG_CLASS_PSEUDO,
    REG_CLASS_GR,
    REG_CLASS_GRU8,
    REG_CLASS_GRL8,
    REG_CLASS_GRH16,
    REG_CLASS_GRH32,
    REG_CLASS_SEG,
    REG_CLASS_MM,
    REG_CLASS_XMM,
    REG_CLASS_YMM,
    REG_CLASS_ZMM,
    REG_CLASS_K,
    REG_CLASS_TMM,
    REG_CLASS_FPST,
    REG_CLASS_ST,
    REG_CLASS_CR,
    REG_CLASS_DR,
    REG_CLASS_TR,
    REG_CLASS_FLAGS,
    REG_CLASS_FLAGS16,
    REG_CLASS_FLAGS32,
    REG_CLASS_STATUS_FLAGS,
    REG_CLASS_DFLAG,
    REG_CLASS_X87,
    REG_CLASS_MXCSR,
    REG_CLASS_MXCSRMASK,
    REG_CLASS_IP,
    REG_CLASS_IP16,
    REG_CLASS_IP32,
    REG_CLASS_ARCH, // Architectural register, that does not belong to any specific class
    REG_CLASS_PIN_GR,
    REG_CLASS_PIN_GRU8,
    REG_CLASS_PIN_GRL8,
    REG_CLASS_PIN_GRH16,
    REG_CLASS_PIN_GRH32,
    REG_CLASS_PIN_XMM,
    REG_CLASS_PIN_YMM,
    REG_CLASS_PIN_ZMM,
    REG_CLASS_PIN_K,
    REG_CLASS_PIN_X87,
    REG_CLASS_PIN_MXCSR,
    REG_CLASS_PIN_FLAGS,
    REG_CLASS_PIN_STATUS_FLAGS,
    REG_CLASS_PIN_DFLAG
};

/*! @ingroup REG
 * Bit flag that represents a REG_CLASS value.
 */
typedef UINT64 REG_CLASS_BITS;

#define _REGCBIT(regClass) (REG_CLASS_BITS(1) << (regClass))
/*! @ingroup REG
 * Mask of REG_CLASS_BITS values for all application registers.
 */
const REG_CLASS_BITS REGCBIT_APP_ALL =
    _REGCBIT(REG_CLASS_PSEUDO) | (_REGCBIT(REG_CLASS_GR)) | (_REGCBIT(REG_CLASS_GRU8)) | (_REGCBIT(REG_CLASS_GRL8)) |
    (_REGCBIT(REG_CLASS_GRH16)) | (_REGCBIT(REG_CLASS_GRH32)) | (_REGCBIT(REG_CLASS_SEG)) | (_REGCBIT(REG_CLASS_MM)) |
    (_REGCBIT(REG_CLASS_XMM)) | (_REGCBIT(REG_CLASS_YMM)) | (_REGCBIT(REG_CLASS_ZMM)) | (_REGCBIT(REG_CLASS_K)) |
    (_REGCBIT(REG_CLASS_TMM)) | (_REGCBIT(REG_CLASS_FPST)) | (_REGCBIT(REG_CLASS_ST)) | (_REGCBIT(REG_CLASS_CR)) |
    (_REGCBIT(REG_CLASS_DR)) | (_REGCBIT(REG_CLASS_TR)) | (_REGCBIT(REG_CLASS_FLAGS)) | (_REGCBIT(REG_CLASS_FLAGS16)) |
    (_REGCBIT(REG_CLASS_FLAGS32)) | (_REGCBIT(REG_CLASS_STATUS_FLAGS)) | (_REGCBIT(REG_CLASS_DFLAG)) | (_REGCBIT(REG_CLASS_X87)) |
    (_REGCBIT(REG_CLASS_MXCSR)) | (_REGCBIT(REG_CLASS_MXCSRMASK)) | (_REGCBIT(REG_CLASS_IP)) | (_REGCBIT(REG_CLASS_IP16)) |
    (_REGCBIT(REG_CLASS_IP32)) | (_REGCBIT(REG_CLASS_ARCH));

/*! @ingroup REG           
 * Mask of REG_CLASS_BITS values for all Pin registers.
 */
const REG_CLASS_BITS REGCBIT_PIN_ALL =

    (_REGCBIT(REG_CLASS_PIN_GR)) | (_REGCBIT(REG_CLASS_PIN_GRU8)) | (_REGCBIT(REG_CLASS_PIN_GRL8)) |
    (_REGCBIT(REG_CLASS_PIN_GRH16)) | (_REGCBIT(REG_CLASS_PIN_GRH32)) | (_REGCBIT(REG_CLASS_PIN_XMM)) |
    (_REGCBIT(REG_CLASS_PIN_YMM)) | (_REGCBIT(REG_CLASS_PIN_ZMM)) | (_REGCBIT(REG_CLASS_PIN_K)) | (_REGCBIT(REG_CLASS_PIN_X87)) |
    (_REGCBIT(REG_CLASS_PIN_MXCSR)) | (_REGCBIT(REG_CLASS_PIN_FLAGS)) | (_REGCBIT(REG_CLASS_PIN_STATUS_FLAGS)) |
    (_REGCBIT(REG_CLASS_PIN_DFLAG));

/*! @ingroup REG
 * Mask of REG_CLASS_BITS values for all valid registers.xx
 */
const REG_CLASS_BITS REGCBIT_ALL_REGS = REGCBIT_APP_ALL | REGCBIT_PIN_ALL;

/*! @ingroup REG
 * Mask of REG_CLASS_BITS values for all application flag registers.
 */
const REG_CLASS_BITS REGCBIT_APP_FLAGS =

    (_REGCBIT(REG_CLASS_FLAGS)) | (_REGCBIT(REG_CLASS_STATUS_FLAGS)) | (_REGCBIT(REG_CLASS_DFLAG));

/*! @ingroup REG
 * Mask of REG_CLASS_BITS values for all Pin flag registers.
 */
const REG_CLASS_BITS REGCBIT_PIN_FLAGS =
    (_REGCBIT(REG_CLASS_PIN_FLAGS)) | (_REGCBIT(REG_CLASS_PIN_STATUS_FLAGS)) | (_REGCBIT(REG_CLASS_PIN_DFLAG));

/*! @ingroup REG
 * Mask of REG_CLASS_BITS values for partial registers (excluding XMM, even if AVX is present).
 */
const REG_CLASS_BITS REGCBIT_PARTIAL = (_REGCBIT(REG_CLASS_GRU8)) | (_REGCBIT(REG_CLASS_GRL8)) | (_REGCBIT(REG_CLASS_GRH16)) |
                                       (_REGCBIT(REG_CLASS_GRH32)) | (_REGCBIT(REG_CLASS_FLAGS16)) |
                                       (_REGCBIT(REG_CLASS_FLAGS32)) | (_REGCBIT(REG_CLASS_IP16)) | (_REGCBIT(REG_CLASS_IP32)) |
                                       (_REGCBIT(REG_CLASS_PIN_GRU8)) | (_REGCBIT(REG_CLASS_PIN_GRL8)) |
                                       (_REGCBIT(REG_CLASS_PIN_GRH16)) | (_REGCBIT(REG_CLASS_PIN_GRH32));

/*! @ingroup REG
 * Additional classification of register.
 */
enum REG_SUBCLASS
{
    REG_SUBCLASS_NONE = 0,
    REG_SUBCLASS_REX,
    REG_SUBCLASS_FULL_STACKPTR,
    REG_SUBCLASS_PIN_FULL_STACKPTR,
    REG_SUBCLASS_PIN_TMP,
    REG_SUBCLASS_PIN_INST_GR,
    REG_SUBCLASS_PIN_INST_GR_H32,
    REG_SUBCLASS_PIN_INST_BUF,
    REG_SUBCLASS_PIN_INST_COND
};

/*! @ingroup REG
 * Combination of REG_SUBCLASS_BITS flags of all instrumentation registers.
 */
const REG_SUBCLASS_BITS REGSBIT_PIN_INST_ALL = (_REGCBIT(REG_SUBCLASS_PIN_INST_GR)) | (_REGCBIT(REG_SUBCLASS_PIN_INST_GR_H32)) |
                                               (_REGCBIT(REG_SUBCLASS_PIN_INST_BUF)) | (_REGCBIT(REG_SUBCLASS_PIN_INST_COND));

/*! @ingroup REG
 * Combination of REG_SUBCLASS_BITS flags of all instrumentation scratch registers.
 */
const REG_SUBCLASS_BITS REGSBIT_PIN_SCRATCH_ALL = (_REGCBIT(REG_SUBCLASS_PIN_INST_GR)) | (_REGCBIT(REG_SUBCLASS_PIN_INST_BUF));

/*! @ingroup REG
 * Combination of REG_SUBCLASS_BITS flags of stack registers (both app and pin).
 */
const REG_SUBCLASS_BITS REGSBIT_STACKPTR_ALL =
    (_REGCBIT(REG_SUBCLASS_FULL_STACKPTR)) | (_REGCBIT(REG_SUBCLASS_PIN_FULL_STACKPTR));

/*! @ingroup REG
 * Classification of registers under register allocation.
 * Registers of the same allocation type can replace each other during register re-allocation.
 */
enum REG_ALLOC_TYPE
{
    REG_ALLOC_NONE = 0, // Non-allocatable register
    REG_ALLOC_PART,     // Register should be assigned to the corresponding part of the full register
    REG_ALLOC_ANY_GR,   // Register can be assigned to any GR
    REG_ALLOC_IDENT,    // Register should be assigned to the corresponding machine register

    REG_ALLOC_CR        = REG_ALLOC_IDENT,
    REG_ALLOC_DR        = REG_ALLOC_IDENT,
    REG_ALLOC_TR        = REG_ALLOC_IDENT,
    REG_ALLOC_ST        = REG_ALLOC_IDENT,
    REG_ALLOC_MM        = REG_ALLOC_IDENT,
    REG_ALLOC_XMM       = REG_ALLOC_IDENT,
    REG_ALLOC_YMM       = REG_ALLOC_IDENT,
    REG_ALLOC_ZMM       = REG_ALLOC_IDENT,
    REG_ALLOC_K         = REG_ALLOC_IDENT,
    REG_ALLOC_TMM       = REG_ALLOC_IDENT, // I don't see why we need specific enum instead of just using REG_ALLOC_IDENT(CHECK)
    REG_ALLOC_SEG       = REG_ALLOC_IDENT,
    REG_ALLOC_STACK_PTR = REG_ALLOC_IDENT,
    REG_ALLOC_X87       = REG_ALLOC_IDENT,
    REG_ALLOC_FLAGS     = REG_ALLOC_IDENT,
    REG_ALLOC_STATUS_FLAGS = REG_ALLOC_IDENT,
    REG_ALLOC_DFLAG        = REG_ALLOC_IDENT
};

#endif // PIN_G_REGINFO_IA32_PH
