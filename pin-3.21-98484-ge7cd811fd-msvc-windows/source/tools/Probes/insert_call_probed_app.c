/*
 * Copyright (C) 2010-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>

#ifdef TARGET_MAC
#define SEGNAME "__DATA,"
#else
#define SEGNAME
#endif

#define __annotation_init()            \
    asm(".section MyAnnot, \"wa\"\n"); \
    asm(".text")

#ifdef TARGET_IA32
#define __annotation_mark(LABEL) __asm__ __volatile__("." #LABEL ": .byte 0x0F, 0x1F, 0x44, 0x00, 0x00\n")

#define __annotation_add(LABEL, VALUE)        \
    asm(".pushsection " SEGNAME "MyAnnot\n\t" \
        ".long ." #LABEL ", " #VALUE "\n\t"   \
        ".popsection")

#else
#define __annotation_mark(LABEL) __asm__ __volatile__("." #LABEL ": .byte 0x0F, 0x1F, 0x80, 0x00, 0x00, 0x00, 0x00\n")

#define __annotation_add(LABEL, VALUE)        \
    asm(".pushsection " SEGNAME "MyAnnot\n\t" \
        ".quad ." #LABEL ", " #VALUE "\n\t"   \
        ".popsection")
#endif

// Use to function to write to the output file (stdout in this case).
// The test tool will call this function to make sure that ithe tool's
// output lines are interleaved with the test function output lines.
void write_line(char* line) { fprintf(stdout, "%s\n", line); }

// create the annotation section
__annotation_init();

void TestFunc()
{
    write_line("Before first notification");
    __annotation_mark(loc1);
    write_line("Before second notification");
    __annotation_mark(loc2);
    write_line("After second notification");
}

// add the annotations to the section
__annotation_add(loc1, 0x1234);
__annotation_add(loc2, 0x5678);

int main()
{
    write_line("Calling test function");
    TestFunc();
    write_line("After test function");
    return 0;
}
