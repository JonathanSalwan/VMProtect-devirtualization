/*
 * Copyright (C) 2020-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#ifdef PIN_G_OPERAND_IA32_PH
#error duplicate inclusion of operand_ia32
#else
#define PIN_G_OPERAND_IA32_PH
/*! @file
  ia32 specific stuff related to OPERAND
*/
/*! @ingroup INS_INSPECTION
 * Tells the number of operands for the instruction.  Several other APIs take
 * an operand index as a parameter.  Those APIs expect an index in the range
 * [0, n-1], where n is the value returned by INS_OperandCount().
 *
 *  @param[in] ins  The instruction.
 *
 * @return  The number of operands for the instruction.
 */
extern UINT32 INS_OperandCount(INS ins);

/*! @ingroup INS_INSPECTION
  @param[in] ins      The instruction.
  @param[in] opIdx    The operand index whose element size is required
                      The operand can be either a memory operand or a register operand.
  @return the number of elements in the requested operand. 
  Returns 1 if this operand has no elements.
*/
extern UINT32 INS_OperandElementCount(INS ins, UINT32 opIdx);

/*! @ingroup INS_INSPECTION
  @param[in] ins      The instruction.
  @param[in] opIdx    The operand index whose element size is required
                      The operand can be either a memory operand or a register operand.
  @return the size of the requested operand element in bytes.
   For scalar instructions, this function will return the same as INS_OperandSize.
   For vector instructions, this function will return the size of a single element.
   For vscatter/vgather, this function will return the size of a single load/store.
*/
extern USIZE INS_OperandElementSize(INS ins, UINT32 opIdx);

/*! @ingroup INS_INSPECTION
 */
extern UINT32 INS_OperandNameId(INS ins, UINT32 n);

/*! @ingroup INS_INSPECTION
  @return true if this operand is a memory reference

  Note: this does not include LEA operands.
*/
extern BOOL INS_OperandIsMemory(INS ins, UINT32 n);

/*! @ingroup INS_INSPECTION
  @return register used as base register in memory operand, or REG_INVALID()
  Effective address = Displacement + BaseReg + IndexReg * Scale
*/
extern REG INS_OperandMemoryBaseReg(INS ins, UINT32 n);

/*! @ingroup INS_INSPECTION
  @return register used as index register in memory operand, or REG_INVALID()
  Effective address = Displacement + BaseReg + IndexReg * Scale
*/
extern REG INS_OperandMemoryIndexReg(INS ins, UINT32 n);

/*! @ingroup INS_INSPECTION
  @return register used as segment register in memory operand, or REG_INVALID()
*/
extern REG INS_OperandMemorySegmentReg(INS ins, UINT32 n);

/*! @ingroup INS_INSPECTION
  @return scale used for addressing in memory operand.
  Effective address = Displacement + BaseReg + IndexReg * Scale
*/
extern UINT32 INS_OperandMemoryScale(INS ins, UINT32 n);

/*! @ingroup INS_INSPECTION
    @return The memory displacement of an instrucation with memory operand.
    @note the displacement is a signed number.
    Effective address = Displacement + BaseReg + IndexReg * Scale
*/
extern ADDRDELTA INS_OperandMemoryDisplacement(INS ins, UINT32 n);

/*! @ingroup INS_INSPECTION
 * @return TRUE if memory operand uses predefined base register and this register
 *         can not be changed
 * Example: movs %ds:(%esi), %es:(%edi)
 * There are two fixed operands
*/
extern BOOL INS_OperandIsFixedMemop(INS ins, UINT32 n);

/*! @ingroup INS_INSPECTION
*/
extern VOID GetNumberAndSizeOfMemAccesses(INS ins, int* numAccesses, int* accessSize, int* indexSize);

/*! @ingroup INS_INSPECTION
  @return the number of memory operands.
  Please note @ref INS_MemoryOperandElementCount for reading the number
  of elements per memory operand.

  @note     Segment prefix operands (i.e. %gs:0x14 ) are memory operands.
*/
extern UINT32 INS_MemoryOperandCount(INS ins);

/*! @ingroup INS_INSPECTION
  @return the number of elements in the requested memory operand.
  For instructions where the memory operands has not elemenets, returns 1.
*/
extern UINT32 INS_MemoryOperandElementCount(INS ins, UINT32 memoryOp);

/*! @ingroup INS_INSPECTION
  @param[in] ins      the instruction.
  @param[in] memoryOp the memory operand index whose size is required.

  @return the size of the requested memory operand in bytes (for REPped instructions this is the size
  accessed in each iteration of the implicit loop).
  The operand of instructions with scattered memory access (like vscatter/vgather) does not have a size,
  and this function should not be called for these functions.
  Use INS_HasScatteredMemoryAccess before calling this function.
*/
extern USIZE INS_MemoryOperandSize(INS ins, UINT32 memoryOp);

/*! @ingroup INS_INSPECTION
  @param[in] ins      the instruction.
  @param[in] memoryOp the memory operand index whose element size is required.

  @return the size of the requested memory operand element in bytes.
   For scalar instructions, this function will return the same as INS_MemoryOperandSize.
   For vector instructions, this function will return the size of a single element.
*/
extern USIZE INS_MemoryOperandElementSize(INS ins, UINT32 memoryOp);

