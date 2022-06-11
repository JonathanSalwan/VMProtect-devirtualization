/*
 * Copyright (C) 2015-2017 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <crtdefs.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>

#ifndef __IO_H__
#define __IO_H__

__BEGIN_DECLS

#define _O_RDONLY       0x0000  /* open for reading only */
#define _O_WRONLY       0x0001  /* open for writing only */
#define _O_RDWR         0x0002  /* open for reading and writing */
#define _O_APPEND       0x0008  /* writes done at eof */

#define _O_CREAT        0x0100  /* create and open file */
#define _O_TRUNC        0x0200  /* open and truncate */
#define _O_EXCL         0x0400  /* open only if file doesn't already exist */

/* O_TEXT files have <cr><lf> sequences translated to <lf> on read()'s,
** and <lf> sequences translated to <cr><lf> on write()'s
*/

#define _O_TEXT         0x4000  /* file mode is text (translated) */
#define _O_BINARY       0x8000  /* file mode is binary (untranslated) */
#define _O_WTEXT        0x10000 /* file mode is UTF16 (translated) */
#define _O_U16TEXT      0x20000 /* file mode is UTF16 no BOM (translated) */
#define _O_U8TEXT       0x40000 /* file mode is UTF8  no BOM (translated) */

int setmode (int fd, int mode);
int _mkdir(const char* dirname);
off64_t _lseeki64(int fd, off64_t offset, int whence);
int _close(int fd);
#if defined(__cplusplus)
int _open(const char* fname, int flags, mode_t mode = 0);
#else
int _open(const char* fname, int flags, ...);
#endif
ssize_t _read(int fd, void* buf, size_t count);
ssize_t _write(int fd, void* buf, size_t count);
off_t _lseek(int fd, off_t offset, int whence);
int _fsync(int fd);
int _open_osfhandle (intptr_t osfhandle, int flags);
__END_DECLS

#endif // __IO_H__
