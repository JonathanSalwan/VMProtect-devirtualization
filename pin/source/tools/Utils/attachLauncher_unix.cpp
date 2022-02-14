/*
 * Copyright (C) 2015-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>

using std::cerr;
using std::cout;
using std::endl;
using std::string;
using std::stringstream;
using std::vector;

static void ParseArguments(const int argc, const char* argv[], vector< string >& pinCmd, vector< string >& appCmd)
{
    if (argc < 4)
    {
        cerr << "LAUNCHER ERROR: Too few arguments to the launcher. Expected at least the following:" << endl
             << "  <launcher> <pin> -- <app>" << endl;
        exit(1);
    }
    const string pin(argv[1]);
    if (string::npos == pin.find("pin"))
    {
        cerr << "LAUNCHER ERROR: Expected to find Pin as the first argument but found '" << pin << "' instead." << endl;
        exit(1);
    }
    pinCmd.push_back(pin);
    unsigned int i = 2;
    for (; i < argc; ++i)
    {
        const string token(argv[i]);
        if ("--" == token) break;
        pinCmd.push_back(token);
    }
    if (argc == i)
    {
        cerr << "LAUNCHER ERROR: Could not find the application delimiter '--'." << endl;
        exit(1);
    }
    for (++i; i < argc; ++i)
    {
        const string token(argv[i]);
        appCmd.push_back(token);
    }
}

// Verify application is ready to be attached by checking that "/proc/CHILD_PID/exe" exists
// and has read and execute permissions
static void VerifyAppReadyForAttach(pid_t child)
{
    stringstream childS;
    childS << child;
    const string file = "/proc/" + childS.str() + "/exe";

    int attempts = 0;

    while (1)
    {
        attempts++;
        errno         = 0;
        const int ret = access(file.c_str(), R_OK | X_OK);
        if (ret == 0)
        {
            break;
        }
        else if (attempts == 1000)
        {
            const string errorMsg = "LAUNCHER: Application not ready to be attached, need execute and read access to " + file +
                                    ", errno=" + string(strerror(errno)) + "\n";
            perror(errorMsg.c_str());
            kill(child, SIGKILL);
            exit(1);
        }
        usleep(1000);
    }
}

static pid_t LaunchApp(const vector< string >& appCmd)
{
    // Create the synchronization pipe. This is used to make sure that the launcher continues only after the child has
    // successfully execed to the application.
    int fd[2];
    if (pipe(fd) != 0)
    {
        perror("LAUNCHER ERROR: Pipe creation failed");
        exit(1);
    }

    // Prepare the argument list.
    const unsigned int appArgc = appCmd.size();
    char** appArgv             = new char*[appArgc + 1]; // additional slot for the NULL terminator
    for (unsigned int i = 0; i < appArgc; ++i)
    {
        appArgv[i] = strdup(appCmd[i].c_str());
    }
    appArgv[appArgc] = NULL; // add the NULL terminator

    // Launch the application.
    const pid_t child = fork();
    if (-1 == child)
    {
        perror("LAUNCHER ERROR: Failed to fork the application process");
        exit(1);
    }
    else if (0 == child)
    {
        // In the child process.
        close(fd[0]); // close the read end of the pipe, the write end will be automatically closed upon a successful exec
        int fdflags = fcntl(fd[1], F_GETFD); // get the file descriptor flags for the write end of the pipe
        if (fdflags < 0)
        {
            perror("LAUNCHER ERROR: Failed to read the file descriptor flags");
            exit(1);
        }
        fdflags |= FD_CLOEXEC;                  // this will set the write end of the pipe to be closed by the exec system call
        if (fcntl(fd[1], F_SETFD, fdflags) < 0) // set the new flags
        {
            perror("LAUNCHER ERROR: Failed to set the file descriptor flags");
            exit(1);
        }

        cout << endl << "LAUNCHER: Running the application with pid [" << getpid() << "]:" << endl << appArgv[0];
        for (unsigned int i = 1; NULL != appArgv[i]; ++i)
        {
            cout << " " << appArgv[i];
        }
        cout << endl;
        const int res = execvp(appArgv[0], appArgv); // does not return on success
        perror("LAUNCHER ERROR: In the child process. Failed to execute the application");
        exit(1);
    }

    // In the parent process.
    close(fd[1]); // close the write end of the pipe since it is not used
    char buf[2] = {0};
    if (read(fd[0], buf, 1) < 0) // wait here until the child execs to the application
    {
        perror("LAUNCHER: read from the pipe failed");
        kill(child, SIGKILL);
        exit(1);
    }
    close(fd[0]); // close the read end of the pipe now that we're done with it

    VerifyAppReadyForAttach(child);

    return child;
}

static pid_t LaunchPin(const vector< string >& pinCmd, const pid_t appPid)
{
    // Prepare the argument list.
    const unsigned int pinArgc = pinCmd.size();
    char** pinArgv             = new char*[pinArgc + 3]; // two additional slots: "-pid <appPid>" and one for the NULL terminator
    pinArgv[0]                 = strdup(pinCmd[0].c_str());

    // Add the attach arguments.
    pinArgv[1] = "-pid";
    stringstream appPidStrm; // prepare the application's pid as a string
    appPidStrm << appPid;
    pinArgv[2] = strdup(appPidStrm.str().c_str());

    // Add the rest of the arguments for Pin (if they exist).
    for (unsigned int origArgs = 1, newArgs = 3; origArgs < pinArgc; ++origArgs, ++newArgs)
    {
        pinArgv[newArgs] = strdup(pinCmd[origArgs].c_str());
    }
    pinArgv[pinArgc + 2] = NULL; // add the NULL terminator

    // Launch Pin.
    const pid_t child = fork();
    if (-1 == child)
    {
        perror("LAUNCHER ERROR: Failed to fork the Pin process");
        exit(1);
    }
    else if (0 == child)
    {
        // In the child process.
        cout << endl << "LAUNCHER: Running Pin:" << endl << pinArgv[0];
        for (unsigned int i = 1; NULL != pinArgv[i]; ++i)
        {
            cout << " " << pinArgv[i];
        }
        cout << endl << endl;
        const int res = execvp(pinArgv[0], pinArgv); // does not return on success
        perror("LAUNCHER ERROR: In the child process. Failed to execute Pin");
        exit(1);
    }

    // In the parent process.
    return child;
}

static void WaitForPin(const pid_t pinPid, const pid_t appPid)
{
    int pinStatus = 0;
    if (pinPid != waitpid(pinPid, &pinStatus, 0))
    {
        perror("LAUNCHER ERROR: Encountered an error while waiting for Pin to exit");
        kill(pinPid, SIGKILL);
        kill(appPid, SIGKILL);
        exit(1);
    }
    if (!WIFEXITED(pinStatus))
    {
        cerr << "LAUNCHER ERROR: The Pin process sent a notification to the launcher without exiting." << endl;
        kill(pinPid, SIGKILL);
        kill(appPid, SIGKILL);
        exit(1);
    }
    else
    {
        int pinCode = WEXITSTATUS(pinStatus);
        if (0 != pinCode)
        {
            cerr << "LAUNCHER ERROR: Pin exited with an abnormal return value: " << pinCode << endl;
            kill(appPid, SIGKILL);
            exit(pinCode);
        }
    }
}

static void WaitForApp(const pid_t appPid)
{
    int appStatus = 0;
    if (appPid != waitpid(appPid, &appStatus, 0))
    {
        perror("LAUNCHER ERROR: Encountered an error while waiting for the application to exit");
        kill(appPid, SIGKILL);
        exit(1);
    }
    if (!WIFEXITED(appStatus))
    {
        cerr << "LAUNCHER ERROR: The application sent a notification to the launcher without exiting." << endl;
        kill(appPid, SIGKILL);
        exit(1);
    }
    else
    {
        int appCode = WEXITSTATUS(appStatus);
        if (0 != appCode)
        {
            cerr << "LAUNCHER ERROR: The application exited with an abnormal return value: " << appCode << endl;
            exit(appCode);
        }
    }
}

int main(const int argc, const char* argv[])
{
    // Parse the given command line and break it down to Pin and application command lines.
    vector< string > pinCmd;
    vector< string > appCmd;
    ParseArguments(argc, argv, pinCmd, appCmd);

    // Launch the application.
    const pid_t appPid = LaunchApp(appCmd);

    // Launch Pin and attach to the application.
    const pid_t pinPid = LaunchPin(pinCmd, appPid);

    // Wait for Pin to return.
    WaitForPin(pinPid, appPid);

    // Wait for the application to return.
    WaitForApp(appPid);

    // Done.
    return 0;
}
