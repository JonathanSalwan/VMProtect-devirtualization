/*
 * Copyright (C) 2012-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <windows.h>
#include <stdio.h>

extern "C" __declspec(dllexport) void maina();

int main(int argc, char* argv[])
{
    maina();
    return (0);
}
