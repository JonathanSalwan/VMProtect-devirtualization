/*
 * Copyright (C) 2015-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

// <COMPONENT>: os-apis
// <FILE-TYPE>: component public header

/*!
 * @defgroup OS_APIS_FILE File
 * @brief Contains file related os apis
 */

#ifndef OS_APIS_FILE_H
#define OS_APIS_FILE_H

#include <sys/types.h>

/*! @ingroup OS_APIS_FILE
 * File open modes
 */
typedef enum
{
    // For File objects                                     // For Directory objects
    // -----------------------------------------------------------------------------
    OS_FILE_OPEN_TYPE_READ          = (1 << 0), // List members
    OS_FILE_OPEN_TYPE_WRITE         = (1 << 1), // Add file
    OS_FILE_OPEN_TYPE_EXECUTE       = (1 << 2), // N/A
    OS_FILE_OPEN_TYPE_APPEND        = (1 << 3), // Add sub-directory
    OS_FILE_OPEN_TYPE_TRUNCATE      = (1 << 4), // N/A
    OS_FILE_OPEN_TYPE_CREATE        = (1 << 5), // N/A
    OS_FILE_OPEN_TYPE_CREATE_EXCL   = (1 << 6), // Create exclusively
    OS_FILE_OPEN_TYPE_DELETE        = (1 << 7), // Delete
    OS_FILE_OPEN_TYPE_CLOSE_ON_EXEC = (1 << 8)  // Close on exec (Unix systems only)
} OS_FILE_OPEN_TYPE;

/*! @ingroup OS_APIS_FILE
 * File access permission modes (not all enums apply in Windows)
 */
enum OS_FILE_PERMISSION
{
    OS_FILE_PERMISSION_TYPE_EXECUTE_OTHERS = 0001, // 0x1
    OS_FILE_PERMISSION_TYPE_WRITE_OTHERS   = 0002, // 0x2
    OS_FILE_PERMISSION_TYPE_READ_OTHERS    = 0004, // 0x4
    OS_FILE_PERMISSION_TYPE_ALL_OTHERS     = 0007, // 0x7
    OS_FILE_PERMISSION_TYPE_EXECUTE_GROUP  = 0010, // 0x8
    OS_FILE_PERMISSION_TYPE_WRITE_GROUP    = 0020, // 0x10
    OS_FILE_PERMISSION_TYPE_READ_GROUP     = 0040, // 0x20
    OS_FILE_PERMISSION_TYPE_ALL_GROUP      = 0070, // 0x38
    OS_FILE_PERMISSION_TYPE_EXECUTE_USER   = 0100, // 0x40
    OS_FILE_PERMISSION_TYPE_WRITE_USER     = 0200, // 0x80
    OS_FILE_PERMISSION_TYPE_READ_USER      = 0400, // 0x100
    OS_FILE_PERMISSION_TYPE_ALL_USER       = 0700, // 0x1C0
};

/*! @ingroup OS_APIS_FILE
 * Bit-mask of OS_FILE_PERMISSION's
 */
typedef INT OS_FILE_PERMISSION_TYPE;

/*! @ingroup OS_APIS_FILE
 * File seek modes
 */
typedef enum
{
    //! Offset is set to the given @b offset bytes
    OS_FILE_SEEK_SET = 0,

    //! Offset is set to its current location plus @b offset bytes
    OS_FILE_SEEK_CUR = (1 << 0),

    //! Offset is set to the size of the file plus @b offset bytes
    OS_FILE_SEEK_END = (1 << 1)
} OS_FILE_SEEK_TYPE;

/*! @ingroup OS_APIS_FILE
 * Machine unique ID for file
 */
typedef struct _OS_FILE_UNIQUE_ID
{
    UINT64 _reserved[2]; // Reserved
} OS_FILE_UNIQUE_ID;

/*! @ingroup OS_APIS_FILE
 * File permission modes
 */
typedef enum
{
    OS_FILE_ATTRIBUTES_NONE      = 0,
    OS_FILE_ATTRIBUTES_EXIST     = (1 << 0), // file exist
    OS_FILE_ATTRIBUTES_REGULAR   = (1 << 1), // file is regular
    OS_FILE_ATTRIBUTES_DIRECTORY = (1 << 2), // file is directory
    OS_FILE_ATTRIBUTES_SYMLINK   = (1 << 3)  // file is symlink
} OS_FILE_ATTRIBUTES;

