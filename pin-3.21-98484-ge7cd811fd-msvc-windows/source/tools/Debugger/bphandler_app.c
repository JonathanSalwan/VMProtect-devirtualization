/*
 * Copyright (C) 2014-2014 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>

int ToolControlled()
{
    return 1;
}

int DeleteToolControlled()
{
    return 1;
}

int ReInsertToolControlled()
{
    return 1;
}

int ReActivateToolControlled()
{
    return 1;
}

int main()
{
    int res = 0;
    res += ToolControlled(); // Should stop by the tool
    res += DeleteToolControlled();
    res += ToolControlled(); // Should not be stopped at all
    res += ReInsertToolControlled();
    res += ToolControlled(); // Should stop by the tool
    res += ReActivateToolControlled();
    res += ToolControlled(); // Should stop by PinADX
    return 7 - res;
}
