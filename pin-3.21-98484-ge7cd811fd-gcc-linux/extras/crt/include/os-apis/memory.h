/*
 * Copyright (C) 2013-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

// <COMPONENT>: os-apis
// <FILE-TYPE>: component public header

/*!
 * @defgroup OS_APIS_MEMORY Memory
 * @brief Contains memory related os apis
 */

#ifndef OS_APIS_MEMORY_H
#define OS_APIS_MEMORY_H

/*! @ingroup OS_APIS_MEMORY
 * Page protection types
 */
typedef enum
{
    OS_PAGE_PROTECTION_TYPE_NOACCESS = 0,
    OS_PAGE_PROTECTION_TYPE_READ     = (1 << 0),
    OS_PAGE_PROTECTION_TYPE_WRITE    = (1 << 1),
    OS_PAGE_PROTECTION_TYPE_EXECUTE  = (1 << 2),
    OS_PAGE_PROTECTION_TYPE_GUARD    = (1 << 3)
} OS_PAGE_PROTECTION_TYPE;

/*! @ingroup OS_APIS_MEMORY
 * Contains a memory region information
 */
typedef struct
{
    VOID* BaseAddress;                  //!< The base address of the mapped region
    USIZE MapSize;                      //!< The size of the mapped region
    UINT Shared;                        //!< Shared or private region
    OS_PAGE_PROTECTION_TYPE Protection; //!< The protection of this region
    UINT Type;                          //!< Unknown...
} OS_MEMORY_AT_ADDR_INFORMATION;

/*! @ingroup OS_APIS_MEMORY
 * Flags for memory mapping.
 */
typedef enum
{
    OS_MEMORY_FLAGS_PRIVATE       = 0,        //!< The memory should be private to the process
    OS_MEMORY_FLAGS_FIXED_ADDRESS = (1 << 0), //!< The memory should be mapped exactly at the specified address
    OS_MEMORY_FLAGS_STACK         = (1 << 1), //!< The memory should be mapped as stack (grows down).
    OS_MEMORY_FLAGS_SHARED        = (1 << 3)  //!< The memory might be shared among processes
} OS_MEMORY_FLAGS;

/*! @ingroup OS_APIS_MEMORY
 * Allocates @b size bytes of memory in the process identified by @b processId with
 * @ref OS_PAGE_PROTECTION_TYPE "@b protectionType protection".
 *
 * @warning
 *  If @b base is not null, will try to allocate at that address, otherwise will fail!
 *
 * @param[in]     processId          PID of the target process
 * @param[in]     protectionType     or'ed @ref OS_PAGE_PROTECTION_TYPE "protection options"
 * @param[in]     size               Size of memory to allocate
 * @param[in]     flags              Properties of the region to allocate. This is a bitwise
 *                                   OR of the enum values in OS_MEMORY_FLAGS.
 * @param[in,out] base               in: Target memory address\n
 *                                   out: Where the memory was allocated
 *
 * @retval OS_RETURN_CODE_NO_ERROR              If the operation succeeded
 * @retval OS_RETURN_CODE_MEMORY_MAP_FAILED     If the operation Failed
 *
 * @remarks
 *  @b Unix: \n
 *      The processId is irrelevant as it is only applicable on the current process.\n
 *      On Unix system, memory can only be allocated for the calling process.\n
 *      The given pid is expected to be that of the current process.
 *      Hence the behavior of this function in case 'processId' is not of the current
 *      process is not defined.
 *  @b Windows: \n
 *      Windows supports memory allocation on a different process on a single syscall.
 *
 * @par Availability:
 *   @b O/S:   Windows, Linux & macOS*\n
 *   @b CPU:   All\n
 */
OS_RETURN_CODE OS_AllocateMemory(NATIVE_PID processId, UINT protectionType, USIZE size, OS_MEMORY_FLAGS flags, VOID** base);

/*! @ingroup OS_APIS_MEMORY
 * Maps a file into memory, similar to mmap (see man page of mmap(2)).
 *
 * @warning
 *  If @b base is not null, will try to allocate at that address, otherwise will fail!
 *
 * @param[in]     processId          PID of the target process
 * @param[in]     protectionType     or'ed @ref OS_PAGE_PROTECTION_TYPE "protection options"
 * @param[in]     size               Size in bytes of memory to allocate
 * @param[in]     flags              Properties of the region to allocate. This is a bitwise
 *                                   OR of the enum values in OS_MEMORY_FLAGS.
 * @param[in]     fd                 File descriptor of the file that we want to map to memory.
 *                                   This argument can be INVALID_NATIVE_FD if one wants to map
 *                                   an anonymous file.
 * @param[in]     offset             Bytes offset in the file to start mapping from.
 * @param[in,out] base               in: Target memory address\n
 *                                   out: Where the memory was allocated
 *
 * @retval OS_RETURN_CODE_NO_ERROR              If the operation succeeded
 * @retval OS_RETURN_CODE_MEMORY_MAP_FAILED     If the operation Failed
 *
 * @remarks
 *  @b Unix: \n
 *      The processId is irrelevant as it is only applicable on the current process.\n
 *      On Unix system, files can be mapped only in the calling process memory space.\n
 *      The given pid is expected to be that of the current process.\n
 *      Hence the behavior of this function in case 'processId' is not of the current
 *      process is not defined.
 *  @b Windows: \n
 *      Windows supports file mapping on a different process memory space.
 *
 * @par Availability:
 *   @b O/S:   Windows, Linux & macOS*\n
 *   @b CPU:   All\n
 */
