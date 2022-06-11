/*
 * Copyright (C) 2020-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#ifdef PIN_G_INS_API_XED_IA32_PH
#error duplicate inclusion of ins_api_xed_ia32
#else
#define PIN_G_INS_API_XED_IA32_PH
/*! @file
  this file contains functions visible in level_core it uses primitives
  from level_core_with_ia32_ins but does not leak out any IA32 stuff!
*/
/*! @ingroup INS_INSPECTION
  @return whether the instruction has a real REP/REPZ/REPNZ prefix (i.e.
          one which is not just part of the opcode encoding but does cause
          the instruction to repeat).
*/
extern BOOL INS_HasRealRep(INS ins);

/*! @ingroup INS_INSPECTION
  @return false if this instruction has a memory operand which
  has unconventional meaning (e.g. vectorized memory reference of
  the GATHER/SCATTER instructions or some of the XSAVE instructions).
*/
extern BOOL INS_IsStandardMemop(INS ins);

/*! @ingroup INS_INSPECTION
  @return TRUE if instruction causes cache line flush
*/
extern BOOL INS_IsCacheLineFlush(INS ins);

/*! @ingroup INS_INSPECTION
  The category of an instruction is a higher level semantic description of an instruction than
  its opcode (@ref INS_Opcode). So, for instance, you can test for a conditional branch by using
  INS_Category(ins) == XED_CATEGORY_COND_BR, whereas using INS_Opcode(ins) would require that
  you enumerate all twenty conditional branch opcodes.

  The full mapping of opcodes to categories can be found in the idata.txt file in the Intel(R) X86 Encoder Decoder
  distribution (which is distributed as part of the Pin kit). The category enumeration can be
  found in the file "xed-category-enum.h".

  A category can be converted into a string by using @ref CATEGORY_StringShort.

  @return Instruction category
*/
extern INT32 INS_Category(const INS ins);

/*! @ingroup INS_INSPECTION
  @return Instruction extension, Use EXTENSION_StringShort to make it a string
*/
extern INT32 INS_Extension(const INS ins);

/*! @ingroup INS_INSPECTION
 *
 *  @return the width of the effective address in bits
 */
extern UINT32 INS_EffectiveAddressWidth(INS ins);

/*! @ingroup INS_INSPECTION
  @return the size of the memory write in bytes (for REP prefixed instructions this is the size of the
  operand written on each iteration of the REP, so 1,2,4 or 8).
  @note DEPRECATED: see @ref INS_MemoryOperandSize() instead.
*/
extern PIN_DEPRECATED_API USIZE INS_MemoryWriteSize(INS ins);

/*! @ingroup INS_INSPECTION
  @return the size of the memory read in bytes (for REP prefixed instructions this is the size of the
  operand read on each iteration of the REP, so 1,2,4 or 8).
  @note DEPRECATED: see @ref INS_MemoryOperandSize() instead.
*/
extern PIN_DEPRECATED_API USIZE INS_MemoryReadSize(INS ins);

/*! @ingroup INS_INSPECTION
  @return predicate for instruction (see @ref PREDICATE)

  CMOVcc and FMOVcc instructions are treated as predicated.
  Rep string ops are treated as predicated.
*/
extern PREDICATE INS_GetPredicate(INS ins);

/*! @ingroup INS_INSPECTION
  @return true if this instruction reads memory

  @note     Segment prefix operands (i.e. %gs:0x14 ) are memory operands.
*/
extern BOOL INS_IsMemoryRead(INS ins);

/*! @ingroup INS_INSPECTION
  @return true if this instruction writes memory

  @note     Segment prefix operands (i.e. %gs:0x14 ) are memory operands.
*/
extern BOOL INS_IsMemoryWrite(INS ins);

/*! @ingroup INS_INSPECTION
  @return true if this instruction has 2 memory read operands
*/
extern BOOL INS_HasMemoryRead2(INS ins);

/*! @ingroup INS_INSPECTION

  If INS_HasFallThrough(INS) is TRUE, then the instruction may
  execute the "natural" next instruction (i.e. the one which starts
  immediately after this one), if it is FALSE, then the instruction
  following the one tested will not (normally) be executed next.  So
  HasFallThrough is TRUE for instructions which don't change the
  control flow (most instructions), or for conditional branches
  (which might change the control flow, but might not), and FALSE for
  unconditional branches and calls (where the next instruction to be
  executed is always explicitly specified).

  Note that an unconditional branch or call to the next instruction
  still doesn't have a fall-through, since a fall through is the
  implicitly following instruction, and an unconditional branch never
  uses that.

  @return true if the instruction type has a fallthrough path based on the opcode
  @note for system calls function returns false.
*/
extern BOOL INS_HasFallThrough(INS ins);

/*! @ingroup INS_INSPECTION
  @return true if the instruction is a SysEnter
*/
extern BOOL INS_IsSysenter(INS ins);