#if defined(TARGET_IA32E)
// 64-bit
#define OS_APIS_STAT_BODY        \
    unsigned long st_dev;        \
    unsigned long st_ino;        \
    unsigned long st_nlink;      \
    unsigned int st_mode;        \
    unsigned int st_uid;         \
    unsigned int st_gid;         \
    unsigned long st_rdev;       \
    long st_size;                \
    long st_blksize;             \
    long st_blocks;              \
    unsigned long st_atim;       \
    unsigned long st_atime_nsec; \
    unsigned long st_mtim;       \
    unsigned long st_mtime_nsec; \
    unsigned long st_ctim;       \
    unsigned long st_ctime_nsec;

#elif defined(TARGET_IA32)
// 32-bit
#define OS_APIS_STAT_BODY         \
    unsigned long long st_dev;    \
    unsigned long __st_ino;       \
    unsigned int st_mode;         \
    unsigned int st_nlink;        \
    unsigned int st_uid;          \
    unsigned int st_gid;          \
    unsigned long long st_rdev;   \
    long long st_size;            \
    unsigned long st_blksize;     \
    unsigned long long st_blocks; \
    unsigned long st_atim;        \
    unsigned long st_atime_nsec;  \
    unsigned long st_mtim;        \
    unsigned long st_mtime_nsec;  \
    unsigned long st_ctim;        \
    unsigned long st_ctime_nsec;  \
    unsigned long long st_ino;

#endif

/*! @ingroup OS_APIS_FILE
 * File status info
 */
typedef struct _OS_STRUCT_STAT
{
    OS_APIS_STAT_BODY
} OS_STRUCT_STAT;

/*! @ingroup OS_APIS_FILE
 * The file name specified by path is opened for reading and/or writing,
 * as specified by the argument @b flags.
 *
 * @param[in]     path      File path.
 * @param[in]     flags     or'ed @ref OS_FILE_OPEN_TYPE. If mode @b OS_FILE_OPEN_TYPE_CREATE \n
 *                          was given, the file is created with @b mode.
 * @param[in]     mode      If a new file will be created as a result of this operation, this is the
 *                          mode of this newly created file. This argument contains values from
 *                          @ref OS_FILE_PERMISSION_TYPE combined with bitwise OR.
 * @param[out]    fd        Returned file descriptor
 *
 * @retval OS_RETURN_CODE_NO_ERROR              If the operation succeeded
 * @retval OS_RETURN_CODE_FILE_OPEN_FAILED      If the operation Failed
 * @return
 *      @b fd    - Assigned with the file descriptor\n
 *
 * @par Availability:
 *   @b O/S:   Windows, Linux & macOS*\n
 *   @b CPU:   All\n
 */
OS_RETURN_CODE OS_OpenFD(const CHAR* path, INT flags, OS_FILE_PERMISSION_TYPE mode, NATIVE_FD* fd);

/*! @ingroup OS_APIS_FILE
 * Attempts to write @b count bytes of data to the object referenced by the descriptor
 * @b fd from the buffer pointed to by @b buffer.
 *
 * @param[in]       fd           File descriptor
 * @param[in]       buffer       Data buffer
 * @param[in,out]   count        Bytes to write
 *
 * @retval     OS_RETURN_CODE_NO_ERROR              If the operation succeeded
 * @retval     OS_RETURN_CODE_FILE_WRITE_FAILED     If the operation failed
 * @return
 *      @b size     Assigned with the number of bytes successfully written\n
 *
 * @par Availability:
 *   @b O/S:   Windows, Linux & macOS* \n
 *   @b CPU:   All \n
 */
OS_RETURN_CODE OS_WriteFD(NATIVE_FD fd, const VOID* buffer, USIZE* count);

/*! @ingroup OS_APIS_FILE
 * Attempts to read @b count bytes of data from the object referenced by the descriptor
 * @b fd to the buffer pointed to by @b buffer.
 *
 * @param[in]       fd           File descriptor
 * @param[in,out]   count        Bytes to read
 * @param[out]      buffer       Output buffer
 *
 * @retval     OS_RETURN_CODE_NO_ERROR              If the operation succeeded
 * @retval     OS_RETURN_CODE_FILE_READ_FAILED      If the operation failed
 * @return
 *      @b size     Assigned with the number of bytes successfully read \n
 *      @b buffer   Contains @b count bytes that were read from @b fd \n
 *
 * @par Availability:
 *   @b O/S:   Windows, Linux & macOS* \n
 *   @b CPU:   All \n
 */
