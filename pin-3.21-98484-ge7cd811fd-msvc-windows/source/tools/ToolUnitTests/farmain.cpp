/*
 * Copyright (C) 2008-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * Expanded from just testing ret far to include tests for 
 * a number of other instructions with implicit registers
 */
#include <stdio.h>

typedef unsigned long long UINT64;
typedef signed long long INT64;
typedef unsigned int UINT32;
typedef unsigned short UINT16;

extern "C" int FarCallTest();
extern "C" void MaskMovQ(void* dest, UINT64 src, UINT64 mask);
extern "C" void* pushESP(void* stack);
extern "C" void* popESP(void* stack);
extern "C" void* push4ESP(void* stack);
extern "C" void* pop4ESP(void* stack);
extern "C" void* pushStarESP(void* stack);
extern "C" void* popStarESP(void* stack);
extern "C" void* pushFlags(void* stack);
extern "C" void* pushIZero(void* stack);
extern "C" void* pushWIZero(void* stack);
extern "C" void* pushIM1(void* stack);
extern "C" void* pushIs16(void* stack);
extern "C" void* pushMemAbs(void* stack);
extern "C" void* pushCS(void* stack);
extern "C" UINT16 readCS();
extern "C" void* pushFS(void* stack);
extern "C" UINT16 readFS();

/* Test maskMovQ */
static int tmq()
{
    UINT64 dest  = 0;
    UINT64 mTrue = UINT64(0x80);
    UINT64 mOnes = UINT64(0xff);
    UINT64 mask  = mTrue << (7 * 8) | mTrue << (5 * 8) | mTrue << (3 * 8) | mTrue << (1 * 8);
    UINT64 src   = UINT64(INT64(-1));

    MaskMovQ(&dest, src, mask);
    if (dest == (mOnes << (7 * 8) | mOnes << (5 * 8) | mOnes << (3 * 8) | mOnes << (1 * 8)))
    {
        printf("MaskMov: OK\n");
        return 0;
    }
    else
    {
        printf("***Fail*** MaskMov: result 0x%08x%08x\n", UINT32(dest >> 32), UINT32(dest));
        return 1;
    }
}

