/*
 * Copyright (C) 2008-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*

compare 64 bit value od edx:eax with the destination memory operand.
  if ==
     store ecx:ebx in the destination memory operand and zf=1
  if !=
     store load the destination memory operand into edx:eax and zf=0

This test checks that the correct memory location is referenced when ebx is
explicitly used in the memory operand of the cmpxchg8b instruction.


*/

#include <stdio.h>

int cmpxchg8_with_explicit_ebx();

unsigned int eaxVal;
unsigned int edxVal;
unsigned char a[] = {0x1, 0xff, 0xff, 0xff, 0x2, 0xff, 0xff, 0xff};
int main()
{
    cmpxchg8_with_explicit_ebx();
    printf("eaxVal %x edxVal %x\n", eaxVal, edxVal);
    /*
    asm( "mov %ebp, %ebx");
    asm( "mov $8, %eax");
    asm( "cmpxchg8b 0x0(%ebx,%eax,1)");
    */
}