OS_RETURN_CODE OS_ReadFD(NATIVE_FD fd, USIZE* count, VOID* buffer);

/*! @ingroup OS_APIS_FILE
 * Reposition the offset of the file descriptor @b fd to the an offset in,
 * the file denoted by offset.
 * The offset is measured with relation to a starting point determined by the
 * directive @b whence.
 *
 * @param[in]       fd           File descriptor
 * @param[in]       whence       @ref OS_FILE_SEEK_TYPE
 * @param[in,out]   offset       Bytes to move
 *
 * @retval     OS_RETURN_CODE_NO_ERROR              If the operation succeeded
 * @retval     OS_RETURN_CODE_FILE_SEEK_FAILED      If the operation failed
 * @return
 *      @b offset    - Assigned with the number of bytes successfully read\n
 *
 * @par Availability:
 *   @b O/S:   Windows, Linux & macOS* \n
 *   @b CPU:   All \n
 */
OS_RETURN_CODE OS_SeekFD(NATIVE_FD fd, INT whence, INT64* offset);

/*! @ingroup OS_APIS_FILE
 * Closes @b fd and removes references to it from the process.
 *
 * @param[in]  fd    File descriptor
 *
 * @retval     OS_RETURN_CODE_NO_ERROR           If the operation succeeded
 * @retval     OS_RETURN_CODE_FILE_CLOSE_FAILED  If the operation failed
 *
 * @par Availability:
 *   @b O/S:   Windows, Linux & macOS* \n
 *   @b CPU:   All \n
 */
OS_RETURN_CODE OS_CloseFD(NATIVE_FD fd);

/*! @ingroup OS_APIS_FILE
 * Delete a file by name
 *
 * @param[in]  name      File path.
 *
 * @retval     OS_RETURN_CODE_NO_ERROR           If the operation succeeded
 * @retval     OS_RETURN_CODE_FILE_DELETE_FAILED If the operation failed
 *
 * @par Availability:
 *   @b O/S:   Windows, Linux & macOS* \n
 *   @b CPU:   All \n
 */
OS_RETURN_CODE OS_DeleteFile(const CHAR* name);

/*! @ingroup OS_APIS_FILE
 * Flush the object referenced by the descriptor @b fd.
 *
 * @param[in] fd    File descriptor
 *
 * @retval     OS_RETURN_CODE_NO_ERROR      If the operation succeeded
 * @retval     OS_RETURN_CODE_FILE_FLUSH_FAILED  If the operation failed
 *
 * @par Availability:
 *   @b O/S:   Windows \n
 *   @b CPU:   All \n
 */
OS_RETURN_CODE OS_FlushFD(NATIVE_FD fd);

/*! @ingroup OS_APIS_FILE
 * Return information about the file pointed to by specified pathname in a buffer pointed to by statbuf.
 *
 * @param[in]   pathname        Path to the file.
 * @param[in]   statbuf         Pointer to an information buffer about the file stats.
 *
 * @retval     OS_RETURN_CODE_NO_ERROR           If the operation succeeded
 * @retval     OS_RETURN_CODE_FILE_QUERY_FAILED  If the operation failed
 *
 * @note       In Windows, errno error might be different than expected since this function is not native to Windows.
 *
 * @par Availability:
 *   @b O/S:  Windows, Linux & macOS* \n
 *   @b CPU:  All \n
 */
OS_RETURN_CODE OS_Stat(const char* pathname, OS_STRUCT_STAT* statbuf);

/*! @ingroup OS_APIS_FILE
 * Return information about the file pointed to by specified file descriptor in a buffer pointed to by statbuf.
 *
 * @param[in]   fd              file descriptor.
 * @param[in]   statbuf         Pointer to an information buffer about the file stats.
 *
 * @retval     OS_RETURN_CODE_NO_ERROR           If the operation succeeded
 * @retval     OS_RETURN_CODE_FILE_QUERY_FAILED  If the operation failed
 *
 * @note       In Windows, errno error might be different than expected since this function is not native to Windows.
 *
 * @par Availability:
 *   @b O/S:   Windows, Linux & macOS* \n
 *   @b CPU:   All \n
 */