/*! @ingroup INS_INSPECTION
  @return true if this operand generates an address, but the address does not
  access memory (e.g. load effective address instruction)
*/
extern BOOL INS_OperandIsAddressGenerator(INS ins, UINT32 n);

/*! @ingroup INS_INSPECTION
  @return true if this operand is a displacement
  (e.g. branch offset)
*/
extern BOOL INS_OperandIsBranchDisplacement(INS ins, UINT32 n);

/*! @ingroup INS_INSPECTION
  @return TRUE if memory operand memopIdx is read
*/
extern BOOL INS_MemoryOperandIsRead(INS ins, UINT32 memopIdx);

/*! @ingroup INS_INSPECTION
  @return TRUE if memory operand memopIdx is written
*/
extern BOOL INS_MemoryOperandIsWritten(INS ins, UINT32 memopIdx);

/*! @ingroup INS_INSPECTION
 *  Convert a memory operand index into a simple operand index.
 *
 *  @param[in] ins       The instruction.
 *  @param[in] memopIdx  Memory operand's index in the range [0, n-1], where n is
 *                       from INS_MemoryOperandCount().
 *
 * @return operand index
*/
extern UINT32 INS_MemoryOperandIndexToOperandIndex(INS ins, UINT32 memopIdx);

/*! @ingroup INS_INSPECTION
  @return true if this operand is a register
*/
extern BOOL INS_OperandIsReg(INS ins, UINT32 n);

/*! @ingroup INS_INSPECTION
  @return true if this operand is a segment register
*/
extern BOOL INS_OperandIsSegmentReg(INS ins, UINT32 n);

/*! @ingroup INS_INSPECTION
  @return register name for this operand, may return REG_INVALID()
*/
extern REG INS_OperandReg(INS ins, UINT32 n);

/*! @ingroup INS_INSPECTION
  @return true if this operand is an immediate
*/
extern BOOL INS_OperandIsImmediate(INS ins, UINT32 n);

/*! @ingroup INS_INSPECTION
  Although return type is UINT64, on 32-bit systems only the lower
  32 bits are utilized. To see how to retrieve immediate values
  with correct width and sign information, see example in test
  tool PinTools/SimpleExamples/oper-imm.cpp.
  @return immediate value for operand
*/
extern UINT64 INS_OperandImmediate(INS ins, UINT32 n);

/*! @ingroup INS_INSPECTION
  @return true if this operand is implied by the opcode
  (e.g. the stack write in a push instruction)
*/
extern BOOL INS_OperandIsImplicit(INS ins, UINT32 n);

/*! @ingroup INS_INSPECTION
  Assumes that reg is a PIN register
  @return true if the ins has
*/
extern BOOL INS_RegIsImplicit(INS ins, REG reg);

/*! @ingroup INS_INSPECTION
  @return operand width in bits.
*/
extern UINT32 INS_OperandWidth(INS ins, UINT32 n);

/*! @ingroup INS_INSPECTION
  @return operand size in bytes
*/
extern UINT32 INS_OperandSize(INS ins, UINT32 n);

/*! @ingroup INS_INSPECTION
 * Tells if an instruction operand is a source; it may also be a destination.
 *
 *  @param[in] ins  The instruction.
 *  @param[in] n    Operand's index in the range [0, n-1], where n is
 *                   from INS_OperandCount().
 *
 * @return  TRUE if the operand is a source.
 */
extern BOOL INS_OperandRead(INS ins, UINT32 n);

/*! @ingroup INS_INSPECTION
 * Tells if an instruction operand is a destination; it may also be a source.
 *
 *  @param[in] ins  The instruction.
 *  @param[in] n    Operand's index in the range [0, n-1], where n is
 *                   from INS_OperandCount().
 *
 * @return  TRUE if the operand is a destination.
 */
extern BOOL INS_OperandWritten(INS ins, UINT32 n);

/*! @ingroup INS_INSPECTION
 * Tells if an instruction operand is just a source (and not a destination).
 *
 *  @param[in] ins  The instruction.
 *  @param[in] n    Operand's index in the range [0, n-1], where n is
 *                   from INS_OperandCount().
 *
 * @return  TRUE if the operand is just a source.
 */
extern BOOL INS_OperandReadOnly(INS ins, UINT32 n);

/*! @ingroup INS_INSPECTION
 * Tells if an instruction operand is just a destination (and not a source).
 *
 *  @param[in] ins  The instruction.
 *  @param[in] n    Operand's index in the range [0, n-1], where n is
 *                   from INS_OperandCount().
 *
 * @return  TRUE if the operand is just a destination.
 */
extern BOOL INS_OperandWrittenOnly(INS ins, UINT32 n);

/*! @ingroup INS_INSPECTION
 * Tells if an instruction operand is both a source and a destination.
 *
 *  @param[in] ins  The instruction.
 *  @param[in] n    Operand's index in the range [0, n-1], where n is
 *                   from INS_OperandCount().
 *
 * @return  TRUE if the operand is both a source and a destination.
 */
extern BOOL INS_OperandReadAndWritten(INS ins, UINT32 n);

/*! @ingroup INS_INSPECTION
 * Returns the mask register used in the given instruction if such exist, REG_INVALID otherwise.
 *
 * @param[in]   ins     The vector instruction in question.
 *
 * @return      The mask register used in vector instruction, or REG_INVALID if no mask register is used.
 */
extern REG INS_MaskRegister(INS ins);

#endif // PIN_G_OPERAND_IA32_PH
