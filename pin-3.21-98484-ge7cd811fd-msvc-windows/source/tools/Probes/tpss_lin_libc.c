/*
 * Copyright (C) 2015-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * This tool mimics the behavior of TPSS on Linux by adding probes to various libc functions.
 * However, in this tool these probes are merely empty wrappers that call the original functions.
 * The objective of the test is to verify that probe generation and insertion don't cause Pin
 * to crash.
 *
 * This file is part of the tpss_lin_libc tool and compiles against the native
 * libc of the machine/compiler in order to extact data types definition from it's headers.
 */

#define _GNU_SOURCE
#include <link.h>
#include <sys/types.h>
#include <sys/timeb.h>
#include <rpc/rpc.h>
#include <rpc/pmap_clnt.h>
#include <semaphore.h>
#include <dlfcn.h>
#include <signal.h>
#include <poll.h>
#include <time.h>
#include <link.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/uio.h>
#include <sys/ioctl.h>
#include <sys/file.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/mman.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include <sys/socket.h>
#include <sys/utsname.h>
#include <sched.h>
#include <time.h>

typedef int* INT_PTR;

typedef void* VOID_PTR;

typedef char* CHAR_PTR;

void printFunctionCalled(const char* funcName);

/* ===================================================================== */
/* Function signatures - these functions will be probed                  */
/* ===================================================================== */

int (*fptrnanosleep)(const struct timespec* __rqtp, struct timespec* __rmtp);

int (*fptrdl_iterate_phdr)(int (*__callback)(struct dl_phdr_info* __info, size_t __size, VOID_PTR __data), VOID_PTR __data);

int (*fptrsystem)(const CHAR_PTR __command);

unsigned int (*fptralarm)(unsigned int __seconds);

ssize_t (*fptrrecvmsg)(int __socket, struct msghdr* __message, int __flags);

ssize_t (*fptrsendmsg)(int __sockfd, const struct msghdr* __msg, int __flags);

int (*fptrpause)(void);

int (*fptrsigtimedwait)(const sigset_t* __set, siginfo_t* __info, const struct timespec* __timeout);

int (*fptrsigwaitinfo)(const sigset_t* __set, siginfo_t* __info);

int (*fptrepoll_wait)(int __epfd, struct epoll_event* __events, int __maxevents, int __timeout);

int (*fptrepoll_pwait)(int __epfd, struct epoll_event* __events, int __maxevents, int __timeout, const sigset_t* __sigmask);

int (*fptrppoll)(struct pollfd* __fds, nfds_t __nfds, const struct timespec* __timeout_ts, const sigset_t* __sigmask);

int (*fptrmsgsnd)(int __msqid, const VOID_PTR __msgp, size_t __msgsz, int __msgflg);

ssize_t (*fptrmsgrcv)(int __msqid, VOID_PTR __msgp, size_t __msgsz, long __msgtyp, int __msgflg);

int (*fptrsemop)(int __semid, struct sembuf* __sops, unsigned __nsops);

int (*fptrsemtimedop)(int __semid, struct sembuf* __sops);

int (*fptrusleep)(useconds_t __usec);

useconds_t (*fptrualarm)(useconds_t __usecs, useconds_t __interval);

int (*fptrgetitimer)(int __which, struct itimerval* __curr_value);

int (*fptrsigwait)(const sigset_t* __set, INT_PTR __sig);

int (*fptrmsgget)(key_t __key, int __msgflg);

int (*fptrsemget)(key_t __key, int __nsems, int __semflg);

pid_t (*fptrwait)(INT_PTR __status);

pid_t (*fptrwaitpid)(pid_t __pid, INT_PTR __atus, int __options);

int (*fptrwaitid)(idtype_t __idtype, id_t __id, siginfo_t* __infop, int __options);

pid_t (*fptrwait3)(INT_PTR __status, int __options, struct rusage* __rusage);

pid_t (*fptrwait4)(pid_t __pid, INT_PTR status, int __options, struct rusage* __rusage);

ssize_t (*fptrreadv)(int __fd, const struct iovec* __iov, int __iovcnt);

ssize_t (*fptrwritev)(int __fd, const struct iovec* __iov, int __iovcnt);

int (*fptrflock)(int __fd, int __operation);

void (*fptrflockfile)(FILE* __filehandle);

void (*fptrfunlockfile)(FILE* __filehandle);

int (*fptrlockf)(int __fd, int __cmd, off_t __len);

int (*fptrsetenv)(const CHAR_PTR __name, const CHAR_PTR __value, int __overwrite);

int (*fptrunsetenv)(const CHAR_PTR __name);

CHAR_PTR (*fptrgetenv)(const CHAR_PTR __name);

void (*fptrperror)(const CHAR_PTR __s);

VOID_PTR (*fptrmmap)(VOID_PTR __addr, size_t __len, int __prot, int __flags, int __fildes, off_t __off);

int (*fptrmunmap)(VOID_PTR __addr, size_t __len);

int (*fptrfileno)(FILE* __stream);

pid_t (*fptrgetpid)(void);

pid_t (*fptrgetppid)(void);

