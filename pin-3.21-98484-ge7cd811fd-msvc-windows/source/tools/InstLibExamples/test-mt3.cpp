/*
 * Copyright (C) 2007-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

// test-mt3.cpp -- a simple multithreaded test program. I added markers to
// this to trigger the controller.

// g++ -o test-mt3 test-mt3.cpp -lpthread

////////////////////////////////////////////////////////////////////////////
#include <cassert>
#include <string>
#include <stdio.h>
#include <iostream>
#include <ostream>
#include <iomanip>
#include <cstdlib>
#include <cctype>
#include <mmintrin.h>
#include <emmintrin.h>
#include <xmmintrin.h>

#include <pthread.h>
using std::cout;
using std::endl;
////////////////////////////////////////////////////////////////////////////
// DEFINES
////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
// TYPES
////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
// PROTOTYPES
////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv, char** envp);

////////////////////////////////////////////////////////////////////////////
// GLOBALS
////////////////////////////////////////////////////////////////////////////
extern "C"
{
    // These functions are markers. The symbols are used to control tracing.
    void marker_start_counting() { asm volatile(".byte 0xbb,0x11,0x22,0x33,0x44,0x64,0x67,0x90" : : : "ebx"); }
    void marker_stop_counting() { asm volatile(".byte 0xbb,0x11,0x22,0x33,0x55,0x64,0x67,0x90" : : : "ebx"); }

    void marker_emit_stats() {}

    void marker_zero_stats() {}
}

class THREAD_STATE_T
{
  public:
    int x;
    THREAD_STATE_T() {}
};

pthread_mutex_t my_mutex;

extern "C" void work1()
{
    int i;
    float x, y, z;
    __m128 m1  = _mm_load_ss(&x);
    __m128 m2  = _mm_load_ss(&y);
    __m128 sum = _mm_add_ss(m1, m2);
    for (i = 0; i < 50; i++)
    {
        __m128 sum = _mm_add_ss(m1, m2);
    }
    _mm_store_ss(&z, sum);
}
extern "C" void work2()
{
    int i;
    float x, y, z;
    __m128 m1  = _mm_load_ss(&x);
    __m128 m2  = _mm_load_ss(&y);
    __m128 sum = _mm_add_ss(m1, m2);
    for (i = 0; i < 50; i++)
    {
        __m128 sum = _mm_sub_ss(m1, m2);
    }
    _mm_store_ss(&z, sum);
}
extern "C" void work3()
{
    int i;
    float x, y, z;
    __m128 m1  = _mm_load_ss(&x);
    __m128 m2  = _mm_load_ss(&y);
    __m128 sum = _mm_add_ss(m1, m2);
    for (i = 0; i < 50; i++)
    {
        __m128 sum = _mm_mul_ss(m1, m2);
    }
    _mm_store_ss(&z, sum);
}
extern "C" void work4()
{
    int i;
    float x = 3, y = 1, z = 2;
    __m128 m1  = _mm_load_ss(&x);
    __m128 m2  = _mm_load_ss(&y);
    __m128 sum = _mm_add_ss(m1, m2);
    for (i = 0; i < 50; i++)
    {
        __m128 sum = _mm_div_ss(m1, m2);
    }
    _mm_store_ss(&z, sum);
}

static void (*work[4])() = {work1, work2, work3, work4};

void* start_routine(void* arg)
{
    marker_zero_stats();
    marker_start_counting();

    THREAD_STATE_T* thread_state = (THREAD_STATE_T*)arg;
    marker_emit_stats();

    for (unsigned int i = 0; i < 50; i++)
    {
        marker_zero_stats();
        marker_start_counting();

        (*work[thread_state->x % 4])();

        marker_stop_counting();
        marker_emit_stats();
    }
    return 0;
}

int main(int argc, char** argv, char** envp)
{
    pthread_attr_t attr;
    int nthreads                 = (argc == 2) ? atoi(argv[1]) : 4;
    THREAD_STATE_T* thread_state = new THREAD_STATE_T[nthreads];
    pthread_t* thread            = new pthread_t[nthreads];
    int r;

    r = pthread_mutex_init(&my_mutex, 0);
    assert(r == 0);

    r = pthread_attr_init(&attr);
    assert(r == 0);

    for (int i = 0; i < nthreads; i++)
    {
        thread_state[i].x = i;
        r                 = pthread_create(thread + i, &attr, start_routine, (void*)(thread_state + i));
        assert(r == 0);
    }

    r = pthread_mutex_lock(&my_mutex);
    assert(r == 0);
    cout << "I'm in the main thread!" << endl;
    r = pthread_mutex_unlock(&my_mutex);
    assert(r == 0);

    for (int i = 0; i < nthreads; i++)
    {
        r = pthread_join(thread[i], 0);
        assert(r == 0);
    }
    return 0;
}

////////////////////////////////////////////////////////////////////////////
