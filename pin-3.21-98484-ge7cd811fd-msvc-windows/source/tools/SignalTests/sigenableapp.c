/*
 * Copyright (C) 2007-2007 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * This is a contrived application that works with the "sigenable" tool.
 */

#include <signal.h>
#include <unistd.h>

extern void NotTraced();
extern void IsTraced();

int main()
{
    /*
     * These function calls should not be traced
     */
    IsTraced();
    NotTraced();

    /*
     * This signal is caught by the tool and enables instrumentation.
     * More commonly, the application wouldn't send the signal, but the
     * user would type "kill -USR2 <pid>" at the command prompt while
     * the application was running under Pin.
     */
    kill(getpid(), SIGUSR2);

    /*
     * This call is traced by Pin.
     */
    IsTraced();
    return 0;
}
