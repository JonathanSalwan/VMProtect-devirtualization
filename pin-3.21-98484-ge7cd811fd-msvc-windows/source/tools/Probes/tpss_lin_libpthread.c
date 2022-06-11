/*
 * Copyright (C) 2015-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * This tool mimics the behavior of TPSS on Linux by adding probes to various libpthread functions.
 * However, in this tool these probes are merely empty wrappers that call the original functions.
 * The objective of the test is to verify that probe generation and insertion don't cause Pin
 * to crash.
 *
 * This file is part of the tpss_lin_libpthread tool and compiles against the native
 * libc of the machine/compiler in order to extact data types definition from it's headers.
 */

#define _POSIX_C_SOURCE 200112L
#include <pthread.h>
#include <sys/types.h>
#include <sys/timeb.h>
#include <rpc/rpc.h>
#include <rpc/pmap_clnt.h>
#include <semaphore.h>
#include <dlfcn.h>
#include <signal.h>
#include <poll.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>

typedef int* INT_PTR;

typedef void* VOID_PTR;

typedef char* CHAR_PTR;

void printFunctionCalled(const char* funcName);

/* ===================================================================== */
/* Function signatures - these functions will be probed                  */
/* ===================================================================== */

int (*fptrpthread_spin_unlock)(pthread_spinlock_t* __lock);

int (*fptrpthread_create)(pthread_t* __newthread, __const pthread_attr_t* __attr, VOID_PTR (*__start_routine)(VOID_PTR),
                          VOID_PTR __arg);

int (*fptrpthread_join)(pthread_t __th, void** __thread_return);

int (*fptrpthread_barrier_init)(pthread_barrier_t* __barrier, __const pthread_barrierattr_t* __attr, unsigned int __count);

int (*fptrpthread_barrier_destroy)(pthread_barrier_t* __barrier);

int (*fptrpthread_barrier_wait)(pthread_barrier_t* __barrier);

int (*fptrpthread_mutex_init)(pthread_mutex_t* __mutex, __const pthread_mutexattr_t* __mutexattr);

int (*fptrpthread_mutex_destroy)(pthread_mutex_t* __mutex);

int (*fptrpthread_mutex_lock)(pthread_mutex_t* __mutex);

int (*fptrpthread_mutex_unlock)(pthread_mutex_t* __mutex);

int (*fptrpthread_mutex_timedlock)(pthread_mutex_t* __mutex, __const struct timespec* __abstime);

int (*fptrpthread_rwlock_init)(pthread_rwlock_t* __rwlock, __const pthread_rwlockattr_t* __attr);

int (*fptrpthread_rwlock_destroy)(pthread_rwlock_t* __rwlock);

int (*fptrpthread_rwlock_rdlock)(pthread_rwlock_t* __rwlock);

int (*fptrpthread_rwlock_wrlock)(pthread_rwlock_t* __rwlock);

int (*fptrpthread_rwlock_unlock)(pthread_rwlock_t* __rwlock);

int (*fptrpthread_rwlock_timedrdlock)(pthread_rwlock_t* __rwlock, __const struct timespec* __abstime);

int (*fptrpthread_rwlock_timedwrlock)(pthread_rwlock_t* __rwlock, __const struct timespec* __abstime);

void (*fptrpthread_exit)(void* __retval);

int (*fptrpthread_cancel)(pthread_t __pthread);

int (*fptrpthread_spin_init)(pthread_spinlock_t* __lock, int __pshared);

int (*fptrpthread_spin_destroy)(pthread_spinlock_t* __lock);

int (*fptrpthread_spin_lock)(pthread_spinlock_t* __lock);

int (*fptrpthread_cond_init)(pthread_cond_t* __cond, __const pthread_condattr_t* __cond_attr);

int (*fptrpthread_cond_destroy)(pthread_cond_t* __cond);

int (*fptrpthread_cond_broadcast)(pthread_cond_t* __cond);

int (*fptrpthread_cond_signal)(pthread_cond_t* __cond);

int (*fptrpthread_cond_timedwait)(pthread_cond_t* __cond, pthread_mutex_t* __mutex, __const struct timespec* __abstime);

int (*fptrpthread_cond_wait)(pthread_cond_t* __cond, pthread_mutex_t* __mutex);