/* test some unpleasant push and pop cases */
static int tPushPop()
{
    void* stack[3];
    void** stackTop = &stack[2];
    void* finalAddr = pushESP(stackTop);
    int fails       = 0;

    // push %esp
    if (finalAddr != &stack[1] || stack[1] != stackTop)
    {
        printf("***Fail*** PushESP : top %p, result %p value %p\n", stackTop, finalAddr, stack[1]);
        fails++;
    }
    else
    {
        printf("push %%esp: OK\n");
    }

    // pop %esp
    stack[0]  = (void*)0x1234;
    finalAddr = popESP(&stack[0]);
    if (finalAddr != (void*)0x1234)
    {
        printf("***Fail*** PopESP: result %p\n", finalAddr);
        fails++;
    }
    else
    {
        printf("pop %%esp: OK\n");
    }

    // push (%esp)
    stack[0]  = 0;
    stack[1]  = (void*)0xdefaced0;
    stack[2]  = 0;
    finalAddr = pushStarESP(&stack[1]);

    if (finalAddr != &stack[0] || stack[0] != stack[1] || stack[1] != (void*)0xdefaced0)
    {
        printf("***Fail***: push (%%esp)\n");
        fails++;
    }
    else
    {
        printf("push (%%esp): OK\n");
    }

    // pop (%esp)
    stack[0]  = (void*)0xfaded;
    stack[2]  = 0;
    finalAddr = popStarESP(&stack[0]);

    if (finalAddr != &stack[1] || stack[0] != stack[1] || stack[1] != (void*)0xfaded || stack[2] != 0)
    {
        printf("***Fail***: pop (%%esp)\n");
        printf("FinalAddr : %p should be %p\n", finalAddr, &stack[1]);
        printf("stack[0]  : %p should be 0xfaded\n", stack[0]);
        printf("stack[1]  : %p should be 0xfaded\n", stack[1]);
        printf("stack[2]  : 0x%08x should be 0\n", UINT32(stack[2]));
        fails++;
    }
    else
    {
        printf("pop (%%esp): OK\n");
    }

    // push 4(%esp)
    stack[0]  = 0;
    stack[1]  = 0;
    stack[2]  = (void*)0xdefaced0;
    finalAddr = push4ESP(&stack[1]);

    if (finalAddr != &stack[0] || stack[0] != stack[2] || stack[2] != (void*)0xdefaced0)
    {
        printf("***Fail***: push 4(%%esp)\n");
        fails++;
    }
    else
    {
        printf("push 4(%%esp): OK\n");
    }

    // pop 4(%esp)
    stack[0]  = (void*)-1;
    stack[1]  = (void*)0xfaded;
    stack[2]  = 0;
    finalAddr = pop4ESP(&stack[0]);

    if (finalAddr != &stack[1] || stack[0] != (void*)-1 || stack[1] != (void*)0xfaded || stack[2] != stack[0])
    {
        printf("***Fail***: pop 4(%%esp)\n");
        printf("FinalAddr : %p should be %p\n", finalAddr, &stack[1]);
        printf("stack[0]  : %p should be -1\n", stack[0]);
        printf("stack[1]  : %p should be 0xfaded\n", stack[1]);
        printf("stack[2]  : 0x%08x should be -1\n", UINT32(stack[2]));
        fails++;
    }
    else
    {
        printf("pop (%%esp): OK\n");
    }

    finalAddr = pushFlags(&stack[1]);
    if (finalAddr != &stack[0])
    {
        printf("***Fail***: pushFlags wrong ESP\n");
        fails++;
    }

    printf("Flags pushed : 0x%08x\n", UINT32(stack[0]) & ~4);

    stack[0] = stack[1] = stack[2] = (void*)-1;
    finalAddr                      = pushIZero(&stack[2]);
    if (finalAddr != &stack[1] || stack[1] != 0)
    {
        printf("***Fail***: pushl $0\n");
        printf("FinalAddr : %p should be %p\n", finalAddr, &stack[1]);
        printf("stack[1]  : %p should be 0\n", stack[1]);
        fails++;
    }
    else
        printf("pushl $0: OK\n");

    stack[0] = stack[1] = stack[2] = (void*)-1;
    finalAddr                      = pushWIZero(&stack[2]);
    if (finalAddr != (((char*)&stack[1]) + 2) || stack[1] != (void*)0xffff)
    {
        printf("***Fail***: pushw $0\n");
        printf("FinalAddr : %p should be %p\n", finalAddr, ((char*)&stack[1]) + 2);
        printf("stack[1]  : %p should be 0xffff\n", stack[1]);
        fails++;
    }
    else
        printf("pushw $0: OK\n");

    stack[0] = stack[1] = stack[2] = (void*)0;
    finalAddr                      = pushIM1(&stack[2]);
    if (finalAddr != &stack[1] || stack[1] != (void*)-1)
    {
        printf("***Fail***: pushl $-1\n");
        printf("FinalAddr : %p should be %p\n", finalAddr, &stack[1]);
        printf("stack[1]  : %p should be -1\n", stack[1]);
        fails++;
    }
    else
        printf("pushl $-1: OK\n");

    stack[0] = stack[1] = stack[2] = (void*)0;
    finalAddr                      = pushIs16(&stack[2]);
    if (finalAddr != &stack[1] || stack[1] != (void*)0xffff8000)
    {
        printf("***Fail***: pushl $0xffff8000\n");
        printf("FinalAddr : %p should be %p\n", finalAddr, &stack[1]);
        printf("stack[1]  : %p should be 0xffff8000\n", stack[1]);
        fails++;
    }
    else
        printf("pushl $0xffff8000: OK\n");

    stack[0] = stack[1] = stack[2] = (void*)-1;
    finalAddr                      = pushMemAbs(&stack[2]);
    if (finalAddr != &stack[1] || stack[1] != (void*)0x01234567)
    {
        printf("***Fail***: pushl absoluteAddress\n");
        printf("FinalAddr : %p should be %p\n", finalAddr, &stack[1]);
        printf("stack[1]  : %p should be 0x01234567\n", stack[1]);
        fails++;
    }
    else
        printf("pushl absoluteAddress: OK\n");

    printf("%%cs = 0x%04x\n", readCS());
    stack[0] = stack[1] = stack[2] = (void*)-1;
    finalAddr                      = pushCS(&stack[2]);
    if (finalAddr != &stack[1]) printf("FinalAddr : %p should be %p\n", finalAddr, &stack[1]);
    printf("After push %%cs, value pushed is %p\n", stack[1]);

    stack[0] = stack[1] = stack[2] = (void*)0xaaaaaaaa;
    finalAddr                      = pushCS(&stack[2]);
    if (finalAddr != &stack[1]) printf("FinalAddr : %p should be %p\n", finalAddr, &stack[1]);
    printf("After push %%cs, value pushed is %p\n", stack[1]);

    printf("%%fs = 0x%04x\n", readFS());
    stack[0] = stack[1] = stack[2] = (void*)0xaaaaaaaa;
    finalAddr                      = pushFS(&stack[2]);
    if (finalAddr != &stack[1]) printf("FinalAddr : %p should be %p\n", finalAddr, &stack[1]);
    printf("After push %%fs, value pushed is 0x%04x\n", UINT32(stack[1]) & 0x0000ffff);

    return fails;
}

int main()
{
    int res   = FarCallTest();
    int fails = res != 5;

    printf("Res is 0x%x\n", res);

    fails += tmq();
    fails += tPushPop();

    return fails;
}
