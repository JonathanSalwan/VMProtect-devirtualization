/****************************************************************************
 ****************************************************************************
 ***
 ***   This header was automatically generated from a Linux kernel header
 ***   of the same name, to make information necessary for userspace to
 ***   call into the kernel available to libc.  It contains only constants,
 ***   structures, and macros generated from the original header, and thus,
 ***   contains no copyrightable information.
 ***
 ***   To edit the content of this header, modify the corresponding
 ***   source file (e.g. under external/kernel-headers/original/) then
 ***   run bionic/libc/kernel/tools/update_all.py
 ***
 ***   Any manual change here will be lost the next time this script will
 ***   be run. You've been warned!
 ***
 ****************************************************************************
 ****************************************************************************/
#include <asm/bitsperlong.h>
#ifndef __SYSCALL
#define __SYSCALL(x, y)
#endif
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#if __BITS_PER_LONG == 32 || defined(__SYSCALL_COMPAT)
#define __SC_3264(_nr, _32, _64) __SYSCALL(_nr, _32)
#else
#define __SC_3264(_nr, _32, _64) __SYSCALL(_nr, _64)
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#endif
#ifdef __SYSCALL_COMPAT
#define __SC_COMP(_nr, _sys, _comp) __SYSCALL(_nr, _comp)
#define __SC_COMP_3264(_nr, _32, _64, _comp) __SYSCALL(_nr, _comp)
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#else
#define __SC_COMP(_nr, _sys, _comp) __SYSCALL(_nr, _sys)
#define __SC_COMP_3264(_nr, _32, _64, _comp) __SC_3264(_nr, _32, _64)
#endif
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define __NR_io_setup 0
#define __NR_io_destroy 1
#define __NR_io_submit 2
#define __NR_io_cancel 3
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define __NR_io_getevents 4
#define __NR_setxattr 5
#define __NR_lsetxattr 6
#define __NR_fsetxattr 7
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define __NR_getxattr 8
#define __NR_lgetxattr 9
#define __NR_fgetxattr 10
#define __NR_listxattr 11
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define __NR_llistxattr 12
#define __NR_flistxattr 13
#define __NR_removexattr 14
#define __NR_lremovexattr 15
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define __NR_fremovexattr 16
#define __NR_getcwd 17
#define __NR_lookup_dcookie 18
#define __NR_eventfd2 19
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define __NR_epoll_create1 20
#define __NR_epoll_ctl 21
#define __NR_epoll_pwait 22
#define __NR_dup 23
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define __NR_dup3 24
#define __NR3264_fcntl 25
#define __NR_inotify_init1 26
#define __NR_inotify_add_watch 27
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define __NR_inotify_rm_watch 28
#define __NR_ioctl 29
#define __NR_ioprio_set 30
#define __NR_ioprio_get 31
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define __NR_flock 32
#define __NR_mknodat 33
#define __NR_mkdirat 34
#define __NR_unlinkat 35
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define __NR_symlinkat 36
#define __NR_linkat 37
#define __NR_renameat 38
#define __NR_umount2 39
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define __NR_mount 40
#define __NR_pivot_root 41
#define __NR_nfsservctl 42
#define __NR3264_statfs 43
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define __NR3264_fstatfs 44
#define __NR3264_truncate 45
#define __NR3264_ftruncate 46
#define __NR_fallocate 47
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define __NR_faccessat 48
#define __NR_chdir 49
#define __NR_fchdir 50
#define __NR_chroot 51
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define __NR_fchmod 52
#define __NR_fchmodat 53
#define __NR_fchownat 54
#define __NR_fchown 55
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define __NR_openat 56
#define __NR_close 57
#define __NR_vhangup 58
#define __NR_pipe2 59
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define __NR_quotactl 60
#define __NR_getdents64 61
#define __NR3264_lseek 62
#define __NR_read 63
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define __NR_write 64
#define __NR_readv 65
#define __NR_writev 66
#define __NR_pread64 67
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define __NR_pwrite64 68
#define __NR_preadv 69
#define __NR_pwritev 70
#define __NR3264_sendfile 71
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define __NR_pselect6 72
#define __NR_ppoll 73
#define __NR_signalfd4 74
#define __NR_vmsplice 75
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define __NR_splice 76
#define __NR_tee 77
#define __NR_readlinkat 78
#define __NR3264_fstatat 79
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define __NR3264_fstat 80
#define __NR_sync 81
#define __NR_fsync 82
#define __NR_fdatasync 83
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#ifdef __ARCH_WANT_SYNC_FILE_RANGE2
#define __NR_sync_file_range2 84
#else
#define __NR_sync_file_range 84
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#endif
#define __NR_timerfd_create 85
#define __NR_timerfd_settime 86
#define __NR_timerfd_gettime 87
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define __NR_utimensat 88
#define __NR_acct 89
#define __NR_capget 90
#define __NR_capset 91
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define __NR_personality 92
#define __NR_exit 93
#define __NR_exit_group 94
#define __NR_waitid 95
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define __NR_set_tid_address 96
#define __NR_unshare 97
#define __NR_futex 98
#define __NR_set_robust_list 99
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define __NR_get_robust_list 100
#define __NR_nanosleep 101
#define __NR_getitimer 102
#define __NR_setitimer 103
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define __NR_kexec_load 104
#define __NR_init_module 105
#define __NR_delete_module 106
#define __NR_timer_create 107
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define __NR_timer_gettime 108
#define __NR_timer_getoverrun 109
#define __NR_timer_settime 110
#define __NR_timer_delete 111
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define __NR_clock_settime 112
#define __NR_clock_gettime 113
#define __NR_clock_getres 114
#define __NR_clock_nanosleep 115
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define __NR_syslog 116
#define __NR_ptrace 117
#define __NR_sched_setparam 118
#define __NR_sched_setscheduler 119
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define __NR_sched_getscheduler 120
#define __NR_sched_getparam 121
#define __NR_sched_setaffinity 122
#define __NR_sched_getaffinity 123
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define __NR_sched_yield 124
#define __NR_sched_get_priority_max 125
#define __NR_sched_get_priority_min 126
#define __NR_sched_rr_get_interval 127
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define __NR_restart_syscall 128
#define __NR_kill 129
#define __NR_tkill 130
#define __NR_tgkill 131
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define __NR_sigaltstack 132
#define __NR_rt_sigsuspend 133
#define __NR_rt_sigaction 134
#define __NR_rt_sigprocmask 135
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define __NR_rt_sigpending 136
#define __NR_rt_sigtimedwait 137
#define __NR_rt_sigqueueinfo 138
#define __NR_rt_sigreturn 139
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define __NR_setpriority 140
#define __NR_getpriority 141
#define __NR_reboot 142
#define __NR_setregid 143
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define __NR_setgid 144
#define __NR_setreuid 145
#define __NR_setuid 146
#define __NR_setresuid 147
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define __NR_getresuid 148
#define __NR_setresgid 149
#define __NR_getresgid 150
#define __NR_setfsuid 151
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define __NR_setfsgid 152
#define __NR_times 153
#define __NR_setpgid 154
#define __NR_getpgid 155
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define __NR_getsid 156
#define __NR_setsid 157
#define __NR_getgroups 158
#define __NR_setgroups 159
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define __NR_uname 160
#define __NR_sethostname 161
#define __NR_setdomainname 162
#define __NR_getrlimit 163
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define __NR_setrlimit 164
#define __NR_getrusage 165
#define __NR_umask 166
#define __NR_prctl 167
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define __NR_getcpu 168
#define __NR_gettimeofday 169
#define __NR_settimeofday 170
#define __NR_adjtimex 171
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define __NR_getpid 172
#define __NR_getppid 173
#define __NR_getuid 174
#define __NR_geteuid 175
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define __NR_getgid 176
#define __NR_getegid 177
#define __NR_gettid 178
#define __NR_sysinfo 179
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define __NR_mq_open 180
#define __NR_mq_unlink 181
#define __NR_mq_timedsend 182
#define __NR_mq_timedreceive 183
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define __NR_mq_notify 184
#define __NR_mq_getsetattr 185
#define __NR_msgget 186
#define __NR_msgctl 187
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define __NR_msgrcv 188
#define __NR_msgsnd 189
#define __NR_semget 190
#define __NR_semctl 191
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define __NR_semtimedop 192
#define __NR_semop 193
#define __NR_shmget 194
#define __NR_shmctl 195
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define __NR_shmat 196
#define __NR_shmdt 197
#define __NR_socket 198
#define __NR_socketpair 199
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define __NR_bind 200
#define __NR_listen 201
#define __NR_accept 202
#define __NR_connect 203
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define __NR_getsockname 204
#define __NR_getpeername 205
#define __NR_sendto 206
#define __NR_recvfrom 207
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define __NR_setsockopt 208
#define __NR_getsockopt 209
#define __NR_shutdown 210
#define __NR_sendmsg 211
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define __NR_recvmsg 212
#define __NR_readahead 213
#define __NR_brk 214
#define __NR_munmap 215
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define __NR_mremap 216
#define __NR_add_key 217
#define __NR_request_key 218
#define __NR_keyctl 219
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define __NR_clone 220
#define __NR_execve 221
#define __NR3264_mmap 222
#define __NR3264_fadvise64 223
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#ifndef __ARCH_NOMMU
#define __NR_swapon 224
#define __NR_swapoff 225
#define __NR_mprotect 226
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define __NR_msync 227
#define __NR_mlock 228
#define __NR_munlock 229
#define __NR_mlockall 230
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define __NR_munlockall 231
#define __NR_mincore 232
#define __NR_madvise 233
#define __NR_remap_file_pages 234
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define __NR_mbind 235
#define __NR_get_mempolicy 236
#define __NR_set_mempolicy 237
#define __NR_migrate_pages 238
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define __NR_move_pages 239
#endif
#define __NR_rt_tgsigqueueinfo 240
#define __NR_perf_event_open 241
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define __NR_accept4 242
#define __NR_recvmmsg 243
#define __NR_arch_specific_syscall 244
#define __NR_wait4 260
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define __NR_prlimit64 261
#define __NR_fanotify_init 262
#define __NR_fanotify_mark 263
#define __NR_name_to_handle_at 264
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define __NR_open_by_handle_at 265
#define __NR_clock_adjtime 266
#define __NR_syncfs 267
#define __NR_setns 268
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define __NR_sendmmsg 269
#define __NR_process_vm_readv 270
#define __NR_process_vm_writev 271
#define __NR_kcmp 272
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define __NR_finit_module 273
#define __NR_sched_setattr 274
#define __NR_sched_getattr 275
#undef __NR_syscalls
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define __NR_syscalls 276
#ifdef __ARCH_WANT_SYSCALL_NO_AT
#define __NR_open 1024
#define __NR_link 1025
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define __NR_unlink 1026
#define __NR_mknod 1027
#define __NR_chmod 1028
#define __NR_chown 1029
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define __NR_mkdir 1030
#define __NR_rmdir 1031
#define __NR_lchown 1032
#define __NR_access 1033
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define __NR_rename 1034
#define __NR_readlink 1035
#define __NR_symlink 1036
#define __NR_utimes 1037
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define __NR3264_stat 1038
#define __NR3264_lstat 1039
#undef __NR_syscalls
#define __NR_syscalls (__NR3264_lstat+1)
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#endif
#ifdef __ARCH_WANT_SYSCALL_NO_FLAGS
#define __NR_pipe 1040
#define __NR_dup2 1041
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define __NR_epoll_create 1042
#define __NR_inotify_init 1043
#define __NR_eventfd 1044
#define __NR_signalfd 1045
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#undef __NR_syscalls
#define __NR_syscalls (__NR_signalfd+1)
#endif
#if (__BITS_PER_LONG == 32 || defined(__SYSCALL_COMPAT)) && defined(__ARCH_WANT_SYSCALL_OFF_T)
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define __NR_sendfile 1046
#define __NR_ftruncate 1047
#define __NR_truncate 1048
#define __NR_stat 1049
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define __NR_lstat 1050
#define __NR_fstat 1051
#define __NR_fcntl 1052
#define __NR_fadvise64 1053
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define __ARCH_WANT_SYS_FADVISE64
#define __NR_newfstatat 1054
#define __ARCH_WANT_SYS_NEWFSTATAT
#define __NR_fstatfs 1055
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define __NR_statfs 1056
#define __NR_lseek 1057
#define __NR_mmap 1058
#undef __NR_syscalls
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define __NR_syscalls (__NR_mmap+1)
#endif
#ifdef __ARCH_WANT_SYSCALL_DEPRECATED
#define __NR_alarm 1059
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define __ARCH_WANT_SYS_ALARM
#define __NR_getpgrp 1060
#define __ARCH_WANT_SYS_GETPGRP
#define __NR_pause 1061
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define __ARCH_WANT_SYS_PAUSE
#define __NR_time 1062
#define __ARCH_WANT_SYS_TIME
#define __ARCH_WANT_COMPAT_SYS_TIME
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define __NR_utime 1063
#define __ARCH_WANT_SYS_UTIME
#define __NR_creat 1064
#define __NR_getdents 1065
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define __ARCH_WANT_SYS_GETDENTS
#define __NR_futimesat 1066
#define __NR_select 1067
#define __ARCH_WANT_SYS_SELECT
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define __NR_poll 1068
#define __NR_epoll_wait 1069
#define __NR_ustat 1070
#define __NR_vfork 1071
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define __NR_oldwait4 1072
#define __NR_recv 1073
#define __NR_send 1074
#define __NR_bdflush 1075
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define __NR_umount 1076
#define __ARCH_WANT_SYS_OLDUMOUNT
#define __NR_uselib 1077
#define __NR__sysctl 1078
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define __NR_fork 1079
#undef __NR_syscalls
#define __NR_syscalls (__NR_fork+1)
#endif
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#if __BITS_PER_LONG == 64 && !defined(__SYSCALL_COMPAT)
#define __NR_fcntl __NR3264_fcntl
#define __NR_statfs __NR3264_statfs
#define __NR_fstatfs __NR3264_fstatfs
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define __NR_truncate __NR3264_truncate
#define __NR_ftruncate __NR3264_ftruncate
#define __NR_lseek __NR3264_lseek
#define __NR_sendfile __NR3264_sendfile
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define __NR_newfstatat __NR3264_fstatat
#define __NR_fstat __NR3264_fstat
#define __NR_mmap __NR3264_mmap
#define __NR_fadvise64 __NR3264_fadvise64
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#ifdef __NR3264_stat
#define __NR_stat __NR3264_stat
#define __NR_lstat __NR3264_lstat
#endif
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#else
#define __NR_fcntl64 __NR3264_fcntl
#define __NR_statfs64 __NR3264_statfs
#define __NR_fstatfs64 __NR3264_fstatfs
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define __NR_truncate64 __NR3264_truncate
#define __NR_ftruncate64 __NR3264_ftruncate
#define __NR_llseek __NR3264_lseek
#define __NR_sendfile64 __NR3264_sendfile
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define __NR_fstatat64 __NR3264_fstatat
#define __NR_fstat64 __NR3264_fstat
#define __NR_mmap2 __NR3264_mmap
#define __NR_fadvise64_64 __NR3264_fadvise64
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#ifdef __NR3264_stat
#define __NR_stat64 __NR3264_stat
#define __NR_lstat64 __NR3264_lstat
#endif
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#endif