VOID_PTR (*fptrmemset)(VOID_PTR __s, int __c, size_t __n);

VOID_PTR (*fptrmemcpy)(VOID_PTR __dest, const VOID_PTR __src, size_t __n);

int (*fptraccess)(const CHAR_PTR __pathname, int __mode);

off_t (*fptrlseek)(int __fd, off_t __offset, int __whence);

off64_t (*fptrlseek64)(int __fd, off64_t __offset, int __whence);

int (*fptrfdatasync)(int __fd);

int (*fptrunlink)(const CHAR_PTR __pathname);

size_t (*fptrstrlen)(const CHAR_PTR __s);

size_t (*fptrwcslen)(const wchar_t* __s);

CHAR_PTR (*fptrstrcpy)(CHAR_PTR __dest, const CHAR_PTR __src);

CHAR_PTR (*fptrstrncpy)(CHAR_PTR __dest, const CHAR_PTR __src, size_t __n);

CHAR_PTR (*fptrstrcat)(CHAR_PTR __dest, const CHAR_PTR __src);

CHAR_PTR (*fptrstrstr)(const CHAR_PTR __haystack, const CHAR_PTR __needle);

CHAR_PTR (*fptrstrchr0)(const CHAR_PTR __s, int __c);

CHAR_PTR (*fptrstrrchr)(const CHAR_PTR __s, int __c);

int (*fptrstrcmp)(const CHAR_PTR __s1, const CHAR_PTR __s2);

int (*fptrstrncmp)(const CHAR_PTR __s1, const CHAR_PTR __s2, size_t __n);

int (*fptrsigaddset)(sigset_t* __set, int __signum);

int (*fptrsigdelset)(sigset_t* __set, int __signum);

int (*fptrsigismember)(const sigset_t* __set, int __signum);

CHAR_PTR (*fptrstrerror)(int __errnum);

int (*fptrbind)(int __sockfd, const struct sockaddr* __addr, socklen_t __addrlen);

int (*fptrlisten)(int __sockfd, int __backlog);

int (*fptruname)(struct utsname* __name);

int (*fptrgethostname)(CHAR_PTR __name, size_t __len);

int (*fptrkill)(pid_t __pid, int __sig);

int (*fptrsched_yield)(void);

int (*fptrtimer_settime)(timer_t __timerid, int __flags, const struct itimerspec* __value, struct itimerspec* __ovalue);

int (*fptrsigaltstack)(const stack_t* __ss, stack_t* __oss);

int (*fptrshutdown)(int, int);

int (*fptrsleep)(unsigned int);

int (*fptrsocket)(int, int, int);

int (*fptrselect)(int, fd_set* __restrict, fd_set* __restrict, fd_set* __restrict, struct timeval* __restrict);

int (*fptrpoll)(struct pollfd*, nfds_t, int);

int (*fptraccept)(int, __SOCKADDR_ARG, socklen_t* __restrict);

int (*fptrconnect)(int, __CONST_SOCKADDR_ARG, socklen_t);

ssize_t (*fptrrecv)(int, VOID_PTR, size_t, int);

ssize_t (*fptrrecvfrom)(int, VOID_PTR __restrict, size_t, int, __SOCKADDR_ARG, socklen_t* __restrict);

ssize_t (*fptrsend)(int, __const VOID_PTR, size_t, int);

wint_t (*fptrgetwc)(__FILE*);

int (*fptrsetitimer)(__itimer_which_t, __const struct itimerval* __restrict, struct itimerval* __restrict);

int (*fptrsigpending)(sigset_t*);

int (*fptrsigaction)(int, __const struct sigaction* __restrict, struct sigaction* __restrict);

__sighandler_t (*fptrsignal)(int, __sighandler_t);

void (*fptrabort)();

ssize_t (*fptrsendto)(int, __const VOID_PTR, size_t, int, __CONST_SOCKADDR_ARG, socklen_t);

int (*fptr_IO_getc)(FILE*);

int (*fptrgetchar)();

wint_t (*fptrgetwchar)();

CHAR_PTR (*fptrgets)(CHAR_PTR);

CHAR_PTR (*fptrfgets)(CHAR_PTR __restrict, int, FILE* __restrict);

wint_t (*fptrfgetwc)(__FILE*);

size_t (*fptrfread)(VOID_PTR __restrict, size_t, size_t, FILE* __restrict);

size_t (*fptrfwrite)(__const VOID_PTR __restrict, size_t, size_t, FILE* __restrict);

int (*fptropen)(__const CHAR_PTR, int, mode_t);

int (*fptrgetw)(FILE*);

void (*fptrfgetc)(__FILE*);

wchar_t* (*fptrfgetws)(wchar_t* __restrict, int, __FILE* __restrict);

int (*fptrpipe)(int[2]);

ssize_t (*fptrread)(int, VOID_PTR, size_t);

ssize_t (*fptrwrite)(int, __const VOID_PTR, size_t);

FILE* (*fptrfopen)(__const CHAR_PTR __restrict, __const CHAR_PTR __restrict);

