/*
 * Copyright (C) 2011-2012 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

.data


.extern var1
.extern var2
.extern var3
.extern TestIargPreserveInReplacement
.extern TestIargPreserveInReplacement1
.extern TestIargPreserveInProbed
.extern TestIargPreserveInProbed1
.extern TestIargPreserveInReplacement2

.text
.global Fld1_x
.type Fld1_x,  @function
Fld1_x:
    fld1
	fld1
	fld1
    ret

.text
.global Fstp_x
.type Fstp_x,  @function
Fstp_x:
    fstpl var1(%rip)
	fstpl var2(%rip)
	fstpl var3(%rip)
    ret

.text
.global Fld1Fstp_a
.type Fld1Fstp_a,  @function
Fld1Fstp_a:
    fld1
	fld1
	fld1
	fstpl var1(%rip)
	fstpl var2(%rip)
	fstpl var3(%rip)
    ret

.global Fld1Fstp_b
.type Fld1Fstp_b,  @function
Fld1Fstp_b:
    fld1
	fld1
	fld1
	fstpl var1(%rip)
	fstpl var2(%rip)
	fstpl var3(%rip)
    ret

.global Fld1Fstp_c
.type Fld1Fstp_c,  @function
Fld1Fstp_c:
    fld1
	fld1
	fld1
	fstpl var1(%rip)
	fstpl var2(%rip)
	fstpl var3(%rip)
    ret

.global Fld1Fstp_d
.type Fld1Fstp_d,  @function
Fld1Fstp_d:
    fld1
	fld1
	fld1
	fstpl var1(%rip)
	fstpl var2(%rip)
	fstpl var3(%rip)
    ret

.global Fld1Fstp_e
.type Fld1Fstp_e,  @function
Fld1Fstp_e:
    fld1
	fld1
	fld1
	fstpl var1(%rip)
	fstpl var2(%rip)
	fstpl var3(%rip)
    ret

.global Fld1Fstp_f
.type Fld1Fstp_f,  @function
Fld1Fstp_f:
    fld1
	fld1
	fld1
	fstpl var1(%rip)
	fstpl var2(%rip)
	fstpl var3(%rip)
    ret

.global Fld1Fstp_g
.type Fld1Fstp_g,  @function
Fld1Fstp_g:
    fld1
	fld1
	fld1
	fstpl var1(%rip)
	fstpl var2(%rip)
	fstpl var3(%rip)
    ret

.global Fld1Fstp_h
.type Fld1Fstp_h,  @function
Fld1Fstp_h:
    fld1
	fld1
	fld1
	fstpl var1(%rip)
	fstpl var2(%rip)
	fstpl var3(%rip)
    ret

.global Fld1Fstp_i
.type Fld1Fstp_i,  @function
Fld1Fstp_i:
    fld1
	fld1
	fld1
	fstpl var1(%rip)
	fstpl var2(%rip)
	fstpl var3(%rip)
    ret

.global Fld1Fstp_j
.type Fld1Fstp_j,  @function
Fld1Fstp_j:
    fld1
	fld1
	fld1
	fstpl var1(%rip)
	fstpl var2(%rip)
	fstpl var3(%rip)
    ret

.global Fld1Fstp_k
.type Fld1Fstp_k,  @function
Fld1Fstp_k:
    fld1
	fld1
	fld1
	fstpl var1(%rip)
	fstpl var2(%rip)
	fstpl var3(%rip)
    ret

.global Fld1Fstp_l
.type Fld1Fstp_l,  @function
Fld1Fstp_l:
    fld1
	fld1
	fld1
	fstpl var1(%rip)
	fstpl var2(%rip)
	fstpl var3(%rip)
    ret

.global Fld1Fstp_m
.type Fld1Fstp_m,  @function
Fld1Fstp_m:
    fld1
	fld1
	fld1
	call TestIargPreserveInReplacement
	fstpl var1(%rip)
	fstpl var2(%rip)
	fstpl var3(%rip)
    ret

.global Fld1Fstp_n
.type Fld1Fstp_n,  @function
Fld1Fstp_n:
    fld1
	fld1
	fld1
	call TestIargPreserveInReplacement1
	fstpl var1(%rip)
	fstpl var2(%rip)
	fstpl var3(%rip)
    ret

.global Fld1Fstp_n1
.type Fld1Fstp_n1,  @function
Fld1Fstp_n1:
    fld1
	fld1
	fld1
	call TestIargPreserveInReplacement2
	fstpl var1(%rip)
	fstpl var2(%rip)
	fstpl var3(%rip)
    ret

.global Fld1Fstp_o
.type Fld1Fstp_o,  @function
Fld1Fstp_o:
    fld1
	fld1
	fld1
	call TestIargPreserveInProbed
	fstpl var1(%rip)
	fstpl var2(%rip)
	fstpl var3(%rip)
    ret

.global Fld1Fstp_p
.type Fld1Fstp_p,  @function
Fld1Fstp_p:
    fld1
	fld1
	fld1
	call TestIargPreserveInProbed1
	fstpl var1(%rip)
	fstpl var2(%rip)
	fstpl var3(%rip)
    ret

.global Fld1Fstp_q
.type Fld1Fstp_q,  @function
Fld1Fstp_q:
    fld1
	fld1
	fld1
	call TestIargPreserveInReplacement2
	fstpl var1(%rip)
	fstpl var2(%rip)
	fstpl var3(%rip)
    ret

.global Fld1FstpSetXmmsMaskMxcsr
.type Fld1FstpSetXmmsMaskMxcsr,  @function
Fld1FstpSetXmmsMaskMxcsr:
    fld1
    fld1
    fld1
    call SetXmmRegsToZero
    call MaskZeroDivideInMxcsr
    fstpl var1(%rip)
    fstpl var2(%rip)
    fstpl var3(%rip)
    ret


.global IntScratchFld1Fstp_a
.type IntScratchFld1Fstp_a,  @function
IntScratchFld1Fstp_a:
    mov  $0xdeadbeef, %rax
    mov  $0xdeadbeef, %rcx
    mov  $0xdeadbeef, %rdx
	mov  $0xdeadbeef, %rdi
	mov  $0xdeadbeef, %rsi
	mov  $0xdeadbeef, %r8
	mov  $0xdeadbeef, %r9
	mov  $0xdeadbeef, %r10
	mov  $0xdeadbeef, %r11
    fld1
    fld1
    fld1
    fstpl var1(%rip)
    fstpl var2(%rip)
    fstpl var3(%rip)
    ret


.global IntScratchFld1Fstp_b
.type IntScratchFld1Fstp_b,  @function
IntScratchFld1Fstp_b:
    mov  $0xdeadbeef, %rax
    mov  $0xdeadbeef, %rcx
    mov  $0xdeadbeef, %rdx
	mov  $0xdeadbeef, %rdi
	mov  $0xdeadbeef, %rsi
	mov  $0xdeadbeef, %r8
	mov  $0xdeadbeef, %r9
	mov  $0xdeadbeef, %r10
	mov  $0xdeadbeef, %r11
    fld1
    fld1
    fld1
    fstpl var1(%rip)
    fstpl var2(%rip)
    fstpl var3(%rip)
    ret

.global IntScratchFld1Fstp_c
.type IntScratchFld1Fstp_c,  @function
IntScratchFld1Fstp_c:
    mov  $0xdeadbeef, %rax
    mov  $0xdeadbeef, %rcx
    mov  $0xdeadbeef, %rdx
	mov  $0xdeadbeef, %rdi
	mov  $0xdeadbeef, %rsi
	mov  $0xdeadbeef, %r8
	mov  $0xdeadbeef, %r9
	mov  $0xdeadbeef, %r10
	mov  $0xdeadbeef, %r11
    fld1
    fld1
    fld1
    fstpl var1(%rip)
    fstpl var2(%rip)
    fstpl var3(%rip)
    ret

.global IntScratchFld1Fstp_d
.type IntScratchFld1Fstp_d,  @function
IntScratchFld1Fstp_d:
    mov  $0xdeadbeef, %rax
    mov  $0xdeadbeef, %rcx
    mov  $0xdeadbeef, %rdx
	mov  $0xdeadbeef, %rdi
	mov  $0xdeadbeef, %rsi
	mov  $0xdeadbeef, %r8
	mov  $0xdeadbeef, %r9
	mov  $0xdeadbeef, %r10
	mov  $0xdeadbeef, %r11
    fld1
    fld1
    fld1
    fstpl var1(%rip)
    fstpl var2(%rip)
    fstpl var3(%rip)
    ret

.global SetIntRegsToZero
.type SetIntRegsToZero,  @function
SetIntRegsToZero:
    xor %rax, %rax
    xor %rcx, %rcx
    xor %rdx, %rdx
	xor %r8, %r8
	xor %r9, %r9
	xor %r10, %r10
	xor %r11, %r11
	xor %rsi, %rsi
	xor %rdi, %rdi
    ret



.global get_gax
.type get_gax,  @function
get_gax:
    ret


.global get_gcx
.type get_gcx, @function
get_gcx:
    mov %rcx,  %rax
    ret

.global get_gbx
.type get_gbx, @function
get_gbx:
    mov %rbx,  %rax
    ret




.global get_gdx
.type get_gdx, @function
get_gdx:
    mov  %rdx, %rax
    ret

.global get_gsi
.type get_gsi, @function
get_gsi:
    mov  %rsi, %rax
    ret

.global get_gdi
.type get_gdi, @function
get_gdi:
    mov  %rdi, %rax
    ret

.global get_gbp
.type get_gbp, @function
get_gbp:
    mov  %rbp, %rax
    ret

.global get_r8
.type get_r8, @function
get_r8:
    mov  %r8, %rax
    ret

.global get_r9
.type get_r9, @function
get_r9:
    mov  %r9, %rax
    ret

.global get_r10
.type get_r10, @function
get_r10:
    mov  %r10, %rax
    ret

.global get_r11
.type get_r11, @function
get_r11:
    mov  %r11, %rax
    ret

.global get_r12
.type get_r12, @function
get_r12:
    mov  %r12, %rax
    ret

.global get_r13
.type get_r13, @function
get_r13:
    mov  %r13, %rax
    ret

.global get_r14
.type get_r14, @function
get_r14:
    mov  %r14, %rax
    ret

.global get_r15
.type get_r15, @function
get_r15:
    mov  %r15, %rax
    ret

.global set_gax
.type set_gax,  @function
set_gax:
    mov %rdi, %rax
    ret

.global set_gbx
.type set_gbx,  @function
set_gbx:
    mov %rdi, %rbx
    ret

.global set_gbp
.type set_gbp,  @function
set_gbp:
    mov %rdi, %rbp
    ret

.global set_gcx
.type set_gcx,  @function
set_gcx:
    mov %rdi, %rcx
    ret

.global set_gdx
.type set_gdx,  @function
set_gdx:
    mov %rdi, %rdx
    ret

.global set_gsi
.type set_gsi,  @function
set_gsi:
    mov %rdi, %rsi
    ret

.global set_gdi
.type set_gdi,  @function
set_gdi:
    mov %rdi, %rdi
    ret

.global set_r8
.type set_r8,  @function
set_r8:
    mov %rdi, %r8
    ret

.global set_r9
.type set_r9,  @function
set_r9:
    mov %rdi, %r9
    ret

.global set_r10
.type set_r10,  @function
set_r10:
    mov %rdi, %r10
    ret

.global set_r11
.type set_r11,  @function
set_r11:
    mov %rdi, %r11
    ret

.global set_r12
.type set_r12,  @function
set_r12:
    mov %rdi, %r12
    ret

.global set_r13
.type set_r13,  @function
set_r13:
    mov %rdi, %r13
    ret

.global set_r14
.type set_r14,  @function
set_r14:
    mov %rdi, %r14
    ret

.global set_r15
.type set_r15,  @function
set_r15:
    mov %rdi, %r15
    ret


.global FldzToTop3
.type FldzToTop3, @function
FldzToTop3:
    fstpl %st(0)
    fstpl %st(0)
    fstpl %st(0)
    fldz
    fldz
    fldz
    ret





.global SetXmmRegsToZero
.type SetXmmRegsToZero, @function
SetXmmRegsToZero:
    pxor %xmm0, %xmm0
    pxor %xmm1, %xmm1
	pxor %xmm2, %xmm2
	pxor %xmm3, %xmm3
	pxor %xmm4, %xmm4
	pxor %xmm5, %xmm5
	pxor %xmm6, %xmm6
	pxor %xmm7, %xmm7
	pxor %xmm8, %xmm8
	pxor %xmm9, %xmm9
	pxor %xmm10, %xmm10
	pxor %xmm11, %xmm11
	pxor %xmm12, %xmm12
	pxor %xmm13, %xmm13
	pxor %xmm14, %xmm14
	pxor %xmm15, %xmm15
    ret



.global GetMxcsr
.type GetMxcsr, @function
GetMxcsr:
   push %rax
   stmxcsr (%rsp)
   pop %rax
   ret


.global MaskZeroDivideInMxcsr
.type MaskZeroDivideInMxcsr, @function
MaskZeroDivideInMxcsr:
   call GetMxcsr
   or  $0x200, %rax
   push %rax
   ldmxcsr (%rsp)
   pop %rax
   ret


.global UnMaskZeroDivideInMxcsr
.type UnMaskZeroDivideInMxcsr, @function
UnMaskZeroDivideInMxcsr:
   call GetMxcsr
   mov  $0x200, %rcx
   not %rcx
   and %rcx, %rax
   push %rax
   ldmxcsr (%rsp)
   pop %rax
   ret

.global Fld2tToTop3
.type Fld2tToTop3, @function
Fld2tToTop3:
    fstpl %st(0)
    fstpl %st(0)
    fstpl %st(0)
	fldl2t
	fldl2t
	fldl2t
    ret

.text
.global GetFlags 
.type GetFlags,  @function
GetFlags:
    pushfq
    pop %rax
	ret



.text
.global SetFlags
.type SetFlags,  @function
SetFlags:
    push setFlagsX(%rip)
    popfq
    ret


.text
.global Fld1Fstp_l1
.type Fld1Fstp_l1,  @function
Fld1Fstp_l1:
    fld1
	fld1
	fld1
	fstp var1(%rip)
	fstp var2(%rip)
	fstp var3(%rip)
    ret

.text
.global Fld1Fstp_l2
.type Fld1Fstp_l2,  @function
Fld1Fstp_l2:
    fld1
	fld1
	fld1
        fstp var1(%rip)
        fstp var2(%rip)
        fstp var3(%rip)
        ret

.text
.global Fld1_aa 
.type Fld1_aa,  @function
Fld1_aa:
    fld1
	fld1
	fld1
    ret


.text
.global Fstp3
.type Fstp3,  @function
Fstp3:
    fstpl %st(0)
    fstpl %st(0)
    fstpl %st(0)
    ret


.text
.global Fld1_ab 
.type Fld1_ab,  @function
Fld1_ab:
    fld1
	fld1
	fld1
    ret


.text
.global Fld1_ac 
.type Fld1_ac,  @function
Fld1_ac:
    fld1
	fld1
	fld1
    ret


.text
.global Fld1_ad 
.type Fld1_ad,  @function
Fld1_ad:
    fld1
	fld1
	fld1
    ret


.text
.global Fld1_ae 
.type Fld1_ae,  @function
Fld1_ae:
    fld1
	fld1
	fld1
    ret




.text
.global Fld1_af
.type Fld1_af,  @function
Fld1_af:
    fld1
	fld1
	fld1
    ret



.text
.global Fld1_ag
.type Fld1_ag,  @function
Fld1_ag:
    fld1
	fld1
	fld1
    ret





.text
.global Fld1Fstp_f1 
.type Fld1Fstp_f1,  @function
Fld1Fstp_f1:
    fld1
	fld1
	fld1
	fstp var1(%rip)
	fstp var2(%rip)
	fstp var3(%rip)
    ret



.text
.global Fld1Fstp_f2 
.type Fld1Fstp_f2,  @function
Fld1Fstp_f2:
    fld1
	fld1
	fld1
        fstp var1(%rip)
        fstp var2(%rip)
        fstp var3(%rip)
    ret


.text
.global AddToXmmRegs 
.type AddToXmmRegs,  @function
AddToXmmRegs:
    lea val1(%rip), %rax
	mov  (%rax), %rax
    paddd  (%rax),  %xmm0
	lea  val2(%rip), %rax
	mov   (%rax), %rax
    paddd  (%rax), %xmm1
	lea  val3(%rip), %rax
	mov (%rax),  %rax
	paddd  (%rax),  %xmm2
	lea val4(%rip),  %rax
	mov  (%rax), %rax
	paddd   (%rax), %xmm3
	lea  val5(%rip), %rax
	mov   (%rax), %rax
	paddd  (%rax),  %xmm4
	lea  val6(%rip), %rax
	mov   (%rax), %rax
	paddd  (%rax),  %xmm5
	lea val7(%rip), %rax
	mov   (%rax), %rax
	paddd   (%rax), %xmm6
	lea  val8(%rip), %rax
	mov  (%rax),  %rax
	paddd  (%rax),  %xmm7
	lea  val9(%rip), %rax
	mov  (%rax),  %rax
	paddd  (%rax),  %xmm8
	lea  val10(%rip), %rax
	mov  (%rax),  %rax
	paddd  (%rax),  %xmm9
	lea  val11(%rip), %rax
	mov  (%rax),  %rax
	paddd  (%rax),  %xmm10
	lea  val12(%rip), %rax
	mov  (%rax),  %rax
	paddd  (%rax),  %xmm11
	lea  val13(%rip), %rax
	mov  (%rax),  %rax
	paddd  (%rax),  %xmm12
	lea  val14(%rip), %rax
	mov  (%rax),  %rax
	paddd  (%rax),  %xmm13
	lea  val15(%rip), %rax
	mov  (%rax),  %rax
	paddd  (%rax),  %xmm14
	lea  val16(%rip), %rax
	mov  (%rax),  %rax
	paddd  (%rax),  %xmm15
    ret
