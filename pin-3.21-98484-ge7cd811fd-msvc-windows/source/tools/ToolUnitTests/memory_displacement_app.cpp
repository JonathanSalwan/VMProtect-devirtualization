/*
 * Copyright (C) 2013-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
Application for testing the correctness of "INS_MemoryDisplacement(INS ins)". 
INS_MemoryDisplacement computes the memory displacement, which is a sign number. 
Tested only on 64 Linux architecture.
When running the application with the tool: "memory_displacement.cpp", the output, which is the displacement value, should be -24.
When running the application natively( without the tool), the output for the IA-32 architecture, which is the displacement value, should be 4.
When running the application natively( without the tool), the output for the IA-32 architecture, which is the displacement value, should be 7.
*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

template< class T > void allocateBuffer(T buffer[])
{
    T i;
    for (i = 0; i < 20; i++)
        buffer[i] = i;
}

extern "C" int neg_disp(void* p);

enum ExitType
{
    RES_SUCCESS = 0,              // 0
    RES_INVALID_ARGS,             // 1
    RES_INVALID_DISPLACEMENT_TYPE // 2
};

/*
    Expected argv arguments:
    [1] Displacement type
*/
int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "invalid number of arguments %d, expecting only one\n", argc);
        fflush(stderr);
        exit(RES_INVALID_ARGS);
    }

    int ans;
    switch (atoi(argv[1]))
    {
        case 0:
        {
            int64_t buffer_int64[20], disp = 10;
            allocateBuffer< int64_t >(buffer_int64);

            //neg_disp computes the memory displacement
            ans = neg_disp(buffer_int64 + disp);
            printf("n: %d\n", ans);
            break;
        }
        case 1:
        {
            uint64_t buffer_uint64[20], disp = 10;
            allocateBuffer< uint64_t >(buffer_uint64);

            //neg_disp computes the memory displacement
            ans = neg_disp(buffer_uint64 + disp);
            printf("n: %d\n", ans);
            break;
        }
        case 2:
        {
            int32_t buffer_int32[20], disp = 10;
            allocateBuffer< int32_t >(buffer_int32);

            //neg_disp computes the memory displacement
            ans = neg_disp(buffer_int32 + disp);
            printf("n: %d\n", ans);
            break;
        }

        default:
            exit(RES_INVALID_DISPLACEMENT_TYPE);
            break;
    }

    return RES_SUCCESS;
}