FILE* (*fptrfdopen)(int, __const CHAR_PTR);

int (*fptrclose)(int);

int (*fptrfclose)(FILE*);

int (*fptrcallrpc)(__const CHAR_PTR, u_long, u_long, u_long, xdrproc_t, __const CHAR_PTR, xdrproc_t, CHAR_PTR);

enum clnt_stat (*fptrclnt_broadcast)(u_long, u_long, u_long, xdrproc_t, caddr_t, xdrproc_t, caddr_t, resultproc_t);

CLIENT* (*fptrclntudp_create)(struct sockaddr_in*, u_long, u_long, struct timeval, INT_PTR);

CLIENT* (*fptrclntudp_bufcreate)(struct sockaddr_in*, u_long, u_long, struct timeval, INT_PTR, u_int, u_int);

struct pmaplist* (*fptrpmap_getmaps)(struct sockaddr_in*);

u_short (*fptrpmap_getport)(struct sockaddr_in*, u_long, u_long, u_int);

enum clnt_stat (*fptrpmap_rmtcall)(struct sockaddr_in*, u_long, u_long, u_long, xdrproc_t, caddr_t, xdrproc_t, caddr_t,
                                   struct timeval, u_long*);

bool_t (*fptrpmap_set)(u_long, u_long, int, u_short);

CLIENT* (*fptrclntraw_create)(u_long, u_long);

void (*fptrsvc_run)();

bool_t (*fptrsvc_sendreply)(SVCXPRT*, xdrproc_t, caddr_t);

SVCXPRT* (*fptrsvcraw_create)();

SVCXPRT* (*fptrsvctcp_create)(int, u_int, u_int);

SVCXPRT* (*fptrsvcudp_bufcreate)(int, u_int, u_int);

SVCXPRT* (*fptrsvcudp_create)(int);

void (*fptr_exit)(int);

int (*fptrsigprocmask)(int, __const sigset_t* __restrict, sigset_t* __restrict);

void (*fptrexit)(int);

int (*fptrpselect)(int, fd_set* __restrict, fd_set* __restrict, fd_set* __restrict, const struct timespec* __restrict,
                   const __sigset_t* __restrict);

int (*fptrioctl)(int __d, int __request, CHAR_PTR __argp);

int (*fptrfcntl)(int __fd, int __cmd, VOID_PTR __argp);

VOID_PTR (*fptr__libc_dlopen_mode)(const CHAR_PTR __name, int __mode);

INT_PTR (*fptr__errno_location)(void);

int (*fptrsyscall)(int __number, long int __arg1, long int __arg2, long int __arg3, long int __arg4, long int __arg5,
                   long int __arg6, long int __arg7);

/* ===================================================================== */
/* Probes - implementation of the wrapper functions                      */
/* ===================================================================== */

int mynanosleep(const struct timespec* __rqtp, struct timespec* __rmtp)
{
    printFunctionCalled("mynanosleep");
    int res = fptrnanosleep(__rqtp, __rmtp);

    return res;
}
int mydl_iterate_phdr(int (*__callback)(struct dl_phdr_info* __info, size_t __size, VOID_PTR __data), VOID_PTR __sec_data)
{
    printFunctionCalled("mydl_iterate_phdr");
    int res = fptrdl_iterate_phdr((__callback), __sec_data);
    return res;
}

int mysystem(const CHAR_PTR __command)
{
    printFunctionCalled("mysystem");
    int res = fptrsystem(__command);

    return res;
}

unsigned int myalarm(unsigned int __seconds)
{
    printFunctionCalled("myalarm");
    unsigned int res = fptralarm(__seconds);

    return res;
}

ssize_t myrecvmsg(int __socket, struct msghdr* __message, int __flags)
{
    printFunctionCalled("myrecvmsg");
    ssize_t res = fptrrecvmsg(__socket, __message, __flags);

    return res;
}

ssize_t mysendmsg(int __sockfd, const struct msghdr* __msg, int __flags)
{
    printFunctionCalled("mysendmsg");
    ssize_t res = fptrsendmsg(__sockfd, __msg, __flags);

    return res;
}

int mypause(void)
{
    printFunctionCalled("mypause");
    int res = fptrpause();

    return res;
}

int mysigtimedwait(const sigset_t* __set, siginfo_t* __info, const struct timespec* __timeout)
{
    printFunctionCalled("mysigtimedwait");
    int res = fptrsigtimedwait(__set, __info, __timeout);

    return res;
}

int mysigwaitinfo(const sigset_t* __set, siginfo_t* __info)
{
    printFunctionCalled("mysigwaitinfo");
    int res = fptrsigwaitinfo(__set, __info);

    return res;
}

int myepoll_wait(int __epfd, struct epoll_event* __events, int __maxevents, int __timeout)
{
    printFunctionCalled("myepoll_wait");
    int res = fptrepoll_wait(__epfd, __events, __maxevents, __timeout);

    return res;
}

int myepoll_pwait(int __epfd, struct epoll_event* __events, int __maxevents, int __timeout)
{
    printFunctionCalled("myepoll_pwait");
    int res = fptrepoll_wait(__epfd, __events, __maxevents, __timeout);

    return res;
}

