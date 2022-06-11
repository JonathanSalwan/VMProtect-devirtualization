/*
 * Copyright (C) 2014-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <sys/time.h>
#include <stdio.h>
#include <string.h>

int main(int argv, char** argc, char** env)
{
    struct timeval tiv;

    gettimeofday(&tiv, NULL);
    printf("gettimeofday : %d\n", (int)tiv.tv_sec);

    gettimeofday(&tiv, NULL);
    printf("gettimeofday : %d\n", (int)tiv.tv_sec);

    gettimeofday(&tiv, NULL);
    printf("gettimeofday : %d\n", (int)tiv.tv_sec);

    char a[50] = {'5'};
    char b[50] = {'0'};

    printf("a : %s, b : %s\n", a, b);
    int cmp_result = strcmp(a, b);
    if (cmp_result > 0)
        printf("a>b\n");
    else if (cmp_result < 0)
        printf("a<b\n");
    else
        printf("a==b\n");

    return 0;
}
