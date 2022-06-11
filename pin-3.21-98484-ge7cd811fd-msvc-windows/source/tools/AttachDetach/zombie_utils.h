/*
 * Copyright (C) 2013-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <string>

using std::string;

#define EXPORT_SYM extern "C"

// Type of the test, indicates the following:
// 1. Which thread (main/secondary) is a zombie thread.
// 2. Is the thread turn into a zombie thread in the first time the first time Pin
//    reattach to the application.
// The tool needs to conduct some extra checks in case the test type is:
// TEST_TYPE_MAIN_THREAD_ZOMBIE_IN_SECOND_REATTACH. (in order to ensure that when Pin tries
// to reattach to the application the main thread is already turned into a zombie thread.
enum TEST_TYPE
{
    TEST_TYPE_DEFAULT = 0,                         // 0
    TEST_TYPE_MAIN_THREAD_ZOMBIE_IN_REATTACH,      // 1
    TEST_TYPE_SECONDARY_THREAD_ZOMBIE_IN_REATTACH, // 2
};

// The tool puts an analysis routine on this function in order to retrieve
// the type of the test.
EXPORT_SYM void NotifyTestType(TEST_TYPE testType = TEST_TYPE_DEFAULT);

// The tool puts an analysis routine on this function in order to retrieve
// the pid of the zombie thread
EXPORT_SYM void NotifyZombiePid(pid_t pid);

// Maximum pid length
static const int MAX_SIZE = 128;

// Check if a thread is a zombie thread.
// @param[in] tid - thread system id.
bool isZombie(pid_t tid);

// Check if Pin gave a message already that it can't attach to the application since the main thread is a zombie thread.
// @param[in] fileName-the message that Pin can't attach to the application will be redirected to this file.
bool NotifyUserPinUnableToAttach(string fileName);

/**************************************************
 * Possible exit status                           *
 **************************************************/

enum ExitType
{
    RES_SUCCESS = 0,         // 0
    RES_FORK_FAILED,         // 1
    RES_EXEC_FAILED,         // 2
    RES_LOAD_FAILED,         // 3
    RES_PIPE_CREATION_ERROR, // 4
    RES_PIPE_ERROR,          // 5
    RES_INVALID_ARGS         // 6
};
