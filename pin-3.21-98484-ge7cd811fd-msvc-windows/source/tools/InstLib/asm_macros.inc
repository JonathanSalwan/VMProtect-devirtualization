; This header file consolidate definitions for Windows
; running on both ia32 and intel64 architecture.
; Note that all definitions here are according to the 'cdecl'
; calling convention (the default in Windows)

;-------------------------------------------------------------------
; Architecture specific macros:
;-------------------------------------------------------------------
; PROLOGUE                  - The prologue of the program - must be placed before
;                             any other declaration
; BEGIN_STACK_FRAME         - Expands to the instructions that build a new stack
;                             frame for a function
; END_STACK_FRAME           - Expands to the instructions that destroy a stack
;                             frame just before calling "ret"
; PARAM1                    - The first argument to a function.
;                             Note that this macro is valid only after building a
;                             stack frame
; PARAM2                    - The second argument to a function.
;                             Note that this macro is valid only after building a
;                             stack frame
; SCRATCH_REG*              - Taken from Calling Convention:
;                             Scratch register are registers that can be used for temporary storage without
;                             restrictions (no need to save before using them and restore after using them)
; SCRATCH_REG1              - Scratch register eax/rax depending on the architecture
; SCRATCH_REG2              - Scratch register ecx/rcx depending on the architecture
; SCRATCH_REG2              - Scratch register edx/rdx depending on the architecture
; RETURN_REG                - The register that holds the return value
; GAX_REG                   - eax/rax depending on the architecture
; GBX_REG                   - ebx/rbx depending on the architecture
; GDI_REG                   - edi/rdi depending on the architecture
; GSI_REG                   - esi/rsi depending on the architecture
; NATIVE_SIZE_SUFFIX <inst> - Add the suffix representing the word size native to
;                             the current target to instruction <inst>.
;                             For example "NATIVE_SIZE_SUFFIX popf" will be expanded
;                             to popfd on ia32, and popfq on intel64.
; STACK_PTR                 - The stack pointer register
; ADDRINT_PTR               - Prefix for memory operands to refer to sizeof(ADDRINT)
; ADDRINT_TYPE              - Assembler type of ADDRINT
;-------------------------------------------------------------------

IFDEF TARGET_IA32
    PROLOGUE MACRO
        .686
        .XMM
        .model flat, c
    ENDM
    BEGIN_STACK_FRAME macro
        push ebp
        mov ebp,esp
    ENDM
    END_STACK_FRAME macro
        mov esp,ebp
        pop ebp
    ENDM
    PARAM1 textequ <[EBP+8]>
    PARAM2 textequ <[EBP+12]>
    RETURN_REG textequ <EAX>
    GDI_REG textequ <EDI>
    GSI_REG textequ <ESI>
	GAX_REG textequ <EAX>
    GBX_REG textequ <EBX>
	GCX_REG textequ <ECX>
	GDX_REG textequ <EDX>
	CL_REG  textequ <CL>
    STACK_PTR textequ <ESP>
    ADDRINT_PTR textequ <DWORD PTR>
    ADDRINT_TYPE textequ <DWORD>
	NATIVE_SIZE_SUFFIX macro inst
	    &inst&d
	ENDM
ELSE
    PROLOGUE MACRO
    ENDM
    BEGIN_STACK_FRAME macro
        push rbp
        mov rbp,rsp
    ENDM
    END_STACK_FRAME macro
        mov rsp,rbp
        pop rbp
    ENDM
    PARAM1 textequ <RCX>
    PARAM2 textequ <RDX>
    RETURN_REG textequ <RAX>
    GDI_REG textequ <RDI>
    GSI_REG textequ <RSI>
	GAX_REG textequ <RAX>
    GBX_REG textequ <RBX>
	GCX_REG textequ <RCX>
	GDX_REG textequ <RDX>
    G8_REG textequ <R8>
	CL_REG  textequ <CL>
    STACK_PTR textequ <RSP>
    ADDRINT_PTR textequ <QWORD PTR>
    ADDRINT_TYPE textequ <QWORD>
	NATIVE_SIZE_SUFFIX macro inst
	    &inst&q
	ENDM
ENDIF

;-------------------------------------------------------------------
; Common
;-------------------------------------------------------------------

SCRATCH_REG1 textequ <GAX_REG>
SCRATCH_REG2 textequ <GCX_REG>
SCRATCH_REG3 textequ <GDX_REG>
IFDEF TARGET_IA32
 SCRATCH_REG4 textequ <G8_REG>
ENDIF

CALLEE_SAVE_REG1 textequ <GBX_REG>
CALLEE_SAVE_REG2 textequ <GSI_REG>
