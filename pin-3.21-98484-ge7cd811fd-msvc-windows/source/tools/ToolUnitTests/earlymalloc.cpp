/*
 * Copyright (C) 2008-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 */

#include <stdio.h>
#include <stdlib.h>
#include "pin.H"

class MyClass
{
  public:
    MyClass() { m_ptr = (char*)malloc(0x1000); }

    ~MyClass() { free(m_ptr); }

    void print() { printf("My pointer is %p\n", m_ptr); }

  private:
    char* m_ptr;
};

MyClass myClass;

/* ===================================================================== */

int main(int argc, char* argv[])
{
    // Initialize pin
    PIN_Init(argc, argv);

    myClass.print();

    // Start the program, never returns
    PIN_StartProgram();

    return 0;
}
