/*
 * Copyright (C) 2014-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <iostream>
#include <cstring>
using std::cout;
using std::endl;

#if defined(TARGET_WINDOWS)
#define EXPORT_SYM __declspec(dllexport)
#else
#define EXPORT_SYM
#endif

extern "C" EXPORT_SYM const char* world() { return "world"; }

extern "C" EXPORT_SYM const char* helloX(char* buf)
{
    strcpy(buf, "hello ");
    strcat(buf, world());
    return buf;
}

extern "C" EXPORT_SYM int main()
{
    char buf[128];
    cout << helloX(buf) << endl;
    return 0;
}
