/*
 * Copyright (C) 2013-2013 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

.text

# void TestAlignment();
.global TestAlignment
.type TestAlignment,  @function
TestAlignment:
    sub     $0x8c, %esp
    movaps  %xmm0, (%esp)
    add     $0x8c, %esp
    ret
