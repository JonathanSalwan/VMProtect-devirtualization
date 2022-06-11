/*
 * Copyright (C) 2014-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * This test application should run instrumented with iarg_explicit_memory_ea tool.
 * This application should perform several basic instructions with explicit meory operands,
 * recording the instruction address for each important instruction.
 * This application will generate a call to checkVar for every important instruction.
 *  The tool iarg_explicit_memory_ea should check (by instrumenting the checkVar function)
 *  that it was able to catch all of the instructions operands.
  */
#include <cstdio>
#include <cstdlib>
#if defined(TARGET_WINDOWS)
#define EXPORT_SYM __declspec(dllexport)
#else
#define EXPORT_SYM
#endif

typedef void (*PF)(const char*, void*, void*);

//Check for LEA/MOV of global variables (intel64: address relative to rip, ia32: absolute address)
extern "C"
{
    int globalVar     = 8;
    int* dynVar       = NULL;
    void** autoVarPtr = NULL;
    void* lbl[9]      = {0};
#ifdef TARGET_WINDOWS
    void DoExplicitMemoryOps();
    //The following pointers are the exact address of the important LEA instructions
    void** lblPtr = lbl;
#endif
}

extern "C" EXPORT_SYM void checkVar(const char* name, void* pc, void* value)
{
    printf("%s %p not checked - probaby tool didn't instrument the checkVar function\n", name, value);
    exit(2);
}

int main()
{
    //Check for LEA/MOV of auto variables (relative to stack pointer)
    //The autoVar is void* because we want it to be in the same size as EAX/RAX depending on the arch.
    void* autoVar = (void*)4;

    //Check for LEA of dynamically allocated variables
    dynVar  = new int;
    *dynVar = 3;

    autoVarPtr = &autoVar;

    //Below are the important LEA and memory instructions that will be checked in the instrumenting tool:
#if defined(TARGET_WINDOWS)
    DoExplicitMemoryOps();
#else
#if defined(TARGET_IA32E)
    asm volatile("lbl1: leaq %1, %%rax; leaq lbl1(%%rip), %%rax; movq %%rax, %0" : "=m"(lbl[0]) : "m"(globalVar) : "%rax");
    asm volatile("lbl2: leaq %1, %%rax; leaq lbl2(%%rip), %%rax; movq %%rax, %0" : "=m"(lbl[1]) : "m"(autoVar) : "%rax");
    asm volatile("lbl3: leaq %1, %%rax; leaq lbl3(%%rip), %%rax; movq %%rax, %0" : "=m"(lbl[2]) : "m"(*dynVar) : "%rax");
    asm volatile("mov $0xcafebabe, %%rax; lbl4: leaq (%%rax), %%rax; leaq lbl4(%%rip), %%rax; movq %%rax, %0"
                 : "=m"(lbl[3])
                 :
                 : "%rax");
    asm volatile("lbl5: leaq (0xdeadbee), %%rax; leaq lbl5(%%rip), %%rax; movq %%rax, %0" : "=m"(lbl[4]) : : "%rax");
    asm volatile("lbl6: movq %1, %%rax; leaq lbl6(%%rip), %%rax; movq %%rax, %0" : "=m"(lbl[5]) : "m"(globalVar) : "%rax");
    asm volatile("lbl7: movq %%rax, %0; leaq lbl7(%%rip), %%rax; movq %%rax, %1" : "=m"(autoVar), "=m"(lbl[6])::"%rax");
    asm volatile("lbl8: leaq %%fs:(-8), %%rax; leaq lbl8(%%rip), %%rax; movq %%rax, %0" : "=m"(lbl[7]) : : "%rax");
    asm volatile("movq $0xdeadbeef, %%rax; lbl9: leaq %%fs:(%%rax), %%rax; leaq lbl9(%%rip), %%rax; movq %%rax, %0"
                 : "=m"(lbl[8])
                 :
                 : "%rax");
#elif defined(TARGET_IA32)
    asm volatile("lbl1: leal %1, %%eax; movl $lbl1, %0" : "=m"(lbl[0]) : "m"(globalVar) : "%eax");
    asm volatile("lbl2: leal %1, %%eax; movl $lbl2, %0" : "=m"(lbl[1]) : "m"(autoVar) : "%eax");
    asm volatile("lbl3: leal %1, %%eax; movl $lbl3, %0" : "=m"(lbl[2]) : "m"(*dynVar) : "%eax");
    asm volatile("mov $0xcafebabe, %%eax; lbl4: leal (%%eax), %%eax; movl $lbl4, %0" : "=m"(lbl[3]) : : "%eax");
    asm volatile("lbl5: leal (0xdeadbee), %%eax; movl $lbl5, %0" : "=m"(lbl[4]) : : "%eax");
    asm volatile("lbl6: movl %1, %%eax; movl $lbl6, %0" : "=m"(lbl[5]) : "m"(globalVar) : "%eax");
    asm volatile("lbl7: movl %%eax, %0; movl $lbl7, %1" : "=m"(autoVar), "=m"(lbl[6])::"%eax");
    asm volatile("lbl8: leal %%gs:(-8), %%eax; movl $lbl8, %0" : "=m"(lbl[7]) : : "%eax");
    asm volatile("movl $0xdeadbeef, %%eax; lbl9: leal %%gs:(%%eax), %%eax; movl $lbl9, %0" : "=m"(lbl[8]) : : "%eax");
#else
#error Unsupported machine architecture
#endif //defined(TARGET_IA32E)
#endif //defined(TARGET_WINDOWS)

    //Make sure checkVar function will not get in-lined by the compiler
    volatile PF checkVarFn = checkVar;

    checkVarFn("LEA for globalVar", lbl[0], &globalVar);
    checkVarFn("LEA for autoVar", lbl[1], autoVarPtr);
    checkVarFn("LEA for dynVar", lbl[2], dynVar);
    checkVarFn("LEA with base register", lbl[3], (void*)0xcafebabe);
    checkVarFn("LEA with immediate operand", lbl[4], (void*)0xdeadbee);
    checkVarFn("MOV from globalVar", lbl[5], &globalVar);
    checkVarFn("MOV to autoVar", lbl[6], autoVarPtr);
    checkVarFn("LEA with segment override, immediate", lbl[7], (void*)-8);
    checkVarFn("LEA with segment override, base reg", lbl[8], (void*)0xdeadbeef);
    return 0;
}
