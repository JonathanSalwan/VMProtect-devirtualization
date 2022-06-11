/*
 * Copyright (C) 2011-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <string.h>

int main()
{
    char buff0[100];
    char buff1[] = "My buffer source";

    memcpy(&buff0[0], &buff1[0], strlen(buff1));
    memcpy(&buff0[0], &buff1[0], strlen(buff1));

    memmove(&buff0[0], &buff1[0], strlen(buff1));
    memmove(&buff0[0], &buff1[0], strlen(buff1));

    return 0;
}