int myppoll(struct pollfd* __fds, nfds_t __nfds, const struct timespec* __timeout_ts, const sigset_t* __sigmask)
{
    printFunctionCalled("myppoll");
    int res = fptrppoll(__fds, __nfds, __timeout_ts, __sigmask);

    return res;
}

int mymsgsnd(int __msqid, const VOID_PTR __msgp, size_t __msgsz, int __msgflg)
{
    printFunctionCalled("mymsgsnd");
    int res = fptrmsgsnd(__msqid, __msgp, __msgsz, __msgflg);

    return res;
}

ssize_t mymsgrcv(int __msqid, VOID_PTR __msgp, size_t __msgsz, long __msgtyp, int __msgflg)
{
    printFunctionCalled("mymsgrcv");
    ssize_t res = fptrmsgrcv(__msqid, __msgp, __msgsz, __msgtyp, __msgflg);

    return res;
}

int mysemtimedop(int __semid, struct sembuf* __sops)
{
    printFunctionCalled("mysemtimedop");
    int res = fptrsemtimedop(__semid, __sops);

    return res;
}

int myusleep(useconds_t __usecs)
{
    printFunctionCalled("myusleep");
    int res = fptrusleep(__usecs);

    return res;
}

useconds_t myualarm(useconds_t __usecs, useconds_t __interval)
{
    printFunctionCalled("myualarm");
    useconds_t res = fptrualarm(__usecs, __interval);

    return res;
}

int mygetitimer(int __which, struct itimerval* __curr_value)
{
    printFunctionCalled("mygetitimer");
    int res = fptrgetitimer(__which, __curr_value);

    return res;
}

int mysigwait(const sigset_t* __set, INT_PTR __sig)
{
    printFunctionCalled("mysigwait");
    int res = fptrsigwait(__set, __sig);

    return res;
}

int mymsgget(key_t __key, int __msgflg)
{
    printFunctionCalled("mymsgget");
    int res = fptrmsgget(__key, __msgflg);

    return res;
}

int mysemget(key_t __key, int __nsems, int __semflg)
{
    printFunctionCalled("mysemget");
    int res = fptrsemget(__key, __nsems, __semflg);

    return res;
}

pid_t mywaitpid(pid_t __pid, INT_PTR __status, int __options)
{
    printFunctionCalled("mywaitpid");
    pid_t res = fptrwaitpid(__pid, __status, __options);

    return res;
}

int mywaitid(idtype_t __idtype, id_t __id, siginfo_t* __infop, int __options)
{
    printFunctionCalled("mywaittid");
    int res = fptrwaitid(__idtype, __id, __infop, __options);

    return res;
}

pid_t mywait3(INT_PTR __status, int __options, struct rusage* __rusage)
{
    printFunctionCalled("mywait3");
    pid_t res = fptrwait3(__status, __options, __rusage);

    return res;
}

pid_t mywait4(pid_t __pid, INT_PTR __status, int __options, struct rusage* __rusage)
{
    printFunctionCalled("mywait4");
    pid_t res = fptrwait4(__pid, __status, __options, __rusage);

    return res;
}

ssize_t myreadv(int __fd, const struct iovec* __iov, int __iovcnt)
{
    printFunctionCalled("myreadv");
    ssize_t res = fptrreadv(__fd, __iov, __iovcnt);

    return res;
}

