/*
 * Copyright (C) 2011-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

// This little application tests calling application functions.
//
#include <stdio.h>
#include <limits.h>
#ifndef TARGET_WINDOWS
#include <stdlib.h>
#endif

#if defined(TARGET_WINDOWS)
#define EXPORT_SYM __declspec(dllexport)
#define FAST_CALL __fastcall
#define STD_CALL __stdcall
#include <windows.h>
#else
#include <inttypes.h>
#define EXPORT_SYM extern
#define FAST_CALL
#define STD_CALL
#endif

#ifdef TARGET_WINDOWS
typedef __int64 i64_type;
#endif

union union_64
{
    double _doub;
    unsigned char _uint8[8];
    i64_type i64;
} UNION_64;

EXPORT_SYM void Bar(i64_type param1, i64_type param2)
{
    int i;
    union union_64* ptr1 = (union union_64*)(&param1);
    union union_64* ptr2 = (union union_64*)(&param2);
    printf("\nHello from Bar!\n");

    printf("param1: ");
    for (i = 0; i < 8; i++)
    {
        printf(" %x ", ptr1->_uint8[i] & 0xff);
    }
    printf("\nparam2: ");
    for (i = 0; i < 8; i++)
    {
        printf(" %x ", ptr2->_uint8[i] & 0xff);
    }

    // verify expected values of replacement param to Bar
    if (ptr1->_uint8[0] != 0xdc)
    {
        printf("ERROR_app1\n");
        exit(-1);
    }
    if (ptr1->_uint8[1] != 0xac)
    {
        printf("ERROR_app2\n");
        exit(-1);
    }
    if (ptr1->_uint8[2] != 0xdc)
    {
        printf("ERROR_app3\n");
        exit(-1);
    }
    if (ptr1->_uint8[3] != 0xac)
    {
        printf("ERROR_app4\n");
        exit(-1);
    }
    if (ptr1->_uint8[4] != 0xdc)
    {
        printf("ERROR_app5\n");
        exit(-1);
    }
    if (ptr1->_uint8[5] != 0xac)
    {
        printf("ERROR_app6\n");
        exit(-1);
    }
    if (ptr1->_uint8[6] != 0xdc)
    {
        printf("ERROR_app7\n");
        exit(-1);
    }
    if (ptr1->_uint8[7] != 0xac)
    {
        printf("ERROR_app8\n");
        exit(-1);
    }

    if (ptr2->_uint8[0] != 0xed)
    {
        printf("ERROR_app9\n");
        exit(-1);
    }
    if (ptr2->_uint8[1] != 0xde)
    {
        printf("ERROR_appa\n");
        exit(-1);
    }
    if (ptr2->_uint8[2] != 0xad)
    {
        printf("ERROR_appb\n");
        exit(-1);
    }
    if (ptr2->_uint8[3] != 0xba)
    {
        printf("ERROR_appc\n");
        exit(-1);
    }
    if (ptr2->_uint8[4] != 0xed)
    {
        printf("ERROR_appd\n");
        exit(-1);
    }
    if (ptr2->_uint8[5] != 0xde)
    {
        printf("ERROR_appe\n");
        exit(-1);
    }
    if (ptr2->_uint8[6] != 0xad)
    {
        printf("ERROR_appf\n");
        exit(-1);
    }
    if (ptr2->_uint8[7] != 0xba)
    {
        printf("ERROR_appg\n");
        exit(-1);
    }
}
