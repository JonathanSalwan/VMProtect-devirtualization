/*
 * Copyright (C) 2008-2019 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
** <COMPONENT>: asm
** <FILE-TYPE>: component public header
*/

#ifndef ASM_GAS_X86_H
#define ASM_GAS_X86_H

/*
 * Toolchain should define ASM_WINDOWS for Windows targets, ASM_MAC for macOS* targets.
 */

#define ASM_FILEBEGIN() .intel_syntax noprefix; .globl _GLOBAL_OFFSET_TABLE_

#define ASM_FILEEND()

#define ASM_NEWLINE ;

#define ASM_FUNCBEGIN(name, rtype, args)    \
    .text;                                  \
    .align 4;                               \
    .globl ASM_NAME(name);                  \
    .type  ASM_NAME(name), @function;       \
    ASM_NAME(name):

#define ASM_FUNCEND(name) .size name, .-name

#define ASM_HEX(val)    0x##val

#if defined(TARGET_WINDOWS) || defined(TARGET_MAC)
#   define ASM_NAME(name)   _##name
#else
#   define ASM_NAME(name)   name
#endif

#define ASM_LABDEF(x)   .l##x##:
#define ASM_LABF(x)     .l##x
#define ASM_LABB(x)     .l##x
#define ASM_GLABDEF(x)  .globl ASM_NAME(x); \
                        ASM_NAME(x):

#define ASM_BYTE()      BYTE PTR
#define ASM_WORD()      WORD PTR
#define ASM_DWORD()     DWORD PTR

/*
 * The only legal 64-bit accesses on IA32 are implied by the instruction
 * mnemonic, so the assembler does not accept "QWORD PTR".  On Intel64, though,
 * there are places where the memory size is not implied by the instruction
 * mnemonic, and "QWORD PTR" is required.
 */
#if defined(HOST_IA32)
#   define ASM_QWORD()
#else
#   define ASM_QWORD()     QWORD PTR
#endif

#define ASM_BYTE_TYPE()     .byte
#define ASM_WORD_TYPE()     .word
#define ASM_DWORD_TYPE()    .int
#if defined(HOST_IA32)
#   define ASM_QWORD_TYPE()
#else
#   define ASM_QWORD_TYPE()     .quad
#endif

#define ASM_NAMED_DATA(label, vtype, value)  \
    .data                           ; \
    .align ASM_##vtype##_SIZE        ; \
    .type ASM_NAME(label), @object  ; \
    .size ASM_NAME(label), ASM_##vtype##_SIZE        ; \
    ASM_NAME(label):                ; \
    ASM_##vtype##_TYPE() value

#if defined(HOST_IA32)
#   define ASM_PIC_INIT(reg)         call 1f; 1: pop reg; lea reg, [reg + _GLOBAL_OFFSET_TABLE_ + 1]
#   define ASM_PC_REL_REF(var,reg) reg + var@GOTOFF
#else
#   define ASM_PIC_INIT(reg)
#   define ASM_PC_REL_REF(var,dummy) rip + var
#endif

#endif /*file guard*/
