/*
 * Copyright (C) 2016-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <cstdio>
#include <cstring>
#include <unistd.h>

int main()
{
    char printf_string[] = "printing using printf\n";
    char write_string[]  = "printing using write\n";

    printf("%s", printf_string);
    fflush(stdout);

    write(STDOUT_FILENO, write_string, strlen(write_string));

    return 0;
}
