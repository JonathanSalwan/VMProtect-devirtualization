/*
 * Copyright (C) 2015-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

// Application that when running under Pin, may reproduce a bug in macOS* kernel that
// causes wait*() calls to inproperly fail with ECHILD.

#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <unistd.h>
#include <iostream>
#include <string>

using std::cerr;
using std::cout;
using std::endl;
using std::string;

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
        return 1;
    }
    else
    {
        int status;
        pid_t ret;
        while (0 == (ret = waitpid(pid, &status, WNOHANG)) || (ret < 0 && errno == EINTR))
            ;
        if (ret < 0)
        {
            cout << "waitpid() failed with errno " << errno << endl;
            return 2;
        }
        else if (status != 0)
        {
            cout << "Parent report: Child process failed. Status of the child process is " << WEXITSTATUS(status) << endl;
            return 3;
        }
        else
            cout << "Parent report: Child process exited successfully" << endl;
    }
    pid = fork();
    if (pid == 0)
    {
        // child process
        execv(childArgvArray[0], childArgvArray);
        cout << "Execve failed " << argv[1] << " " << argv[2] << " " << argv[3] << endl;
        return 4;
    }
    else
    {
        siginfo_t siginfo;
        pid_t ret;
        do
        {
            siginfo.si_pid = 0;
        }
        while ((0 == (ret = waitid(P_PID, pid, &siginfo, WEXITED | WNOHANG)) && siginfo.si_pid == 0) ||
               (ret < 0 && errno == EINTR));
        if (ret < 0)
        {
            cout << "waitid() failed with errno " << errno << endl;
            return 5;
        }
        else if (siginfo.si_status != 0)
        {
            cout << "Parent report: Child process failed. Status of the child process is " << WEXITSTATUS(siginfo.si_status)
                 << endl;
            return 6;
        }
        else
            cout << "Parent report: Child process exited successfully" << endl;
    }
    return 0;
}
