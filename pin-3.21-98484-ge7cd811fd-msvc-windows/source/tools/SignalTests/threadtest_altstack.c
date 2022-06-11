/*
 * Copyright (C) 2014-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <pthread.h>

stack_t main_sigstk;
FILE* log_fd;

void PrintAndCompareAlternateStack(const char* prefix, const stack_t* compare_stack)
{
    stack_t sigstk;

    int sigalt_result = sigaltstack(NULL, &sigstk);
    fprintf(log_fd, "%s: sigalt_result == %d {ss_sp=%p; ss_size=%d; ss_flags=%x}\n", prefix, sigalt_result, sigstk.ss_sp,
            (int)sigstk.ss_size, sigstk.ss_flags);
    if (sigalt_result < 0)
    {
        perror("ERROR printing alternate stack");
    }
    if (!sigstk.ss_sp)
    {
        printf("%s: NO alternate stack\n", prefix);
    }
    if (compare_stack)
    {
        printf("%s: current stack %s to compare_stack\n", prefix,
               ((sigstk.ss_sp == compare_stack->ss_sp) && sigstk.ss_size == compare_stack->ss_size) ? "equals" : "different");
    }
}

void* doThread(void* arg)
{
    PrintAndCompareAlternateStack("Thread", &main_sigstk);
    return NULL;
}

int main()
{
    int err;

    log_fd = fopen("./threadtest_altstack.log", "w");
    if (log_fd < 0) perror("Could not open log file");

    // set current alternate stack
    main_sigstk.ss_sp = malloc(SIGSTKSZ);

    if (main_sigstk.ss_sp == NULL)
    {
        printf("Could not allocate ss_sp\n");
        return -1;
    }
    main_sigstk.ss_size  = SIGSTKSZ;
    main_sigstk.ss_flags = 0;

    err = sigaltstack(&main_sigstk, NULL);
    if (err)
    {
        perror("Got error from sigaltstack");
    }
    PrintAndCompareAlternateStack("Main", NULL);

    pthread_t tid;
    err = pthread_create(&tid, NULL, doThread, NULL);
    if (err)
    {
        perror("Could not create a thread\n");
    }

    err = pthread_join(tid, NULL);
    if (err)
    {
        perror("Could not create a thread\n");
    }

    if (main_sigstk.ss_sp) free(main_sigstk.ss_sp);

    return 0;
}