int (*fptrpthread_key_create)(pthread_key_t* __key, void (*__destr_function)(VOID_PTR));

int (*fptrpthread_key_delete)(pthread_key_t __key);

int (*fptrnanosleep)(__const struct timespec* __requested_time, struct timespec* __remaining);

int (*fptrsem_init)(sem_t* __sem, int __pshared, unsigned int __value);

int (*fptrsem_destroy)(sem_t* __sem);

int (*fptrsem_wait)(sem_t* __sem);

int (*fptrsem_post)(sem_t* __sem);

int (*fptrpthread_sigmask)(int __how, __const sigset_t* __newmask, sigset_t* __oldmask);

int (*fptrsigaction)(int __sig, __const struct sigaction* __act, struct sigaction* __oact);

int (*fptrsigsuspend)(__const sigset_t* __set);

int (*fptrpthread_mutex_trylock)(pthread_mutex_t* __mutex);

int (*fptrpthread_spin_trylock)(pthread_spinlock_t* __lock);

int (*fptrpthread_setspecific)(pthread_key_t __key, const VOID_PTR __value);

VOID_PTR (*fptrpthread_getspecific)(pthread_key_t __key);

pthread_t (*fptrpthread_self)(void);

void (*fptrpthread_cleanup_push)(void (*routine)(VOID_PTR), VOID_PTR __arg);

void (*fptrpthread_cleanup_pop)(int __execute);

int (*fptrpthread_attr_setstacksize)(pthread_attr_t* __attr, size_t __stacksize);

int (*fptrpthread_attr_getstacksize)(pthread_attr_t* __attr, size_t* __stacksize);

int (*fptrpthread_attr_setstack)(pthread_attr_t* __attr, VOID_PTR __stackaddr, size_t __stacksize);

int (*fptrpthread_attr_getstack)(pthread_attr_t* __attr, void** s__tackaddr, size_t* __stacksize);

int (*fptrpthread_getattr_np)(pthread_t, pthread_attr_t*);

int (*fptrpthread_attr_destroy)(pthread_attr_t* __attr);

int (*fptrpthread_setcancelstate)(int __state, int* __oldstate);

int (*fptrsem_timedwait)(sem_t* __sem, const struct timespec* __abs_timeout);

int (*fptrsystem)(const char* __command);

int (*fptrsigwait)(const sigset_t* __set, int* __sig);

pid_t (*fptrwait)(int* __status);

pid_t (*fptrwaitpid)(pid_t __pid, int* __status, int __options);

pid_t (*fptrwait3)(int* __status, int __options, struct rusage* __rusage);

pid_t (*fptrwait4)(pid_t __pid, int* status, int __options, struct rusage* __rusage);

int (*fptrflock)(int __fd, int __operation);

void (*fptrflockfile)(FILE* __filehandle);

void (*fptrfunlockfile)(FILE* __filehandle);

int (*fptrfcntl)(int __fd, int __cmd, VOID_PTR __argp);

/* ===================================================================== */
/* Probes - implementation of the wrapper functions                      */
/* ===================================================================== */

int mypthread_spin_unlock(pthread_spinlock_t* __lock)
{
    printFunctionCalled("mypthread_spin_unlock");
    int res = fptrpthread_spin_unlock(__lock);

    return res;
}

int mypthread_create(pthread_t* __newthread, __const pthread_attr_t* __attr, void* (*__start_routine)(void*), VOID_PTR __arg)
{
    printFunctionCalled("mypthread_create");
    int res = fptrpthread_create(__newthread, __attr, __start_routine, __arg);

    return res;
}

int mypthread_join(pthread_t __th, void** __thread_return)
{
    printFunctionCalled("mypthread_join");
    int res = fptrpthread_join(__th, __thread_return);

    return res;
}

int mypthread_barrier_init(pthread_barrier_t* __barrier, __const pthread_barrierattr_t* __attr, unsigned int __count)
{
    printFunctionCalled("mypthread_barrier_init");
    int res = fptrpthread_barrier_init(__barrier, __attr, __count);

    return res;
}

int mypthread_barrier_destroy(pthread_barrier_t* __barrier)
{
    printFunctionCalled("mypthread_barrier_destroy");
    int res = fptrpthread_barrier_destroy(__barrier);

    return res;
}

