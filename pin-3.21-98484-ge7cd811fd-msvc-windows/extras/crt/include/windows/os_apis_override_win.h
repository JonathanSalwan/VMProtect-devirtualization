/*
 * Copyright (C) 2015-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

// <COMPONENT>: os-apis
// <FILE-TYPE>: component public header

#include "os-apis.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef OS_RETURN_CODE (*OS_FnPtrWriteFD)(NATIVE_FD fd, const void* buffer, USIZE* size);
    typedef OS_RETURN_CODE (*OS_FnPtrReadFD)(NATIVE_FD fd, USIZE* size, void* buffer);
    typedef OS_RETURN_CODE (*OS_FnPtrIsConsoleFD)(NATIVE_FD fd, INT* isConsole);
    typedef UINT32 (*OS_FnNtCreateFile)(NATIVE_FD* hFile, const CHAR* fileName, UINT32 accessMask, UINT32 objAttributes,
                                        UINT32 fileAttributes, UINT32 shareAccess, UINT32 createDisposition,
                                        UINT32 createOptions);
    typedef UINT32 (*OS_FnNtQueryAttributesFile)(const CHAR* fileName, UINT32 objAttributes, void* fbi);
    typedef UINT32 (*OS_FnRemoveFile)(const CHAR* fileName, UINT32 objAttributes);
    typedef UINT32 (*OS_FnNtAllocateVirtualMemory)(VOID** baseAddress, ADDRINT zeroBits, ADDRINT* regionSize,
                                                   UINT32 allocationType, UINT32 protect);
    typedef UINT32 (*OS_FnNtProtectVirtualMemory)(VOID** baseAddress, ADDRINT* regionSize, UINT32 newProtect, UINT32* oldProtect);
    typedef BOOL_T (*OS_IsBrokerApiEnabled)(BrokerApi api);

    typedef struct _FileApiOverrides
    {
        OS_FnPtrWriteFD writeFd;
        OS_FnPtrReadFD readFd;
        OS_FnPtrIsConsoleFD isConsoleFd;
        OS_FnNtCreateFile ntCreateFile;
        OS_FnNtQueryAttributesFile ntQueryAttributesFile;
        OS_FnRemoveFile RemoveFile;
        OS_FnNtAllocateVirtualMemory ntAllocateVirtualMemory;
        OS_FnNtProtectVirtualMemory ntProtectVirtualMemory;
        OS_IsBrokerApiEnabled isBrokerApiEnabled;
    } FileApiOverrides;

    VOID OS_SetFileApiOverrides(FileApiOverrides* overrides);
    FileApiOverrides* OS_GetFileApiOverrides();

    extern OS_FnPtrCreateProcess pOS_CreateProcess;

#ifdef __cplusplus
}
#endif
