/*
 * Copyright (C) 2008-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 * In addition to what this test checks, we also check these:
 * - Checking if an environment variable which is set after Pin took over the application is being passed to
 *   the application which will be executed by the current application
 *   Application is executed from child (unlike follow_child_app1.cpp)
 */

// Application that creates new process

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <stdlib.h>
using std::cout;
using std::endl;

//Wait for a process completion
//Verify it returned the expected exit code

int main(int argc, char* argv[])
{
    char* childArgvArray[5];
    childArgvArray[0] = argv[1];
    childArgvArray[1] = argv[2];
    childArgvArray[2] = argv[3];
    childArgvArray[3] = argv[4];
    childArgvArray[4] = argv[5];

    pid_t pid = fork();
    if (pid == 0)
    {
        // Checking if an environment variable which is set after Pin took over the application is being passed to
        // the application we're about to execute below (as expected)
        setenv("ParentEnv", "1", 1);

        // child process
        execv(childArgvArray[0], childArgvArray);
        cout << "Execve failed " << argv[1] << " " << argv[2] << " " << argv[3] << endl;
    }
    else
    {
        int status;
        waitpid(pid, &status, 0);
        if (status != 0)
            cout << "Parent report: Child process failed. Status of the child process is " << WEXITSTATUS(status) << endl;
        else
            cout << "Parent report: Child process exited successfully" << endl;
    }
    return 0;
}