int mypthread_barrier_wait(pthread_barrier_t* __barrier)
{
    printFunctionCalled("mypthread_barrier_wait");
    int res = fptrpthread_barrier_wait(__barrier);

    return res;
}

int mypthread_mutex_init(pthread_mutex_t* __mutex, __const pthread_mutexattr_t* __mutexattr)
{
    printFunctionCalled("mypthread_mutex_init");
    int res = fptrpthread_mutex_init(__mutex, __mutexattr);

    return res;
}

int mypthread_mutex_destroy(pthread_mutex_t* __mutex)
{
    printFunctionCalled("mypthread_mutex_destroy");
    int res = fptrpthread_mutex_destroy(__mutex);

    return res;
}

int mypthread_mutex_lock(pthread_mutex_t* __mutex)
{
    printFunctionCalled("mypthread_mutex_lock");
    int res = fptrpthread_mutex_lock(__mutex);

    return res;
}

int mypthread_mutex_unlock(pthread_mutex_t* __mutex)
{
    printFunctionCalled("mypthread_mutex_unlock");
    int res = fptrpthread_mutex_unlock(__mutex);

    return res;
}

int mypthread_mutex_timedlock(pthread_mutex_t* __mutex, __const struct timespec* __abstime)
{
    printFunctionCalled("mypthread_mutex_timedlock");
    int res = fptrpthread_mutex_timedlock(__mutex, __abstime);

    return res;
}

int mypthread_rwlock_init(pthread_rwlock_t* __rwlock, __const pthread_rwlockattr_t* __attr)
{
    printFunctionCalled("mypthread_rwlock_init");
    int res = fptrpthread_rwlock_init(__rwlock, __attr);

    return res;
}

int mypthread_rwlock_destroy(pthread_rwlock_t* __rwlock)
{
    printFunctionCalled("mypthread_rwlock_destroy");
    int res = fptrpthread_rwlock_destroy(__rwlock);

    return res;
}

int mypthread_rwlock_rdlock(pthread_rwlock_t* __rwlock)
{
    printFunctionCalled("mypthread_rwlock_rdlock");
    int res = fptrpthread_rwlock_rdlock(__rwlock);

    return res;
}

int mypthread_rwlock_wrlock(pthread_rwlock_t* __rwlock)
{
    printFunctionCalled("mypthread_rwlock_wrlock");
    int res = fptrpthread_rwlock_wrlock(__rwlock);

    return res;
}

int mypthread_rwlock_unlock(pthread_rwlock_t* __rwlock)
{
    printFunctionCalled("mypthread_rwlock_unlock");
    int res = fptrpthread_rwlock_unlock(__rwlock);

    return res;
}

int mypthread_rwlock_timedrdlock(pthread_rwlock_t* __rwlock, __const struct timespec* __abstime)
{
    printFunctionCalled("mypthread_rwlock_timedrdlock");
    int res = fptrpthread_rwlock_timedrdlock(__rwlock, __abstime);

    return res;
}

int mypthread_rwlock_timedwrlock(pthread_rwlock_t* __rwlock, __const struct timespec* __abstime)
{
    printFunctionCalled("mypthread_rwlock_timedwrlock");
    int res = fptrpthread_rwlock_timedwrlock(__rwlock, __abstime);

    return res;
}

void mypthread_exit(void* __retval)
{
    printFunctionCalled("mypthread_exit");
    fptrpthread_exit(__retval);
}

int mypthread_cancel(pthread_t __thr)
{
    printFunctionCalled("mypthread_cancel");
    int res = fptrpthread_cancel(__thr);

    return res;
}

int mypthread_spin_init(pthread_spinlock_t* __lock, int __pshared)
{
    printFunctionCalled("mypthread_spin_init");
    int res = fptrpthread_spin_init(__lock, __pshared);

    return res;
}

int mypthread_spin_destroy(pthread_spinlock_t* __lock)
{
    printFunctionCalled("mypthread_spin_destroy");
    int res = fptrpthread_spin_destroy(__lock);

    return res;
}

int mypthread_spin_lock(pthread_spinlock_t* __lock)
{
    printFunctionCalled("mypthread_spin_lock");
    int res = fptrpthread_spin_lock(__lock);

    return res;
}

