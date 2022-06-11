/*
 * Copyright (C) 2008-2017 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
** <COMPONENT>: asm
** <FILE-TYPE>: component public header
*/

#ifndef ASM_H
#define ASM_H


/*
 * This header provides a set of C macros for use in assembly language files.  Using
 * these macros can help make assembly sources easier to read and can also provide some
 * amount of portability between different assembler syntaxes.
 *
 * Client code can use the following macros:
 *
 *      ASM_FILEBEGIN()
 *          Use this at the start of the source file.
 *
 *      ASM_FILEEND()
 *          Use this at the end of the source file.
 *
 *      ASM_FUNCBEGIN(name, rtype, (args))
 *          Use this before defining a function.  The macro does NOT generate any prolog
 *          instructions, so you still need to do that yourself.
 *
 *          @param name     The name of the function.  The macro automatically includes
 *                           any leading underscore, etc. if necessary.
 *          @param rtype    The C / C++ type returned by this function.  (Used to auto-generate
 *                           a header file with external declarations.)
 *          @param (args)   A list of C / C++ parameters separated by commas and enclosed in
 *                           parens.  (Used to auto-generate a header file.)
 *
 *          When a header file is auto-generated, any comment block that immediately precedes
 *          the ASM_FUNCBEGIN() is also included in the header.  If that comment block
 *          contains Doxygen style comments, the auto-generated header can be processed with
 *          Doxygen.
 *
 *      ASM_FUNCEND(name)
 *          Use this immediately after a function definition.  The macro does NOT generate
 *          any epilog code, so you still need to do that yourself.
 *
 *          @param name     The name of the function, exactly as it appeared in the
 *                           ASM_FUNCBEGIN().
 *
 *      ASM_HEX(val)
 *          Use this whenever a hex constant is needed.
 *
 *          @param val      The value of the constant in base 16, but without any leading 0x.
 *
 *      ASM_LABDEF(n)
 *          Use this to define a local label in a function.
 *
 *          @param n        A number (decimal) which identifies the label.
 *
 *      ASM_LABF(n)
 *          Use this in an instruction operand when refering to a label that is *forward*
 *          in the file.
 *
 *          @param n        The label's number.
 *
 *      ASM_LABB(n)
 *          Use this in an instruction operand when refering to a label that is *backward*
 *          in the file.
 *
 *          @param n        The label's number.
 *
 *      ASM_BYTE()
 *      ASM_WORD()
 *      ASM_DWORD()
 *      ASM_QWORD()
 *          Use these to disambiguate the size of a memory reference in x86 assembly code.
 *
 *      ASM_REGARG0
 *      ASM_REGARG1
 *      ASM_REGARG2
 *      ASM_REGARG3
 *      ASM_REGARG4
 *      ASM_REGARG5
 *          Use these to refer to the register that contains a function argument on the
 *          Intel(R) 64 architecture.  These macros help insulate source code from the differences
 *          between the Unix and Windows calling standards.
 *
 *      ASM_<type>_SIZE
 *          Size of type <type> in bytes.
 *
 *      ASM_<type>_TYPE
 *          Keyword to declare instance of type <type>
 *
 *      ASM_NAMED_DATA(label, type, value)
 *          declares a (local) variable with type 'type', name 'label'
 *          and initial value of 'value'
 *
 *      ASM_PIC_INIT(reg)
 *          Initialization code for function that reference PIC code/data.
 *          'reg' is assigned as the PIC register to be used for later references
 *          to PIC data/code
 *
 *      ASM_PC_REL_REF(var,reg)
 *          Reference to a PIC variable using PC relative addressing.
 *          'var' is the variable to reference while 'reg' is the PIC register.
 *
 * Toolchains must define the following macros in order to enable the ASM support:
 *
 *      ASM_TC_GAS, ASM_TC_MASM, ASM_TC_NASM
 *          Must define one of these, according to the assembler provided by the toolchain.
 */

#define ASM_BYTE_SIZE       1
#define ASM_WORD_SIZE       2
#define ASM_DWORD_SIZE      4
#define ASM_QWORD_SIZE      8


#if defined(ASM_TC_GAS)
#   include "asm/gas-x86.h"
#elif defined(ASM_TC_MASM)
#   include "asm/masm-x86.h"
#elif defined(ASM_TC_NASM)
#   include "asm/nasm-x86.h"
#else
#   error "Must define assembler type and architecture"
#endif

#if defined(HOST_IA32E) && defined(TARGET_WINDOWS)
#   include "asm/windows-intel64.h"
#elif (defined(TARGET_ANDROID) || defined(TARGET_MAC) || defined(TARGET_LINUX)) && defined(HOST_IA32E)
#   include "asm/unix-intel64.h"
#endif

#endif /*file guard*/
