/*
 * Copyright (C) 2009-2012 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

// This is a very short endless-loop function for 
// checking probe insertion and moving IP of thread that fails on probe

.type ShortFunc, @function
.global ShortFunc

ShortFunc:
.L:
push %rax
pop %rax
push %rax
pop %rax
nop
jmp .L

.type ShortFunc2, @function
.global ShortFunc2

ShortFunc2:
.L21:
test %rax, %rax
je .L22
push %rax
pop %rax
.L22:
jmp .L21
