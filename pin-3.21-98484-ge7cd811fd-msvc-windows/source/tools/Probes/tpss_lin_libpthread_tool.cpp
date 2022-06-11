/*
 * Copyright (C) 2012-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * This tool mimics the behavior of TPSS on Linux by adding probes to various libpthread functions.
 * However, in this tool these probes are merely empty wrappers that call the original functions.
 * The objective of the test is to verify that probe generation and insertion don't cause Pin
 * to crash.
 *
 * This file is part of the tpss_lin_libpthread tool and compiles against the tool's libc (e.g. PIN CRT).
 * This file implements all the intrumenting logic thus requires PIN headers.
 */
#include "pin.H"
#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
using std::cerr;
using std::endl;
using std::hex;
using std::ios;
using std::ofstream;
using std::string;

typedef int* INT_PTR;

typedef void* VOID_PTR;

ofstream OutFile;

/* ===================================================================== */
/* Pointers to the original functions that we probe                      */
/* ===================================================================== */
extern AFUNPTR fptrpthread_spin_unlock;
extern AFUNPTR fptrpthread_create;
extern AFUNPTR fptrpthread_join;
extern AFUNPTR fptrpthread_barrier_init;
extern AFUNPTR fptrpthread_barrier_destroy;
extern AFUNPTR fptrpthread_barrier_wait;
extern AFUNPTR fptrpthread_mutex_init;
extern AFUNPTR fptrpthread_mutex_destroy;
extern AFUNPTR fptrpthread_mutex_lock;
extern AFUNPTR fptrpthread_mutex_unlock;
extern AFUNPTR fptrpthread_mutex_timedlock;
extern AFUNPTR fptrpthread_rwlock_init;
extern AFUNPTR fptrpthread_rwlock_destroy;
extern AFUNPTR fptrpthread_rwlock_rdlock;
extern AFUNPTR fptrpthread_rwlock_wrlock;
extern AFUNPTR fptrpthread_rwlock_unlock;
extern AFUNPTR fptrpthread_rwlock_timedrdlock;
extern AFUNPTR fptrpthread_rwlock_timedwrlock;
extern AFUNPTR fptrpthread_exit;
extern AFUNPTR fptrpthread_cancel;
extern AFUNPTR fptrpthread_spin_init;
extern AFUNPTR fptrpthread_spin_destroy;
extern AFUNPTR fptrpthread_spin_lock;
extern AFUNPTR fptrpthread_cond_init;
extern AFUNPTR fptrpthread_cond_destroy;
extern AFUNPTR fptrpthread_cond_broadcast;
extern AFUNPTR fptrpthread_cond_signal;
extern AFUNPTR fptrpthread_cond_timedwait;
extern AFUNPTR fptrpthread_cond_wait;
extern AFUNPTR fptrpthread_key_create;
extern AFUNPTR fptrpthread_key_delete;
extern AFUNPTR fptrnanosleep;
extern AFUNPTR fptrsem_init;
extern AFUNPTR fptrsem_destroy;
extern AFUNPTR fptrsem_wait;
extern AFUNPTR fptrsem_post;
extern AFUNPTR fptrpthread_sigmask;
extern AFUNPTR fptrsigaction;
extern AFUNPTR fptrsigsuspend;
extern AFUNPTR fptrpthread_mutex_trylock;
extern AFUNPTR fptrpthread_spin_trylock;
extern AFUNPTR fptrpthread_setspecific;
extern AFUNPTR fptrpthread_getspecific;
extern AFUNPTR fptrpthread_self;
extern AFUNPTR fptrpthread_cleanup_push;
extern AFUNPTR fptrpthread_cleanup_pop;
extern AFUNPTR fptrpthread_attr_setstacksize;
extern AFUNPTR fptrpthread_attr_getstacksize;
extern AFUNPTR fptrpthread_attr_setstack;
extern AFUNPTR fptrpthread_attr_getstack;
extern AFUNPTR fptrpthread_getattr_np;
extern AFUNPTR fptrsystem;
extern AFUNPTR fptrwait;
extern AFUNPTR fptrwaitpid;
extern AFUNPTR fptrwait3;
extern AFUNPTR fptrwait4;
extern AFUNPTR fptrflock;
extern AFUNPTR fptrflockfile;
extern AFUNPTR fptrfunlockfile;
extern AFUNPTR fptrfcntl;

/* ===================================================================== */
/* Replacement functions implemented in the other part of the tool       */
/* ===================================================================== */
extern "C"
{
    void mypthread_spin_unlock();
    void mypthread_create();
    void mypthread_join();
    void mypthread_barrier_init();
    void mypthread_barrier_destroy();
    void mypthread_barrier_wait();
    void mypthread_mutex_init();
    void mypthread_mutex_destroy();
    void mypthread_mutex_lock();
    void mypthread_mutex_unlock();
    void mypthread_mutex_timedlock();
    void mypthread_rwlock_init();
    void mypthread_rwlock_destroy();
    void mypthread_rwlock_rdlock();
    void mypthread_rwlock_wrlock();
    void mypthread_rwlock_unlock();
    void mypthread_rwlock_timedrdlock();
    void mypthread_rwlock_timedwrlock();
    void mypthread_exit();
    void mypthread_cancel();
    void mypthread_spin_init();
    void mypthread_spin_destroy();
    void mypthread_spin_lock();
    void mypthread_cond_init();
    void mypthread_cond_destroy();
    void mypthread_cond_broadcast();
    void mypthread_cond_signal();
    void mypthread_cond_timedwait();
    void mypthread_cond_wait();
    void mypthread_key_create();
    void mynanosleep();
    void mypthread_key_delete();
    void mysem_init();
    void mysem_destroy();
    void mysem_wait();
    void mysem_post();
    void mypthread_sigmask();
    void mysigaction();
    void mysigsuspend();
    void mypthread_mutex_trylock();
    void mypthread_spin_trylock();
    void mypthread_setspecific();
    void mypthread_getspecific();
    void mypthread_self();
    void mypthread_cleanup_push();
    void mypthread_cleanup_pop();
    void mypthread_attr_setstacksize();
    void mypthread_attr_getstacksize();
    void mypthread_attr_setstack();
    void mypthread_attr_getstack();
    void mypthread_getattr_np();
    void mysystem();
    void mywait();
    void mywaitpid();
    void mywait3();
    void mywait4();
    void myflock();
    void myflockfile();
    void myfunlockfile();
    void myfcntl();
}