/*! @ingroup INS_INSPECTION
  @return true if the instruction is a Xbegin
*/
extern BOOL INS_IsXbegin(INS ins);

/*! @ingroup INS_INSPECTION
  @return true if the instruction is a Xend
*/
extern BOOL INS_IsXend(INS ins);

/*! @ingroup INS_INSPECTION
  @return TRUE if the instruction is Lea.
*/
extern BOOL INS_IsLea(INS ins);

/*! @ingroup INS_INSPECTION
  @return TRUE if the instruction is a nop.
*/
extern BOOL INS_IsNop(INS ins);

/*! @ingroup INS_INSPECTION
  @return String with the opcode of the instruction
*/
extern std::string OPCODE_StringShort(UINT32 opcode);

/*! @ingroup INS_INSPECTION
  Only use this function if you really want a printable version of the instruction's opcode.

  If you are trying to test what the opcode of an instruction is, use @ref INS_Opcode and compare it
  with one of the XED_ICLASS values, or use one of the INS_Is... calls such as @ref INS_IsMov,
  rather than using this function and comparing a string. Generating and comparing strings is
  much slower than comparing small integer values!

  @return String form of mnemonic
*/
extern std::string INS_Mnemonic(INS ins);

/*! @ingroup INS_INSPECTION
 @return true if ins is a branch instruction.
 A branch can be conditional or unconditional.
 @note In some cases branch instructions are actually system calls.
 In case ins is in practice a system call, the function returns false.
 For example, JMP_FAR in Windows IA-32 can be a system call under certain conditions.
 */
extern BOOL INS_IsBranch(INS ins);

/*! @ingroup INS_INSPECTION
  These are supposed  to be near relative branches.
  @return true if the target address is an offset from the instruction
  pointer or is an immediate.
*/
extern BOOL INS_IsDirectBranch(INS ins);

/*! @ingroup INS_INSPECTION
  These are supposed  to be near relative branches.
  @return true if the target address is an offset from the instruction
  pointer or is an immediate.
*/
extern BOOL INS_IsDirectCall(INS ins);

/*! @ingroup INS_INSPECTION
  These are supposed  to be near relative branches.
  @return true if the target address is an offset from the instruction
  pointer or is an immediate.
  @note DEPRECATED: see @ref INS_IsDirectControlFlow() instead.
*/
extern PIN_DEPRECATED_API BOOL INS_IsDirectBranchOrCall(INS ins);

/*! @ingroup INS_INSPECTION
 @return TRUE if ins is a control-flow instruction, and its target address is an offset from the instruction
 pointer or is an immediate.
 */
extern BOOL INS_IsDirectControlFlow(INS ins);

/*! @ingroup INS_INSPECTION
 @return true if the ins is a halt or reserved undefined (UD2) instruction.
*/
extern BOOL INS_IsHalt(INS ins);

/*! @ingroup INS_INSPECTION
  @return TRUE if ins is a Branch or Call instruction,
  includes both direct and indirect instruction types.\n
  @note DEPRECATED: Use @ref INS_IsValidForIpointTakenBranch() instead to check if you can use IPOINT_TAKEN_BRANCH.
  Or use @ref INS_IsControlFlow() to check if instruction is a control flow.
*/
extern PIN_DEPRECATED_API BOOL INS_IsBranchOrCall(INS ins);

/*! @ingroup INS_INSPECTION
  @return true if ins "stutters", so re-executes even though it is not a branch or call.
  REP prefixed string ops do this.
*/
extern BOOL INS_Stutters(INS ins);

/*! @ingroup INS_INSPECTION
  @return true if this is a call to the next instruction, which is an idiom for materializing
  the instruction pointer
*/
extern BOOL INS_IsPcMaterialization(INS ins);

/*! @ingroup INS_INSPECTION
 @return true if ins is a Call instruction.
 Note: In case ins is in practice a system call, the function returns false.
 */
extern BOOL INS_IsCall(INS ins);

/*! @ingroup INS_INSPECTION
 @return true if ins is a Control Flow instruction.
 @note In general, any Branch, Call or Ret is a control flow instruction. However, we exclude any instruction which
 in practice is a system call (e.g. FAR_JMP in Windows with IA32 is sometimes a syscall).\n
 In addition, note that it returns true for Xbegin and Xend instructions.
 Use this function to verify that an instruction is valid for instrumentation with @ref IARG_BRANCH_TARGET_ADDR.
 */
extern BOOL INS_IsControlFlow(INS ins);

/*! @ingroup INS_INSPECTION
  @return true if ins is a Far Call instruction
*/
extern BOOL INS_IsFarCall(INS ins);

/*! @ingroup INS_INSPECTION
  @return true if ins is a Far Jump instruction
*/
extern BOOL INS_IsFarJump(INS ins);