OS_RETURN_CODE OS_Fstat(NATIVE_FD fd, OS_STRUCT_STAT* statbuf);

/*! @ingroup OS_APIS_FILE
 * identical to stat (see above), except that if pathname is a symbolic link, then the link itself is stat-ed, not the
 * file that it refers to.
 *
 * @param[in]   pathname        Path to the file.
 * @param[in]   statbuf         Pointer to an information buffer about the file stats.
 *
 * @retval     OS_RETURN_CODE_NO_ERROR           If the operation succeeded
 * @retval     OS_RETURN_CODE_FILE_QUERY_FAILED  If the operation failed
 *
 * @note       In Windows, errno error might be different than expected since this function is not native to Windows.
 *
 * @par Availability:
 *   @b O/S:  Windows, Linux & macOS* \n
 *   @b CPU:  All \n
 */
OS_RETURN_CODE OS_Lstat(const char* pathname, OS_STRUCT_STAT* statbuf);

/*! @ingroup OS_APIS_FILE
 * Query @b fd's file attributes
 *
 * @param[in]  fd    File descriptor
 * @param[out] attr  File attributes
 *
 * @retval     OS_RETURN_CODE_NO_ERROR           If the operation succeeded
 * @retval     OS_RETURN_CODE_FILE_QUERY_FAILED  If the operation failed
 *
 * @par Availability:
 *   @b O/S:   Windows, Linux & macOS* \n
 *   @b CPU:   All \n
 */
OS_RETURN_CODE OS_GetFDAttributes(NATIVE_FD fd, OS_FILE_ATTRIBUTES* attr);

/*! @ingroup OS_APIS_FILE
 * Retrieve the current permissions for the specified file.
 *
 * @param[in]  fd            File descriptor
 * @param[out] permissions   or'ed @ref OS_FILE_PERMISSION_TYPE.
 *
 * @retval     OS_RETURN_CODE_NO_ERROR           If the operation succeeded
 * @retval     OS_RETURN_CODE_FILE_QUERY_FAILED  If the operation failed
 *
 * @remark
 * On Unix, will provide the user (not group/other) permissions.
 *
 * @par Availability:
 *   @b O/S:   Windows, Linux & macOS* \n
 *   @b CPU:   All \n
 */
OS_RETURN_CODE OS_FilePermissionsFD(NATIVE_FD fd, OS_FILE_PERMISSION_TYPE* permissions);

/*! @ingroup OS_APIS_FILE
 * Retrieves size in bytes of @b fd.
 *
 * @param[in]  fd            File descriptor
 * @param[out] size          Number of bytes.
 *
 * @retval     OS_RETURN_CODE_NO_ERROR           If the operation succeeded
 * @retval     OS_RETURN_CODE_FILE_QUERY_FAILED  If the operation failed
 *
 * @par Availability:
 *   @b O/S:   Windows, Linux & macOS* \n
 *   @b CPU:   All \n
 */
OS_RETURN_CODE OS_FileSizeFD(NATIVE_FD fd, USIZE* size);

/*! @ingroup OS_APIS_FILE
 * Retrieves size in bytes of @b fd.
 *
 * @param[out] cwd           Buffer to receive the CWD.
 * @param[in]  len           Max buffer length.
 *
 * @retval     OS_RETURN_CODE_NO_ERROR           If the operation succeeded
 * @retval     OS_RETURN_CODE_FILE_QUERY_FAILED  If the operation failed
 *
 * @par Availability:
 *   @b O/S:   Windows, Linux & macOS* \n
 *   @b CPU:   All \n
 */
OS_RETURN_CODE OS_Cwd(CHAR* cwd, INT len);

/*! @ingroup OS_APIS_FILE
 * Sets a new current directory. Optionally records the previous directory.
 *
 * @param[in]  dir           The directory to move to.
 * @param[out] cwd          Buffer to receive the CWD.
 * @param[in]  len          Max buffer length for cwd.
 *
 * @retval     OS_RETURN_CODE_NO_ERROR           If the operation succeeded
 * @retval     OS_RETURN_CODE_FILE_QUERY_FAILED  If current directory capture failed
 * @retval     OS_RETURN_CODE_FILE_OPEN_FAILED   If the operation failed
 *
 * @par Availability:
 *   @b O/S:   Windows, Linux & macOS* \n
 *   @b CPU:   All \n
 */
