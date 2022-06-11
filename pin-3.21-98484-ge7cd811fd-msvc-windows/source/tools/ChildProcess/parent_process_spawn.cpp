/*
 * Copyright (C) 2008-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

// Application that creates new process using posix_spawn

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <spawn.h>
#include <iostream>
#include <string>

extern char** environ;

using std::cerr;
using std::cout;
using std::endl;
using std::string;

//Wait for a process completion
//Verify it returned the expected exit code

int main(int argc, char* argv[])
{
    posix_spawnattr_t attr;
    posix_spawn_file_actions_t file_actions;
    pid_t pid;

    posix_spawn_file_actions_init(&file_actions);
    posix_spawnattr_init(&attr);
    int err = posix_spawn(&pid, argv[1], &file_actions, &attr, argv + 1, environ);
    if (err != 0)
    {
        // child process
        cout << "posix_spawn failed with errno " << err << ": " << argv[1] << " " << argv[2] << " " << argv[3] << endl;
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

    posix_spawnattr_destroy(&attr);
    posix_spawn_file_actions_destroy(&file_actions);
    return 0;
}