int mypthread_cond_init(pthread_cond_t* __cond, __const pthread_condattr_t* __cond_attr)
{
    printFunctionCalled("mypthread_cond_init");
    int res = fptrpthread_cond_init(__cond, __cond_attr);

    return res;
}

int mypthread_cond_destroy(pthread_cond_t* __cond)
{
    printFunctionCalled("mypthread_cond_destroy");
    int res = fptrpthread_cond_destroy(__cond);

    return res;
}

int mypthread_cond_broadcast(pthread_cond_t* __cond)
{
    printFunctionCalled("mypthread_cond_broadcast");
    int res = fptrpthread_cond_broadcast(__cond);

    return res;
}

int mypthread_cond_signal(pthread_cond_t* __cond)
{
    printFunctionCalled("mypthread_cond_signal");
    int res = fptrpthread_cond_signal(__cond);

    return res;
}

int mypthread_cond_timedwait(pthread_cond_t* __cond, pthread_mutex_t* __mutex, __const struct timespec* __abstime)
{
    printFunctionCalled("mypthread_cond_timedwait");
    int res = fptrpthread_cond_timedwait(__cond, __mutex, __abstime);

    return res;
}

int mypthread_cond_wait(pthread_cond_t* __cond, pthread_mutex_t* __mutex)
{
    printFunctionCalled("mypthread_cond_wait");
    int res = fptrpthread_cond_wait(__cond, __mutex);

    return res;
}

int mypthread_key_create(pthread_key_t* __key, void (*__destr_function)(void*))
{
    printFunctionCalled("mypthread_key_create");
    int res = fptrpthread_key_create(__key, __destr_function);

    return res;
}

int mypthread_key_delete(pthread_key_t __key)
{
    printFunctionCalled("mypthread_key_delete");
    int res = fptrpthread_key_delete(__key);

    return res;
}

int mynanosleep(__const struct timespec* __requested_time, struct timespec* __remaining)
{
    printFunctionCalled("mynanosleep");
    int res = fptrnanosleep(__requested_time, __remaining);

    return res;
}

int mysem_init(sem_t* __sem, int __pshared, unsigned int __value)
{
    printFunctionCalled("mysem_init");
    int res = fptrsem_init(__sem, __pshared, __value);

    return res;
}

int mysem_destroy(sem_t* __sem)
{
    printFunctionCalled("mysem_destroy");
    int res = fptrsem_destroy(__sem);

    return res;
}

int mysem_wait(sem_t* __sem)
{
    printFunctionCalled("mysem_wait");
    int res = fptrsem_wait(__sem);

    return res;
}

int mysem_post(sem_t* __sem)
{
    printFunctionCalled("mysem_post");
    int res = fptrsem_post(__sem);

    return res;
}

int mypthread_sigmask(int __how, __const sigset_t* __newmask, sigset_t* __oldmask)
{
    printFunctionCalled("mypthread_sigmask");
    int res = fptrpthread_sigmask(__how, __newmask, __oldmask);

    return res;
}

int mysigaction(int __sig, __const struct sigaction* __act, struct sigaction* __oact)
{
    printFunctionCalled("mysigaction");
    int res = fptrsigaction(__sig, __act, __oact);

    return res;
}

int mysigsuspend(__const sigset_t* __set)
{
    printFunctionCalled("mysigsuspend");
    int res = fptrsigsuspend(__set);

    return res;
}

int mypthread_mutex_trylock(pthread_mutex_t* __mutex)
{
    printFunctionCalled("mypthread_mutex_trylock");
    int res = fptrpthread_mutex_trylock(__mutex);

    return res;
}

int mypthread_spin_trylock(pthread_spinlock_t* __lock)
{
    printFunctionCalled("mypthread_spin_trylock");
    int res = fptrpthread_spin_trylock(__lock);

    return res;
}

int mypthread_setspecific(pthread_key_t __key, const VOID_PTR __value)
{
    printFunctionCalled("mypthread_setspecific");
    int res = fptrpthread_setspecific(__key, __value);

    return res;
}

VOID_PTR mypthread_getspecific(pthread_key_t __key)
{
    printFunctionCalled("mypthread_getspecific");
    VOID_PTR res = fptrpthread_getspecific(__key);

    return res;
}

