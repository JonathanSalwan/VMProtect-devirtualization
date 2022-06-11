/*
 * Copyright (C) 2014-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * This header file consolidate definitions for macOS* and Linux
 * running on both ia32 and intel64 architecture.
 */

/*-------------------------------------------------------------------
 * OS specific macros:
 *-------------------------------------------------------------------
 * NAME(symbol)                - Decorate 'symbol' as a global symbol
 * DECLARE_FUNCTION(symbol)    - Declare 'symbol' as function type
 * DECLARE_FUNCTION_AS(symbol) - Declare 'symbol' as function type without decorations
 * END_FUNCTION(symbol)        - Mark the end of the function 'symbol'
 * PLT_ADDRESS(symbol)         - call target for 'symbol' to make a call to external function.
 *-------------------------------------------------------------------
 */
#ifdef TARGET_MAC
#define NAME(x) _##x
#define DECLARE_FUNCTION(fun) .global NAME(fun);
#define DECLARE_FUNCTION_AS(fun) .global fun;
#define END_FUNCTION(fun)        \
    .global NAME(fun##_endfunc); \
    NAME(fun##_endfunc) :
#define PLT_ADDRESS(fun) NAME(fun)
#else
#define NAME(x) x
#define DECLARE_FUNCTION(fun) \
    .globl NAME(fun);         \
    .type NAME(fun), @function
#define DECLARE_FUNCTION_AS(fun) DECLARE_FUNCTION(fun)
#define END_FUNCTION(fun) .size NAME(fun), .- NAME(fun)
#define PLT_ADDRESS(fun) NAME(fun)##@plt
#endif

/*-------------------------------------------------------------------
 * Architecture specific macros:
 *-------------------------------------------------------------------
 * BEGIN_STACK_FRAME         - Expands to the instructions that build a new stack
 *                             frame for a function
 * END_STACK_FRAME           - Expands to the instructions that destroy a stack
 *                             frame just before calling "ret"
 * PARAM1                    - The first argument to a function.
 *                             Note that this macro is valid only after building a
 *                             stack frame
 * PARAM2                    - The second argument to a function.
 *                             Note that this macro is valid only after building a
 *                             stack frame
 * SCRATCH_REG*              - Taken from Calling Convention:
 *                             Scratch register are registers that can be used for temporary storage without
 *                             restrictions (no need to save before using them and restore after using them)
 * SCRATCH_REG1              - Scratch register eax/rax depending on the architecture
 * SCRATCH_REG2              - Scratch register ecx/rcx depending on the architecture
 * SCRATCH_REG3              - Scratch register edx/rdx depending on the architecture
 * SCRATCH_REG4              - Scratch register rsi (available only in intel64)
 * CALLEE_SAVE_REG1          - Callee-save register ebx/rbx depending on the architecture (need to be pushed
 *                             before used and popped when not used anymore before ret instruction
 * CALLEE_SAVE_REG2          - Callee-save register esx/r12 depending on the architecture (need to be pushed
 *                             before used and popped when not used anymore before ret instruction
 * RETURN_REG                - The register that holds the return value
 * GAX_REG                   - eax/rax depending on the architecture
 * GBX_REG                   - ebx/rbx depending on the architecture
 * GCX_REG                   - ecx/rcx depending on the architecture
 * GDX_REG                   - edx/rdx depending on the architecture
 * GSI_REG                   - esi/rsi depending on the architecture
 * G12_REG                   - r12 (available only in intel64)
 * STACK_PTR                 - The stack pointer register
 * PIC_VAR(v)                - Reference memory at 'v' in PIC notation (not supported in 32 bit mode)
 * SYSCALL_PARAM1            - The first argument to a system call
 *                             Note that this macro is valid only after building a
 *                             stack frame
 * PREPARE_UNIX_SYSCALL(num) - Prepare to run a syscall numbered 'num'
 *                             Assign the syscall number (plus some required
 *                             transformation to the register that holds the
 *                             syscall number).
 * INVOKE_SYSCALL            - The instruction sequence that does the actual invocation of a syscall.
 *-------------------------------------------------------------------
 */
#if defined(TARGET_IA32)
#define BEGIN_STACK_FRAME \
    push % ebp;           \
    mov % esp, % ebp
#define END_STACK_FRAME \
    mov % ebp, % esp;   \
    pop % ebp
#define PARAM1 8(% ebp)
#define PARAM2 0xc(% ebp)
#define RETURN_REG % eax
#define GAX_REG % eax
#define GBX_REG % ebx
#define GCX_REG % ecx
#define CL_REG % cl
#define GDX_REG % edx
#define GSI_REG % esi
#define STACK_PTR % esp
#define PIC_VAR(a) a
#ifdef TARGET_MAC
#define SYSCALL_PARAM1 (% esp)
#define PREPARE_UNIX_SYSCALL(num) \
    mov num, % eax;               \
    or $0x40000, % eax
#define INVOKE_SYSCALL            \
    calll LTrap##__LINE__;        \
    jmpl LTrapDone##__LINE__;     \
    LTrap##__LINE__ : popl % edx; \
    movl % esp, % ecx;            \
    sysenter;                     \
    LTrapDone##__LINE__:
#else
#define PREPARE_UNIX_SYSCALL(num) mov num, % eax
#define SYSCALL_PARAM1 % ebx
#define INVOKE_SYSCALL int $0x80
#endif
#elif defined(TARGET_IA32E)
#define BEGIN_STACK_FRAME \
    push % rbp;           \
    mov % rsp, % rbp
#define END_STACK_FRAME \
    mov % rbp, % rsp;   \
    pop % rbp
#define PARAM1 % rdi
#define PARAM2 % rsi
#define RETURN_REG % rax
#define GAX_REG % rax
#define GBX_REG % rbx
#define GCX_REG % rcx
#define CL_REG % cl
#define GDX_REG % rdx
#define GSI_REG % rsi
#define G12_REG % r12
#define STACK_PTR % rsp
#define PIC_VAR(a) a(% rip)
#define SYSCALL_PARAM1 % rdi
#define INVOKE_SYSCALL syscall
#ifdef TARGET_MAC
#define PREPARE_UNIX_SYSCALL(num) \
    mov num, % rax;               \
    or $0x2000000, % rax
#else
#define PREPARE_UNIX_SYSCALL(num) mov num, % rax
#endif
#endif

/*
 * Common
 */
#define SCRATCH_REG1 GAX_REG
#define SCRATCH_REG2 GCX_REG
#define SCRATCH_REG3 GDX_REG
#if defined(TARGET_IA32E)
#define SCRATCH_REG4 GSI_REG
#endif

#define CALLEE_SAVE_REG1 GBX_REG
#if defined(TARGET_IA32)
#define CALLEE_SAVE_REG2 GSI_REG
#elif defined(TARGET_IA32E)
#define CALLEE_SAVE_REG2 G12_REG
#endif