/*! @ingroup INS_INSPECTION
  @return true if the target address is a direct far jump
*/
extern BOOL INS_IsDirectFarJump(INS ins);

/*! @ingroup INS_INSPECTION
 @return TRUE if ins can be instrumented at location IPOINT_AFTER.
 */
extern BOOL INS_IsValidForIpointAfter(INS ins);

/*! @ingroup INS_INSPECTION
 @return TRUE if ins can be instrumented at location IPOINT_TAKEN_BRANCH.
 */
extern BOOL INS_IsValidForIpointTakenBranch(INS ins);

/*! @ingroup INS_INSPECTION
 * @return TRUE iff the ins is a VGATHER* instruction
 */
extern BOOL INS_IsVgather(INS ins);

/*! @ingroup INS_INSPECTION
 * @return TRUE iff the ins is a VSCATTER* instruction
 */
extern BOOL INS_IsVscatter(INS ins);

/*! @ingroup INS_INSPECTION
 * @return TRUE iff the ins has a memory vector, i.e. the memory is accessed using a base address and an
 *         index vector with a scale.
 */
extern BOOL INS_HasMemoryVector(INS ins);

/*! @ingroup INS_INSPECTION
 * @return TRUE iff the ins has scattered memory access.
 * Scattered memory access means that the instruction does not access
 * the memory as a single sequential block, but rather is scattered 
 * into multiple memory blocks.
 * For exmaple, with the vscatter/vgather instructions, the index vector register 
 * is broken down to multiple index values that are used together with the base address,
 * scale and displacement to access multiple blocks.
 */
extern BOOL INS_HasScatteredMemoryAccess(INS ins);

/*! @ingroup INS_INSPECTION
 * Return TRUE iff the ins is an AMX instruction
 */
extern BOOL INS_IsAmx(INS ins);

/*! @ingroup INS_INSPECTION
  @return the segment selector and displacement from a direct far jump.
*/
extern VOID INS_GetFarPointer(INS ins, UINT16& segment_selector, UINT32& displacement);

/*! @ingroup INS_INSPECTION
  @return true if ins is a procedure call.
  This filters out call instructions that are (ab)used for other purposes
*/
extern BOOL INS_IsProcedureCall(INS ins);

/*! @ingroup INS_INSPECTION
   Return TRUE if the INS is an interrupt category instruction
*/
extern BOOL INS_IsInterrupt(INS ins);

/*! @ingroup INS_INSPECTION
 * Returns true if the given ins is a ret instruction.
 * @note This function will return true for iret instructions as well.
 * @note See also @ref INS_IsIRet.
 * @return true if the instruction is ret or iret.
 */
extern BOOL INS_IsRet(INS ins);

/*! @ingroup INS_INSPECTION
  @return true if ins is a System Return instruction
*/
extern BOOL INS_IsSysret(INS ins);

/*! @ingroup INS_INSPECTION
 @return true if ins is a far ret instruction
*/
extern BOOL INS_IsFarRet(INS ins);

/*! @ingroup INS_INSPECTION
  @return true if this instruction is a prefetch
*/
extern BOOL INS_IsPrefetch(INS ins);

/*! @ingroup INS_INSPECTION
  returns true if this instruction is a subtract.
*/
extern BOOL INS_IsSub(const INS ins);

/*! @ingroup INS_INSPECTION
  returns true if this instruction is a move.
*/
extern BOOL INS_IsMov(const INS ins);

/*! @ingroup INS_INSPECTION
  returns true if this instruction is a mov reg1, reg1. And reg1 is a full reg
*/
extern BOOL INS_IsMovFullRegRegSame(const INS ins);

/*! @ingroup INS_INSPECTION
  @return true if this instruction performs an atomic update of memory.
  @note An XCHG instruction accessing memory is atomic with or without a LOCK prefix.
*/
extern BOOL INS_IsAtomicUpdate(const INS ins);

/*! @ingroup INS_INSPECTION
  @return true if this instruction is an rdtsc or rdtscp.
*/
extern BOOL INS_IsRDTSC(const INS ins);

/*! @ingroup INS_INSPECTION
  @return true if this instruction is a maskmovq or maskmovdqu.
*/
extern BOOL INS_IsMaskMov(const INS ins);

/*! @ingroup INS_INSPECTION
  @return    register used as the repeat counter by this REP prefixed instruction, or REG_INVALID()
             if the instruction does not have a real REP prefix.
*/
extern REG INS_RepCountRegister(INS ins);

/*! @ingroup INS_INSPECTION
  @return true if the branch target comes from a register or memory.
  result is valid only if @ref INS_IsControlFlow() is true.
  @note DEPRECATED: see @ref INS_IsIndirectControlFlow() instead.
*/
extern PIN_DEPRECATED_API BOOL INS_IsIndirectBranchOrCall(INS ins);

