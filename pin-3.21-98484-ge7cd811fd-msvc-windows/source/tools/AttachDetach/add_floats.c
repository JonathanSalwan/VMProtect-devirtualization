/*
 * Copyright (C) 2018-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/**
 * @file
 * An app which add 2 floating point number (expecting to use ADDSS).
 * This app is being used to test Pin_Detach() flow. More specifically test extended state is restored correctly after
 * Pin detached from application.
 */
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

int main(int argc, char* argv[])
{
    float a = 1.0f, b = 2.0f;

    // Detach should occur after performing this operation (somewhere after)
    float ret = a + b;
    printf("ret 1: %f\n", ret);

    ret = a + b;
    printf("ret 2: %f\n", ret);

    printf("Success\n");
    return 0;
}