/* ===================================================================== */
/* Commandline Switches                                                  */
/* ===================================================================== */

KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "tpss_lin_libpthread.txt", "specify tool log file name");

/* ===================================================================== */
/* Utility functions                                                     */
/* ===================================================================== */

// Print help information
INT32 Usage()
{
    cerr << "This tool mimics the behavior of TPSS on Linux by adding probes to various libpthread functions." << endl;
    cerr << KNOB_BASE::StringKnobSummary();
    cerr << endl;
    return 1;
}

// Utility function to return the time
string CurrentTime()
{
    char tmpbuf[128];
    time_t thetime = time(NULL);
    ctime_r(&thetime, tmpbuf);
    return tmpbuf;
}

extern "C" void printFunctionCalled(const char* funcName)
{
    OutFile << CurrentTime() << funcName << " called " << endl;
    OutFile.flush();
}

/* ===================================================================== */
/* Instrumnetation functions                                             */
/* ===================================================================== */

// Image load callback - inserts the probes.
void ImgLoad(IMG img, void* v)
{
    // Called every time a new image is loaded

    if ((IMG_Name(img).find("libpthread.so") != string::npos) || (IMG_Name(img).find("LIBPTHREAD.SO") != string::npos) ||
        (IMG_Name(img).find("LIBPTHREAD.so") != string::npos))
    {
        RTN rtnpthread_spin_unlock = RTN_FindByName(img, "pthread_spin_unlock");
        if (RTN_Valid(rtnpthread_spin_unlock) && RTN_IsSafeForProbedReplacement(rtnpthread_spin_unlock))
        {
            OutFile << CurrentTime() << "Inserting probe for pthread_spin_unlock at " << RTN_Address(rtnpthread_spin_unlock)
                    << endl;
            OutFile.flush();
            AFUNPTR fptr            = (RTN_ReplaceProbed(rtnpthread_spin_unlock, AFUNPTR(mypthread_spin_unlock)));
            fptrpthread_spin_unlock = fptr;
        }

        RTN rtnpthread_create = RTN_FindByName(img, "pthread_create");
        if (RTN_Valid(rtnpthread_create) && RTN_IsSafeForProbedReplacement(rtnpthread_create))
        {
            OutFile << CurrentTime() << "Inserting probe for pthread_create at " << RTN_Address(rtnpthread_create) << endl;
            OutFile.flush();
            AFUNPTR fptr       = (RTN_ReplaceProbed(rtnpthread_create, AFUNPTR(mypthread_create)));
            fptrpthread_create = fptr;
        }

        RTN rtnpthread_join = RTN_FindByName(img, "pthread_join");
        if (RTN_Valid(rtnpthread_join) && RTN_IsSafeForProbedReplacement(rtnpthread_join))
        {
            OutFile << CurrentTime() << "Inserting probe for pthread_join at " << RTN_Address(rtnpthread_join) << endl;
            OutFile.flush();
            AFUNPTR fptr     = (RTN_ReplaceProbed(rtnpthread_join, AFUNPTR(mypthread_join)));
            fptrpthread_join = fptr;
        }

        RTN rtnpthread_barrier_init = RTN_FindByName(img, "pthread_barrier_init");
        if (RTN_Valid(rtnpthread_barrier_init) && RTN_IsSafeForProbedReplacement(rtnpthread_barrier_init))
        {
            OutFile << CurrentTime() << "Inserting probe for pthread_barrier_init at " << RTN_Address(rtnpthread_barrier_init)
                    << endl;
            OutFile.flush();
            AFUNPTR fptr             = (RTN_ReplaceProbed(rtnpthread_barrier_init, AFUNPTR(mypthread_barrier_init)));
            fptrpthread_barrier_init = fptr;
        }

        RTN rtnpthread_barrier_destroy = RTN_FindByName(img, "pthread_barrier_destroy");
        if (RTN_Valid(rtnpthread_barrier_destroy) && RTN_IsSafeForProbedReplacement(rtnpthread_barrier_destroy))
        {
            OutFile << CurrentTime() << "Inserting probe for pthread_barrier_destroy at "
                    << RTN_Address(rtnpthread_barrier_destroy) << endl;
            OutFile.flush();
            AFUNPTR fptr                = (RTN_ReplaceProbed(rtnpthread_barrier_destroy, AFUNPTR(mypthread_barrier_destroy)));
            fptrpthread_barrier_destroy = fptr;
        }

        RTN rtnpthread_barrier_wait = RTN_FindByName(img, "pthread_barrier_wait");
        if (RTN_Valid(rtnpthread_barrier_wait) && RTN_IsSafeForProbedReplacement(rtnpthread_barrier_wait))
        {
            OutFile << CurrentTime() << "Inserting probe for pthread_barrier_wait at " << RTN_Address(rtnpthread_barrier_wait)
                    << endl;
            OutFile.flush();
            AFUNPTR fptr             = (RTN_ReplaceProbed(rtnpthread_barrier_wait, AFUNPTR(mypthread_barrier_wait)));
            fptrpthread_barrier_wait = fptr;
        }

        RTN rtnpthread_mutex_init = RTN_FindByName(img, "pthread_mutex_init");
        if (RTN_Valid(rtnpthread_mutex_init) && RTN_IsSafeForProbedReplacement(rtnpthread_mutex_init))
        {
            OutFile << CurrentTime() << "Inserting probe for pthread_mutex_init at " << RTN_Address(rtnpthread_mutex_init)
                    << endl;
            OutFile.flush();
            AFUNPTR fptr           = (RTN_ReplaceProbed(rtnpthread_mutex_init, AFUNPTR(mypthread_mutex_init)));
            fptrpthread_mutex_init = fptr;
        }

        RTN rtnpthread_mutex_destroy = RTN_FindByName(img, "pthread_mutex_destroy");
        if (RTN_Valid(rtnpthread_mutex_destroy) && RTN_IsSafeForProbedReplacement(rtnpthread_mutex_destroy))
        {
            OutFile << CurrentTime() << "Inserting probe for pthread_mutex_destroy at " << RTN_Address(rtnpthread_mutex_destroy)
                    << endl;
            OutFile.flush();
            AFUNPTR fptr              = (RTN_ReplaceProbed(rtnpthread_mutex_destroy, AFUNPTR(mypthread_mutex_destroy)));
            fptrpthread_mutex_destroy = fptr;
        }

        RTN rtnpthread_mutex_lock = RTN_FindByName(img, "pthread_mutex_lock");
        if (RTN_Valid(rtnpthread_mutex_lock) && RTN_IsSafeForProbedReplacement(rtnpthread_mutex_lock))
        {
            OutFile << CurrentTime() << "Inserting probe for pthread_mutex_lock at " << RTN_Address(rtnpthread_mutex_lock)
                    << endl;
            OutFile.flush();
            AFUNPTR fptr           = (RTN_ReplaceProbed(rtnpthread_mutex_lock, AFUNPTR(mypthread_mutex_lock)));
            fptrpthread_mutex_lock = fptr;
        }

        RTN rtnpthread_mutex_unlock = RTN_FindByName(img, "pthread_mutex_unlock");
        if (RTN_Valid(rtnpthread_mutex_unlock) && RTN_IsSafeForProbedReplacement(rtnpthread_mutex_unlock))
        {
            OutFile << CurrentTime() << "Inserting probe for pthread_mutex_unlock at " << RTN_Address(rtnpthread_mutex_unlock)
                    << endl;
            OutFile.flush();
            AFUNPTR fptr             = (RTN_ReplaceProbed(rtnpthread_mutex_unlock, AFUNPTR(mypthread_mutex_unlock)));
            fptrpthread_mutex_unlock = fptr;
        }

        RTN rtnpthread_mutex_timedlock = RTN_FindByName(img, "pthread_mutex_timedlock");
        if (RTN_Valid(rtnpthread_mutex_timedlock) && RTN_IsSafeForProbedReplacement(rtnpthread_mutex_timedlock))
        {
            OutFile << CurrentTime() << "Inserting probe for pthread_mutex_timedlock at "
                    << RTN_Address(rtnpthread_mutex_timedlock) << endl;
            OutFile.flush();
            AFUNPTR fptr                = (RTN_ReplaceProbed(rtnpthread_mutex_timedlock, AFUNPTR(mypthread_mutex_timedlock)));
            fptrpthread_mutex_timedlock = fptr;
        }

        RTN rtnpthread_rwlock_init = RTN_FindByName(img, "pthread_rwlock_init");
        if (RTN_Valid(rtnpthread_rwlock_init) && RTN_IsSafeForProbedReplacement(rtnpthread_rwlock_init))
        {
            OutFile << CurrentTime() << "Inserting probe for pthread_rwlock_init at " << RTN_Address(rtnpthread_rwlock_init)
                    << endl;
            OutFile.flush();
            AFUNPTR fptr            = (RTN_ReplaceProbed(rtnpthread_rwlock_init, AFUNPTR(mypthread_rwlock_init)));
            fptrpthread_rwlock_init = fptr;
        }

        RTN rtnpthread_rwlock_destroy = RTN_FindByName(img, "pthread_rwlock_destroy");
        if (RTN_Valid(rtnpthread_rwlock_destroy) && RTN_IsSafeForProbedReplacement(rtnpthread_rwlock_destroy))
        {
            OutFile << CurrentTime() << "Inserting probe for pthread_rwlock_destroy at " << RTN_Address(rtnpthread_rwlock_destroy)
                    << endl;
            OutFile.flush();
            AFUNPTR fptr               = (RTN_ReplaceProbed(rtnpthread_rwlock_destroy, AFUNPTR(mypthread_rwlock_destroy)));
            fptrpthread_rwlock_destroy = fptr;
        }

        RTN rtnpthread_rwlock_rdlock = RTN_FindByName(img, "pthread_rwlock_rdlock");
        if (RTN_Valid(rtnpthread_rwlock_rdlock) && RTN_IsSafeForProbedReplacement(rtnpthread_rwlock_rdlock))
        {
            OutFile << CurrentTime() << "Inserting probe for pthread_rwlock_rdlock at " << RTN_Address(rtnpthread_rwlock_rdlock)
                    << endl;
            OutFile.flush();
            AFUNPTR fptr              = (RTN_ReplaceProbed(rtnpthread_rwlock_rdlock, AFUNPTR(mypthread_rwlock_rdlock)));
            fptrpthread_rwlock_rdlock = fptr;
        }

        RTN rtnpthread_rwlock_wrlock = RTN_FindByName(img, "pthread_rwlock_wrlock");
        if (RTN_Valid(rtnpthread_rwlock_wrlock) && RTN_IsSafeForProbedReplacement(rtnpthread_rwlock_wrlock))
        {
            OutFile << CurrentTime() << "Inserting probe for pthread_rwlock_wrlock at " << RTN_Address(rtnpthread_rwlock_wrlock)
                    << endl;
            OutFile.flush();
            AFUNPTR fptr              = (RTN_ReplaceProbed(rtnpthread_rwlock_wrlock, AFUNPTR(mypthread_rwlock_wrlock)));
            fptrpthread_rwlock_wrlock = fptr;
        }

        RTN rtnpthread_rwlock_unlock = RTN_FindByName(img, "pthread_rwlock_unlock");
        if (RTN_Valid(rtnpthread_rwlock_unlock) && RTN_IsSafeForProbedReplacement(rtnpthread_rwlock_unlock))
        {
            OutFile << CurrentTime() << "Inserting probe for pthread_rwlock_unlock at " << RTN_Address(rtnpthread_rwlock_unlock)
                    << endl;
            OutFile.flush();
            AFUNPTR fptr              = (RTN_ReplaceProbed(rtnpthread_rwlock_unlock, AFUNPTR(mypthread_rwlock_unlock)));
            fptrpthread_rwlock_unlock = fptr;
        }

        RTN rtnpthread_rwlock_timedrdlock = RTN_FindByName(img, "pthread_rwlock_timedrdlock");
        if (RTN_Valid(rtnpthread_rwlock_timedrdlock) && RTN_IsSafeForProbedReplacement(rtnpthread_rwlock_timedrdlock))
        {
            OutFile << CurrentTime() << "Inserting probe for pthread_rwlock_timedrdlock at "
                    << RTN_Address(rtnpthread_rwlock_timedrdlock) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnpthread_rwlock_timedrdlock, AFUNPTR(mypthread_rwlock_timedrdlock)));
            fptrpthread_rwlock_timedrdlock = fptr;
        }

        RTN rtnpthread_rwlock_timedwrlock = RTN_FindByName(img, "pthread_rwlock_timedwrlock");
        if (RTN_Valid(rtnpthread_rwlock_timedwrlock) && RTN_IsSafeForProbedReplacement(rtnpthread_rwlock_timedwrlock))
        {
            OutFile << CurrentTime() << "Inserting probe for pthread_rwlock_timedwrlock at "
                    << RTN_Address(rtnpthread_rwlock_timedwrlock) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnpthread_rwlock_timedwrlock, AFUNPTR(mypthread_rwlock_timedwrlock)));
            fptrpthread_rwlock_timedwrlock = fptr;
        }

        RTN rtnpthread_exit = RTN_FindByName(img, "pthread_exit");
        if (RTN_Valid(rtnpthread_exit) && RTN_IsSafeForProbedReplacement(rtnpthread_exit))
        {
            OutFile << CurrentTime() << "Inserting probe for pthread_exit at " << RTN_Address(rtnpthread_exit) << endl;
            OutFile.flush();
            AFUNPTR fptr     = (RTN_ReplaceProbed(rtnpthread_exit, AFUNPTR(mypthread_exit)));
            fptrpthread_exit = fptr;
        }

        RTN rtnpthread_cancel = RTN_FindByName(img, "pthread_cancel");
        if (RTN_Valid(rtnpthread_cancel) && RTN_IsSafeForProbedReplacement(rtnpthread_cancel))
        {
            OutFile << CurrentTime() << "Inserting probe for pthread_cancel at " << RTN_Address(rtnpthread_cancel) << endl;
            OutFile.flush();
            AFUNPTR fptr       = (RTN_ReplaceProbed(rtnpthread_cancel, AFUNPTR(mypthread_cancel)));
            fptrpthread_cancel = fptr;
        }

        RTN rtnpthread_spin_init = RTN_FindByName(img, "pthread_spin_init");
        if (RTN_Valid(rtnpthread_spin_init) && RTN_IsSafeForProbedReplacement(rtnpthread_spin_init))
        {
            OutFile << CurrentTime() << "Inserting probe for pthread_spin_init at " << RTN_Address(rtnpthread_spin_init) << endl;
            OutFile.flush();
            AFUNPTR fptr          = (RTN_ReplaceProbed(rtnpthread_spin_init, AFUNPTR(mypthread_spin_init)));
            fptrpthread_spin_init = fptr;
        }

        RTN rtnpthread_spin_destroy = RTN_FindByName(img, "pthread_spin_destroy");
        if (RTN_Valid(rtnpthread_spin_destroy) && RTN_IsSafeForProbedReplacement(rtnpthread_spin_destroy))
        {
            OutFile << CurrentTime() << "Inserting probe for pthread_spin_destroy at " << RTN_Address(rtnpthread_spin_destroy)
                    << endl;
            OutFile.flush();
            AFUNPTR fptr             = (RTN_ReplaceProbed(rtnpthread_spin_destroy, AFUNPTR(mypthread_spin_destroy)));
            fptrpthread_spin_destroy = fptr;
        }

        RTN rtnpthread_spin_lock = RTN_FindByName(img, "pthread_spin_lock");
        if (RTN_Valid(rtnpthread_spin_lock) && RTN_IsSafeForProbedReplacement(rtnpthread_spin_lock))
        {
            OutFile << CurrentTime() << "Inserting probe for pthread_spin_lock at " << RTN_Address(rtnpthread_spin_lock) << endl;
            OutFile.flush();
            AFUNPTR fptr          = (RTN_ReplaceProbed(rtnpthread_spin_lock, AFUNPTR(mypthread_spin_lock)));
            fptrpthread_spin_lock = fptr;
        }

        RTN rtnpthread_cond_init = RTN_FindByName(img, "pthread_cond_init");
        if (RTN_Valid(rtnpthread_cond_init) && RTN_IsSafeForProbedReplacement(rtnpthread_cond_init))
        {
            OutFile << CurrentTime() << "Inserting probe for pthread_cond_init at " << RTN_Address(rtnpthread_cond_init) << endl;
            OutFile.flush();
            AFUNPTR fptr          = (RTN_ReplaceProbed(rtnpthread_cond_init, AFUNPTR(mypthread_cond_init)));
            fptrpthread_cond_init = fptr;
        }

        RTN rtnpthread_cond_destroy = RTN_FindByName(img, "pthread_cond_destroy");
        if (RTN_Valid(rtnpthread_cond_destroy) && RTN_IsSafeForProbedReplacement(rtnpthread_cond_destroy))
        {
            OutFile << CurrentTime() << "Inserting probe for pthread_cond_destroy at " << RTN_Address(rtnpthread_cond_destroy)
                    << endl;
            OutFile.flush();
            AFUNPTR fptr             = (RTN_ReplaceProbed(rtnpthread_cond_destroy, AFUNPTR(mypthread_cond_destroy)));
            fptrpthread_cond_destroy = fptr;
        }

        RTN rtnpthread_cond_broadcast = RTN_FindByName(img, "pthread_cond_broadcast");
        if (RTN_Valid(rtnpthread_cond_broadcast) && RTN_IsSafeForProbedReplacement(rtnpthread_cond_broadcast))
        {
            OutFile << CurrentTime() << "Inserting probe for pthread_cond_broadcast at " << RTN_Address(rtnpthread_cond_broadcast)
                    << endl;
            OutFile.flush();
            AFUNPTR fptr               = (RTN_ReplaceProbed(rtnpthread_cond_broadcast, AFUNPTR(mypthread_cond_broadcast)));
            fptrpthread_cond_broadcast = fptr;
        }

        RTN rtnpthread_cond_signal = RTN_FindByName(img, "pthread_cond_signal");
        if (RTN_Valid(rtnpthread_cond_signal) && RTN_IsSafeForProbedReplacement(rtnpthread_cond_signal))
        {
            OutFile << CurrentTime() << "Inserting probe for pthread_cond_signal at " << RTN_Address(rtnpthread_cond_signal)
                    << endl;
            OutFile.flush();
            AFUNPTR fptr            = (RTN_ReplaceProbed(rtnpthread_cond_signal, AFUNPTR(mypthread_cond_signal)));
            fptrpthread_cond_signal = fptr;
        }

        RTN rtnpthread_cond_timedwait = RTN_FindByName(img, "pthread_cond_timedwait");
        if (RTN_Valid(rtnpthread_cond_timedwait) && RTN_IsSafeForProbedReplacement(rtnpthread_cond_timedwait))
        {
            OutFile << CurrentTime() << "Inserting probe for pthread_cond_timedwait at " << RTN_Address(rtnpthread_cond_timedwait)
                    << endl;
            OutFile.flush();
            AFUNPTR fptr               = (RTN_ReplaceProbed(rtnpthread_cond_timedwait, AFUNPTR(mypthread_cond_timedwait)));
            fptrpthread_cond_timedwait = fptr;
        }

        RTN rtnpthread_cond_wait = RTN_FindByName(img, "pthread_cond_wait");
        if (RTN_Valid(rtnpthread_cond_wait) && RTN_IsSafeForProbedReplacement(rtnpthread_cond_wait))
        {
            OutFile << CurrentTime() << "Inserting probe for pthread_cond_wait at " << RTN_Address(rtnpthread_cond_wait) << endl;
            OutFile.flush();
            AFUNPTR fptr          = (RTN_ReplaceProbed(rtnpthread_cond_wait, AFUNPTR(mypthread_cond_wait)));
            fptrpthread_cond_wait = fptr;
        }

        RTN rtnpthread_key_create = RTN_FindByName(img, "pthread_key_create");
        if (RTN_Valid(rtnpthread_key_create) && RTN_IsSafeForProbedReplacement(rtnpthread_key_create))
        {
            OutFile << CurrentTime() << "Inserting probe for pthread_key_create at " << RTN_Address(rtnpthread_key_create)
                    << endl;
            OutFile.flush();
            AFUNPTR fptr           = (RTN_ReplaceProbed(rtnpthread_key_create, AFUNPTR(mypthread_key_create)));
            fptrpthread_key_create = fptr;
        }

        RTN rtnnanosleep = RTN_FindByName(img, "nanosleep");
        if (RTN_Valid(rtnnanosleep) && RTN_IsSafeForProbedReplacement(rtnnanosleep))
        {
            OutFile << CurrentTime() << "Inserting probe for nanosleep at " << RTN_Address(rtnnanosleep) << endl;
            OutFile.flush();
            AFUNPTR fptr  = (RTN_ReplaceProbed(rtnnanosleep, AFUNPTR(mynanosleep)));
            fptrnanosleep = fptr;
        }

        RTN rtnpthread_key_delete = RTN_FindByName(img, "pthread_key_delete");
        if (RTN_Valid(rtnpthread_key_delete) && RTN_IsSafeForProbedReplacement(rtnpthread_key_delete))
        {
            OutFile << CurrentTime() << "Inserting probe for pthread_key_delete at " << RTN_Address(rtnpthread_key_delete)
                    << endl;
            OutFile.flush();
            AFUNPTR fptr           = (RTN_ReplaceProbed(rtnpthread_key_delete, AFUNPTR(mypthread_key_delete)));
            fptrpthread_key_delete = fptr;
        }

        RTN rtnsem_init = RTN_FindByName(img, "sem_init");
        if (RTN_Valid(rtnsem_init) && RTN_IsSafeForProbedReplacement(rtnsem_init))
        {
            OutFile << CurrentTime() << "Inserting probe for sem_init at " << RTN_Address(rtnsem_init) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnsem_init, AFUNPTR(mysem_init)));
            fptrsem_init = fptr;
        }

        RTN rtnsem_destroy = RTN_FindByName(img, "sem_destroy");
        if (RTN_Valid(rtnsem_destroy) && RTN_IsSafeForProbedReplacement(rtnsem_destroy))
        {
            OutFile << CurrentTime() << "Inserting probe for sem_destroy at " << RTN_Address(rtnsem_destroy) << endl;
            OutFile.flush();
            AFUNPTR fptr    = (RTN_ReplaceProbed(rtnsem_destroy, AFUNPTR(mysem_destroy)));
            fptrsem_destroy = fptr;
        }

        RTN rtnsem_wait = RTN_FindByName(img, "sem_wait");
        if (RTN_Valid(rtnsem_wait) && RTN_IsSafeForProbedReplacement(rtnsem_wait))
        {
            OutFile << CurrentTime() << "Inserting probe for sem_wait at " << RTN_Address(rtnsem_wait) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnsem_wait, AFUNPTR(mysem_wait)));
            fptrsem_wait = fptr;
        }

        RTN rtnsem_post = RTN_FindByName(img, "sem_post");
        if (RTN_Valid(rtnsem_post) && RTN_IsSafeForProbedReplacement(rtnsem_post))
        {
            OutFile << CurrentTime() << "Inserting probe for sem_post at " << RTN_Address(rtnsem_post) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnsem_post, AFUNPTR(mysem_post)));
            fptrsem_post = fptr;
        }

        RTN rtnpthread_sigmask = RTN_FindByName(img, "pthread_sigmask");
        if (RTN_Valid(rtnpthread_sigmask) && RTN_IsSafeForProbedReplacement(rtnpthread_sigmask))
        {
            OutFile << CurrentTime() << "Inserting probe for pthread_sigmask at " << RTN_Address(rtnpthread_sigmask) << endl;
            OutFile.flush();
            AFUNPTR fptr        = (RTN_ReplaceProbed(rtnpthread_sigmask, AFUNPTR(mypthread_sigmask)));
            fptrpthread_sigmask = fptr;
        }

        RTN rtnsigaction = RTN_FindByName(img, "sigaction");
        if (RTN_Valid(rtnsigaction) && RTN_IsSafeForProbedReplacement(rtnsigaction))
        {
            OutFile << CurrentTime() << "Inserting probe for sigaction at " << RTN_Address(rtnsigaction) << endl;
            OutFile.flush();
            AFUNPTR fptr  = (RTN_ReplaceProbed(rtnsigaction, AFUNPTR(mysigaction)));
            fptrsigaction = fptr;
        }

        RTN rtnsigsuspend = RTN_FindByName(img, "sigsuspend");
        if (RTN_Valid(rtnsigsuspend) && RTN_IsSafeForProbedReplacement(rtnsigsuspend))
        {
            OutFile << CurrentTime() << "Inserting probe for sigsuspend at " << RTN_Address(rtnsigsuspend) << endl;
            OutFile.flush();
            AFUNPTR fptr   = (RTN_ReplaceProbed(rtnsigsuspend, AFUNPTR(mysigsuspend)));
            fptrsigsuspend = fptr;
        }

        RTN rtnpthread_mutex_trylock = RTN_FindByName(img, "pthread_mutex_trylock");
        if (RTN_Valid(rtnpthread_mutex_trylock) && RTN_IsSafeForProbedReplacement(rtnpthread_mutex_trylock))
        {
            OutFile << CurrentTime() << "Inserting probe for pthread_mutex_trylock at " << RTN_Address(rtnpthread_mutex_trylock)
                    << endl;
            OutFile.flush();
            AFUNPTR fptr              = (RTN_ReplaceProbed(rtnpthread_mutex_trylock, AFUNPTR(mypthread_mutex_trylock)));
            fptrpthread_mutex_trylock = fptr;
        }

        RTN rtnpthread_spin_trylock = RTN_FindByName(img, "pthread_spin_trylock");
        if (RTN_Valid(rtnpthread_spin_trylock) && RTN_IsSafeForProbedReplacement(rtnpthread_spin_trylock))
        {
            OutFile << CurrentTime() << "Inserting probe for pthread_spin_trylock at " << RTN_Address(rtnpthread_spin_trylock)
                    << endl;
            OutFile.flush();
            AFUNPTR fptr             = (RTN_ReplaceProbed(rtnpthread_spin_trylock, AFUNPTR(mypthread_spin_trylock)));
            fptrpthread_spin_trylock = fptr;
        }

        RTN rtnpthread_setspecific = RTN_FindByName(img, "pthread_setspecific");
        if (RTN_Valid(rtnpthread_setspecific) && RTN_IsSafeForProbedReplacement(rtnpthread_setspecific))
        {
            OutFile << CurrentTime() << "Inserting probe for pthread_setspecific at " << RTN_Address(rtnpthread_setspecific)
                    << endl;
            OutFile.flush();
            AFUNPTR fptr            = (RTN_ReplaceProbed(rtnpthread_setspecific, AFUNPTR(mypthread_setspecific)));
            fptrpthread_setspecific = fptr;
        }

        RTN rtnpthread_getspecific = RTN_FindByName(img, "pthread_getspecific");
        if (RTN_Valid(rtnpthread_getspecific) && RTN_IsSafeForProbedReplacement(rtnpthread_getspecific))
        {
            OutFile << CurrentTime() << "Inserting probe for pthread_getspecific at " << RTN_Address(rtnpthread_getspecific)
                    << endl;
            OutFile.flush();
            AFUNPTR fptr            = (RTN_ReplaceProbed(rtnpthread_getspecific, AFUNPTR(mypthread_getspecific)));
            fptrpthread_getspecific = fptr;
        }

        RTN rtnpthread_self = RTN_FindByName(img, "pthread_self");
        if (RTN_Valid(rtnpthread_self) && RTN_IsSafeForProbedReplacement(rtnpthread_self))
        {
            OutFile << CurrentTime() << "Inserting probe for pthread_self at " << RTN_Address(rtnpthread_self) << endl;
            OutFile.flush();
            AFUNPTR fptr     = (RTN_ReplaceProbed(rtnpthread_self, AFUNPTR(mypthread_self)));
            fptrpthread_self = fptr;
        }

        RTN rtnpthread_cleanup_push = RTN_FindByName(img, "pthread_cleanup_push");
        if (RTN_Valid(rtnpthread_cleanup_push) && RTN_IsSafeForProbedReplacement(rtnpthread_cleanup_push))
        {
            OutFile << CurrentTime() << "Inserting probe for pthread_cleanup_push at " << RTN_Address(rtnpthread_cleanup_push)
                    << endl;
            OutFile.flush();
            AFUNPTR fptr             = (RTN_ReplaceProbed(rtnpthread_cleanup_push, AFUNPTR(mypthread_cleanup_push)));
            fptrpthread_cleanup_push = fptr;
        }

        RTN rtnpthread_cleanup_pop = RTN_FindByName(img, "pthread_cleanup_pop");
        if (RTN_Valid(rtnpthread_cleanup_pop) && RTN_IsSafeForProbedReplacement(rtnpthread_cleanup_pop))
        {
            OutFile << CurrentTime() << "Inserting probe for pthread_cleanup_pop at " << RTN_Address(rtnpthread_cleanup_pop)
                    << endl;
            OutFile.flush();
            AFUNPTR fptr            = (RTN_ReplaceProbed(rtnpthread_cleanup_pop, AFUNPTR(mypthread_cleanup_pop)));
            fptrpthread_cleanup_pop = fptr;
        }

        RTN rtnpthread_attr_setstacksize = RTN_FindByName(img, "pthread_attr_setstacksize");
        if (RTN_Valid(rtnpthread_attr_setstacksize) && RTN_IsSafeForProbedReplacement(rtnpthread_attr_setstacksize))
        {
            OutFile << CurrentTime() << "Inserting probe for pthread_attr_setstacksize at "
                    << RTN_Address(rtnpthread_attr_setstacksize) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnpthread_attr_setstacksize, AFUNPTR(mypthread_attr_setstacksize)));
            fptrpthread_attr_setstacksize = fptr;
        }

        RTN rtnpthread_attr_getstacksize = RTN_FindByName(img, "pthread_attr_getstacksize");
        if (RTN_Valid(rtnpthread_attr_getstacksize) && RTN_IsSafeForProbedReplacement(rtnpthread_attr_getstacksize))
        {
            OutFile << CurrentTime() << "Inserting probe for pthread_attr_getstacksize at "
                    << RTN_Address(rtnpthread_attr_getstacksize) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnpthread_attr_getstacksize, AFUNPTR(mypthread_attr_getstacksize)));
            fptrpthread_attr_getstacksize = fptr;
        }

        RTN rtnpthread_attr_setstack = RTN_FindByName(img, "pthread_attr_setstack");
        if (RTN_Valid(rtnpthread_attr_setstack) && RTN_IsSafeForProbedReplacement(rtnpthread_attr_setstack))
        {
            OutFile << CurrentTime() << "Inserting probe for pthread_attr_setstack at " << RTN_Address(rtnpthread_attr_setstack)
                    << endl;
            OutFile.flush();
            AFUNPTR fptr              = (RTN_ReplaceProbed(rtnpthread_attr_setstack, AFUNPTR(mypthread_attr_setstack)));
            fptrpthread_attr_setstack = fptr;
        }

        RTN rtnpthread_attr_getstack = RTN_FindByName(img, "pthread_attr_getstack");
        if (RTN_Valid(rtnpthread_attr_getstack) && RTN_IsSafeForProbedReplacement(rtnpthread_attr_getstack))
        {
            OutFile << CurrentTime() << "Inserting probe for pthread_attr_setstack at " << RTN_Address(rtnpthread_attr_getstack)
                    << endl;
            OutFile.flush();
            AFUNPTR fptr              = (RTN_ReplaceProbed(rtnpthread_attr_getstack, AFUNPTR(mypthread_attr_getstack)));
            fptrpthread_attr_getstack = fptr;
        }

        RTN rtnpthread_getattr_np = RTN_FindByName(img, "pthread_getattr_np");
        if (RTN_Valid(rtnpthread_getattr_np) && RTN_IsSafeForProbedReplacement(rtnpthread_getattr_np))
        {
            OutFile << CurrentTime() << "Inserting probe for pthread_getattr_np at " << RTN_Address(rtnpthread_getattr_np)
                    << endl;
            OutFile.flush();
            AFUNPTR fptr           = (RTN_ReplaceProbed(rtnpthread_getattr_np, AFUNPTR(mypthread_getattr_np)));
            fptrpthread_getattr_np = fptr;
        }

        RTN rtnsystem = RTN_FindByName(img, "system");
        if (RTN_Valid(rtnsystem) && RTN_IsSafeForProbedReplacement(rtnsystem))
        {
            OutFile << CurrentTime() << "Inserting probe for system at " << RTN_Address(rtnsystem) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnsystem, AFUNPTR(mysystem)));
            fptrsystem   = fptr;
        }

        RTN rtnwait = RTN_FindByName(img, "wait");
        if (RTN_Valid(rtnwait) && RTN_IsSafeForProbedReplacement(rtnwait))
        {
            OutFile << CurrentTime() << "Inserting probe for wait at " << RTN_Address(rtnwait) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnwait, AFUNPTR(mywait)));
            fptrwait     = fptr;
        }

        RTN rtnwaitpid = RTN_FindByName(img, "waitpid");
        if (RTN_Valid(rtnwaitpid) && RTN_IsSafeForProbedReplacement(rtnwaitpid))
        {
            OutFile << CurrentTime() << "Inserting probe for waitpid at " << RTN_Address(rtnwaitpid) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnwaitpid, AFUNPTR(mywaitpid)));
            fptrwaitpid  = fptr;
        }

        RTN rtnwait3 = RTN_FindByName(img, "wait3");
        if (RTN_Valid(rtnwait3) && RTN_IsSafeForProbedReplacement(rtnwait3))
        {
            OutFile << CurrentTime() << "Inserting probe for wait3 at " << RTN_Address(rtnwait3) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnwait3, AFUNPTR(mywait3)));
            fptrwait3    = fptr;
        }

        RTN rtnwait4 = RTN_FindByName(img, "wait4");
        if (RTN_Valid(rtnwait3) && RTN_IsSafeForProbedReplacement(rtnwait4))
        {
            OutFile << CurrentTime() << "Inserting probe for wait4 at " << RTN_Address(rtnwait4) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnwait3, AFUNPTR(mywait4)));
            fptrwait4    = fptr;
        }

        RTN rtnflock = RTN_FindByName(img, "flock");
        if (RTN_Valid(rtnflock) && RTN_IsSafeForProbedReplacement(rtnflock))
        {
            OutFile << CurrentTime() << "Inserting probe for flock at " << RTN_Address(rtnflock) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnflock, AFUNPTR(myflock)));
            fptrflock    = fptr;
        }

        RTN rtnflockfile = RTN_FindByName(img, "flockfile");
        if (RTN_Valid(rtnflockfile) && RTN_IsSafeForProbedReplacement(rtnflockfile))
        {
            OutFile << CurrentTime() << "Inserting probe for flockfile at " << RTN_Address(rtnflockfile) << endl;
            OutFile.flush();
            AFUNPTR fptr  = (RTN_ReplaceProbed(rtnflockfile, AFUNPTR(myflockfile)));
            fptrflockfile = fptr;
        }

        RTN rtnfunlockfile = RTN_FindByName(img, "funlockfile");
        if (RTN_Valid(rtnfunlockfile) && RTN_IsSafeForProbedReplacement(rtnfunlockfile))
        {
            OutFile << CurrentTime() << "Inserting probe for funlockfile at " << RTN_Address(rtnfunlockfile) << endl;
            OutFile.flush();
            AFUNPTR fptr    = (RTN_ReplaceProbed(rtnfunlockfile, AFUNPTR(myfunlockfile)));
            fptrfunlockfile = fptr;
        }

        RTN rtnfcntl = RTN_FindByName(img, "fcntl");
        if (RTN_Valid(rtnfcntl) && RTN_IsSafeForProbedReplacement(rtnfcntl))
        {
            OutFile << CurrentTime() << "Inserting probe for fcntl at " << RTN_Address(rtnfcntl) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnfcntl, AFUNPTR(myfcntl)));
            fptrfcntl    = fptr;
        }

    } // libpthread.so
    // finished instrumentation
}

/* ===================================================================== */
/* Main function                                                         */
/* ===================================================================== */

int main(int argc, char* argv[])
{
    // Initialize Pin
    PIN_InitSymbols();
    if (PIN_Init(argc, argv))
    {
        return Usage();
    }

    OutFile.open(KnobOutputFile.Value().c_str());
    OutFile << hex;
    OutFile.setf(ios::showbase);
    OutFile << CurrentTime() << "started!" << endl;
    OutFile.flush();

    // Register the instrumentation callback
    IMG_AddInstrumentFunction(ImgLoad, 0);

    // Start the application
    PIN_StartProgramProbed(); // never returns

    return 0;
}
