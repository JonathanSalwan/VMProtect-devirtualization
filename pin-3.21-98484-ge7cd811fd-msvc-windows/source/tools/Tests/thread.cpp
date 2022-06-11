/*
 * Copyright (C) 2010-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 * Check some threads interfaces.
 */

#include <sys/types.h>
#include <sys/wait.h>
#include "pin_tests_util.H"

OS_THREAD_ID father_id = INVALID_OS_THREAD_ID;

VOID AppStart(VOID* v)
{
    father_id = PIN_GetTid();
    PRINT_ME_AND_MY_FATHER();
    TEST(father_id != INVALID_OS_THREAD_ID, "PIN_GetTid failed");
}

VOID ThreadStart(THREADID threadIndex, CONTEXT* ctxt, INT32 flags, VOID* v)
{
    PRINT_ME_AND_MY_FATHER();
    // only the new thread should check it
    if (PIN_GetParentTid() != INVALID_OS_THREAD_ID)
    {
        TEST(PIN_GetParentTid() == father_id, "PIN_GetParentTid failed");
    }

    TEST(PIN_ThreadUid() != INVALID_PIN_THREAD_UID, "PIN_ThreadUid failed");
}

int main(INT32 argc, CHAR** argv)
{
    PIN_Init(argc, argv);

    PIN_AddApplicationStartFunction(AppStart, 0);
    PIN_AddThreadStartFunction(ThreadStart, 0);

    // Never returns
    PIN_StartProgram();

    return 0;
}
