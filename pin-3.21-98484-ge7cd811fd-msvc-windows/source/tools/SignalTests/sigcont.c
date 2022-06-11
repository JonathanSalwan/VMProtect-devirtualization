/*
 * Copyright (C) 2007-2007 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * Verify that Pin can handle an application that receives a SIGCONT.
 */

#include <signal.h>
#include <unistd.h>

int main()
{
    kill(getpid(), SIGCONT);
    return 0;
}