ssize_t mywritev(int __fd, const struct iovec* __iov, int __iovcnt)
{
    printFunctionCalled("mywritev");
    ssize_t res = fptrwritev(__fd, __iov, __iovcnt);

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

int mylockf(int __fd, int __cmd, off_t __len)
{
    printFunctionCalled("mylockf");
    int res = fptrlockf(__fd, __cmd, __len);

    return res;
}

int mysetenv(const CHAR_PTR __name, const CHAR_PTR __value, int __overwrite)
{
    printFunctionCalled("mysetenv");
    int res = fptrsetenv(__name, __value, __overwrite);

    return res;
}

int myunsetenv(const CHAR_PTR __name)
{
    printFunctionCalled("myunsetenv");
    int res = fptrunsetenv(__name);

    return res;
}

CHAR_PTR mygetenv(const CHAR_PTR __name)
{
    printFunctionCalled("mygetenv");
    CHAR_PTR res = fptrgetenv(__name);

    return res;
}

void myperror(const CHAR_PTR __s)
{
    printFunctionCalled("myperror");
    fptrperror(__s);
}

VOID_PTR mymmap(VOID_PTR __addr, size_t __len, int __prot, int __flags, int __fildes, off_t __off)
{
    printFunctionCalled("mymmap");
    VOID_PTR res = fptrmmap(__addr, __len, __prot, __flags, __fildes, __off);

    return res;
}

int mymunmap(VOID_PTR __addr, size_t __len)
{
    printFunctionCalled("mymunmap");
    int res = fptrmunmap(__addr, __len);

    return res;
}

int myfileno(FILE* __stream)
{
    printFunctionCalled("myfileno");
    int res = fptrfileno(__stream);

    return res;
}

pid_t mygetpid(void)
{
    printFunctionCalled("mygetpid");
    pid_t res = fptrgetpid();

    return res;
}

pid_t mygetppid(void)
{
    printFunctionCalled("mygetppid");
    pid_t res = fptrgetppid();

    return res;
}

VOID_PTR mymemset(VOID_PTR __s, int __c, size_t __n)
{
    printFunctionCalled("mymemset");
    VOID_PTR res = fptrmemset(__s, __c, __n);

    return res;
}

VOID_PTR mymemcpy(VOID_PTR __dest, const VOID_PTR __src, size_t __n)
{
    printFunctionCalled("mymemcpy");
    VOID_PTR res = fptrmemcpy(__dest, __src, __n);

    return res;
}

int myaccess(const CHAR_PTR __pathname, int __mode)
{
    printFunctionCalled("myaccess");
    int res = fptraccess(__pathname, __mode);

    return res;
}

off_t mylseek(int __fd, off_t __offset, int __whence)
{
    printFunctionCalled("mylseek");
    off_t res = fptrlseek(__fd, __offset, __whence);

    return res;
}

off64_t mylseek64(int __fd, off64_t __offset, int __whence)
{
    printFunctionCalled("mylseek64");
    off64_t res = fptrlseek64(__fd, __offset, __whence);

    return res;
}

int myfdatasync(int __fd)
{
    printFunctionCalled("myfdatasync");
    int res = fptrfdatasync(__fd);

    return res;
}

int myunlink(const CHAR_PTR __pathname)
{
    printFunctionCalled("myunlink");
    int res = fptrunlink(__pathname);

    return res;
}

size_t mystrlen(const CHAR_PTR __s)
{
    printFunctionCalled("mystrlen");
    size_t res = fptrstrlen(__s);

    return res;
}

size_t mywcslen(const wchar_t* __s)
{
    printFunctionCalled("mywcslen");
    size_t res = fptrwcslen(__s);

    return res;
}

CHAR_PTR mystrcpy(CHAR_PTR __dest, const CHAR_PTR __src)
{
    printFunctionCalled("mystrcpy");
    CHAR_PTR res = fptrstrcpy(__dest, __src);

    return res;
}

CHAR_PTR mystrncpy(CHAR_PTR __dest, const CHAR_PTR __src, size_t __n)
{
    printFunctionCalled("mystrncpy");
    CHAR_PTR res = fptrstrncpy(__dest, __src, __n);

    return res;
}

CHAR_PTR mystrcat(CHAR_PTR __dest, const CHAR_PTR __src)
{
    printFunctionCalled("mystrcat");
    CHAR_PTR res = fptrstrcat(__dest, __src);

    return res;
}

CHAR_PTR mystrstr(const CHAR_PTR __haystack, const CHAR_PTR __needle)
{
    printFunctionCalled("mystrstr");
    CHAR_PTR res = fptrstrstr(__haystack, __needle);

    return res;
}

CHAR_PTR mystrchr0(const CHAR_PTR __s, int __c)
{
    printFunctionCalled("mystrchr0");
    CHAR_PTR res = fptrstrchr0(__s, __c);

    return res;
}

CHAR_PTR mystrrchr(const CHAR_PTR __s, int __c)
{
    printFunctionCalled("mystrrchr");
    CHAR_PTR res = fptrstrrchr(__s, __c);

    return res;
}

int mystrcmp(const CHAR_PTR __s1, const CHAR_PTR __s2)
{
    printFunctionCalled("mystrcmp");
    int res = fptrstrcmp(__s1, __s2);

    return res;
}

int mystrncmp(const CHAR_PTR __s1, const CHAR_PTR __s2, size_t __n)
{
    printFunctionCalled("mystrncmp");
    int res = fptrstrncmp(__s1, __s2, __n);

    return res;
}

int mysigaddset(sigset_t* __set, int __signum)
{
    printFunctionCalled("mysigaddset");
    int res = fptrsigaddset(__set, __signum);

    return res;
}

int mysigdelset(sigset_t* __set, int __signum)
{
    printFunctionCalled("mysigdelset");
    int res = fptrsigdelset(__set, __signum);

    return res;
}

int mysigismember(sigset_t* __set, int __signum)
{
    printFunctionCalled("mysigismember");
    int res = fptrsigismember(__set, __signum);

    return res;
}

CHAR_PTR mystrerror(int __errnum)
{
    printFunctionCalled("mystrerror");
    CHAR_PTR res = fptrstrerror(__errnum);

    return res;
}

int mybind(int __sockfd, const struct sockaddr* __addr, socklen_t __addrlen)
{
    printFunctionCalled("mybind");
    int res = fptrbind(__sockfd, __addr, __addrlen);

    return res;
}

int mylisten(int __sockfd, int __backlog)
{
    printFunctionCalled("mylisten");
    int res = fptrlisten(__sockfd, __backlog);

    return res;
}

int myuname(struct utsname* __name)
{
    printFunctionCalled("myuname");
    int res = fptruname(__name);

    return res;
}

int mygethostname(CHAR_PTR __name, size_t __len)
{
    printFunctionCalled("mygethostname");
    int res = fptrgethostname(__name, __len);

    return res;
}

int mykill(pid_t __pid, int __sig)
{
    printFunctionCalled("mykill");
    int res = fptrkill(__pid, __sig);

    return res;
}

int mysched_yield(void)
{
    printFunctionCalled("mysched_yield");
    int res = fptrsched_yield();

    return res;
}

int mytimer_settime(timer_t __timerid, int __flags, const struct itimerspec* __value, struct itimerspec* __ovalue)
{
    printFunctionCalled("mytimer_settime");
    int res = fptrtimer_settime(__timerid, __flags, __value, __ovalue);

    return res;
}

int mysigaltstack(const stack_t* __ss, stack_t* __oss)
{
    printFunctionCalled("mysigaltstacke");
    int res = fptrsigaltstack(__ss, __oss);

    return res;
}

int mysleep(unsigned int __seconds)
{
    printFunctionCalled("mysleep");
    int res = fptrsleep(__seconds);

    return res;
}

int mysocket(int __domain, int __type, int __protocol)
{
    printFunctionCalled("mysocket");
    int res = fptrsocket(__domain, __type, __protocol);

    return res;
}

int myshutdown(int __fd, int __how)
{
    printFunctionCalled("myshutdown");
    int res = fptrshutdown(__fd, __how);

    return res;
}

int myselect(int __nfds, fd_set* __restrict __readfds, fd_set* __restrict __writefds, fd_set* __restrict __exceptfds,
             struct timeval* __restrict __timeout)
{
    printFunctionCalled("myselect");
    int res = fptrselect(__nfds, __readfds, __writefds, __exceptfds, __timeout);

    return res;
}

int mypoll(struct pollfd* __fds, nfds_t __nfds, int __timeout)
{
    printFunctionCalled("mypoll");
    int res = fptrpoll(__fds, __nfds, __timeout);

    return res;
}

int myaccept(int __fd, __SOCKADDR_ARG __addr, socklen_t* __restrict __addr_len)
{
    printFunctionCalled("myaccept");
    int res = fptraccept(__fd, __addr, __addr_len);

    return res;
}

int myconnect(int __fd, __CONST_SOCKADDR_ARG __addr, socklen_t __len)
{
    printFunctionCalled("myconnect");
    int res = fptrconnect(__fd, __addr, __len);

    return res;
}

ssize_t myrecv(int __fd, VOID_PTR __buf, size_t __n, int __flags)
{
    printFunctionCalled("myrecv");
    ssize_t res = fptrrecv(__fd, __buf, __n, __flags);

    return res;
}

ssize_t myrecvfrom(int __fd, VOID_PTR __restrict __buf, size_t __n, int __flags, __SOCKADDR_ARG __addr,
                   socklen_t* __restrict __addr_len)
{
    printFunctionCalled("myrecvfrom");
    ssize_t res = fptrrecvfrom(__fd, __buf, __n, __flags, __addr, __addr_len);

    return res;
}

ssize_t mysend(int __fd, __const VOID_PTR __buf, size_t __n, int __flags)
{
    printFunctionCalled("mysend");
    ssize_t res = fptrsend(__fd, __buf, __n, __flags);

    return res;
}

wint_t mygetwc(__FILE* __stream)
{
    printFunctionCalled("mygetwc");
    wint_t res = fptrgetwc(__stream);

    return res;
}

void myexit(int __status)
{
    printFunctionCalled("myexit");
    fptrexit(__status);
}

int mysetitimer(__itimer_which_t __which, __const struct itimerval* __restrict __new, struct itimerval* __restrict __old)
{
    printFunctionCalled("mysetitimer");
    int res = fptrsetitimer(__which, __new, __old);

    return res;
}

int mysigpending(sigset_t* __set)
{
    printFunctionCalled("mysigpending");
    int res = fptrsigpending(__set);

    return res;
}

__sighandler_t mysignal(int __sig, __sighandler_t __handler)

{
    printFunctionCalled("mysignal");
    __sighandler_t res = fptrsignal(__sig, __handler);

    return res;
}

void myabort()
{
    printFunctionCalled("myabort");
    fptrabort();
}

int myclose(int __fd)
{
    printFunctionCalled("myclose");
    int res = fptrclose(__fd);

    return res;
}

ssize_t mysendto(int __fd, __const VOID_PTR __buf, size_t __n, int __flags, __CONST_SOCKADDR_ARG __addr, socklen_t __addr_len)
{
    printFunctionCalled("mysendto");
    ssize_t res = fptrsendto(__fd, __buf, __n, __flags, __addr, __addr_len);

    return res;
}

int my_IO_getc(FILE* __stream)
{
    printFunctionCalled("my_IO_getc");
    int res = fptr_IO_getc(__stream);

    return res;
}

int mygetchar()
{
    printFunctionCalled("mygetchar");
    int res = fptrgetchar();

    return res;
}

wint_t mygetwchar()
{
    printFunctionCalled("mygetwchar");
    wint_t res = fptrgetwchar();

    return res;
}

CHAR_PTR mygets(CHAR_PTR __s)
{
    printFunctionCalled("mygets");
    CHAR_PTR res = fptrgets(__s);

    return res;
}

CHAR_PTR myfgets(CHAR_PTR __restrict __s, int __n, FILE* __restrict __stream)
{
    printFunctionCalled("myfgets");
    CHAR_PTR res = fptrfgets(__s, __n, __stream);

    return res;
}

wint_t myfgetwc(__FILE* __stream)
{
    printFunctionCalled("myfgetwc");
    wint_t res = fptrfgetwc(__stream);

    return res;
}

size_t myfread(VOID_PTR __restrict __ptr, size_t __size, size_t __n, FILE* __restrict __stream)
{
    printFunctionCalled("myfread");
    size_t res = fptrfread(__ptr, __size, __n, __stream);

    return res;
}

size_t myfwrite(__const VOID_PTR __restrict __ptr, size_t __size, size_t __n, FILE* __restrict __s)
{
    printFunctionCalled("myfwrite");
    size_t res = fptrfwrite(__ptr, __size, __n, __s);

    return res;
}

int myopen(__const CHAR_PTR __file, int __flags, mode_t __mode)
{
    printFunctionCalled("myopen");
    int res = fptropen(__file, __flags, __mode);

    return res;
}

int mygetw(FILE* __stream)
{
    printFunctionCalled("mygetw");
    int res = fptrgetw(__stream);

    return res;
}

void myfgetc(__FILE* __stream)
{
    printFunctionCalled("myfgetc");
    fptrfgetc(__stream);
}

wchar_t* myfgetws(wchar_t* __restrict __ws, int __n, __FILE* __restrict __stream)
{
    printFunctionCalled("myfgetws");
    wchar_t* res = fptrfgetws(__ws, __n, __stream);

    return res;
}

int mypipe(int* __pipedes)
{
    printFunctionCalled("mypipe");
    int res = fptrpipe(__pipedes);

    return res;
}

ssize_t myread(int __fd, VOID_PTR __buf, size_t __nbytes)
{
    printFunctionCalled("myread");
    ssize_t res = fptrread(__fd, __buf, __nbytes);

    return res;
}

ssize_t mywrite(int __fd, __const VOID_PTR __buf, size_t __n)
{
    printFunctionCalled("mywrite");
    ssize_t res = fptrwrite(__fd, __buf, __n);

    return res;
}

FILE* myfopen(__const CHAR_PTR __restrict __filename, __const CHAR_PTR __restrict __modes)
{
    printFunctionCalled("myfopen");
    FILE* res = fptrfopen(__filename, __modes);

    return res;
}

FILE* myfdopen(int __fd, __const CHAR_PTR __modes)
{
    printFunctionCalled("myfdopen");
    FILE* res = fptrfdopen(__fd, __modes);

    return res;
}

int mycallrpc(__const CHAR_PTR __host, u_long __prognum, u_long __versnum, u_long __procnum, xdrproc_t __inproc,
              __const CHAR_PTR __in, xdrproc_t __outproc, CHAR_PTR __out)
{
    printFunctionCalled("mycallrpc");
    int res = fptrcallrpc(__host, __prognum, __versnum, __procnum, __inproc, __in, __outproc, __out);

    return res;
}

enum clnt_stat myclnt_broadcast(u_long __prog, u_long __vers, u_long __proc, xdrproc_t __xargs, caddr_t __argsp,
                                xdrproc_t __xresults, caddr_t __resultsp, resultproc_t __eachresult)
{
    printFunctionCalled("myclnt_broadcast");
    enum clnt_stat res = fptrclnt_broadcast(__prog, __vers, __proc, __xargs, __argsp, __xresults, __resultsp, __eachresult);

    return res;
}

CLIENT* myclntudp_create(struct sockaddr_in* __raddr, u_long __program, u_long __version, struct timeval __wait_resend,
                         INT_PTR __sockp)
{
    printFunctionCalled("myclntudp_create");
    CLIENT* res = fptrclntudp_create(__raddr, __program, __version, __wait_resend, __sockp);

    return res;
}

CLIENT* myclntudp_bufcreate(struct sockaddr_in* __raddr, u_long __program, u_long __version, struct timeval __wait_resend,
                            INT_PTR __sockp, u_int __sendsz, u_int __recvsz)
{
    printFunctionCalled("myclntudp_bufcreate");
    CLIENT* res = fptrclntudp_bufcreate(__raddr, __program, __version, __wait_resend, __sockp, __sendsz, __recvsz);

    return res;
}

struct pmaplist* mypmap_getmaps(struct sockaddr_in* __address)
{
    printFunctionCalled("mypmap_getmaps");
    struct pmaplist* res = fptrpmap_getmaps(__address);

    return res;
}

u_short mypmap_getport(struct sockaddr_in* __address, u_long __program, u_long __version, u_int __protocol)
{
    printFunctionCalled("mypmap_getport");
    u_short res = fptrpmap_getport(__address, __program, __version, __protocol);

    return res;
}

enum clnt_stat mypmap_rmtcall(struct sockaddr_in* __addr, u_long __prog, u_long __vers, u_long __proc, xdrproc_t __xdrargs,
                              caddr_t __argsp, xdrproc_t __xdrres, caddr_t __resp, struct timeval __tout, u_long* __port_ptr)
{
    printFunctionCalled("mypmap_rmtcall");
    enum clnt_stat res =
        fptrpmap_rmtcall(__addr, __prog, __vers, __proc, __xdrargs, __argsp, __xdrres, __resp, __tout, __port_ptr);

    return res;
}

bool_t mypmap_set(u_long __program, u_long __vers, int __protocol, u_short __port)
{
    printFunctionCalled("mypmap_set");
    bool_t res = fptrpmap_set(__program, __vers, __protocol, __port);

    return res;
}

CLIENT* myclntraw_create(u_long __prog, u_long __vers)
{
    printFunctionCalled("myclntraw_create");
    CLIENT* res = fptrclntraw_create(__prog, __vers);

    return res;
}

void mysvc_run()
{
    printFunctionCalled("mysvc_run");
    fptrsvc_run();
}

bool_t mysvc_sendreply(SVCXPRT* xprt, xdrproc_t __xdr_results, caddr_t __xdr_location)
{
    printFunctionCalled("mysvc_sendreply");
    bool_t res = fptrsvc_sendreply(xprt, __xdr_results, __xdr_location);

    return res;
}

SVCXPRT* mysvcraw_create()
{
    printFunctionCalled("mysvcraw_create");
    SVCXPRT* res = fptrsvcraw_create();

    return res;
}

SVCXPRT* mysvctcp_create(int __sock, u_int __sendsize, u_int __recvsize)
{
    printFunctionCalled("mypmap_rmtcall");
    SVCXPRT* res = fptrsvctcp_create(__sock, __sendsize, __recvsize);

    return res;
}

SVCXPRT* mysvcudp_bufcreate(int __sock, u_int __sendsz, u_int __recvsz)
{
    printFunctionCalled("mysvcudp_bufcreate");
    SVCXPRT* res = fptrsvcudp_bufcreate(__sock, __sendsz, __recvsz);

    return res;
}

SVCXPRT* mysvcudp_create(int __sock)
{
    printFunctionCalled("mysvcudp_create");
    SVCXPRT* res = fptrsvcudp_create(__sock);

    return res;
}

void my_exit(int __status)
{
    printFunctionCalled("my_exit");
    fptr_exit(__status);
}

int my_nanosleep(const struct timespec* __rqtp, struct timespec* __rmtp)
{
    printFunctionCalled("my_nanosleep");
    int res = fptrnanosleep(__rqtp, __rmtp);

    return res;
}

int mysigprocmask(int __how, __const sigset_t* __restrict __set, sigset_t* __restrict __oset)
{
    printFunctionCalled("mysigprocmask");
    int res = fptrsigprocmask(__how, __set, __oset);

    return res;
}

int mypselect(int __nfds, fd_set* __restrict __readfds, fd_set* __restrict __writefds, fd_set* __restrict __exceptfds,
              const struct timespec* __restrict __timeout, const __sigset_t* __restrict __sigmask)
{
    printFunctionCalled("mypselect");
    int res = fptrpselect(__nfds, __readfds, __writefds, __exceptfds, __timeout, __sigmask);

    return res;
}

pid_t mywait(INT_PTR __status)
{
    printFunctionCalled("mywait");
    int res = fptrwait(__status);

    return res;
}

int myfclose(FILE* __stream)
{
    printFunctionCalled("myfclose");
    int res = fptrfclose(__stream);

    return res;
}

int myioctl(int __d, int __request, CHAR_PTR __argp)
{
    printFunctionCalled("myioctl");
    int res = fptrioctl(__d, __request, __argp);

    return res;
}

int myfcntl(int __fd, int __cmd, VOID_PTR __argp)
{
    printFunctionCalled("myfcntl");
    int res = fptrfcntl(__fd, __cmd, __argp);

    return res;
}

VOID_PTR my__libc_dlopen_mode(const CHAR_PTR __name, int __mode)
{
    printFunctionCalled("my__libc_dlopen_mode");
    VOID_PTR res = fptr__libc_dlopen_mode(__name, __mode);

    return res;
}

INT_PTR my__errno_location(void)
{
    printFunctionCalled("my__errno_location");
    INT_PTR res = fptr__errno_location();

    return res;
}

int mysyscall(int __number, long int __arg1, long int __arg2, long int __arg3, long int __arg4, long int __arg5, long int __arg6,
              long int __arg7)
{
    printFunctionCalled("mysyscall");
    int res = fptrsyscall(__number, __arg1, __arg2, __arg3, __arg4, __arg5, __arg6, __arg7);

    return res;
}

int mysigaction(int __sig, __const struct sigaction* __restrict __act, struct sigaction* __restrict __oact)
{
    printFunctionCalled("mysigaction");
    int res = fptrsigaction(__sig, __act, __oact);

    return res;
}
