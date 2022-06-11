/*
 * Copyright (C) 2017-2017 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#ifdef TARGET_WINDOWS
#define ASMNAME(name)
#else
#define ASMNAME(name) asm(name)
#endif

extern "C"
{
    int iretTest() ASMNAME("iretTest");
}

int main()
{
    iretTest();
    return 0;
}
