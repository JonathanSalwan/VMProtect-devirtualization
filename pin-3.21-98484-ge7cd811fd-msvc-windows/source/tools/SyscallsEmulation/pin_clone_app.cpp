/*
 * Copyright (C) 2016-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <sched.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cerrno>

/* Stack size for cloned child */
#define STACK_SIZE (1024 * 1024)

#define errExit(msg)        \
    do                      \
    {                       \
        perror(msg);        \
        exit(EXIT_FAILURE); \
    }                       \
    while (0)

/* Pipe for connection between main process and child */
int myPipe[2];

/* Start function for cloned child */
static int childFunc(void* arg)
{
    /* Close unused read side */
    close(myPipe[0]);

    /* Write to the pipe */
    write(myPipe[1], "child wrote!!", strlen("child wrote!!"));

    /* exit() closes the write side */
    exit(EXIT_SUCCESS);
}

/* main function */
int main(int argc, char* argv[])
{
    char* stack    = NULL;
    char* stackTop = NULL;
    char buf       = '\0';
    int oldStatus  = -1;
    pid_t pid      = -1;

    /* Allocate stack for child */
    stack = (char*)malloc(STACK_SIZE);
    if (NULL == stack)
    {
        errExit("malloc");
    }
    stackTop = stack + STACK_SIZE;

    /* Inialize pipe */
    if (-1 == pipe(myPipe))
    {
        errExit("pipe");
    }

    /* Set read side to fail if the write side still empty */
    oldStatus = fcntl(myPipe[0], F_GETFL);
    if (-1 == oldStatus)
    {
        errExit("fcntl");
    }
    fcntl(myPipe[0], F_SETFL, oldStatus | O_NONBLOCK);

    /* Create child */
    pid = clone(childFunc, stackTop, CLONE_VFORK, NULL);
    if (-1 == pid)
    {
        errExit("clone");
    }

    /* Close unused write side */
    close(myPipe[1]);

    /* Try to read from pipe, the child should already wrote to the pipe */
    if (-1 == read(myPipe[0], &buf, 1))
    {
        errExit("nonBlocking read failed!");
    }

    /* Close read side */
    close(myPipe[0]);

    free(stack);

    exit(EXIT_SUCCESS);
}
