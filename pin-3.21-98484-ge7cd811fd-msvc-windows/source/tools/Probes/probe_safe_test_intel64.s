/*
 * Copyright (C) 2011-2012 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

# 
# contains important code patterns
# doesn't actually do anything
# Used to make sure that the probe is done only when allowed.

#very short func, not valid for probe
.global too_short
.type too_short, function
too_short:
    ret
    .size       too_short, .-too_short

.type hidden_no_nops, function
hidden_no_nops:
    nop
    push %rbx
    pop %rbx
    push %rbx
    pop %rbx
    push %rbx
    pop %rbx
    ret
    .size hidden_no_nops, .-hidden_no_nops

#very short func followed by padding nops, valid for probe
.global too_short_with_nops
.type too_short_with_nops, function
too_short_with_nops:
    ret
    .size too_short_with_nops, .-too_short_with_nops

.type hidden_nops, function
hidden_nops:
    nop
    nop
    nop
    nop
    nop
    nop
    ret
    .size hidden_nops, .-hidden_nops

    
.global call_hidden_function
.type call_hidden_function, function
call_hidden_function:
    push %rbx
    call hidden_no_nops
    call hidden_nops
    pop %rbx
    ret
    .size call_hidden_function, .-call_hidden_function

.global call_function
.type call_function, function

call_function:
    push %rbx
    call too_short
    call too_short_with_nops
    pop %rbx
    ret
    .size call_function, .-call_function

