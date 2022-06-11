/*
 * Copyright (C) 2020-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#ifdef PIN_G_INS_IA32_PH
#error duplicate inclusion of ins_ia32
#else
#define PIN_G_INS_IA32_PH
/*! @file
  ELF specific stuff related to INS
*/

/*! @ingroup INS_INSPECTION
  @return Maximum number of read operands
*/
extern UINT32 INS_MaxNumRRegs(INS x);

/*! @ingroup INS_INSPECTION
  @return Maximum number of write operands
*/
extern UINT32 INS_MaxNumWRegs(INS x);

/*! @ingroup INS_INSPECTION
   @return true if ins uses reg as a read operand
 */
extern BOOL INS_RegRContain(const INS ins, const REG reg);

/*! @ingroup INS_INSPECTION
   @return true if ins uses reg as a write operand
 */
extern BOOL INS_RegWContain(const INS ins, const REG reg);

/*! @ingroup INS_INSPECTION
   @return true if ins uses reg as a read operand
 */
extern BOOL INS_FullRegRContain(const INS ins, const REG reg);

/*! @ingroup INS_INSPECTION
   @return true if ins uses reg as a write operand
 */
extern BOOL INS_FullRegWContain(const INS ins, const REG reg);

/*! @ingroup INS_INSPECTION
  Access to the stack simply means that the instruction accesses memory relative
  to the stack pointer (ESP or RSP), or the frame pointer (EBP or RBP). In code compiled
  without a frame pointer (where EBP/RBP is used as a general register), this may give
  a misleading result.

  @return TRUE if ins is a read from the stack
*/
extern BOOL INS_IsStackRead(const INS ins);

/*! @ingroup INS_INSPECTION
  Detection of stack accesses is done in the same way as for @ref INS_IsStackRead, so the
  same caveats apply here too.

  @return TRUE if ins is a write to the stack
*/
extern BOOL INS_IsStackWrite(const INS ins);

/*! @ingroup INS_INSPECTION
  Is an IP-relative read
*/
extern BOOL INS_IsIpRelRead(const INS ins);

/*! @ingroup INS_INSPECTION
  Is an IP-relative write
*/
extern BOOL INS_IsIpRelWrite(const INS ins);

/*! @ingroup INS_INSPECTION
  We treat these instructions as predicated
     conditional move (CMOVcc)
     floating conditional move (FCMOVcc)
     rep string ops (since they don't execute if GCX==0)
*/
extern BOOL INS_IsPredicated(INS ins);

/*! @ingroup INS_INSPECTION
  @return true if this is an original instruction
*/
extern BOOL INS_IsOriginal(INS ins);

/*! @ingroup INS_INSPECTION
  @return std::string disassembly of instruction
*/
extern std::string INS_Disassemble(INS ins);

/*! @ingroup INS_INSPECTION
    @return The memory displacement of an instrucation with memory operand.
    @note: the displacement is a signed number.
 */
extern ADDRDELTA INS_MemoryDisplacement(INS ins);

/*! @ingroup INS_INSPECTION
  @return The base register used in the instruction's memory operand, or REG_INVALID()
  if there is no base register.
 */
extern REG INS_MemoryBaseReg(INS ins);

/*! @ingroup INS_INSPECTION
  @return The index register used in the instruction's memory operand, or REG_INVALID()
  if there is no index register.
 */
extern REG INS_MemoryIndexReg(INS ins);

/*! @ingroup INS_INSPECTION
  @return The scale factor (1,2,4,8) by which the index register in the
  instruction's memory operand is multiplied.
 */
extern UINT32 INS_MemoryScale(INS ins);

/*! @ingroup INS_INSPECTION
  Change all occurrences of old_reg to new_reg in the  r/w sets of the ins.
  Return TRUE if at least one occurrence changed.
*/
extern BOOL INS_ChangeReg(const INS ins, const REG old_reg, const REG new_reg, const BOOL as_read);

#endif // PIN_G_INS_IA32_PH
