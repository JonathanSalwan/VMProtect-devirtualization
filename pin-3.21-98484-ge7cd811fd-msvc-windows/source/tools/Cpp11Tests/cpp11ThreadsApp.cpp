/*
 * Copyright (C) 2014-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <iostream>
#include <thread>
#include <vector>
using std::cout;
using std::endl;
using std::thread;
using std::vector;

int num_threads = 4;
int vector_size = 1024;

void thread_func(int pid, vector< int >& v, int scalar, int L, int R)
{
    cout << "Thread " << pid << endl;
    for (int i = L; i < R; i++)
    {
        v[i] = scalar;
    }
}

bool verify_vector(const vector< int > v, int scalar)
{
    for (int i = 0; i < vector_size; i++)
    {
        if (v[i] != scalar)
        {
            return false;
        }
    }
    return true;
}

int main()
{
    std::thread threads[num_threads];
    std::vector< int > v(vector_size);
    int scalar = 50;
    int chunk  = vector_size / num_threads;

    for (int i = 0; i < num_threads; ++i)
    {
        threads[i] = thread(thread_func, i, std::ref(v), scalar, i * chunk, (i + 1) * chunk);
    }

    for (int i = 0; i < num_threads; i++)
    {
        threads[i].join();
    }

    if (!verify_vector(v, scalar))
    {
        cout << "Found a cell not initialized! " << endl;
    }
    else
    {
        cout << "All vector is initialized to scalar" << endl;
    }

    //restarting threads to initialize vectore to new scalar
    scalar = 20;
    for (int i = 0; i < num_threads; ++i)
    {
        threads[i] = thread(thread_func, i, std::ref(v), scalar, i * chunk, (i + 1) * chunk);
    }

    for (int i = 0; i < num_threads; i++)
    {
        threads[i].join();
    }

    if (!verify_vector(v, scalar))
    {
        cout << "Found a cell not initialized! " << endl;
    }
    else
    {
        cout << "All vector is initialized to scalar" << endl;
    }

    return 0;
}