OS_RETURN_CODE OS_Chdir(const CHAR* dir, CHAR* cwd, INT len);

/*! @ingroup OS_APIS_FILE
 * Opens a directory for browsing.
 *
 * @param[in]  name         Directory file name to open.
 * @param[out] fd           Pointer to returned file descriptor.
 *
 * @retval     OS_RETURN_CODE_NO_ERROR           If the operation succeeded
 * @retval     OS_RETURN_CODE_FILE_OPEN_FAILED   If the operation failed
 *
 * @par Availability:
 *   @b O/S:   Windows, Linux & macOS* \n
 *   @b CPU:   All \n
 */
OS_RETURN_CODE OS_OpenDirFD(const CHAR* name, NATIVE_FD* fd);

/*! @ingroup OS_APIS_FILE
 * Creates a directory.
 *
 * @param[in]  name         Directory file name to create.
 * @param[in]  mode         File mode to create the directory.
 *
 * @retval     OS_RETURN_CODE_NO_ERROR           If the operation succeeded.
 * @retval     OS_RETURN_CODE_FILE_EXIST         Directory exists.
 * @retval     OS_RETURN_CODE_FILE_OPEN_FAILED   If the operation failed.
 *
 * @par Availability:
 *   @b O/S:   Windows, Linux & macOS* \n
 *   @b CPU:   All \n
 */
OS_RETURN_CODE OS_MkDir(const CHAR* name, OS_FILE_PERMISSION_TYPE mode);

/*! @ingroup OS_APIS_FILE
 * Deletes a directory.
 *
 * @param[in]  name         Directory file name to delete.
 *
 * @retval     OS_RETURN_CODE_NO_ERROR           If the operation succeeded.
 * @retval     OS_RETURN_CODE_FILE_DELETE_FAILED If the operation failed.
 *
 * @par Availability:
 *   @b O/S:   Windows, Linux & macOS* \n
 *   @b CPU:   All \n
 */
OS_RETURN_CODE OS_DeleteDirectory(const CHAR* name);

/*! @ingroup OS_APIS_FILE
 * Check whether a file descriptor is the application's console.
 * A file descriptor is the application's console if:
 *   1. The file of the underlying descriptor is a terminal device.
 *   2. The terminal device is the tty master (or console) of the current application.
 *
 * The practical implication of a console file descriptor is that every write
 * operation to it needs to be flushed right away because the user on the
 * other side expects to see output immediately.
 *
 * @param[in]  fd           File descriptor to check.
 * @param[out] isConsole    non-zero if 'fd' is the application's console.
 *                          Zero otherwise.
 *
 * @retval     OS_RETURN_CODE_NO_ERROR           If the operation succeeded.
 * @retval     OS_RETURN_CODE_FILE_QUERY_FAILED  If the operation failed.
 *
 * @par Availability:
 *   @b O/S:   Windows, Linux & macOS* \n
 *   @b CPU:   All \n
 */
OS_RETURN_CODE OS_IsConsoleFD(NATIVE_FD fd, INT* isConsole);

/*! @ingroup OS_APIS_FILE
 * Attempts to read @b count bytes of data from the directory referenced by the descriptor
 * @b fd to the buffer pointed to by @b buffer.
 * A console file descriptor usually needs to be flushed after every read/write operation.
 *
 * @param[in]       fd           File descriptor
 * @param[in,out]   count        Bytes to read/Bytes read
 * @param[out]      buffer       Output buffer
 *
 * @retval     OS_RETURN_CODE_NO_ERROR              If the operation succeeded
 * @retval     OS_RETURN_CODE_FILE_READ_FAILED      If the operation failed
 * @return
 *      @b count     - Assigned with the number of bytes successfully read \n
 *      @b buffer    - Contains @b count bytes that were read from @b fd \n
 *
 * @par Availability:
 *   @b O/S:   Windows, Linux & macOS* \n
 *   @b CPU:   All \n
 */
OS_RETURN_CODE OS_ReadDirectoryFD(NATIVE_FD fd, USIZE* count, VOID* buffer);

/*! @ingroup OS_APIS_FILE
 * Get the permissions type requested when @b fd was opened.
 * This is useful to check whether we can read or write @b fd.
 *
 * @param[in]  fd           File descriptor to check.
 * @param[out] mode         The permissions of the file descriptor.
 *
 * @retval     OS_RETURN_CODE_NO_ERROR           If the operation succeeded.
 * @retval     OS_RETURN_CODE_FILE_QUERY_FAILED  If the operation failed.
 *
 * @par Availability:
 *   @b O/S:   Windows, Linux & macOS* \n
 *   @b CPU:   All \n
 */
