/*
 * Copyright (C) 2008-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

// Application that creates new process

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <iostream>
#include <string>
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
        // child process
        execv(childArgvArray[0], childArgvArray);
        cout << "Execve failed " << argv[1] << " " << argv[2] << " " << argv[3] << endl;
    }
    else
    {
        int status;
        waitpid(pid, &status, 0);
        if (status != 0)
            cout << "Child report: Grand child process failed with status " << WEXITSTATUS(status) << endl;
        else
            cout << "Child report: Grand child process exited successfully" << endl;
    }
    return 0;
}
