/*
 * Copyright (C) 2011-2015 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

.text
#ifdef TARGET_MAC
.global _Fld1_a
_Fld1_a:
#else
.global Fld1_a
.type Fld1_a,  @function
Fld1_a:
#endif
    fld1
	fld1
	fld1
    ret


#ifdef TARGET_MAC
.global _Fld1_b
_Fld1_b:
#else
.global Fld1_b
.type Fld1_b,  @function
Fld1_b:
#endif
    fld1
	fld1
	fld1
    ret


#ifdef TARGET_MAC
.global _FldzToTop3_a
_FldzToTop3_a:
#else
.global FldzToTop3_a
.type FldzToTop3_a,  @function
FldzToTop3_a:
#endif
    fstp %st(0)
	fstp %st(0)
	fstp %st(0)
	fldz
	fldz
	fldz
    ret

#ifdef TARGET_MAC
.global _mmx_save
_mmx_save:
#else
.global mmx_save
.type mmx_save,  @function
mmx_save:
#endif
  fxsave (%rdi)
  RET


#ifdef TARGET_MAC
.global _mmx_restore
_mmx_restore:
#else
.global mmx_restore
.type mmx_restore,  @function
mmx_restore:
#endif
  fxrstor (%rdi)
  RET



