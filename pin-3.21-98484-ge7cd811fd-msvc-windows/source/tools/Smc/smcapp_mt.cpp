/*
 * Copyright (C) 2008-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 *  An example of multi-threaded SMC application. 
 */
#include "smc_util.h"
#include "../Utils/sys_memory.h"
#include "../Utils/thread_pool.h"

/*!
 * Execute clones of the specified function object in all threads of the thread pool.
 * @return number of threads in which the object was executed successfully 
 */
static unsigned long ExecuteInAllThreads(THREAD_POOL* tPool, const FUNC_OBJ& funcObj)
{
    const unsigned long numThreads = tPool->NumThreads();
    unsigned long startedThreads   = 0;
    unsigned long succeededThreads = 0;

    for (unsigned long tid = 0; tid < numThreads; ++tid, ++startedThreads)
    {
        FUNC_OBJ* runObj = funcObj.Clone();
        if (!tPool->Start(tid, runObj))
        {
            cerr << "Thread " << tid << ": " << runObj->Name() << ": start failed" << endl;
            break;
        }
    }

    for (unsigned long tid = 0; tid < startedThreads; ++tid)
    {
        FUNC_OBJ* runObj = static_cast< FUNC_OBJ* >(tPool->Wait(tid));
        if (runObj == 0)
        {
            cerr << "Thread " << tid << ": " << funcObj.Name() << ": wait failed" << endl;
            continue;
        }

        cerr << "Thread " << tid << ": " << runObj->Name() << ": " << runObj->ErrorMessage() << endl;
        if (runObj->Status())
        {
            ++succeededThreads;
        }
        delete runObj;
    }

    return succeededThreads;
}

/*!
 * The main procedure of the application.
 */
int main(int argc, char* argv[])
{
    void* dynamicBuffer;
    dynamicBuffer = MemAlloc(PI_FUNC::MAX_SIZE, MEM_READ_WRITE_EXEC);
    if (dynamicBuffer == 0)
    {
        cerr << "MemAlloc failed" << endl;
        return 1;
    }

    const unsigned long numThreads = 3;
    THREAD_POOL threadPool;

    cerr << "Create " << numThreads << " threads" << endl;
    if (threadPool.Create(numThreads) != numThreads)
    {
        cerr << "Thread creation failed" << endl;
        return 1;
    }
    cerr << "All threads are created" << endl;

    for (int i = 0; i < 3; ++i)
    {
        unsigned long succeededThreads;
        {
            FOO_FUNC fooFunc;
            fooFunc.Copy(dynamicBuffer);
            cerr << "Execute " << fooFunc.Name() << " in all threads" << endl;
            succeededThreads = ExecuteInAllThreads(&threadPool, fooFunc);
            if (succeededThreads != numThreads)
            {
                cerr << fooFunc.Name() << ": failed in " << (numThreads - succeededThreads) << " threads" << endl;
                return 1;
            }
        }
        {
            BAR_FUNC barFunc;
            barFunc.Copy(dynamicBuffer);
            cerr << "Execute " << barFunc.Name() << " in all threads" << endl;
            succeededThreads = ExecuteInAllThreads(&threadPool, barFunc);
            if (succeededThreads != numThreads)
            {
                cerr << barFunc.Name() << ": failed in " << (numThreads - succeededThreads) << " threads" << endl;
                return 1;
            }
        }
    }

    return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