/*! @ingroup INS_INSPECTION
  @return true if ins is a control-flow instruction, and its target address is provided through memory or
  through a register.
*/
extern BOOL INS_IsIndirectControlFlow(INS ins);

/*! @ingroup INS_INSPECTION
  @return Segment overide prefix used by INS
*/
extern REG INS_SegmentRegPrefix(INS ins);

/*! @ingroup INS_INSPECTION
  Test if this instruction has an explict memory operand.
  We include the memory operand in lea.

  @return TRUE if the instruction is has an explicit memory operand
*/
extern BOOL INS_HasExplicitMemoryReference(INS ins);

/*! @ingroup INS_INSPECTION
  Sets the disassembly syntax to Intel format. (Destination on the left)
*/
extern VOID PIN_SetSyntaxIntel();

/*! @ingroup INS_INSPECTION
  Sets the disassembly syntax to ATT SYSV format. (Destination on the right)
*/
extern VOID PIN_SetSyntaxATT();

/*! @ingroup INS_INSPECTION
  Sets the disassembly syntax to Intel(R) X86 Encoder Decoder detailed format which lists all resources read and written.
*/
extern VOID PIN_SetSyntaxXED();

/*! @ingroup INS_INSPECTION
  @return kth read register of instruction x, including implicit reads (e.g. stack
  pointer is read by push on IA-32 architectures)
*/
extern REG INS_RegR(INS x, UINT32 k);

/*! @ingroup INS_INSPECTION
  @return kth write register of instruction x, including implicit writes
  (e.g. stack pointer is written by push on IA-32 architectures)
*/
extern REG INS_RegW(INS x, UINT32 k);

/*! @ingroup INS_INSPECTION
  On ia-32 and Intel64 the opcodes are constants of the form XED_ICLASS_name.
  The full list of valid opcodes can be found in the idata.txt file in the Intel(R) X86 Encoder Decoder
  distribution (which is distributed as part of the Pin kit), and the enum
  definitions are in the file "xed-iclass-enum.h".

  Use @ref INS_Mnemonic if you want a string.

  @return Opcode of instruction
*/
extern OPCODE INS_Opcode(INS ins);

/*! @ingroup INS_INSPECTION
    @return String form of category
 */
extern std::string CATEGORY_StringShort(UINT32 num);

/*! @ingroup INS_INSPECTION
    @return String form of ISA extension
 */
extern std::string EXTENSION_StringShort(UINT32 num);

/*! @ingroup INS_INSPECTION
  @return true if this Seg prefixed instruction reads memory
*/
extern BOOL INS_SegPrefixIsMemoryRead(INS ins);

/*! @ingroup INS_INSPECTION
  @return true if this Seg prefixed instruction writes to memory
*/
extern BOOL INS_SegPrefixIsMemoryWrite(INS ins);

/*! @ingroup INS_INSPECTION
  @return true if the instruction has an address size prefix.
*/
extern BOOL INS_AddressSizePrefix(INS ins);

/*! @ingroup INS_INSPECTION
  @return true if the instruction has an  branch not taken hint prefix
*/
extern BOOL INS_BranchNotTakenPrefix(INS ins);

/*! @ingroup INS_INSPECTION
  @return true if the instruction has an branch taken hint prefix
*/
extern BOOL INS_BranchTakenPrefix(INS ins);

/*! @ingroup INS_INSPECTION
  @return true if the instruction has a lock prefix.
*/
extern BOOL INS_LockPrefix(INS ins);

/*! @ingroup INS_INSPECTION
  @return true if the instruction has an operand size prefix.
*/
extern BOOL INS_OperandSizePrefix(INS ins);

/*! @ingroup INS_INSPECTION
  @return true if the instruction has an REP (0xF3) prefix.
*/
extern BOOL INS_RepPrefix(INS ins);

/*! @ingroup INS_INSPECTION
  @return true if the instruction has an REPNE (0xF2) prefix.
*/
extern BOOL INS_RepnePrefix(INS ins);

/*! @ingroup INS_INSPECTION
  @return true if the instruction has a segment prefix;
  Call INS_SegmentRegPrefix() to get the actual prefix register name.
*/
extern BOOL INS_SegmentPrefix(INS ins);

/*! @ingroup INS_INSPECTION
  @return true if the instruction is an "xchg"
*/
extern BOOL INS_IsXchg(INS ins);

/*! @ingroup INS_INSPECTION
  @return true if the instruction is a string op (can be REP prefixed).
 */
extern BOOL INS_IsStringop(INS ins);

/*! @ingroup INS_INSPECTION
 * Returns true iff the given ins is an iret instruction.
 * @note See also @ref INS_IsRet.
 * @return true if the instruction is an iret.
 */
extern BOOL INS_IsIRet(INS ins);

#endif // PIN_G_INS_API_XED_IA32_PH
