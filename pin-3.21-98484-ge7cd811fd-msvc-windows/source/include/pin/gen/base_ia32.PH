/*
 * Copyright (C) 2004-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#ifdef PIN_G_BASE_IA32_PH
#error duplicate inclusion of base_ia32
#else
#define PIN_G_BASE_IA32_PH
/*! @file
 *  This file contains basic abstractions for IA32
 */
/*! @ingroup INS_INSPECTION
    @brief Predicate abstraction for IA-32 and Intel(R) 64 instructions
 */
typedef enum
{
    // ***
    // N.B. Do not change the order here. We have a static table which is indexed by these
    // values (in ins_api_xed_ia32.cpp).
    // ***
    PREDICATE_ALWAYS_TRUE, ///< Default predication on IA-32 and Intel(R) 64 architectures
    PREDICATE_INVALID,
    // Predicates for CMOV instructions, names chosen to align with XED names for the CMOVs
    PREDICATE_BELOW,              ///< Below          (CF==1)
    PREDICATE_BELOW_OR_EQUAL,     ///< Below or Equal (CF==1 or  ZF==1)
    PREDICATE_LESS,               ///< Less           (SF!=OF)
    PREDICATE_LESS_OR_EQUAL,      ///< Less or Equal  (ZF==1 or SF!=OF)
    PREDICATE_NOT_BELOW,          ///< Not Below      (CF==0)
    PREDICATE_NOT_BELOW_OR_EQUAL, ///< Above      (CF==0 and ZF==0)
    PREDICATE_NOT_LESS,           ///< Greater or Equal (SF==OF)
    PREDICATE_NOT_LESS_OR_EQUAL,  ///< Greater        (ZF==0 and SF==OF)
    PREDICATE_NOT_OVERFLOW,       ///< Not Overflow   (OF==0)
    PREDICATE_NOT_PARITY,         ///< Not Parity     (PF==0)
    PREDICATE_NOT_SIGN,           ///< Not Sign       (SF==0)
    PREDICATE_NOT_ZERO,           ///< Not Zero       (ZF==0)
    PREDICATE_OVERFLOW,           ///< Overflow       (OF==1)
    PREDICATE_PARITY,             ///< Parity         (PF==1)
    PREDICATE_SIGN,               ///< Sign           (SF==1)
    PREDICATE_ZERO,               ///< Zero           (ZF==1)
    // Predicates for REP string ops
    PREDICATE_CX_NON_ZERO,        ///< CX  != 0
    PREDICATE_ECX_NON_ZERO,       ///< ECX != 0
    PREDICATE_RCX_NON_ZERO,       ///< RCX != 0
    PREDICATE_SAVED_GCX_NON_ZERO, ///< (Internal) Use the saved value of one of the previous three tests
    PREDICATE_LAST
} PREDICATE;

#endif // PIN_G_BASE_IA32_PH
