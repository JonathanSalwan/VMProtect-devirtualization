/*
 * Copyright (C) 2004-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include "pin.H"

#if defined(PIN_GNU_COMPATIBLE)
#endif

#if !defined(TARGET_WINDOWS)
#include <sys/syscall.h>
#include <iostream>

#include "syscall_names.H"

// This is a generic isa/kernel version method to translate syscall numbers to
// names. Every kernel has different syscalls, and different numbers for the
// same syscalls, so this is a static map from numbers to names.
// Hopefully g++ handles switches well
//
// To add a syscall "foo"
// In syscall_names.H:
// 1.  Add NUM_SYS_foo to the end of SYSCALL_NUMS
// In syscall_names.C:
// 2.  Add "SYS_foo" to the end of _syscall_names[]
// 3.  Add DO_CASE(foo); to the end of the big switch statement
//
// To remove a syscall
// 0.  Don't
// 1.  wrap the DO_CASE statement with:
//     #ifdef __NR_foo
//       DO_CASE(foo);
//     #endif
//

enum SYSCALL_NUMS
{
    NUM_SYS_NYI,
    NUM_SYS_UNKNOWN,
    ///////START OF LIST////////////
    NUM_SYS_restart_syscall,
    NUM_SYS_exit,
    NUM_SYS_fork,
    NUM_SYS_read,
    NUM_SYS_write,
    NUM_SYS_open,
    NUM_SYS_close,
    NUM_SYS_waitpid,
    NUM_SYS_creat,
    NUM_SYS_link,
    NUM_SYS_unlink,
    NUM_SYS_execve,
    NUM_SYS_chdir,
    NUM_SYS_time,
    NUM_SYS_mknod,
    NUM_SYS_chmod,
    NUM_SYS_lchown,
    NUM_SYS_break,
    NUM_SYS_oldstat,
    NUM_SYS_lseek,
    NUM_SYS_getpid,
    NUM_SYS_mount,
    NUM_SYS_umount,
    NUM_SYS_setuid,
    NUM_SYS_getuid,
    NUM_SYS_stime,
    NUM_SYS_ptrace,
    NUM_SYS_alarm,
    NUM_SYS_oldfstat,
    NUM_SYS_pause,
    NUM_SYS_utime,
    NUM_SYS_stty,
    NUM_SYS_gtty,
    NUM_SYS_access,
    NUM_SYS_nice,
    NUM_SYS_ftime,
    NUM_SYS_sync,
    NUM_SYS_kill,
    NUM_SYS_rename,
    NUM_SYS_mkdir,
    NUM_SYS_rmdir,
    NUM_SYS_dup,
    NUM_SYS_pipe,
    NUM_SYS_times,
    NUM_SYS_prof,
    NUM_SYS_brk,
    NUM_SYS_setgid,
    NUM_SYS_getgid,
    NUM_SYS_signal,
    NUM_SYS_geteuid,
    NUM_SYS_getegid,
    NUM_SYS_acct,
    NUM_SYS_umount2,
    NUM_SYS_lock,
    NUM_SYS_ioctl,
    NUM_SYS_fcntl,
    NUM_SYS_mpx,
    NUM_SYS_setpgid,
    NUM_SYS_ulimit,
    NUM_SYS_oldolduname,
    NUM_SYS_umask,
    NUM_SYS_chroot,
    NUM_SYS_ustat,
    NUM_SYS_dup2,
    NUM_SYS_getppid,
    NUM_SYS_getpgrp,
    NUM_SYS_setsid,
    NUM_SYS_sigaction,
    NUM_SYS_sgetmask,
    NUM_SYS_ssetmask,
    NUM_SYS_setreuid,
    NUM_SYS_setregid,
    NUM_SYS_sigsuspend,
    NUM_SYS_sigpending,
    NUM_SYS_sethostname,
    NUM_SYS_setrlimit,
    NUM_SYS_getrlimit,
    NUM_SYS_getrusage,
    NUM_SYS_gettimeofday,
    NUM_SYS_settimeofday,
    NUM_SYS_getgroups,
    NUM_SYS_setgroups,
    NUM_SYS_select,
    NUM_SYS_symlink,
    NUM_SYS_oldlstat,
    NUM_SYS_readlink,
    NUM_SYS_uselib,
    NUM_SYS_swapon,
    NUM_SYS_reboot,
    NUM_SYS_readdir,
    NUM_SYS_mmap,
    NUM_SYS_munmap,
    NUM_SYS_truncate,
    NUM_SYS_ftruncate,
    NUM_SYS_fchmod,
    NUM_SYS_fchown,
    NUM_SYS_getpriority,
    NUM_SYS_setpriority,
    NUM_SYS_profil,
    NUM_SYS_statfs,
    NUM_SYS_fstatfs,
    NUM_SYS_ioperm,
    NUM_SYS_socketcall,
    NUM_SYS_syslog,
    NUM_SYS_setitimer,
    NUM_SYS_getitimer,
    NUM_SYS_stat,
    NUM_SYS_lstat,
    NUM_SYS_fstat,
    NUM_SYS_olduname,
    NUM_SYS_iopl,
    NUM_SYS_vhangup,
    NUM_SYS_idle,
    NUM_SYS_vm86old,
    NUM_SYS_wait4,
    NUM_SYS_swapoff,
    NUM_SYS_sysinfo,
    NUM_SYS_ipc,
    NUM_SYS_fsync,
    NUM_SYS_sigreturn,
    NUM_SYS_clone,
    NUM_SYS_setdomainname,
    NUM_SYS_uname,
    NUM_SYS_modify_ldt,
    NUM_SYS_adjtimex,
    NUM_SYS_mprotect,
    NUM_SYS_sigprocmask,
    NUM_SYS_create_module,
    NUM_SYS_init_module,
    NUM_SYS_delete_module,
    NUM_SYS_get_kernel_syms,
    NUM_SYS_quotactl,
    NUM_SYS_getpgid,
    NUM_SYS_fchdir,
    NUM_SYS_bdflush,
    NUM_SYS_sysfs,
    NUM_SYS_personality,
    NUM_SYS_afs_syscall,
    NUM_SYS_setfsuid,
    NUM_SYS_setfsgid,
    NUM_SYS__llseek,
    NUM_SYS_getdents,
    NUM_SYS__newselect,
    NUM_SYS_flock,
    NUM_SYS_msync,
    NUM_SYS_readv,
    NUM_SYS_writev,
    NUM_SYS_getsid,
    NUM_SYS_fdatasync,
    NUM_SYS__sysctl,
    NUM_SYS_mlock,
    NUM_SYS_munlock,
    NUM_SYS_mlockall,
    NUM_SYS_munlockall,
    NUM_SYS_sched_setparam,
    NUM_SYS_sched_getparam,
    NUM_SYS_sched_setscheduler,
    NUM_SYS_sched_getscheduler,
    NUM_SYS_sched_yield,
    NUM_SYS_sched_get_priority_max,
    NUM_SYS_sched_get_priority_min,
    NUM_SYS_sched_rr_get_interval,
    NUM_SYS_nanosleep,
    NUM_SYS_mremap,
    NUM_SYS_setresuid,
    NUM_SYS_getresuid,
    NUM_SYS_vm86,
    NUM_SYS_query_module,
    NUM_SYS_poll,
    NUM_SYS_nfsservctl,
    NUM_SYS_setresgid,
    NUM_SYS_getresgid,
    NUM_SYS_prctl,
    NUM_SYS_rt_sigreturn,
    NUM_SYS_rt_sigaction,
    NUM_SYS_rt_sigprocmask,
    NUM_SYS_rt_sigpending,
    NUM_SYS_rt_sigtimedwait,
    NUM_SYS_rt_sigqueueinfo,
    NUM_SYS_rt_sigsuspend,
    NUM_SYS_pread64,
    NUM_SYS_pwrite64,
    NUM_SYS_chown,
    NUM_SYS_getcwd,
    NUM_SYS_capget,
    NUM_SYS_capset,
    NUM_SYS_sigaltstack,
    NUM_SYS_sendfile,
    NUM_SYS_getpmsg,
    NUM_SYS_putpmsg,
    NUM_SYS_vfork,
    NUM_SYS_ugetrlimit,
    NUM_SYS_mmap2,
    NUM_SYS_truncate64,
    NUM_SYS_ftruncate64,
    NUM_SYS_stat64,
    NUM_SYS_lstat64,
    NUM_SYS_fstat64,
    NUM_SYS_lchown32,
    NUM_SYS_getuid32,
    NUM_SYS_getgid32,
    NUM_SYS_geteuid32,
    NUM_SYS_getegid32,
    NUM_SYS_setreuid32,
    NUM_SYS_setregid32,
    NUM_SYS_getgroups32,
    NUM_SYS_setgroups32,
    NUM_SYS_fchown32,
    NUM_SYS_setresuid32,
    NUM_SYS_getresuid32,
    NUM_SYS_setresgid32,
    NUM_SYS_getresgid32,
    NUM_SYS_chown32,
    NUM_SYS_setuid32,
    NUM_SYS_setgid32,
    NUM_SYS_setfsuid32,
    NUM_SYS_setfsgid32,
    NUM_SYS_pivot_root,
    NUM_SYS_mincore,
    NUM_SYS_madvise,
    NUM_SYS_madvise1,
    NUM_SYS_getdents64,
    NUM_SYS_fcntl64,
    NUM_SYS_security,
    NUM_SYS_gettid,
    NUM_SYS_readahead,
    NUM_SYS_setxattr,
    NUM_SYS_lsetxattr,
    NUM_SYS_fsetxattr,
    NUM_SYS_getxattr,
    NUM_SYS_lgetxattr,
    NUM_SYS_fgetxattr,
    NUM_SYS_listxattr,
    NUM_SYS_llistxattr,
    NUM_SYS_flistxattr,
    NUM_SYS_removexattr,
    NUM_SYS_lremovexattr,
    NUM_SYS_fremovexattr,
    NUM_SYS_tkill,
    NUM_SYS_sendfile64,
    NUM_SYS_futex,
    NUM_SYS_sched_setaffinity,
    NUM_SYS_sched_getaffinity,
    NUM_SYS_set_thread_area,
    NUM_SYS_get_thread_area,
    NUM_SYS_io_setup,
    NUM_SYS_io_destroy,
    NUM_SYS_io_getevents,
    NUM_SYS_io_submit,
    NUM_SYS_io_cancel,
    NUM_SYS_alloc_hugepages,
    NUM_SYS_free_hugepages,
    NUM_SYS_exit_group,
    NUM_SYS_set_tid_address
    //Add new NUM_SYS_ here
    ///////END OF LIST////////////
};

const char* _syscall_names[] = {
    "NYI", "UNKNOWN",
    ///////START OF LIST////////////
    "SYS_restart_syscall", "SYS_exit", "SYS_fork", "SYS_read", "SYS_write", "SYS_open", "SYS_close", "SYS_waitpid", "SYS_creat",
    "SYS_link", "SYS_unlink", "SYS_execve", "SYS_chdir", "SYS_time", "SYS_mknod", "SYS_chmod", "SYS_lchown", "SYS_break",
    "SYS_oldstat", "SYS_lseek", "SYS_getpid", "SYS_mount", "SYS_umount", "SYS_setuid", "SYS_getuid", "SYS_stime", "SYS_ptrace",
    "SYS_alarm", "SYS_oldfstat", "SYS_pause", "SYS_utime", "SYS_stty", "SYS_gtty", "SYS_access", "SYS_nice", "SYS_ftime",
    "SYS_sync", "SYS_kill", "SYS_rename", "SYS_mkdir", "SYS_rmdir", "SYS_dup", "SYS_pipe", "SYS_times", "SYS_prof", "SYS_brk",
    "SYS_setgid", "SYS_getgid", "SYS_signal", "SYS_geteuid", "SYS_getegid", "SYS_acct", "SYS_umount2", "SYS_lock", "SYS_ioctl",
    "SYS_fcntl", "SYS_mpx", "SYS_setpgid", "SYS_ulimit", "SYS_oldolduname", "SYS_umask", "SYS_chroot", "SYS_ustat", "SYS_dup2",
    "SYS_getppid", "SYS_getpgrp", "SYS_setsid", "SYS_sigaction", "SYS_sgetmask", "SYS_ssetmask", "SYS_setreuid", "SYS_setregid",
    "SYS_sigsuspend", "SYS_sigpending", "SYS_sethostname", "SYS_setrlimit", "SYS_getrlimit", "SYS_getrusage", "SYS_gettimeofday",
    "SYS_settimeofday", "SYS_getgroups", "SYS_setgroups", "SYS_select", "SYS_symlink", "SYS_oldlstat", "SYS_readlink",
    "SYS_uselib", "SYS_swapon", "SYS_reboot", "SYS_readdir", "SYS_mmap", "SYS_munmap", "SYS_truncate", "SYS_ftruncate",
    "SYS_fchmod", "SYS_fchown", "SYS_getpriority", "SYS_setpriority", "SYS_profil", "SYS_statfs", "SYS_fstatfs", "SYS_ioperm",
    "SYS_socketcall", "SYS_syslog", "SYS_setitimer", "SYS_getitimer", "SYS_stat", "SYS_lstat", "SYS_fstat", "SYS_olduname",
    "SYS_iopl", "SYS_vhangup", "SYS_idle", "SYS_vm86old", "SYS_wait4", "SYS_swapoff", "SYS_sysinfo", "SYS_ipc", "SYS_fsync",
    "SYS_sigreturn", "SYS_clone", "SYS_setdomainname", "SYS_uname", "SYS_modify_ldt", "SYS_adjtimex", "SYS_mprotect",
    "SYS_sigprocmask", "SYS_create_module", "SYS_init_module", "SYS_delete_module", "SYS_get_kernel_syms", "SYS_quotactl",
    "SYS_getpgid", "SYS_fchdir", "SYS_bdflush", "SYS_sysfs", "SYS_personality", "SYS_afs_syscall", "SYS_setfsuid", "SYS_setfsgid",
    "SYS__llseek", "SYS_getdents", "SYS__newselect", "SYS_flock", "SYS_msync", "SYS_readv", "SYS_writev", "SYS_getsid",
    "SYS_fdatasync", "SYS__sysctl", "SYS_mlock", "SYS_munlock", "SYS_mlockall", "SYS_munlockall", "SYS_sched_setparam",
    "SYS_sched_getparam", "SYS_sched_setscheduler", "SYS_sched_getscheduler", "SYS_sched_yield", "SYS_sched_get_priority_max",
    "SYS_sched_get_priority_min", "SYS_sched_rr_get_interval", "SYS_nanosleep", "SYS_mremap", "SYS_setresuid", "SYS_getresuid",
    "SYS_vm86", "SYS_query_module", "SYS_poll", "SYS_nfsservctl", "SYS_setresgid", "SYS_getresgid", "SYS_prctl",
    "SYS_rt_sigreturn", "SYS_rt_sigaction", "SYS_rt_sigprocmask", "SYS_rt_sigpending", "SYS_rt_sigtimedwait",
    "SYS_rt_sigqueueinfo", "SYS_rt_sigsuspend", "SYS_pread64", "SYS_pwrite64", "SYS_chown", "SYS_getcwd", "SYS_capget",
    "SYS_capset", "SYS_sigaltstack", "SYS_sendfile", "SYS_getpmsg", "SYS_putpmsg", "SYS_vfork", "SYS_ugetrlimit", "SYS_mmap2",
    "SYS_truncate64", "SYS_ftruncate64", "SYS_stat64", "SYS_lstat64", "SYS_fstat64", "SYS_lchown32", "SYS_getuid32",
    "SYS_getgid32", "SYS_geteuid32", "SYS_getegid32", "SYS_setreuid32", "SYS_setregid32", "SYS_getgroups32", "SYS_setgroups32",
    "SYS_fchown32", "SYS_setresuid32", "SYS_getresuid32", "SYS_setresgid32", "SYS_getresgid32", "SYS_chown32", "SYS_setuid32",
    "SYS_setgid32", "SYS_setfsuid32", "SYS_setfsgid32", "SYS_pivot_root", "SYS_mincore", "SYS_madvise", "SYS_madvise1",
    "SYS_getdents64", "SYS_fcntl64", "SYS_security", "SYS_gettid", "SYS_readahead", "SYS_setxattr", "SYS_lsetxattr",
    "SYS_fsetxattr", "SYS_getxattr", "SYS_lgetxattr", "SYS_fgetxattr", "SYS_listxattr", "SYS_llistxattr", "SYS_flistxattr",
    "SYS_removexattr", "SYS_lremovexattr", "SYS_fremovexattr", "SYS_tkill", "SYS_sendfile64", "SYS_futex",
    "SYS_sched_setaffinity", "SYS_sched_getaffinity", "SYS_set_thread_area", "SYS_get_thread_area", "SYS_io_setup",
    "SYS_io_destroy", "SYS_io_getevents", "SYS_io_submit", "SYS_io_cancel", "SYS_alloc_hugepages", "SYS_free_hugepages",
    "SYS_exit_group", "SYS_set_tid_address"
    // Add new "SYS_" here
    ///////END OF _syscall_names[]////////////

};

const char* SYS_SyscallName(int num)
{
#define DO_CASE(n) \
    case __NR_##n: \
        return _syscall_names[NUM_SYS_##n]

    switch (num)
    {
#ifdef __NR_restart_syscall
        DO_CASE(restart_syscall);
#endif
        DO_CASE(exit);
        DO_CASE(fork);
        DO_CASE(read);
        DO_CASE(write);
        DO_CASE(open);
        DO_CASE(close);
        DO_CASE(waitpid);
        DO_CASE(creat);
        DO_CASE(link);
        DO_CASE(unlink);
        DO_CASE(execve);
        DO_CASE(chdir);
        DO_CASE(time);
        DO_CASE(mknod);
        DO_CASE(chmod);
        DO_CASE(lchown);
        DO_CASE(break);
        DO_CASE(oldstat);
        DO_CASE(lseek);
        DO_CASE(getpid);
        DO_CASE(mount);
        DO_CASE(umount);
        DO_CASE(setuid);
        DO_CASE(getuid);
        DO_CASE(stime);
        DO_CASE(ptrace);
        DO_CASE(alarm);
        DO_CASE(oldfstat);
        DO_CASE(pause);
        DO_CASE(utime);
        DO_CASE(stty);
        DO_CASE(gtty);
        DO_CASE(access);
        DO_CASE(nice);
        DO_CASE(ftime);
        DO_CASE(sync);
        DO_CASE(kill);
        DO_CASE(rename);
        DO_CASE(mkdir);
        DO_CASE(rmdir);
        DO_CASE(dup);
        DO_CASE(pipe);
        DO_CASE(times);
        DO_CASE(prof);
        DO_CASE(brk);
        DO_CASE(setgid);
        DO_CASE(getgid);
        DO_CASE(signal);
        DO_CASE(geteuid);
        DO_CASE(getegid);
        DO_CASE(acct);
        DO_CASE(umount2);
        DO_CASE(lock);
        DO_CASE(ioctl);
        DO_CASE(fcntl);
        DO_CASE(mpx);
        DO_CASE(setpgid);
        DO_CASE(ulimit);
        DO_CASE(oldolduname);
        DO_CASE(umask);
        DO_CASE(chroot);
        DO_CASE(ustat);
        DO_CASE(dup2);
        DO_CASE(getppid);
        DO_CASE(getpgrp);
        DO_CASE(setsid);
        DO_CASE(sigaction);
        DO_CASE(sgetmask);
        DO_CASE(ssetmask);
        DO_CASE(setreuid);
        DO_CASE(setregid);
        DO_CASE(sigsuspend);
        DO_CASE(sigpending);
        DO_CASE(sethostname);
        DO_CASE(setrlimit);
        DO_CASE(getrlimit);
        DO_CASE(getrusage);
        DO_CASE(gettimeofday);
        DO_CASE(settimeofday);
        DO_CASE(getgroups);
        DO_CASE(setgroups);
        DO_CASE(select);
        DO_CASE(symlink);
        DO_CASE(oldlstat);
        DO_CASE(readlink);
        DO_CASE(uselib);
        DO_CASE(swapon);
        DO_CASE(reboot);
        DO_CASE(readdir);
        DO_CASE(mmap);
        DO_CASE(munmap);
        DO_CASE(truncate);
        DO_CASE(ftruncate);
        DO_CASE(fchmod);
        DO_CASE(fchown);
        DO_CASE(getpriority);
        DO_CASE(setpriority);
        DO_CASE(profil);
        DO_CASE(statfs);
        DO_CASE(fstatfs);
        DO_CASE(ioperm);
        DO_CASE(socketcall);
        DO_CASE(syslog);
        DO_CASE(setitimer);
        DO_CASE(getitimer);
        DO_CASE(stat);
        DO_CASE(lstat);
        DO_CASE(fstat);
        DO_CASE(olduname);
        DO_CASE(iopl);
        DO_CASE(vhangup);
        DO_CASE(idle);
        DO_CASE(vm86old);
        DO_CASE(wait4);
        DO_CASE(swapoff);
        DO_CASE(sysinfo);
        DO_CASE(ipc);
        DO_CASE(fsync);
        DO_CASE(sigreturn);
        DO_CASE(clone);
        DO_CASE(setdomainname);
        DO_CASE(uname);
        DO_CASE(modify_ldt);
        DO_CASE(adjtimex);
        DO_CASE(mprotect);
        DO_CASE(sigprocmask);
        DO_CASE(create_module);
        DO_CASE(init_module);
        DO_CASE(delete_module);
        DO_CASE(get_kernel_syms);
        DO_CASE(quotactl);
        DO_CASE(getpgid);
        DO_CASE(fchdir);
        DO_CASE(bdflush);
        DO_CASE(sysfs);
        DO_CASE(personality);
        DO_CASE(afs_syscall);
        DO_CASE(setfsuid);
        DO_CASE(setfsgid);
        DO_CASE(_llseek);
        DO_CASE(getdents);
        DO_CASE(_newselect);
        DO_CASE(flock);
        DO_CASE(msync);
        DO_CASE(readv);
        DO_CASE(writev);
        DO_CASE(getsid);
        DO_CASE(fdatasync);
        DO_CASE(_sysctl);
        DO_CASE(mlock);
        DO_CASE(munlock);
        DO_CASE(mlockall);
        DO_CASE(munlockall);
        DO_CASE(sched_setparam);
        DO_CASE(sched_getparam);
        DO_CASE(sched_setscheduler);
        DO_CASE(sched_getscheduler);
        DO_CASE(sched_yield);
        DO_CASE(sched_get_priority_max);
        DO_CASE(sched_get_priority_min);
        DO_CASE(sched_rr_get_interval);
        DO_CASE(nanosleep);
        DO_CASE(mremap);
        DO_CASE(setresuid);
        DO_CASE(getresuid);
        DO_CASE(vm86);
        DO_CASE(query_module);
        DO_CASE(poll);
        DO_CASE(nfsservctl);
        DO_CASE(setresgid);
        DO_CASE(getresgid);
        DO_CASE(prctl);
        DO_CASE(rt_sigreturn);
        DO_CASE(rt_sigaction);
        DO_CASE(rt_sigprocmask);
        DO_CASE(rt_sigpending);
        DO_CASE(rt_sigtimedwait);
        DO_CASE(rt_sigqueueinfo);
        DO_CASE(rt_sigsuspend);
#ifdef __NR_pread64
        DO_CASE(pread64);
#endif
#ifdef __NR_pwrite64
        DO_CASE(pwrite64);
#endif
        DO_CASE(chown);
        DO_CASE(getcwd);
        DO_CASE(capget);
        DO_CASE(capset);
        DO_CASE(sigaltstack);
        DO_CASE(sendfile);
        DO_CASE(getpmsg);
        DO_CASE(putpmsg);
        DO_CASE(vfork);
        DO_CASE(ugetrlimit);
        DO_CASE(mmap2);
#ifdef __NR_truncate64
        DO_CASE(truncate64);
#endif
#ifdef __NR_ftruncate64
        DO_CASE(ftruncate64);
#endif
#ifdef __NR_stat64
        DO_CASE(stat64);
#endif
#ifdef __NR_lstat64
        DO_CASE(lstat64);
#endif
#ifdef __NR_fstat64
        DO_CASE(fstat64);
#endif
        DO_CASE(lchown32);
        DO_CASE(getuid32);
        DO_CASE(getgid32);
        DO_CASE(geteuid32);
        DO_CASE(getegid32);
        DO_CASE(setreuid32);
        DO_CASE(setregid32);
        DO_CASE(getgroups32);
        DO_CASE(setgroups32);
        DO_CASE(fchown32);
        DO_CASE(setresuid32);
        DO_CASE(getresuid32);
        DO_CASE(setresgid32);
        DO_CASE(getresgid32);
        DO_CASE(chown32);
        DO_CASE(setuid32);
        DO_CASE(setgid32);
        DO_CASE(setfsuid32);
        DO_CASE(setfsgid32);
        DO_CASE(pivot_root);
        DO_CASE(mincore);
        DO_CASE(madvise);
#ifdef __NR_madvise1
#if __NR_madvise != __NR_madvise1
        DO_CASE(madvise1); //DUPLICATE of madvise?
#endif
#endif // __NR_madvise1 defined
#ifdef __NR_getdents64
        DO_CASE(getdents64);
#endif
#ifdef __NR_fcntl64
        DO_CASE(fcntl64);
#endif
#ifdef __NR_security
        DO_CASE(security);
#endif
#ifdef __NR_gettid
        DO_CASE(gettid);
#endif
#ifdef __NR_readahead
        DO_CASE(readahead);
#endif
#ifdef __NR_setxattr
        DO_CASE(setxattr);
#endif
#ifdef __NR_lsetxattr
        DO_CASE(lsetxattr);
#endif
#ifdef __NR_fsetxattr
        DO_CASE(fsetxattr);
#endif
#ifdef __NR_getxattr
        DO_CASE(getxattr);
#endif
#ifdef __NR_lgetxattr
        DO_CASE(lgetxattr);
#endif
#ifdef __NR_fgetxattr
        DO_CASE(fgetxattr);
#endif
#ifdef __NR_listxattr
        DO_CASE(listxattr);
#endif
#ifdef __NR_llistxattr
        DO_CASE(llistxattr);
#endif
#ifdef __NR_flistxattr
        DO_CASE(flistxattr);
#endif
#ifdef __NR_removexattr
        DO_CASE(removexattr);
#endif
#ifdef __NR_lremovexattr
        DO_CASE(lremovexattr);
#endif
#ifdef __NR_fremovexattr
        DO_CASE(fremovexattr);
#endif
#ifdef __NR_tkill
        DO_CASE(tkill);
#endif
#ifdef __NR_sendfile64
        DO_CASE(sendfile64);
#endif
#ifdef __NR_futex
        DO_CASE(futex);
#endif
#ifdef __NR_sched_setaffinity
        DO_CASE(sched_setaffinity);
#endif
#ifdef __NR_sched_getaffinity
        DO_CASE(sched_getaffinity);
#endif
#ifdef __NR_set_thread_area
        DO_CASE(set_thread_area);
#endif
#ifdef __NR_get_thread_area
        DO_CASE(get_thread_area);
#endif
#ifdef __NR_io_setup
        DO_CASE(io_setup);
#endif
#ifdef __NR_io_destroy
        DO_CASE(io_destroy);
#endif
#ifdef __NR_io_getevents
        DO_CASE(io_getevents);
#endif
#ifdef __NR_io_submit
        DO_CASE(io_submit);
#endif
#ifdef __NR_io_cancel
        DO_CASE(io_cancel);
#endif
#ifdef __NR_io_alloc_hugepages
        DO_CASE(alloc_hugepages);
#endif
#ifdef __NR_io_free_hugepages
        DO_CASE(free_hugepages);
#endif
#ifdef __NR_exit_group
        DO_CASE(exit_group);
#endif
#ifdef __NR_set_tid_address
        DO_CASE(set_tid_address);
#endif
            // Add new DO_CASE here
            ///////END OF SWITCH////////////

        default:
            cout << "UNKNOWN SYSCALL: " << num << endl;
            return _syscall_names[NUM_SYS_UNKNOWN];
    }

#undef DO_CASE
}
#endif
