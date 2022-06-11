/*
 * Copyright (C) 2007-2007 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * void set_tp( ADDRINT )
 */
    .text
    .align 16
    .global set_tp
    .proc set_tp
set_tp:
    .prologue
    .body
    mov r13=r32;;
    br.ret.sptk.many b0;;    
    .endp set_tp


/*
 * ADDRINT get_tp()
 */
    .text
    .align 16
    .global get_tp
    .proc get_tp
get_tp:
    .prologue
    .body
    mov r8=r13;;
    br.ret.sptk.many b0;;    
    .endp get_tp
