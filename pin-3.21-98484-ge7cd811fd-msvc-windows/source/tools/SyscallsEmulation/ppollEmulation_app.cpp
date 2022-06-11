/*
 * Copyright (C) 2013-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * There are three tests that use this application:
 * 1. ppollEmulationSuccessful.test- tests the correctness of the emulation of a successful ppoll system call.
 * 2. ppollEmulationTimeOut.test- tests the correctness of the emulation of a ppoll system call
 *    which was interrupted by a time out.
 * 3. ppollEmulationSignalInterrupt.test- tests the correctness of the emulation of a ppoll system call
 *    which was interrupted by a signal.
 * 4. ppollEmulationNullSigmask.test - tests that Pin properly handles a NULL signal mask passed to the ppoll
 *    system call.
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <poll.h>
#include <setjmp.h>
#include <errno.h>
#include <sys/stat.h>

#define MSG "MSG\n"

volatile int istimer = 0;
pthread_t main_th;
volatile bool stop      = false;
const char* logFileName = NULL;
const char* pipeName    = NULL;
int fdMainThread        = -1;

enum ExitType
{
    RES_SUCCESS = 0,         // 0
    RES_OPEN_FILE_ERROR,     // 1
    RES_MAKE_PIPE_ERROR,     // 2
    RES_INVALID_ARGS,        // 3
    RES_OPEN_FD_ERROR,       // 4
    RES_WRITE_TO_FD_ERROR,   // 5
    RES_WRONG_SIGNAL_ERROR,  // 6
    RES_THREADCREATE_FAILED, // 7
    RES_SIGACTION_FAILED,    // 8
    RES_ERROR                // 9
};

void* thread_handler(void* arg)
{
    int fdWriteThread = open(pipeName, O_RDWR);
    if (fdWriteThread < 0)
    {
        perror("Failed to open fd");
        exit(RES_OPEN_FD_ERROR);
    }
    for (unsigned int i = 0; i < 10; i++)
    {
        sleep(10);
        int ret = write(fdWriteThread, MSG, sizeof(MSG));
        if (ret < 0)
        {
            perror("Failed to write to fd");
            close(fdWriteThread);
            exit(RES_WRITE_TO_FD_ERROR);
        }
    }
    stop = true;
    close(fdWriteThread);
    return NULL;
}

void* timer_handler(void* arg)
{
    struct timespec tm;
    for (unsigned int i = 0; i < 100; i++)
    {
        tm.tv_sec  = 0;
        tm.tv_nsec = 250 * 1000 * 1000;
        nanosleep(&tm, NULL);
        pthread_kill(main_th, SIGALRM);
    }
    return NULL;
}

void sig_handler(int signo, siginfo_t* info, void* context)
{
    if (signo == SIGALRM)
    {
        istimer = 1;
    }
    else
    {
        printf("Signal %d received\n", signo);
        exit(RES_WRONG_SIGNAL_ERROR);
    }
}

void* do_nothing(void*)
{
    while (1)
        sleep(5);
    return NULL;
}

void poll_exit()
{
    close(fdMainThread);
    remove(pipeName);
}

/*
    Expected argv arguments:
    [1] testType
    [2] log file name
    [3] pipe name
*/
int main(int argc, char* argv[])
{
    if (argc != 4)
    {
        fprintf(stderr, "Not enough arguments\n");
        fflush(stderr);
        return RES_INVALID_ARGS;
    }

    int testType = atoi(argv[1]);
    logFileName  = argv[2];
    FILE* pFile  = fopen(logFileName, "w");
    if (pFile == NULL)
    {
        perror("Error opening file");
        return RES_OPEN_FILE_ERROR;
    }

    pipeName = argv[3];
    pthread_t th1;
    sigset_t ppollSigmask;
    sigemptyset(&ppollSigmask);
    sigset_t* ppollSigmaskPtr = &ppollSigmask;
    struct pollfd list;
    char buf[20];
    struct sigaction sigact;
    struct timespec tem;

    int mode = S_IWGRP | S_IWOTH | S_IWUSR | S_IRGRP | S_IROTH | S_IRUSR;
    if (mkfifo(pipeName, mode) < 0)
    {
        perror("mkfifo");
        return RES_MAKE_PIPE_ERROR;
    }

    chmod(pipeName, mode);
    fdMainThread = open(pipeName, O_RDWR);

    if (fdMainThread < 0)
    {
        perror("Failed to open fd");
        return RES_OPEN_FD_ERROR;
    }
    atexit(poll_exit);
    main_th = pthread_self();

    switch (testType)
    {
        case 4:
            ppollSigmaskPtr = NULL;
            // fallthrough to case 1
        case 1:
        {
            if (pthread_create(&th1, NULL, thread_handler, 0) != 0)
            {
                printf("pthread_create error\n");
                return RES_THREADCREATE_FAILED;
            }
            tem.tv_sec  = 60;
            tem.tv_nsec = 0;
            break;
        }
        case 2:
        {
            if (pthread_create(&th1, NULL, timer_handler, 0) != 0)
            {
                printf("pthread_create error\n");
                return RES_THREADCREATE_FAILED;
            }
            tem.tv_sec  = 60;
            tem.tv_nsec = 0;
            sigaddset(&ppollSigmask, SIGQUIT);
            int signalsToHandle[] = {SIGINT, SIGTERM, SIGALRM};
            for (unsigned int i = 0; i < sizeof(signalsToHandle) / sizeof(int); i++)
            {
                sigact.sa_flags     = SA_SIGINFO | SA_NODEFER;
                sigact.sa_sigaction = sig_handler;
                sigemptyset(&sigact.sa_mask);
                if (sigaction(signalsToHandle[i], &sigact, 0) < 0)
                {
                    perror("sigaction failed");
                    return RES_SIGACTION_FAILED;
                }
            }
            break;
        }
        case 3:
        {
            if (pthread_create(&th1, NULL, do_nothing, 0) != 0)
            {
                printf("pthread_create error\n");
                return RES_THREADCREATE_FAILED;
            }
            tem.tv_sec  = 0;
            tem.tv_nsec = 250 * 1000 * 1000;
            break;
        }
        default:
            // do nothing
            break;
    }

    while (!stop)
    {
        usleep(1000);
        list.fd     = fdMainThread;
        list.events = (POLLIN | POLLHUP | POLLERR | POLLNVAL);
        int ret     = ppoll(&list, 1, &tem, ppollSigmaskPtr);

        if (ret == 0)
        {
            fprintf(pFile, "The call timed out and no file descriptors were ready\n");
            stop = true;
        }
        else if (ret < 0)
        {
            if ((errno == EINTR) && istimer)
            {
                istimer = 0;
                fprintf(pFile, "sigalarm interrupted the ppoll system call\n");
                stop = true;
            }
            else
            {
                fprintf(stderr, "received wrong signal\n");
                return RES_WRONG_SIGNAL_ERROR;
            }
        }
        else if (list.revents & POLLIN)
        {
            fprintf(pFile, "PPOLL succeeded:%d\n", (int)ret);
            int count = read(fdMainThread, buf, 20);
            if (count < 0)
                perror("read");
            else
                fprintf(pFile, "ppoll succeeded\n");
            ;
            stop = true;
        }
        else
        {
            fprintf(stderr, "other error\n");
            return RES_ERROR;
        }
    }

    return RES_SUCCESS;
}
