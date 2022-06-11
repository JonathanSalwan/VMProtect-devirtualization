/*
 * Copyright (C) 2008-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

// Verify that Pin does not crash if the application terminates with a
// fatal synchronous signal.

int main()
{
    int* p = (int*)0x9;
    *p     = 8;
    return 0;
}
