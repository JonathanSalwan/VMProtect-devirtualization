/*
 * Copyright (C) 2011-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * This test checks a problem that occured while Pin was detaching from an application in probe mode. Pin spawns a
 * detach thread for doing the detach work and creates a child process for stopping the applicaiton while the detach
 * work is being done. The problem occured when the detach process exited and an application thread calling the
 * "wait" system call caught the exit status instead of the dedicated detach thread. This test replicates this
 * scenario:
 * The main thread creates a child process and a secondary thread, and then calls the wait system call.
 * The secondary thread tells Pin to detach. Once the detach is finished, it releases the child process and exits.
 * The child process waits to be released and exits.
 * The test fails if the main thread catches any child other than the one created from within the application.
 *
 * KNOWN LIMITATION: As of today (Jun 9, 2011), the parent of the application (which could also be an application
 *                   process), will receive notification when the detach-thread and the detach-process in Pin exit.
 *
 */

#include <cstdio>
#include <pthread.h>
#include <string>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <cstdlib>
#include <sched.h>
#include <unistd.h>

using std::string;

// return values for parent and child
enum retValues
{ // VALUE   DESCRIPTION                                             WHO RETURNS THIS

    RET_SUCCESS = 0,  //   0     everything is OK                                        both parent and child
    RET_PIPE_ERROR,   //   1     pipe creation failed                                    parent
    RET_FORK_ERROR,   //   2     fork system call failed                                 parent
    RET_READ_ERROR,   //   3     read system call failed                                 child
    RET_WAIT_ERROR,   //   4     wait system call failed with errno other than EINTR     parent
    RET_ILLEGAL_CHILD //   5     wait returned with pid other than the child             parent
};

const int READ  = 0;
const int WRITE = 1;

#ifdef TARGET_LINUX
#define WAITPID_FLAGS (__WCLONE | __WALL)
#elif defined(TARGET_MAC)
#define WAITPID_FLAGS 0
#endif

// This function is replaced by the tool. When it is called, it detaches Pin.
extern "C" void TellPinToDetach(unsigned long* updateWhenReady) { return; }

// The child waits for the secondary thread to release it from the read() system call.
// This will (hopefully) allow the parent to wait for any unexpceted child processes spawned by Pin.
int doChild(int fd[])
{
    close(fd[WRITE]);
    char buf[2];
    int res = RET_SUCCESS;
    if (read(fd[READ], buf, 1) < 0)
    { // Wait here until detach is complete
        perror("CHILD APP ERROR: read failed");
        res = RET_READ_ERROR;
    }
    close(fd[READ]);
    return res;
}

// The secondary thread detaches Pin and only then releases the child process by closing the pipe.
void* doSecondaryThread(void* fd)
{
    unsigned long pinDetached = false; // This will change to true by the tool after Pin has detached
    TellPinToDetach(&pinDetached);
    while (!pinDetached)
    {
        sched_yield();
    }
    close(((int*)fd)[WRITE]); // Close the WRITE end of the pipe to release the child
    return NULL;
}

// The parent spawns a secondary thread that synchronizes Pin's detach and the child's exit.
// After spawning the thread, the main thread waits until the child exits. If for any reason,
// it gets a different child process, an error is printed.
int doParent(pid_t child, int fd[])
{
    close(fd[READ]);
    int status;
    pid_t retCh = -1;
    pthread_t secTh;
    pthread_create(&secTh, 0, doSecondaryThread, fd);
    do
    {
        retCh = waitpid(0, &status, WAITPID_FLAGS);
        if (retCh < 0)
        { // handle system call error
            if (errno == EINTR)
            {
                continue;
            }
            perror("PARENT APP ERROR: wait failed");
            return RET_WAIT_ERROR;
        }
        else if (retCh != child)
        { // this means that we got a child created by Pin - not good!
            fprintf(stderr, "PARENT APP ERROR: wait returned with pid: %d, expecting child: %d\n", retCh, child);
            return RET_ILLEGAL_CHILD;
        }
    }
    while (!WIFEXITED(status));

    pthread_join(secTh, 0);

    return RET_SUCCESS;
}

int main()
{
    int fd[2];

    if (pipe(fd) != 0)
    {
        perror("PARENT APP ERROR: pipe creation failed");
        return RET_PIPE_ERROR;
    }

    pid_t child = fork();

    if (child < 0)
    {
        perror("PARENT APP ERROR: fork failed");
        return RET_FORK_ERROR;
    }

    if (child == 0)
    { // child's code
        return doChild(fd);
    }
    else
    { // parent's code
        return doParent(child, fd);
    }
}
