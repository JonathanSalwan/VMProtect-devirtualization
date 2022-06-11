/*
 * Copyright (C) 2009-2009 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

// This is a very short endless-loop function for 
// checking probe insertion and moving IP of thread that fails on probe

.type ShortFunc, @function
.global ShortFunc

ShortFunc:
.L:
push %eax
pop %eax
push %eax
pop %eax
nop
jmp .L

.type ShortFunc2, @function
.global ShortFunc2

ShortFunc2:
.L21:
test %eax, %eax
je .L22
push %eax
pop %eax
.L22:
jmp .L21
