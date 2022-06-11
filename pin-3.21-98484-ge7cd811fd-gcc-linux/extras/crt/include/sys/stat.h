/*
 * Copyright (C) 2021-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * Copyright (C) 2008 The Android Open Source Project
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef _SYS_STAT_H_
#define _SYS_STAT_H_

#include <sys/cdefs.h>
#include <sys/types.h>
#include <sys/time.h>
#include <linux/stat.h>

#include <endian.h>

__BEGIN_DECLS

#if defined(__aarch64__)
#define __STAT64_BODY           \
  unsigned long st_dev;         \
  unsigned long st_ino;         \
  unsigned int st_mode;         \
  unsigned int st_nlink;        \
  uid_t st_uid;                 \
  gid_t st_gid;                 \
  unsigned long st_rdev;        \
  unsigned long __pad1;         \
  long st_size;                 \
  int st_blksize;               \
  int __pad2;                   \
  long st_blocks;               \
  long st_atime;                \
  unsigned long st_atime_nsec;  \
  long st_mtime;                \
  unsigned long st_mtime_nsec;  \
  long st_ctime;                \
  unsigned long st_ctime_nsec;  \
  unsigned int __unused4;       \
  unsigned int __unused5;       \

#elif defined(__mips__)
#define __STAT64_BODY           \
  unsigned int st_dev;          \
  unsigned int __pad0[3];       \
  unsigned long long st_ino;    \
  unsigned int st_mode;         \
  unsigned int st_nlink;        \
  uid_t st_uid;                 \
  gid_t st_gid;                 \
  unsigned int st_rdev;         \
  unsigned int __pad1[3];       \
  long long st_size;            \
  unsigned int st_atime;        \
  unsigned int st_atime_nsec;   \
  unsigned int st_mtime;        \
  unsigned int st_mtime_nsec;   \
  unsigned int st_ctime;        \
  unsigned int st_ctime_nsec;   \
  unsigned int st_blksize;      \
  unsigned int __pad2;          \
  unsigned long long st_blocks; \

#elif defined(TARGET_IA32E)
#define __STAT64_BODY           \
  unsigned long st_dev;         \
  unsigned long st_ino;         \
  unsigned long st_nlink;       \
  unsigned int st_mode;         \
  uid_t st_uid;                 \
  gid_t st_gid;                 \
  unsigned int __pad0;          \
  unsigned long st_rdev;        \
  long st_size;                 \
  long st_blksize;              \
  long st_blocks;               \
  unsigned long st_atime;       \
  unsigned long st_atime_nsec;  \
  unsigned long st_mtime;       \
  unsigned long st_mtime_nsec;  \
  unsigned long st_ctime;       \
  unsigned long st_ctime_nsec;  \
  long __pad3[3];               \

#else
#define __STAT64_BODY           \
  unsigned long long st_dev;    \
  unsigned char __pad0[4];      \
  unsigned long __st_ino;       \
  unsigned int st_mode;         \
  unsigned int st_nlink;        \
  uid_t st_uid;                 \
  gid_t st_gid;                 \
  unsigned long long st_rdev;   \
  unsigned char __pad3[4];      \
  long long st_size;            \
  unsigned long st_blksize;     \
  unsigned long long st_blocks; \
  unsigned long st_atime;       \
  unsigned long st_atime_nsec;  \
  unsigned long st_mtime;       \
  unsigned long st_mtime_nsec;  \
  unsigned long st_ctime;       \
  unsigned long st_ctime_nsec;  \
  unsigned long long st_ino;    \

#endif

struct stat   { __STAT64_BODY };

#undef __STAT64_BODY

#define st_atimensec st_atime_nsec
#define st_mtimensec st_mtime_nsec
#define st_ctimensec st_ctime_nsec

#ifdef __USE_BSD
/* Permission macros provided by glibc for compatibility with BSDs. */
#define ACCESSPERMS (S_IRWXU | S_IRWXG | S_IRWXO) /* 0777 */
#define ALLPERMS    (S_ISUID | S_ISGID | S_ISVTX | S_IRWXU | S_IRWXG | S_IRWXO) /* 07777 */
#define DEFFILEMODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH) /* 0666 */
#endif


extern int chmod(const char*, mode_t);
extern int fchmod(int, mode_t);
extern int mkdir(const char*, mode_t);

/*
 * stat() retrieve information about the file pointed to by pathname
 * and return the information in the buffer pointed to by statbuf.
 * No permissions are required on the file itself, but execute (search) permission
 * is required on all of the directories in pathname that lead to the file.
 * This version is similar to stat64() and supports large file.
 *
 * Note: the following members have hard-coded values in Windows -
 * st_uid (0), st_gid (0), st_ino (0), st_ctime (0) and st_nlink (1).
 * st_dev and st_rdev represents in Windows the drive number of the disk containing the file.
 */
extern int stat(const char *pathname, struct stat *statbuf);

/*
 * fstat() retrieve information about the file specified by the file descriptor fd
 * and return the information in the buffer pointed to by statbuf.
 * No permissions are required on the file itself, but execute (search) permission
 * is required on all of the directories in pathname that lead to the file.
 * This version is similar to fstat64() and supports large file.
 *
 * Note: the following members have hard-coded values in Windows -
 * st_uid (0), st_gid (0), st_ino (0), st_ctime (0) and st_nlink (1).
 *
 * Note: if st_dev represents a device then st_dev will hold the device file-descriptor, otherwise - 0.
 *
 */
extern int fstat(int fd, struct stat *statbuf);

/*
 * lstat() is identical to stat(), except that if pathname is a symbolic link, then
 * it returns information about the link itself, not the file that the link refers to.
 * This version is similar to lstat64() and supports large file
 */
extern int lstat(const char *pathname, struct stat *statbuf);

extern int fstatat(int, const char*, struct stat*, int);

extern int mknod(const char*, mode_t, dev_t);
extern mode_t umask(mode_t);

#if defined(__BIONIC_FORTIFY)

extern mode_t __umask_chk(mode_t);
extern mode_t __umask_real(mode_t) __RENAME(umask);
__errordecl(__umask_invalid_mode, "umask called with invalid mode");

__BIONIC_FORTIFY_INLINE
mode_t umask(mode_t mode) {
#if !defined(__clang__)
  if (__builtin_constant_p(mode)) {
    if ((mode & 0777) != mode) {
      __umask_invalid_mode();
    }
    return __umask_real(mode);
  }
#endif
  return __umask_chk(mode);
}
#endif /* defined(__BIONIC_FORTIFY) */

extern int mkfifo(const char*, mode_t);

extern int fchmodat(int, const char*, mode_t, int);
extern int mkdirat(int, const char*, mode_t);
extern int mknodat(int, const char*, mode_t, dev_t);

#define UTIME_NOW  ((1L << 30) - 1L)
#define UTIME_OMIT ((1L << 30) - 2L)
extern int utimensat(int fd, const char *path, const struct timespec times[2], int flags);
extern int futimens(int fd, const struct timespec times[2]);

__END_DECLS

#endif /* _SYS_STAT_H_ */
