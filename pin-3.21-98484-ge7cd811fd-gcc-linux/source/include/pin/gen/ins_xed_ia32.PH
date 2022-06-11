/*
 * Copyright (C) 2020-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#ifdef PIN_G_INS_XED_IA32_PH
#error duplicate inclusion of ins_xed_ia32
#else
#define PIN_G_INS_XED_IA32_PH
/*! @file
  Intel(R) X86 Encoder Decoder low-level support routines.

  This supports the other core_ia32 files and is the lowest level interface
  to Intel(R) XED encoding/decoding.
*/
/*! @ingroup INS_INSPECTION
  @return This provides the pointer to the decoded Intel(R) X86 Encoder Decoder instruction.
*/
extern xed_decoded_inst_t* INS_XedDec(INS ins);

/*! @ingroup INS_INSPECTION
  Convert a Intel(R) X86 Encoder Decoder xed_reg_enum_t register to a Pin REG type.
  @returns a valid pin register, but never an invalid one. Asserts if there is no exact map.
 */
extern REG INS_XedExactMapToPinReg(xed_reg_enum_t xed_reg);

/*! @ingroup INS_INSPECTION
  Convert a Intel(R) X86 Encoder Decoder xed_reg_enum_t register to a Pin REG type.
  @returns a valid pin register, but never an invalid one. Asserts if there is no exact map.
  @note DEPRECATED: Use  INS_XedExactMapToPinReg(xed_reg_enum_t xed_reg) instead
 */
extern PIN_DEPRECATED_API REG INS_XedExactMapToPinReg(unsigned int r);

/*! @ingroup INS_INSPECTION
  Convert  Pin REG type (application register) to an Intel(R) X86 Encoder Decoder xed_reg_enum_t
  register.
  @returns xed reg mapping to pin reg. if no map returns XED_REG_INVALID
 */
extern xed_reg_enum_t INS_XedExactMapFromPinReg(REG pin_reg);

#endif // PIN_G_INS_XED_IA32_PH
