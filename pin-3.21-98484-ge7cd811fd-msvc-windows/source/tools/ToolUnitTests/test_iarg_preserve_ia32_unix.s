/*
 * Copyright (C) 2011-2012 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

.data


.extern var1
.extern var2
.extern var3
.extern setFlagsX
.extern TestIargPreserveInReplacement
.extern TestIargPreserveInReplacement1
.extern TestIargPreserveInProbed
.extern TestIargPreserveInProbed1
.extern TestIargPreserveInReplacement2


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
	fstp var1
	fstp var2
	fstp var3
    ret



.text
.global Fld1Fstp_f2 
.type Fld1Fstp_f2,  @function
Fld1Fstp_f2:
    fld1
	fld1
	fld1
	fstp var1
	fstp var2
	fstp var3
    ret


.text
.global AddToXmmRegs 
.type AddToXmmRegs,  @function
AddToXmmRegs:
    lea val1, %eax
	mov  (%eax), %eax
    paddd  (%eax),  %xmm0
	lea  val2, %eax
	mov   (%eax), %eax
    paddd  (%eax), %xmm1
	lea  val3, %eax
	mov (%eax),  %eax
	paddd  (%eax),  %xmm2
	lea val4,  %eax
	mov  (%eax), %eax
	paddd   (%eax), %xmm3
	lea  val5, %eax
	mov   (%eax), %eax
	paddd  (%eax),  %xmm4
	lea  val6, %eax
	mov   (%eax), %eax
	paddd  (%eax),  %xmm5
	lea val7, %eax
	mov   (%eax), %eax
	paddd   (%eax), %xmm6
	lea  val8, %eax
	mov  (%eax),  %eax
	paddd  (%eax),  %xmm7
    ret


.text
.global GetFlags 
.type GetFlags,  @function
GetFlags:
    pushfl
    pop %eax
	ret



.text
.global SetFlags
.type SetFlags,  @function
SetFlags:
    push setFlagsX
    popfl
    ret

.text
.global Fld1Fstp_l1
.type Fld1Fstp_l1,  @function
Fld1Fstp_l1:
    fld1
	fld1
	fld1
	fstp var1
	fstp var2
	fstp var3
    ret

.text
.global Fld1Fstp_l2
.type Fld1Fstp_l2,  @function
Fld1Fstp_l2:
    fld1
	fld1
	fld1
	fstp var1
	fstp var2
	fstp var3
    ret

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
    fstpl var1
	fstpl var2
	fstpl var3
    ret

.text
.global Fld1Fstp_a
.type Fld1Fstp_a,  @function
Fld1Fstp_a:
    fld1
	fld1
	fld1
	fstpl var1
	fstpl var2
	fstpl var3
    ret

.global Fld1Fstp_b
.type Fld1Fstp_b,  @function
Fld1Fstp_b:
    fld1
	fld1
	fld1
	fstpl var1
	fstpl var2
	fstpl var3
    ret

.global Fld1Fstp_c
.type Fld1Fstp_c,  @function
Fld1Fstp_c:
    fld1
	fld1
	fld1
	fstpl var1
	fstpl var2
	fstpl var3
    ret

.global Fld1Fstp_d
.type Fld1Fstp_d,  @function
Fld1Fstp_d:
    fld1
	fld1
	fld1
	fstpl var1
	fstpl var2
	fstpl var3
    ret

.global Fld1Fstp_e
.type Fld1Fstp_e,  @function
Fld1Fstp_e:
    fld1
	fld1
	fld1
	fstpl var1
	fstpl var2
	fstpl var3
    ret

.global Fld1Fstp_f
.type Fld1Fstp_f,  @function
Fld1Fstp_f:
    fld1
	fld1
	fld1
	fstpl var1
	fstpl var2
	fstpl var3
    ret

.global Fld1Fstp_g
.type Fld1Fstp_g,  @function
Fld1Fstp_g:
    fld1
	fld1
	fld1
	fstpl var1
	fstpl var2
	fstpl var3
    ret

.global Fld1Fstp_h
.type Fld1Fstp_h,  @function
Fld1Fstp_h:
    fld1
	fld1
	fld1
	fstpl var1
	fstpl var2
	fstpl var3
    ret

.global Fld1Fstp_i
.type Fld1Fstp_i,  @function
Fld1Fstp_i:
    fld1
	fld1
	fld1
	fstpl var1
	fstpl var2
	fstpl var3
    ret

.global Fld1Fstp_j
.type Fld1Fstp_j,  @function
Fld1Fstp_j:
    fld1
	fld1
	fld1
	fstpl var1
	fstpl var2
	fstpl var3
    ret

.global Fld1Fstp_k
.type Fld1Fstp_k,  @function
Fld1Fstp_k:
    fld1
	fld1
	fld1
	fstpl var1
	fstpl var2
	fstpl var3
    ret

.global Fld1Fstp_l
.type Fld1Fstp_l,  @function
Fld1Fstp_l:
    fld1
	fld1
	fld1
	fstpl var1
	fstpl var2
	fstpl var3
    ret

.global Fld1Fstp_m
.type Fld1Fstp_m,  @function
Fld1Fstp_m:
    fld1
	fld1
	fld1
	call TestIargPreserveInReplacement
	fstpl var1
	fstpl var2
	fstpl var3
    ret

.global Fld1Fstp_n
.type Fld1Fstp_n,  @function
Fld1Fstp_n:
    fld1
	fld1
	fld1
	call TestIargPreserveInReplacement1
	fstpl var1
	fstpl var2
	fstpl var3
    ret

.global Fld1Fstp_n1
.type Fld1Fstp_n1,  @function
Fld1Fstp_n1:
    fld1
	fld1
	fld1
	call TestIargPreserveInReplacement2
	fstpl var1
	fstpl var2
	fstpl var3
    ret

.global Fld1Fstp_o
.type Fld1Fstp_o,  @function
Fld1Fstp_o:
    fld1
	fld1
	fld1
	call TestIargPreserveInProbed
	fstpl var1
	fstpl var2
	fstpl var3
    ret

.global Fld1Fstp_p
.type Fld1Fstp_p,  @function
Fld1Fstp_p:
    fld1
	fld1
	fld1
	call TestIargPreserveInProbed1
	fstpl var1
	fstpl var2
	fstpl var3
    ret

.global Fld1Fstp_q
.type Fld1Fstp_q,  @function
Fld1Fstp_q:
    fld1
	fld1
	fld1
	call TestIargPreserveInReplacement2
	fstpl var1
	fstpl var2
	fstpl var3
    ret

.global Fld1FstpSetXmmsMaskMxcsr
.type Fld1FstpSetXmmsMaskMxcsr,  @function
Fld1FstpSetXmmsMaskMxcsr:
    fld1
    fld1
    fld1
    call SetXmmRegsToZero
    call MaskZeroDivideInMxcsr
    fstpl var1
    fstpl var2
    fstpl var3
    ret


.global IntScratchFld1Fstp_a
.type IntScratchFld1Fstp_a,  @function
IntScratchFld1Fstp_a:
    mov  $0xdeadbeef, %eax
    mov  $0xdeadbeef, %ecx
    mov  $0xdeadbeef, %edx
    fld1
    fld1
    fld1
    fstpl var1
    fstpl var2
    fstpl var3
    ret


.global IntScratchFld1Fstp_b
.type IntScratchFld1Fstp_b,  @function
IntScratchFld1Fstp_b:
    mov  $0xdeadbeef, %eax
    mov  $0xdeadbeef, %ecx
    mov  $0xdeadbeef, %edx
    fld1
    fld1
    fld1
    fstpl var1
    fstpl var2
    fstpl var3
    ret

.global IntScratchFld1Fstp_c
.type IntScratchFld1Fstp_c,  @function
IntScratchFld1Fstp_c:
    mov  $0xdeadbeef, %eax
    mov  $0xdeadbeef, %ecx
    mov  $0xdeadbeef, %edx
    fld1
    fld1
    fld1
    fstpl var1
    fstpl var2
    fstpl var3
    ret

.global IntScratchFld1Fstp_d
.type IntScratchFld1Fstp_d,  @function
IntScratchFld1Fstp_d:
    mov $0xdeadbeef, %eax
    mov  $0xdeadbeef, %ecx
    mov  $0xdeadbeef, %edx
    fld1
    fld1
    fld1
    fstpl var1
    fstpl var2
    fstpl var3
    ret

.global SetIntRegsToZero
.type SetIntRegsToZero,  @function
SetIntRegsToZero:
    xor %eax, %eax
    xor %ecx, %ecx
    xor %edx, %edx
    ret



.global get_gax
.type get_gax,  @function
get_gax:
    ret

.global get_gbx
.type get_gbx,  @function
get_gbx:
    mov %ebx,  %eax
    ret


.global get_gcx
.type get_gcx, @function
get_gcx:
    mov %ecx,  %eax
    ret


.global get_gdx
.type get_gdx, @function
get_gdx:
    mov  %edx, %eax
    ret

.global get_gsi
.type get_gsi,  @function
get_gsi:
    mov %esi,  %eax
    ret

.global get_gdi
.type get_gdi,  @function
get_gdi:
    mov %edi,  %eax
    ret

.global get_gbp
.type get_gbp,  @function
get_gbp:
    mov %ebp,  %eax
    ret

.global set_gcx
.type set_gcx, @function
set_gcx:
    mov 4(%esp),  %ecx
    ret

.global set_gax
.type set_gax, @function
set_gax:
    mov 4(%esp),  %eax
    ret

.global set_gbx
.type set_gbx, @function
set_gbx:
    mov 4(%esp),  %ebx
    ret

.global set_gdx
.type set_gdx, @function
set_gdx:
    mov 4(%esp),  %edx
    ret

.global set_gdi
.type set_gdi, @function
set_gdi:
    mov 4(%esp),  %edi
    ret

.global set_gsi
.type set_gsi, @function
set_gsi:
    mov 4(%esp),  %esi
    ret

.global set_gbp
.type set_gbp, @function
set_gbp:
    mov 4(%esp),  %ebp
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
    ret



.global GetMxcsr
.type GetMxcsr, @function
GetMxcsr:
   push %eax
   stmxcsr (%esp)
   pop %eax
   ret


.global MaskZeroDivideInMxcsr
.type MaskZeroDivideInMxcsr, @function
MaskZeroDivideInMxcsr:
   call GetMxcsr
   or  $0x200, %eax
   push %eax
   ldmxcsr (%esp)
   pop %eax
   ret


.global UnMaskZeroDivideInMxcsr
.type UnMaskZeroDivideInMxcsr, @function
UnMaskZeroDivideInMxcsr:
   call GetMxcsr
   mov  $0x200, %ecx
   not %ecx
   and %ecx, %eax
   push %eax
   ldmxcsr (%esp)
   pop %eax
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