OS_RETURN_CODE OS_GetFDAccessMode(NATIVE_FD fd, OS_FILE_OPEN_TYPE* mode);

/*! @ingroup OS_APIS_FILE
 * Reads the symbolic link pointed by @b path.
 * Returns the path that the symlink points to.
 *
 * @param[in]     path      Path to the symbolic link to read.
 * @param[out]    buf       The path where the symlink points to.
 * @param[in,out] size      The number of bytes in @b buf, returns the
 *                          number of bytes filled inside @b buf.
 *
 * @retval     OS_RETURN_CODE_NO_ERROR           If the operation succeeded.
 * @retval     OS_RETURN_CODE_FILE_QUERY_FAILED  If the operation failed.
 *
 * @par Availability:
 *   @b O/S:   Linux & macOS* \n
 *   @b CPU:   All \n
 */
OS_RETURN_CODE OS_ReadLink(const CHAR* path, CHAR* buf, USIZE* size);

/*! @ingroup OS_APIS_FILE
 * Query file attributes of the file denoted by @b path
 *
 * @param[in]  path  Path to file
 * @param[out] attr  File attributes
 *
 * @retval     OS_RETURN_CODE_NO_ERROR           If the operation succeeded
 * @retval     OS_RETURN_CODE_FILE_QUERY_FAILED  If the operation failed
 *
 * @par Availability:
 *   @b O/S:   Windows, Linux & macOS* \n
 *   @b CPU:   All \n
 */
OS_RETURN_CODE OS_GetFileAttributes(const CHAR* path, OS_FILE_ATTRIBUTES* attr);

/*! @ingroup OS_APIS_FILE
 * Renamed the file @b oldPath to @b newPath
 *
 * @param[in]  oldPath  Filename to rename
 * @param[in]  newPath  New filename to rename to
 *
 * @retval     OS_RETURN_CODE_NO_ERROR              If the operation succeeded
 * @retval     OS_RETURN_CODE_FILE_OPERATION_FAILED If the operation failed
 *
 * @par Availability:
 *   @b O/S:   Windows, Linux & macOS* \n
 *   @b CPU:   All \n
 */
OS_RETURN_CODE OS_RenameFile(const CHAR* oldPath, const CHAR* newPath);

/*! @ingroup OS_APIS_FILE
 * Get unique ID for an opened file, identified by a file descriptor.
 * Two opened file descriptors for the same physical file are guaranteed
 * to have the same unique ID.
 *
 * @param[in]  fd       File descriptor to calculate an ID.
 * @param[out] uniqueId The calculate unique ID (opaque binary data)
 *
 * @retval     OS_RETURN_CODE_NO_ERROR              If the operation succeeded
 * @retval     OS_RETURN_CODE_FILE_QUERY_FAILED     If the operation failed
 *
 * @par Availability:
 *   @b O/S:   Linux \n
 *   @b CPU:   All \n
 */
OS_RETURN_CODE OS_FileUniqueID(NATIVE_FD fd, OS_FILE_UNIQUE_ID* uniqueId);

/*! @ingroup OS_APIS_FILE
 * Creates a copy of a file descriptor using the lowest available free file descriptor.
 * This function considers the value returned from OS_GetLowestFileDescriptorToUse()
 * when duplicating the descriptor.
 *
 * @param[in]  fd             File descriptor to duplicate an ID.
 * @param[in]  dupCloseOnExec TRUE to duplicate the close-on-exec property of the descriptor,
 *                            FALSE otherwise.
 * @param[out] outFd          Result duplicated file descriptor.
 *
 * @retval     OS_RETURN_CODE_NO_ERROR              If the operation succeeded
 * @retval     OS_RETURN_CODE_FILE_OPERATION_FAILED If the operation failed
 *
 * @par Availability:
 *   @b O/S:   Linux & macOS* \n
 *   @b CPU:   All \n
 */
OS_RETURN_CODE OS_DuplicateFD(NATIVE_FD fd, BOOL_T dupCloseOnExec, NATIVE_FD* outFd);

