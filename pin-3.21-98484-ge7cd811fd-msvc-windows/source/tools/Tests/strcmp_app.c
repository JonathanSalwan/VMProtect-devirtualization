/*
 * Copyright (C) 2013-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <string.h>
#include <stdio.h>

char str1[] = "foo";
char str2[] = "foo";
char str3[] = "not_foo";

int main(int argc, char* argv[])
{
    char* p_str2 = str2;
    char* p_str3 = str3;

    if (strcmp(str1, p_str2) == 0) fprintf(stderr, " string equals to foo\n");
    if (strcmp(str1, p_str3) == 0) fprintf(stderr, " string isn't equal to foo\n");
    return 0;
}
