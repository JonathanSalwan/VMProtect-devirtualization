/*
 * Copyright (C) 2016-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#define __ISO_C_VISIBLE 1999
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdlib.h>
#include <execinfo.h>
#include <iostream>
using std::cout;
using std::endl;

extern "C" void qux() { cout << "qux" << endl; }

extern "C" void baz()
{
    void* buf[128];
    int nptrs = backtrace(buf, sizeof(buf) / sizeof(buf[0]));
    assert(nptrs > 0);
    char** bt = backtrace_symbols(buf, nptrs);
    assert(NULL != bt);
    for (int i = 0; i < nptrs; i++)
    {
        cout << bt[i] << endl;
    }
    free(bt);
}

extern "C" void bar()
{
    baz();
    qux();
}

extern "C" __attribute__((visibility("default"))) void foo();

extern "C" void foo()
{
    bar();
    qux();
}
