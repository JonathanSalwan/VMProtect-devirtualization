/*
 * Copyright (C) 2011-2011 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <windows.h>

int main()
{
    RaiseException(0xC0000094, 0, 0, 0);
    return 0;
}