pthread_t mypthread_self(void)
{
    printFunctionCalled("mypthread_self");
    pthread_t res = fptrpthread_self();

    return res;
}

void mypthread_cleanup_push(void (*__routine)(void*), VOID_PTR __arg)
{
    printFunctionCalled("mypthread_cleanup_push");
    fptrpthread_cleanup_push(__routine, __arg);
}

void mypthread_cleanup_pop(int __execute)
{
    printFunctionCalled("mypthread_cleanup_pop");
    fptrpthread_cleanup_pop(__execute);
}

int mypthread_attr_setstacksize(pthread_attr_t* __attr, size_t __stacksize)
{
    printFunctionCalled("mypthread_attr_setstacksize");
    int res = fptrpthread_attr_setstacksize(__attr, __stacksize);

    return res;
}

int mypthread_attr_getstacksize(pthread_attr_t* __attr, size_t* __stacksize)
{
    printFunctionCalled("mypthread_attr_getstacksize");
    int res = fptrpthread_attr_getstacksize(__attr, __stacksize);

    return res;
}

int mypthread_attr_setstack(pthread_attr_t* __attr, VOID_PTR __stackaddr, size_t __stacksize)
{
    printFunctionCalled("mypthread_attr_setstack");
    int res = fptrpthread_attr_setstack(__attr, __stackaddr, __stacksize);

    return res;
}

int mypthread_attr_getstack(pthread_attr_t* __attr, void** s__tackaddr, size_t* __stacksize)
{
    printFunctionCalled("mypthread_attr_getstack");
    int res = fptrpthread_attr_getstack(__attr, s__tackaddr, __stacksize);

    return res;
}

int mypthread_getattr_np(pthread_t __thr, pthread_attr_t* __attr)
{
    printFunctionCalled("mypthread_getattr_np");
    int res = fptrpthread_getattr_np(__thr, __attr);
    return res;
}

int mypthread_attr_destroy(pthread_attr_t* __attr)
{
    printFunctionCalled("mypthread_attr_destroy");
    int res = fptrpthread_attr_destroy(__attr);

    return res;
}

int mypthread_setcancelstate(int __state, int* __oldstate)
{
    printFunctionCalled("mypthread_setcancelstate");
    int res = fptrpthread_setcancelstate(__state, __oldstate);

    return res;
}

int mysem_timedwait(sem_t* __sem, const struct timespec* __abs_timeout)
{
    printFunctionCalled("mysem_timedwait");
    int res = fptrsem_timedwait(__sem, __abs_timeout);

    return res;
}

int mysystem(const char* __command)
{
    printFunctionCalled("mysystem");
    int res = fptrsystem(__command);

    return res;
}

int mysigwait(const sigset_t* __set, int* __sig)
{
    printFunctionCalled("mysigwait");
    int res = fptrsigwait(__set, __sig);

    return res;
}

pid_t mywait(int* __status)
{
    printFunctionCalled("mywait");
    int res = fptrwait(__status);

    return res;
}

pid_t mywaitpid(pid_t __pid, int* __status, int __options)
{
    printFunctionCalled("mywaitpid");
    pid_t res = fptrwaitpid(__pid, __status, __options);

    return res;
}

pid_t mywait3(int* __status, int __options, struct rusage* __rusage)
{
    printFunctionCalled("mywait3");
    pid_t res = fptrwait3(__status, __options, __rusage);

    return res;
}

pid_t mywait4(pid_t __pid, int* __status, int __options, struct rusage* __rusage)
{
    printFunctionCalled("mywait4");
    pid_t res = fptrwait4(__pid, __status, __options, __rusage);

    return res;
}

int myflock(int __fd, int __operation)
{
    printFunctionCalled("myflock");
    int res = fptrflock(__fd, __operation);

    return res;
}

void myflockfile(FILE* __filehandle)
{
    printFunctionCalled("myflockfile");
    fptrflockfile(__filehandle);
}

void myfunlockfile(FILE* __filehandle)
{
    printFunctionCalled("myfunlockfile");
    fptrfunlockfile(__filehandle);
}

int myfcntl(int __fd, int __cmd, VOID_PTR __argp)
{
    printFunctionCalled("myfcntl");
    int res = fptrfcntl(__fd, __cmd, __argp);

    return res;
}