OS_RETURN_CODE OS_MapFileToMemory(NATIVE_PID processId, UINT protectionType, USIZE size, OS_MEMORY_FLAGS flags, NATIVE_FD fd,
                                  UINT64 offset, VOID** base);

/*! @ingroup OS_APIS_MEMORY
 * Free's @b size bytes of memory at @b base address in the process identified by @b processId.
 *
 * @param[in] processId          PID of the target process
 * @param[in] base               Target memory address
 * @param[in] size               Size in bytes of memory to free
 *
 * @retval     OS_RETURN_CODE_NO_ERROR              If the operation succeeded
 * @retval     OS_RETURN_CODE_MEMORY_FREE_FAILED    If the operation failed
 *
 * @remarks
 *  @b Unix: \n
 *      The processId is irrelevant as it is only applicable on the current process. \n
 *      On Unix system, memory can only be released in the current process.\n
 *      The given pid is expected to be that of the current process.
 *      Hence the behavior of this function in case 'processId' is not of the current
 *      process is not defined.
 *  @b Windows: \n
 *      Windows supports memory deallocation on a different process.
 *
 * @par Availability:
 *   @b O/S:   Windows, Linux & macOS* \n
 *   @b CPU:   All \n
 */
OS_RETURN_CODE OS_FreeMemory(NATIVE_PID processId, VOID* base, USIZE size);

/*! @ingroup OS_APIS_MEMORY
 * Query the system page size.
 *
 * @param[out] size    System page size
 *
 * @retval     OS_RETURN_CODE_NO_ERROR      If the operation succeeded
 * @retval     OS_RETURN_CODE_QUERY_FAILED  If the operation failed
 *
 * @par Availability:
 *   @b O/S:   Windows, Linux & macOS* \n
 *   @b CPU:   All \n
 */
OS_RETURN_CODE OS_GetPageSize(USIZE* size);

/*! @ingroup OS_APIS_MEMORY
 * Retrieves the information on the memory block containing  @b memoryAddr.
 *
 * @param[in]  processId          PID of the target process
 * @param[in]  memoryAddr         Target memory address. This memory address can reside
 *                                inside the page.
 * @param[out] info               The address information
 *                                On Unix, if there is no mapped memory block that contains @b memoryAddr
 *                                         the next mapped memory block will be returned.
 *                                         If no such mapped memory block exists, an empty memory block will be returned
 *                                On Windows, we return the containing memory block regardless if it is mapped or not.
 *
 * @retval     OS_RETURN_CODE_NO_ERROR      If the operation succeeded
 * @retval     OS_RETURN_CODE_QUERY_FAILED  If the operation failed
 *
 * @remarks
 *  @b Unix: \n
 *      The processId is irrelevant as it is only applicable on the current process. \n
 *      On Unix system, memory can only be queried for the current process.\n
 *      The given pid is expected to be that of the current process.
 *      Hence the behavior of this function in case 'processId' is not of the current
 *      process is not defined.\n
 *  @b Windows: \n
 *      Windows supports memory query on a different process memory space.
 *
 * @par Availability:
 *   @b O/S:   Windows, Linux & macOS* \n
 *   @b CPU:   All \n
 */
OS_RETURN_CODE OS_QueryMemory(NATIVE_PID processId, VOID* memoryAddr, OS_MEMORY_AT_ADDR_INFORMATION* info);

/*! @ingroup OS_APIS_MEMORY
 * Changes protection for the target process's memory page(s) containing
 * any part of the address range in the interval [base, base+size-1].
 * base must be aligned to a page boundary.
 *
 * @param[in]  processId          PID of the target process
 * @param[in]  base               The address of the starting page - must be page aligned.
 * @param[in]  size               Size in bytes of the region to change protection - must be multiple of page size.
 * @param[in]  newProtectionType  The new protection mode
 *
 * @retval     OS_RETURN_CODE_NO_ERROR               If the operation succeeded
 * @retval     OS_RETURN_CODE_MEMORY_PROTECT_FAILED  If the operation failed
 *
 *  @b Unix: \n
 *      The processId is irrelevant as it is only applicable on the current process. \n
 *      On Unix system, memory protection can only be changed for the current process' memory space.\n
 *      The given pid is expected to be that of the current process.
 *      Hence the behavior of this function in case 'processId' is not of the current
 *      process is not defined.
 *  @b Windows: \n
 *      Windows supports memory protection change on a different process memory space.
 *
 * @par Availability:
 *   - @b O/S:   Windows, Linux & macOS*
 *   - @b CPU:   All
 */
OS_RETURN_CODE OS_ProtectMemory(NATIVE_PID processId, VOID* base, USIZE size, UINT newProtectionType);

#endif // file guard
