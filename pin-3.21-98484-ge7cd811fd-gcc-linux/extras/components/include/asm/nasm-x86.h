/*
 * Copyright (C) 2008-2016 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
** <COMPONENT>: asm
** <FILE-TYPE>: component public header
*/

#ifndef ASM_NASM_X86_H
#define ASM_NASM_X86_H


#define ASM_FILEBEGIN()
#define ASM_FILEEND()

#define ASM_FUNCBEGIN(name, rtype, args)    \
    section .text ASM_NEWLINE               \
    global ASM_NAME(name)  ASM_NEWLINE      \
    ASM_NAME(name):

#define ASM_FUNCEND(name)

#define ASM_HEX(val)    0##val##h

#if defined(TARGET_MAC)
#   define ASM_NAME(name)   _##name
#else
#   define ASM_NAME(name)   name
#endif

#define ASM_LABDEF(x)   .l##x##:
#define ASM_LABF(x)     .l##x
#define ASM_LABB(x)     .l##x
#define ASM_GLABDEF(x)  global ASM_NAME(x) ASM_NEWLINE \
                        ASM_NAME(x):

#define ASM_BYTE()      byte
#define ASM_WORD()      word
#define ASM_DWORD()     dword
#define ASM_QWORD()

#define ASM_BYTE_TYPE()     db
#define ASM_WORD_TYPE()     dw
#define ASM_DWORD_TYPE()    dd
#define ASM_QWORD_TYPE()

#define ASM_NAMED_DATA(label, type, value)   \
    section .data                        ASM_NEWLINE \
    align ASM_##type##_SIZE              ASM_NEWLINE \
    global ASM_NAME(label)               ASM_NEWLINE \
    ASM_NAME(label):  ASM_##type##_TYPE() value

#if defined(HOST_IA32)
#define ASM_PIC_INIT_TMP3(line) Ltmp ## line
#define ASM_PIC_INIT_TMP2(line) ASM_PIC_INIT_TMP3(line)
#define ASM_PIC_INIT_TMP ASM_PIC_INIT_TMP2(__LINE__)
#   define ASM_PIC_INIT(reg)         call ASM_PIC_INIT_TMP ASM_NEWLINE \
                                     ASM_PIC_INIT_TMP: pop reg ASM_NEWLINE \
                                     sub reg, ASM_PIC_INIT_TMP
#   define ASM_PC_REL_REF(var,reg) reg + ASM_NAME(var)
#else
#   define ASM_PIC_INIT(reg)
#   define ASM_PC_REL_REF(var,dummy) ASM_NAME(var) wrt rip
#endif

#endif /*file guard*/
