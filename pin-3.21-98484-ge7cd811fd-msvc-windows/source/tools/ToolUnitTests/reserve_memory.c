/*
 * Copyright (C) 2008-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <unistd.h>
#include <assert.h>

int main(int argc, char* argv[])
{
    FILE* f         = fopen(argv[1], "r");
    int access_flag = atoi(argv[2]);

    if (!f)
    {
        fprintf(stderr, "cannot open file %s\n", argv[1]);
        return 0;
    }

    long int low = 0, high = 0, size = 0;
    int tid;
    char desc[64];
    char* ptr = (char*)NULL;
    int i;

    while (!feof(f))
    {
        fscanf(f, "%lx %lx %s %d", &low, &high, desc, &tid);
        /*fprintf(stdout, "%lx %lx %s %d\n", low, high, desc, tid);*/
        if (feof(f)) break;

        size = high - low;
        fprintf(stdout, "%lx\n", size);
        assert(size > 0);

        for (i = 0; i < size; i += getpagesize())
        {
            ptr = (char*)(low + i);
            if (access_flag) *ptr = 0;
            fprintf(stdout, "%lx\n", (low + i));
        }
    }
    fclose(f);
    return 0;
}
