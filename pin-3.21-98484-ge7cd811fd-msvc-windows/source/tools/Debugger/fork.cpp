/*
 * Copyright (C) 2008-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

static void DoChild();
static void DoCommon();

int main()
{
    pid_t pid = fork();
    if (pid != 0)
        waitpid(pid, 0, 0);
    else
        DoChild();
    DoCommon();
    return 0;
}

int Glob = 0;

static void DoChild()
{
    Glob++;
}

static void DoCommon()
{
    /* debugger places a breakpoint here */
}