/*! @ingroup OS_APIS_FILE
 * Truncates a file referenced by fd to a size of length bytes.
 * If the original file size is bigger than length, the extra data is lost.
 * If the original file size is smaller than length,the file extended.
 *
 * @param[in]  fd             File descriptor to change his size.
 * @param[in]  length         File new length.
 *
 * @retval     OS_RETURN_CODE_NO_ERROR              If the operation succeeded
 * @retval     OS_RETURN_CODE_FILE_OPERATION_FAILED If the operation failed
 *
 * @par Availability:
 *   @b O/S:   Windows & Linux & macOS* \n
 *   @b CPU:   All \n
 */
OS_RETURN_CODE OS_Ftruncate(NATIVE_FD fd, INT64 length);

/*! @ingroup OS_APIS_FILE
 *
 * Expands all symbolic links or junction points and resolves references to
 * '.', '..' and extra '/' characters in the null-terminated string
 * named by 'path' to produce a canonicalized absolute pathname.
 *
 * @param[in]  path                 path name. can be full or relative
 * @param[in]  checkPathExistence   if TRUE - function checks if the canonical path exists and returns NULL in
 *                                  case it does not. if FALSE - function returns the canonical path (regardless
 *                                  of it's existence)
 * @param[out] resolvedPath         canonicalized absolute pathname
 *
 * @retval     OS_RETURN_CODE_NO_ERROR              If the operation succeeded
 * @retval     OS_RETURN_CODE_FILE_OPERATION_FAILED If the operation failed
 *
 * @par Availability:
 *   @b O/S:   Windows & Linux & macOS* \n
 *   @b CPU:   All \n
 */
OS_RETURN_CODE OS_Realpath(const char* path, BOOL_T checkPathExistence, char* resolvedPath);

/*! @ingroup OS_APIS_FILE
 * Record a file descriptor as opened by OS-APIs.
 * Later, we allow to query whether a certain file descriptor was opened by
 * OS-APIs or not.
 *
 * @param[in]  fd       File descriptor to record.
 *
 * @par Availability:
 *   @b O/S:   Linux & macOS* \n
 *   @b CPU:   All \n
 */
void OS_ReportFileOpen(NATIVE_FD fd);

/*! @ingroup OS_APIS_FILE
 * Record that a file descriptor opened by OS-APIs is not longer valid,
 * and it is closed.
 * Later, we allow to query whether a certain file descriptor was opened by
 * OS-APIs or not.
 *
 * @param[in]  fd       File descriptor to record.
 *
 * @par Availability:
 *   @b O/S:   Linux & macOS* \n
 *   @b CPU:   All \n
 */
void OS_ReportFileClose(NATIVE_FD fd);

/*! @ingroup OS_APIS_FILE
 * Getter function to retrieve the lowest value of file descriptor that
 * OS-APIs should use.
 * OS-APIs will attempt to only use file descriptors which are larger or
 * equal to the value that this function returns.
 *
 * This function is implemented as a weak reference symbol so libraries
 * linked with OS-APIs may provide their own implementation of this function
 * and return a different value.
 *
 * @retval     Lowest file descriptor to use.
 *
 * @par Availability:
 *   @b O/S:   Linux & macOS* \n
 *   @b CPU:   All \n
 */
NATIVE_FD OS_GetLowestFileDescriptorToUse();

/*! @ingroup OS_APIS_FILE
 * Relocate the file descriptor to the range permitted according to
 * OS_GetLowestFileDescriptorToUse(), then record a file descriptor
 * as opened by OS-APIs.
 * Later, we allow to query whether a certain file descriptor was opened by
 * OS-APIs or not.
 *
 * @param[in,out] fd       File descriptor to relocate and record.
 *
 * @par Availability:
 *   @b O/S:   Linux & macOS* \n
 *   @b CPU:   All \n
 */
void OS_RelocateAndReportFileOpen(NATIVE_FD* fd);

/*! @ingroup OS_APIS_FILE
 * Queries whether a certain file descriptor was opened by
 * OS-APIs or not.
 *
 * @param[in]  fd       File descriptor to query.
 *
 * @par Availability:
 *   @b O/S:   Linux & macOS* \n
 *   @b CPU:   All \n
 */
BOOL_T OS_WasFileReportedOpen(NATIVE_FD fd);

#define OS_APIS_STDERR_FILENAME ((const CHAR*)((ADDRINT)2))

#endif // file guard
