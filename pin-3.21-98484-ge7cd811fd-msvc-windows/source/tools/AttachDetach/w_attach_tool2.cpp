/*
 * Copyright (C) 2008-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

//includes section start

#include "pin.H"

#include <iostream>

#include <fstream>

#include <time.h>

#include <sys/timeb.h>

using std::cerr;
using std::endl;
using std::flush;
using std::ofstream;
using std::string;

namespace WIND
{
//required for waitortimercallback and ptimerapcroutine
#define _WIN32_WINNT 0x0501

//include sockets
#include <winsock2.h>

// include windows.h in its own namespace to avoid conflicting definitions
#include <windows.h>

const DWORD tls_out_of_indexes = TLS_OUT_OF_INDEXES;
} // namespace WIND

//includes section footer

//globals section start

/* ===================================================================== */
/* Commandline Switches */
/* ===================================================================== */

KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "w_attach_tool2.txt", "specify tool log file name");

KNOB< BOOL > KnobDoPrint(KNOB_MODE_WRITEONCE, "pintool", "do_print", "0",
                         "set if print from replacement functions to outputfile is desired");

KNOB< UINT32 > KnobStressDetachReAttach(KNOB_MODE_WRITEONCE, "pintool", "stress_dr", "0",
                                        "stress test for detach & reattach mechanism");

/* ===================================================================== */

INT32 Usage()
{
    cerr << KNOB_BASE::StringKnobSummary();
    cerr << endl;
    cerr.flush();
    return -1;
}

ofstream OutFile;

WIND::DWORD dwIndexTls;

WIND::HRESULT(APIENTRY* fptrPrintDlgExW)(WIND::LPPRINTDLGEXW);

WIND::RPC_STATUS(WINAPI* fptrRpcNsBindingLookupBeginW)(unsigned long, WIND::RPC_WSTR, WIND::RPC_IF_HANDLE, WIND::UUID __RPC_FAR*,
                                                       unsigned long, WIND::RPC_NS_HANDLE __RPC_FAR*);

WIND::HANDLE(WINAPI* fptrGetStdHandle)(WIND::DWORD);

WIND::BOOL(WINAPI* fptrReleaseMutex)(WIND::HANDLE);

WIND::BOOL(WINAPI* fptrGetOverlappedResult)(WIND::HANDLE, WIND::LPOVERLAPPED, WIND::LPDWORD, WIND::BOOL);

WIND::DWORD(WINAPI* fptrWaitForSingleObject)(WIND::HANDLE, WIND::DWORD);

long(WINAPI* fptrBroadcastSystemMessageA)(WIND::DWORD, WIND::LPDWORD, WIND::UINT, WIND::WPARAM, WIND::LPARAM);

WIND::DWORD(WINAPI* fptrSignalObjectAndWait)(WIND::HANDLE, WIND::HANDLE, WIND::DWORD, WIND::BOOL);

WIND::RPC_STATUS(WINAPI* fptrRpcNsBindingLookupBeginA)(unsigned long, WIND::RPC_CSTR, WIND::RPC_IF_HANDLE, WIND::UUID __RPC_FAR*,
                                                       unsigned long, WIND::RPC_NS_HANDLE __RPC_FAR*);

WIND::HRESULT(APIENTRY* fptrPrintDlgExA)(WIND::LPPRINTDLGEXA);

WIND::HANDLE(WINAPI* fptrCreateTimerQueue)();

WIND::RPC_STATUS(WINAPI* fptrRpcNsBindingImportNext)(WIND::RPC_NS_HANDLE, WIND::RPC_BINDING_HANDLE __RPC_FAR*);

WIND::DWORD(WINAPI* fptrQueueUserAPC)(WIND::PAPCFUNC, WIND::HANDLE, WIND::ULONG_PTR);

WIND::RPC_STATUS(WINAPI* fptrRpcNsBindingLookupNext)(WIND::RPC_NS_HANDLE, WIND::RPC_BINDING_VECTOR __RPC_FAR* __RPC_FAR*);

int(WSAAPI* fptrsend)(WIND::SOCKET, const char FAR*, int, int);

WIND::BOOL(WINAPI* fptrPeekMessageA)(WIND::LPMSG, WIND::HWND, WIND::UINT, WIND::UINT, WIND::UINT);

VOID(WINAPI* fptrRtlLeaveCriticalSection)(WIND::LPCRITICAL_SECTION);

WIND::BOOL(WINAPI* fptrReadFileScatter)(WIND::HANDLE, WIND::FILE_SEGMENT_ELEMENT**, WIND::DWORD, __reserved WIND::LPDWORD,
                                        WIND::LPOVERLAPPED);

WIND::BOOL(WINAPI* fptrWaitCommEvent)(WIND::HANDLE, WIND::LPDWORD, WIND::LPOVERLAPPED);

WIND::LRESULT(WINAPI* fptrSendMessageW)(WIND::HWND, WIND::UINT, WIND::WPARAM, WIND::LPARAM);

int(WSAAPI* fptrWSARecv)(WIND::SOCKET, WIND::LPWSABUF, WIND::DWORD, WIND::LPDWORD, WIND::LPDWORD, WIND::LPWSAOVERLAPPED,
                         WIND::LPWSAOVERLAPPED_COMPLETION_ROUTINE);

WIND::LRESULT(WINAPI* fptrSendMessageA)(WIND::HWND, WIND::UINT, WIND::WPARAM, WIND::LPARAM);

WIND::BOOL(WINAPI* fptrWriteFile)(WIND::HANDLE, __in_bcount(nNumberOfBytesToWrite) WIND::LPCVOID, WIND::DWORD, WIND::LPDWORD,
                                  WIND::LPOVERLAPPED);

WIND::DWORD(WINAPI* fptrResumeThread)(WIND::HANDLE);

WIND::RPC_STATUS(WINAPI* fptrRpcMgmtEpEltInqNextA)(WIND::RPC_EP_INQ_HANDLE, WIND::RPC_IF_ID __RPC_FAR*,
                                                   WIND::RPC_BINDING_HANDLE __RPC_FAR*, WIND::UUID __RPC_FAR*,
                                                   __deref_opt_out WIND::RPC_CSTR __RPC_FAR*);

WIND::DWORD(WINAPI* fptrMsgWaitForMultipleObjectsEx)(WIND::DWORD, __in_ecount_opt(nCount) CONST WIND::HANDLE*, WIND::DWORD,
                                                     WIND::DWORD, WIND::DWORD);

long(WINAPI* fptrBroadcastSystemMessageW)(WIND::DWORD, WIND::LPDWORD, WIND::UINT, WIND::WPARAM, WIND::LPARAM);

WIND::BOOL(WINAPI* fptrConnectNamedPipe)(WIND::HANDLE, WIND::LPOVERLAPPED);

VOID(WINAPI* fptrFreeLibraryAndExitThread)(WIND::HMODULE, WIND::DWORD);

VOID(WINAPI* fptrInitializeCriticalSection)(WIND::LPCRITICAL_SECTION);

int(WSAAPI* fptrrecvfrom)(WIND::SOCKET, char FAR*, int, int, struct sockaddr FAR*, int FAR*);

WIND::INT_PTR(WINAPI* fptrDialogBoxIndirectParamW)(WIND::HINSTANCE, WIND::LPCDLGTEMPLATEW, WIND::HWND, WIND::DLGPROC,
                                                   WIND::LPARAM);

WIND::BOOL(WINAPI* fptrDeleteTimerQueue)(WIND::HANDLE);

WIND::DWORD(WINAPI* fptrMsgWaitForMultipleObjects)(WIND::DWORD, __in_ecount_opt(nCount) CONST WIND::HANDLE*, WIND::BOOL,
                                                   WIND::DWORD, WIND::DWORD);

WIND::BOOL(WINAPI* fptrQueueUserWorkItem)(WIND::LPTHREAD_START_ROUTINE, WIND::PVOID, WIND::ULONG);

WIND::HANDLE(WINAPI* fptrConnectToPrinterDlg)(WIND::HWND, WIND::DWORD);

WIND::BOOL(WINAPI* fptrDeviceIoControl)(WIND::HANDLE, WIND::DWORD, __in_bcount_opt(nInBufferSize) WIND::LPVOID, WIND::DWORD,
                                        __out_bcount_part_opt(nOutBufferSize, *lpBytesReturned) WIND::LPVOID, WIND::DWORD,
                                        WIND::LPDWORD, WIND::LPOVERLAPPED);

WIND::SOCKET(WSAAPI* fptrWSASocketA)(int, int, int, WIND::LPWSAPROTOCOL_INFOA, WIND::GROUP, WIND::DWORD);

int(WSAAPI* fptrWSARecvFrom)(WIND::SOCKET, WIND::LPWSABUF, WIND::DWORD, WIND::LPDWORD, WIND::LPDWORD, struct sockaddr FAR*,
                             WIND::LPINT, WIND::LPWSAOVERLAPPED, WIND::LPWSAOVERLAPPED_COMPLETION_ROUTINE);

WIND::RPC_STATUS(WINAPI* fptrRpcStringBindingComposeA)(WIND::RPC_CSTR, WIND::RPC_CSTR, WIND::RPC_CSTR, WIND::RPC_CSTR,
                                                       WIND::RPC_CSTR, __deref_opt_out WIND::RPC_CSTR __RPC_FAR*);

WIND::WSAEVENT(WSAAPI* fptrWSACreateEvent)();

VOID(WINAPI* fptrExitProcess)(WIND::UINT);

int(WSAAPI* fptrconnect)(WIND::SOCKET, const struct sockaddr FAR*, int);

WIND::BOOL(WINAPI* fptrGetMessageW)(WIND::LPMSG, WIND::HWND, WIND::UINT, WIND::UINT);

WIND::BOOL(WSAAPI* fptrWSAResetEvent)(WIND::WSAEVENT);

WIND::RPC_STATUS(WINAPI* fptrRpcStringBindingComposeW)(WIND::RPC_WSTR, WIND::RPC_WSTR, WIND::RPC_WSTR, WIND::RPC_WSTR,
                                                       WIND::RPC_WSTR, __deref_opt_out WIND::RPC_WSTR __RPC_FAR*);

WIND::BOOL(APIENTRY* fptrPrintDlgA)(WIND::LPPRINTDLGA);

WIND::HANDLE(WINAPI* fptrOpenWaitableTimerA)(WIND::DWORD, WIND::BOOL, WIND::LPCSTR);

WIND::HHOOK(WINAPI* fptrSetWindowsHookExW)(int, WIND::HOOKPROC, WIND::HINSTANCE, WIND::DWORD);

int(WSAAPI* fptrWSASend)(WIND::SOCKET, WIND::LPWSABUF, WIND::DWORD, WIND::LPDWORD, WIND::DWORD, WIND::LPWSAOVERLAPPED,
                         WIND::LPWSAOVERLAPPED_COMPLETION_ROUTINE);

WIND::HANDLE(WINAPI* fptrOpenWaitableTimerW)(WIND::DWORD, WIND::BOOL, WIND::LPCWSTR);

WIND::BOOL(WINAPI* fptrDeleteTimerQueueTimer)(WIND::HANDLE, WIND::HANDLE, WIND::HANDLE);

WIND::RPC_STATUS(WINAPI* fptrRpcNsBindingImportDone)(WIND::RPC_NS_HANDLE __RPC_FAR*);

WIND::BOOL(WINAPI* fptrReadConsoleInputW)(WIND::HANDLE, WIND::PINPUT_RECORD, WIND::DWORD, WIND::LPDWORD);

WIND::HANDLE(WINAPI* fptrCreateMutexA)(WIND::LPSECURITY_ATTRIBUTES, WIND::BOOL, WIND::LPCSTR);

WIND::BOOL(WINAPI* fptrRegisterWaitForSingleObject)(WIND::PHANDLE, WIND::HANDLE, WIND::WAITORTIMERCALLBACK, WIND::PVOID,
                                                    WIND::ULONG, WIND::ULONG);

WIND::RPC_STATUS(WINAPI* fptrRpcMgmtInqIfIds)(WIND::RPC_BINDING_HANDLE, WIND::RPC_IF_ID_VECTOR __RPC_FAR* __RPC_FAR*);

WIND::HANDLE(WINAPI* fptrCreateSemaphoreA)(WIND::LPSECURITY_ATTRIBUTES, WIND::LONG, WIND::LONG, WIND::LPCSTR);

WIND::HANDLE(WINAPI* fptrCreateThread)(WIND::LPSECURITY_ATTRIBUTES, WIND::SIZE_T, WIND::LPTHREAD_START_ROUTINE, WIND::LPVOID,
                                       WIND::DWORD, WIND::LPDWORD);

long(WINAPI* fptrBroadcastSystemMessageExW)(WIND::DWORD, WIND::LPDWORD, WIND::UINT, WIND::WPARAM, WIND::LPARAM, WIND::PBSMINFO);

WIND::BOOL(WINAPI* fptrCreateProcessAsUserA)(WIND::HANDLE, WIND::LPCSTR, WIND::LPSTR, WIND::LPSECURITY_ATTRIBUTES,
                                             WIND::LPSECURITY_ATTRIBUTES, WIND::BOOL, WIND::DWORD, WIND::LPVOID, WIND::LPCSTR,
                                             WIND::LPSTARTUPINFOA, WIND::LPPROCESS_INFORMATION);

WIND::HANDLE(WINAPI* fptrCreateSemaphoreW)(WIND::LPSECURITY_ATTRIBUTES, WIND::LONG, WIND::LONG, WIND::LPCWSTR);

WIND::HANDLE(WINAPI* fptrCreateMutexW)(WIND::LPSECURITY_ATTRIBUTES, WIND::BOOL, WIND::LPCWSTR);

WIND::RPC_STATUS (*fptrRpcMgmtWaitServerListen)();

VOID(WINAPI* fptrExitThread)(WIND::DWORD);

int(WSAAPI* fptrrecv)(WIND::SOCKET, char FAR*, int, int);

WIND::BOOL(WINAPI* fptrCreateProcessAsUserW)(WIND::HANDLE, WIND::LPCWSTR, WIND::LPWSTR, WIND::LPSECURITY_ATTRIBUTES,
                                             WIND::LPSECURITY_ATTRIBUTES, WIND::BOOL, WIND::DWORD, WIND::LPVOID, WIND::LPCWSTR,
                                             WIND::LPSTARTUPINFOW, WIND::LPPROCESS_INFORMATION);

VOID(WINAPI* fptrRtlEnterCriticalSection)(WIND::LPCRITICAL_SECTION);

WIND::BOOL(WINAPI* fptrReadConsoleA)(WIND::HANDLE, WIND::LPVOID, WIND::DWORD, WIND::LPDWORD, WIND::LPVOID);

VOID(WINAPI* fptrRtlDeleteCriticalSection)(WIND::LPCRITICAL_SECTION);

WIND::DWORD(WINAPI* fptrWaitForMultipleObjectsEx)(WIND::DWORD, __in_ecount(nCount) CONST WIND::HANDLE*, WIND::BOOL, WIND::DWORD,
                                                  WIND::BOOL);

WIND::BOOL(WINAPI* fptrTerminateProcess)(WIND::HANDLE, WIND::UINT);

WIND::BOOL(WINAPI* fptrFindCloseChangeNotification)(WIND::HANDLE);

WIND::HANDLE(WINAPI* fptrFindFirstPrinterChangeNotification)(WIND::HANDLE, WIND::DWORD, WIND::DWORD, WIND::LPVOID);

WIND::DWORD(WINAPI* fptrWaitForInputIdle)(WIND::HANDLE, WIND::DWORD);

unsigned char*(WINAPI* fptrNdrNsSendReceive)(WIND::PMIDL_STUB_MESSAGE, unsigned char*, WIND::RPC_BINDING_HANDLE*);

WIND::BOOL(WINAPI* fptrFindClosePrinterChangeNotification)(WIND::HANDLE);

WIND::INT_PTR(WINAPI* fptrDialogBoxIndirectParamA)(WIND::HINSTANCE, WIND::LPCDLGTEMPLATEA, WIND::HWND, WIND::DLGPROC,
                                                   WIND::LPARAM);

WIND::BOOL(WINAPI* fptrReadConsoleW)(WIND::HANDLE, WIND::LPVOID, WIND::DWORD, WIND::LPDWORD, WIND::LPVOID);

WIND::BOOL(WINAPI* fptrGetMessageA)(WIND::LPMSG, WIND::HWND, WIND::UINT, WIND::UINT);

WIND::BOOL(WINAPI* fptrSetWaitableTimer)(WIND::HANDLE, const WIND::LARGE_INTEGER*, WIND::LONG, WIND::PTIMERAPCROUTINE,
                                         WIND::LPVOID, WIND::BOOL);

WIND::DWORD(WINAPI* fptrSleepEx)(WIND::DWORD, WIND::BOOL);

WIND::HMODULE(WINAPI* fptrLoadLibraryExW)(WIND::LPCWSTR, __reserved WIND::HANDLE, unsigned long);

WIND::BOOL(WINAPI* fptrCallNamedPipeW)(WIND::LPCWSTR, __in_bcount_opt(nInBufferSize) WIND::LPVOID, WIND::DWORD,
                                       __out_bcount_part_opt(nOutBufferSize, *lpBytesRead) WIND::LPVOID, WIND::DWORD,
                                       WIND::LPDWORD, WIND::DWORD);

WIND::BOOL(WINAPI* fptrSendNotifyMessageA)(WIND::HWND, WIND::UINT, WIND::WPARAM, WIND::LPARAM);

WIND::BOOL(WINAPI* fptrInitializeCriticalSectionAndSpinCount)(WIND::LPCRITICAL_SECTION, WIND::DWORD);

WIND::BOOL(APIENTRY* fptrPrintDlgW)(WIND::LPPRINTDLGW);

WIND::RPC_STATUS(WINAPI* fptrRpcMgmtEpEltInqNextW)(WIND::RPC_EP_INQ_HANDLE, WIND::RPC_IF_ID __RPC_FAR*,
                                                   WIND::RPC_BINDING_HANDLE __RPC_FAR*, WIND::UUID __RPC_FAR*,
                                                   __deref_opt_out WIND::RPC_WSTR __RPC_FAR*);

WIND::RPC_STATUS(WINAPI* fptrRpcMgmtEpEltInqDone)(WIND::RPC_EP_INQ_HANDLE __RPC_FAR*);

WIND::BOOL(WINAPI* fptrPostMessageA)(WIND::HWND, WIND::UINT, WIND::WPARAM, WIND::LPARAM);

WIND::BOOL(WINAPI* fptrTerminateThread)(WIND::HANDLE, WIND::DWORD);

WIND::HMODULE(WINAPI* fptrLoadLibraryW)(WIND::LPCWSTR);

WIND::HANDLE(WINAPI* fptrCreateMailslotW)(WIND::LPCWSTR, WIND::DWORD, WIND::DWORD, WIND::LPSECURITY_ATTRIBUTES);

WIND::BOOL(WINAPI* fptrSetEvent)(WIND::HANDLE);

WIND::BOOL(WINAPI* fptrReadConsoleInputA)(WIND::HANDLE, WIND::PINPUT_RECORD, WIND::DWORD, WIND::LPDWORD);

WIND::BOOL(WINAPI* fptrCreateProcessWithLogonW)(WIND::LPCWSTR, WIND::LPCWSTR, WIND::LPCWSTR, WIND::DWORD, WIND::LPCWSTR,
                                                WIND::LPWSTR, WIND::DWORD, WIND::LPVOID, WIND::LPCWSTR, WIND::LPSTARTUPINFOW,
                                                WIND::LPPROCESS_INFORMATION);

WIND::BOOL(WINAPI* fptrCallNamedPipeA)(WIND::LPCSTR, __in_bcount_opt(nInBufferSize) WIND::LPVOID, WIND::DWORD,
                                       __out_bcount_part_opt(nOutBufferSize, *lpBytesRead) WIND::LPVOID, WIND::DWORD,
                                       WIND::LPDWORD, WIND::DWORD);

WIND::BOOL(WINAPI* fptrSendNotifyMessageW)(WIND::HWND, WIND::UINT, WIND::WPARAM, WIND::LPARAM);

WIND::HANDLE(WINAPI* fptrCreateMailslotA)(WIND::LPCSTR, WIND::DWORD, WIND::DWORD, WIND::LPSECURITY_ATTRIBUTES);

int(WSAAPI* fptrselect)(int, WIND::fd_set FAR*, WIND::fd_set FAR*, WIND::fd_set FAR*, const struct timeval FAR*);

WIND::HANDLE(WINAPI* fptrCreateRemoteThread)(WIND::HANDLE, WIND::LPSECURITY_ATTRIBUTES, WIND::SIZE_T,
                                             WIND::LPTHREAD_START_ROUTINE, WIND::LPVOID, WIND::DWORD, WIND::LPDWORD);

WIND::HHOOK(WINAPI* fptrSetWindowsHookExA)(int, WIND::HOOKPROC, WIND::HINSTANCE, WIND::DWORD);

long(WINAPI* fptrBroadcastSystemMessageExA)(WIND::DWORD, WIND::LPDWORD, WIND::UINT, WIND::WPARAM, WIND::LPARAM, WIND::PBSMINFO);

WIND::HANDLE(WINAPI* fptrOpenProcess)(WIND::DWORD, WIND::BOOL, WIND::DWORD);

WIND::DWORD(WSAAPI* fptrWSAWaitForMultipleEvents)(WIND::DWORD, const WIND::WSAEVENT FAR*, WIND::BOOL, WIND::DWORD, WIND::BOOL);

WIND::RPC_STATUS(WINAPI* fptrRpcMgmtEpEltInqBegin)(WIND::RPC_BINDING_HANDLE, unsigned long, WIND::RPC_IF_ID __RPC_FAR*,
                                                   unsigned long, WIND::UUID __RPC_FAR*, WIND::RPC_EP_INQ_HANDLE __RPC_FAR*);

WIND::BOOL(WINAPI* fptrWriteFileGather)(WIND::HANDLE, WIND::FILE_SEGMENT_ELEMENT**, WIND::DWORD, __reserved WIND::LPDWORD,
                                        WIND::LPOVERLAPPED);

WIND::DWORD(WINAPI* fptrWaitForMultipleObjects)(WIND::DWORD, __in_ecount(nCount) CONST WIND::HANDLE*, WIND::BOOL, WIND::DWORD);

unsigned char*(WINAPI* fptrNdrSendReceive)(WIND::PMIDL_STUB_MESSAGE, unsigned char*);

WIND::HANDLE(WINAPI* fptrCreateWaitableTimerW)(WIND::LPSECURITY_ATTRIBUTES, WIND::BOOL, WIND::LPCWSTR);

WIND::BOOL(WINAPI* fptrWaitMessage)();

WIND::DWORD(WINAPI* fptrSetCriticalSectionSpinCount)(WIND::LPCRITICAL_SECTION, WIND::DWORD);

WIND::BOOL(WINAPI* fptrWaitNamedPipeW)(WIND::LPCWSTR, WIND::DWORD);

int(WSAAPI* fptrsendto)(WIND::SOCKET, const char FAR*, int, int, const struct sockaddr FAR*, int);

WIND::BOOL(WINAPI* fptrWaitNamedPipeA)(WIND::LPCSTR, WIND::DWORD);

WIND::BOOL(WINAPI* fptrResetEvent)(WIND::HANDLE);

WIND::BOOL(WINAPI* fptrCreateTimerQueueTimer)(WIND::PHANDLE, WIND::HANDLE, WIND::WAITORTIMERCALLBACK, WIND::PVOID, WIND::DWORD,
                                              WIND::DWORD, WIND::ULONG);

WIND::HANDLE(WINAPI* fptrCreateWaitableTimerA)(WIND::LPSECURITY_ATTRIBUTES, WIND::BOOL, WIND::LPCSTR);

WIND::BOOL(WINAPI* fptrDuplicateHandle)(WIND::HANDLE, WIND::HANDLE, WIND::HANDLE, WIND::LPHANDLE, WIND::DWORD, WIND::BOOL,
                                        WIND::DWORD);

int(WSAAPI* fptrclosesocket)(WIND::SOCKET);

WIND::FARPROC(WINAPI* fptrGetProcAddress)(WIND::HMODULE, WIND::LPCSTR);

WIND::SOCKET(WSAAPI* fptrsocket)(int, int, int);

WIND::BOOL(WSAAPI* fptrWSASetEvent)(WIND::WSAEVENT);

WIND::HANDLE(WINAPI* fptrCreateEventW)(WIND::LPSECURITY_ATTRIBUTES, WIND::BOOL, WIND::BOOL, WIND::LPCWSTR);

WIND::BOOL(WINAPI* fptrPostThreadMessageW)(WIND::DWORD, WIND::UINT, WIND::WPARAM, WIND::LPARAM);

int(WSAAPI* fptrWSASendTo)(WIND::SOCKET, WIND::LPWSABUF, WIND::DWORD, WIND::LPDWORD, WIND::DWORD, const struct sockaddr FAR*, int,
                           WIND::LPWSAOVERLAPPED, WIND::LPWSAOVERLAPPED_COMPLETION_ROUTINE);

WIND::HANDLE(WINAPI* fptrCreateFileW)(WIND::LPCWSTR, WIND::DWORD, WIND::DWORD, WIND::LPSECURITY_ATTRIBUTES, WIND::DWORD,
                                      WIND::DWORD, WIND::HANDLE);

WIND::HANDLE(WINAPI* fptrCreateEventA)(WIND::LPSECURITY_ATTRIBUTES, WIND::BOOL, WIND::BOOL, WIND::LPCSTR);

WIND::RPC_STATUS(WINAPI* fptrRpcServerListen)(unsigned int, unsigned int, unsigned int);

WIND::HANDLE(WINAPI* fptrCreateFileA)(WIND::LPCSTR, WIND::DWORD, WIND::DWORD, WIND::LPSECURITY_ATTRIBUTES, WIND::DWORD,
                                      WIND::DWORD, WIND::HANDLE);

WIND::BOOL(WINAPI* fptrDeleteTimerQueueEx)(WIND::HANDLE, WIND::HANDLE);

WIND::LRESULT(WINAPI* fptrSendMessageTimeoutW)(WIND::HWND, WIND::UINT, WIND::WPARAM, WIND::LPARAM, WIND::UINT, WIND::UINT,
                                               WIND::PDWORD_PTR);

WIND::BOOL(WINAPI* fptrPostThreadMessageA)(WIND::DWORD, WIND::UINT, WIND::WPARAM, WIND::LPARAM);

WIND::SOCKET(WSAAPI* fptrWSASocketW)(int, int, int, WIND::LPWSAPROTOCOL_INFOW, WIND::GROUP, WIND::DWORD);

WIND::BOOL(WSAAPI* fptrWSAGetOverlappedResult)(WIND::SOCKET, WIND::LPWSAOVERLAPPED, WIND::LPDWORD, WIND::BOOL, WIND::LPDWORD);

int(WSAAPI* fptrWSAConnect)(WIND::SOCKET, const struct sockaddr FAR*, int, WIND::LPWSABUF, WIND::LPWSABUF, WIND::LPQOS,
                            WIND::LPQOS);

WIND::HANDLE(WINAPI* fptrFindFirstChangeNotificationA)(WIND::LPCSTR, WIND::BOOL, WIND::DWORD);

WIND::HANDLE(WINAPI* fptrCreateNamedPipeW)(WIND::LPCWSTR, WIND::DWORD, WIND::DWORD, WIND::DWORD, WIND::DWORD, WIND::DWORD,
                                           WIND::DWORD, WIND::LPSECURITY_ATTRIBUTES);

WIND::SOCKET(WSAAPI* fptraccept)(WIND::SOCKET, struct sockaddr FAR*, int FAR*);

WIND::HANDLE(WINAPI* fptrOpenSemaphoreA)(WIND::DWORD, WIND::BOOL, WIND::LPCSTR);

WIND::BOOL(WINAPI* fptrUnregisterWaitEx)(WIND::HANDLE, WIND::HANDLE);

WIND::RPC_STATUS(WINAPI* fptrRpcMgmtIsServerListening)(WIND::RPC_BINDING_HANDLE);

WIND::DWORD(WINAPI* fptrWaitForSingleObjectEx)(WIND::HANDLE, WIND::DWORD, WIND::BOOL);

WIND::HANDLE(WINAPI* fptrCreateNamedPipeA)(WIND::LPCSTR, WIND::DWORD, WIND::DWORD, WIND::DWORD, WIND::DWORD, WIND::DWORD,
                                           WIND::DWORD, WIND::LPSECURITY_ATTRIBUTES);

WIND::HANDLE(WINAPI* fptrFindFirstChangeNotificationW)(WIND::LPCWSTR, WIND::BOOL, WIND::DWORD);

int(WINAPI* fptrMessageBoxExA)(WIND::HWND, WIND::LPCSTR, WIND::LPCSTR, WIND::UINT, WIND::WORD);

WIND::BOOL(WINAPI* fptrGetQueuedCompletionStatus)(WIND::HANDLE, WIND::LPDWORD, WIND::PULONG_PTR, WIND::LPOVERLAPPED, WIND::DWORD);

WIND::BOOL(WINAPI* fptrCancelWaitableTimer)(WIND::HANDLE);

int(WINAPI* fptrMessageBoxW)(WIND::HWND, WIND::LPCWSTR, WIND::LPCWSTR, WIND::UINT);

WIND::SOCKET(WSAAPI* fptrWSAAccept)(WIND::SOCKET, struct sockaddr FAR*, WIND::LPINT, WIND::LPCONDITIONPROC, WIND::DWORD_PTR);

WIND::BOOL(WINAPI* fptrUnregisterWait)(WIND::HANDLE);

WIND::BOOL(WSAAPI* fptrWSACloseEvent)(WIND::WSAEVENT);

WIND::HANDLE(WINAPI* fptrCreateIoCompletionPort)(WIND::HANDLE, WIND::HANDLE, WIND::ULONG_PTR, WIND::DWORD);

WIND::INT_PTR(WINAPI* fptrDialogBoxParamW)(WIND::HINSTANCE, WIND::LPCWSTR, WIND::HWND, WIND::DLGPROC, WIND::LPARAM);

int(WINAPI* fptrMessageBoxA)(WIND::HWND, WIND::LPCSTR, WIND::LPCSTR, WIND::UINT);

int(WINAPI* fptrMessageBoxExW)(WIND::HWND, WIND::LPCWSTR, WIND::LPCWSTR, WIND::UINT, WIND::WORD);

WIND::INT_PTR(WINAPI* fptrDialogBoxParamA)(WIND::HINSTANCE, WIND::LPCSTR, WIND::HWND, WIND::DLGPROC, WIND::LPARAM);

VOID(WINAPI* fptrSleep)(WIND::DWORD);

WIND::RPC_STATUS(WINAPI* fptrRpcNsBindingLookupDone)(WIND::RPC_NS_HANDLE __RPC_FAR*);

WIND::HANDLE(WINAPI* fptrOpenMutexA)(WIND::DWORD, WIND::BOOL, WIND::LPCSTR);

WIND::DWORD(WINAPI* fptrSuspendThread)(WIND::HANDLE);

VOID(WINAPI* fptrRaiseException)
(WIND::DWORD, WIND::DWORD, WIND::DWORD, __in_ecount_opt(nNumberOfArguments) CONST WIND::ULONG_PTR*);

WIND::RPC_STATUS(WINAPI* fptrRpcNsBindingImportBeginW)(unsigned long, WIND::RPC_WSTR, WIND::RPC_IF_HANDLE, WIND::UUID __RPC_FAR*,
                                                       WIND::RPC_NS_HANDLE __RPC_FAR*);

WIND::BOOL(WINAPI* fptrReleaseSemaphore)(WIND::HANDLE, WIND::LONG, WIND::LPLONG);

WIND::HFILE(WINAPI* fptrOpenFile)(WIND::LPCSTR, WIND::LPOFSTRUCT, WIND::UINT);

WIND::BOOL(WINAPI* fptrReadFile)(WIND::HANDLE, __out_bcount_part(nNumberOfBytesToRead, *lpNumberOfBytesRead) WIND::LPVOID,
                                 WIND::DWORD, WIND::LPDWORD, WIND::LPOVERLAPPED);

WIND::BOOL(WINAPI* fptrWriteFileEx)(WIND::HANDLE, __in_bcount(nNumberOfBytesToWrite) WIND::LPCVOID, WIND::DWORD,
                                    WIND::LPOVERLAPPED, WIND::LPOVERLAPPED_COMPLETION_ROUTINE);

WIND::RPC_STATUS(WINAPI* fptrRpcNsBindingImportBeginA)(unsigned long, WIND::RPC_CSTR, WIND::RPC_IF_HANDLE, WIND::UUID __RPC_FAR*,
                                                       WIND::RPC_NS_HANDLE __RPC_FAR*);

WIND::BOOL(WINAPI* fptrPulseEvent)(WIND::HANDLE);

WIND::RPC_STATUS(WINAPI* fptrRpcCancelThread)(void*);

WIND::BOOL(WINAPI* fptrCloseHandle)(WIND::HANDLE);

WIND::HANDLE(WINAPI* fptrOpenMutexW)(WIND::DWORD, WIND::BOOL, WIND::LPCWSTR);

WIND::BOOL(WINAPI* fptrPostMessageW)(WIND::HWND, WIND::UINT, WIND::WPARAM, WIND::LPARAM);

WIND::RPC_STATUS(WINAPI* fptrRpcEpResolveBinding)(WIND::RPC_BINDING_HANDLE, WIND::RPC_IF_HANDLE);

WIND::BOOL(WINAPI* fptrBindIoCompletionCallback)(WIND::HANDLE, WIND::LPOVERLAPPED_COMPLETION_ROUTINE, WIND::ULONG);

WIND::BOOL(WINAPI* fptrPeekMessageW)(WIND::LPMSG, WIND::HWND, WIND::UINT, WIND::UINT, WIND::UINT);

WIND::LRESULT(WINAPI* fptrSendMessageTimeoutA)(WIND::HWND, WIND::UINT, WIND::WPARAM, WIND::LPARAM, WIND::UINT, WIND::UINT,
                                               WIND::PDWORD_PTR);

WIND::HANDLE(WINAPI* fptrOpenEventA)(WIND::DWORD, WIND::BOOL, WIND::LPCSTR);

WIND::BOOL(WINAPI* fptrCreateProcessA)(WIND::LPCSTR, WIND::LPSTR, WIND::LPSECURITY_ATTRIBUTES, WIND::LPSECURITY_ATTRIBUTES,
                                       WIND::BOOL, WIND::DWORD, WIND::LPVOID, WIND::LPCSTR, WIND::LPSTARTUPINFOA,
                                       WIND::LPPROCESS_INFORMATION);

WIND::HANDLE(WINAPI* fptrOpenEventW)(WIND::DWORD, WIND::BOOL, WIND::LPCWSTR);

WIND::BOOL(WINAPI* fptrPostQueuedCompletionStatus)(WIND::HANDLE, WIND::DWORD, WIND::ULONG_PTR, WIND::LPOVERLAPPED);

WIND::BOOL(WINAPI* fptrCreateProcessW)(WIND::LPCWSTR, WIND::LPWSTR, WIND::LPSECURITY_ATTRIBUTES, WIND::LPSECURITY_ATTRIBUTES,
                                       WIND::BOOL, WIND::DWORD, WIND::LPVOID, WIND::LPCWSTR, WIND::LPSTARTUPINFOW,
                                       WIND::LPPROCESS_INFORMATION);

WIND::BOOL(WINAPI* fptrTransactNamedPipe)(WIND::HANDLE, __in_bcount_opt(nInBufferSize) WIND::LPVOID, WIND::DWORD,
                                          __out_bcount_part_opt(nOutBufferSize, *lpBytesRead) WIND::LPVOID, WIND::DWORD,
                                          WIND::LPDWORD, WIND::LPOVERLAPPED);

WIND::BOOL(WINAPI* fptrReadFileEx)(WIND::HANDLE, __out_bcount(nNumberOfBytesToRead) WIND::LPVOID, WIND::DWORD, WIND::LPOVERLAPPED,
                                   WIND::LPOVERLAPPED_COMPLETION_ROUTINE);

WIND::HANDLE(WINAPI* fptrOpenSemaphoreW)(WIND::DWORD, WIND::BOOL, WIND::LPCWSTR);

WIND::BOOL(WINAPI* fptrReplyMessage)(WIND::LRESULT);

typedef int* INT_PTR;

//globals section footer

//functions section start

string CurrentTime()

{
    // Utility function to return the time

    char tmpbuf[128];
    _strtime_s(tmpbuf, 128);
    return string("[") + string(tmpbuf) + string("] ");
}

typedef int(__cdecl* DO_LOOP_TYPE)();

static volatile int doLoopPred = 1;

static volatile int globalCounter = 0;

static volatile int attachCycles = 0;

int rep_DoLoop()
{
    PIN_LockClient();

    int localPred = doLoopPred;

    PIN_UnlockClient();

    return localPred;
}

WIND::HRESULT APIENTRY myPrintDlgExW(WIND::LPPRINTDLGEXW lpDlg)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering PrintDlgExW" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::HRESULT res = fptrPrintDlgExW(lpDlg);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving PrintDlgExW" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::RPC_STATUS WINAPI myRpcNsBindingLookupBeginW(unsigned long EntryNameSyntax, WIND::RPC_WSTR EntryName,
                                                   WIND::RPC_IF_HANDLE IfSpec, WIND::UUID __RPC_FAR* ObjUuid,
                                                   unsigned long BindingMaxCount, WIND::RPC_NS_HANDLE __RPC_FAR* LookupContext)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering RpcNsBindingLookupBeginW" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::RPC_STATUS res =
        fptrRpcNsBindingLookupBeginW(EntryNameSyntax, EntryName, IfSpec, ObjUuid, BindingMaxCount, LookupContext);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving RpcNsBindingLookupBeginW" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::HANDLE WINAPI myGetStdHandle(WIND::DWORD hStdHandle)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering GetStdHandle" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::HANDLE res = fptrGetStdHandle(hStdHandle);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving GetStdHandle" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::BOOL WINAPI myReleaseMutex(WIND::HANDLE hMutex)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering ReleaseMutex" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::BOOL res = fptrReleaseMutex(hMutex);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving ReleaseMutex" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::BOOL WINAPI myGetOverlappedResult(WIND::HANDLE hFile, WIND::LPOVERLAPPED lpOverlapped,
                                        WIND::LPDWORD lpNumberOfBytesTransferred, WIND::BOOL bWait)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering GetOverlappedResult" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::BOOL res = fptrGetOverlappedResult(hFile, lpOverlapped, lpNumberOfBytesTransferred, bWait);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving GetOverlappedResult" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::DWORD WINAPI myWaitForSingleObject(WIND::HANDLE hHandle, WIND::DWORD dwMilliseconds)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering WaitForSingleObject" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::DWORD res = fptrWaitForSingleObject(hHandle, dwMilliseconds);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving WaitForSingleObject" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

long WINAPI myBroadcastSystemMessageA(WIND::DWORD flags, WIND::LPDWORD lpInfo, WIND::UINT Msg, WIND::WPARAM wParam,
                                      WIND::LPARAM lParam)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering BroadcastSystemMessageA" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    long res = fptrBroadcastSystemMessageA(flags, lpInfo, Msg, wParam, lParam);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving BroadcastSystemMessageA" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::DWORD WINAPI mySignalObjectAndWait(WIND::HANDLE hObjectToSignal, WIND::HANDLE hObjectToWaitOn, WIND::DWORD dwMilliseconds,
                                         WIND::BOOL bAlertable)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering SignalObjectAndWait" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::DWORD res = fptrSignalObjectAndWait(hObjectToSignal, hObjectToWaitOn, dwMilliseconds, bAlertable);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving SignalObjectAndWait" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::RPC_STATUS WINAPI myRpcNsBindingLookupBeginA(unsigned long EntryNameSyntax, WIND::RPC_CSTR EntryName,
                                                   WIND::RPC_IF_HANDLE IfSpec, WIND::UUID __RPC_FAR* ObjUuid,
                                                   unsigned long BindingMaxCount, WIND::RPC_NS_HANDLE __RPC_FAR* LookupContext)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering RpcNsBindingLookupBeginA" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::RPC_STATUS res =
        fptrRpcNsBindingLookupBeginA(EntryNameSyntax, EntryName, IfSpec, ObjUuid, BindingMaxCount, LookupContext);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving RpcNsBindingLookupBeginA" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::HRESULT APIENTRY myPrintDlgExA(WIND::LPPRINTDLGEXA lpDlg)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering PrintDlgExA" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::HRESULT res = fptrPrintDlgExA(lpDlg);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving PrintDlgExA" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::HANDLE WINAPI myCreateTimerQueue()

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering CreateTimerQueue" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::HANDLE res = fptrCreateTimerQueue();

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving CreateTimerQueue" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::RPC_STATUS WINAPI myRpcNsBindingImportNext(WIND::RPC_NS_HANDLE ImportContext, WIND::RPC_BINDING_HANDLE __RPC_FAR* Binding)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering RpcNsBindingImportNext" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::RPC_STATUS res = fptrRpcNsBindingImportNext(ImportContext, Binding);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving RpcNsBindingImportNext" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::DWORD WINAPI myQueueUserAPC(WIND::PAPCFUNC pfnAPC, WIND::HANDLE hThread, WIND::ULONG_PTR dwData)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering QueueUserAPC" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::DWORD res = fptrQueueUserAPC(pfnAPC, hThread, dwData);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving QueueUserAPC" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::RPC_STATUS WINAPI myRpcNsBindingLookupNext(WIND::RPC_NS_HANDLE LookupContext,
                                                 WIND::RPC_BINDING_VECTOR __RPC_FAR* __RPC_FAR* BindingVec)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering RpcNsBindingLookupNext" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::RPC_STATUS res = fptrRpcNsBindingLookupNext(LookupContext, BindingVec);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving RpcNsBindingLookupNext" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

int WSAAPI mysend(WIND::SOCKET s, const char FAR* buf, int len, int flags)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering send" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    int res = fptrsend(s, buf, len, flags);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving send" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::BOOL WINAPI myPeekMessageA(WIND::LPMSG lpMsg, WIND::HWND hWnd, WIND::UINT wMsgFilterMin, WIND::UINT wMsgFilterMax,
                                 WIND::UINT wRemoveMsg)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering PeekMessageA" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::BOOL res = fptrPeekMessageA(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving PeekMessageA" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

VOID WINAPI myRtlLeaveCriticalSection(WIND::LPCRITICAL_SECTION lpCriticalSection)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering RtlLeaveCriticalSection" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    fptrRtlLeaveCriticalSection(lpCriticalSection);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving RtlLeaveCriticalSection" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);
}

WIND::BOOL WINAPI myReadFileScatter(WIND::HANDLE hFile, WIND::FILE_SEGMENT_ELEMENT** aSegmentArray,
                                    WIND::DWORD nNumberOfBytesToRead, __reserved WIND::LPDWORD lpReserved,
                                    WIND::LPOVERLAPPED lpOverlapped)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering ReadFileScatter" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::BOOL res = fptrReadFileScatter(hFile, aSegmentArray, nNumberOfBytesToRead, lpReserved, lpOverlapped);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving ReadFileScatter" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::BOOL WINAPI myWaitCommEvent(WIND::HANDLE hFile, WIND::LPDWORD lpEvtMask, WIND::LPOVERLAPPED lpOverlapped)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering WaitCommEvent" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::BOOL res = fptrWaitCommEvent(hFile, lpEvtMask, lpOverlapped);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving WaitCommEvent" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::LRESULT WINAPI mySendMessageW(WIND::HWND hWnd, WIND::UINT Msg, WIND::WPARAM wParam, WIND::LPARAM lParam)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering SendMessageW" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::LRESULT res = fptrSendMessageW(hWnd, Msg, wParam, lParam);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving SendMessageW" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

int WSAAPI myWSARecv(WIND::SOCKET s, WIND::LPWSABUF lpBuffers, WIND::DWORD dwBufferCount, WIND::LPDWORD lpNumberOfBytesRecvd,
                     WIND::LPDWORD lpFlags, WIND::LPWSAOVERLAPPED lpOverlapped,
                     WIND::LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering WSARecv" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    int res = fptrWSARecv(s, lpBuffers, dwBufferCount, lpNumberOfBytesRecvd, lpFlags, lpOverlapped, lpCompletionRoutine);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving WSARecv" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::LRESULT WINAPI mySendMessageA(WIND::HWND hWnd, WIND::UINT Msg, WIND::WPARAM wParam, WIND::LPARAM lParam)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering SendMessageA" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::LRESULT res = fptrSendMessageA(hWnd, Msg, wParam, lParam);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving SendMessageA" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::BOOL WINAPI myWriteFile(WIND::HANDLE hFile, __in_bcount(nNumberOfBytesToWrite) WIND::LPCVOID lpBuffer,
                              WIND::DWORD nNumberOfBytesToWrite, WIND::LPDWORD lpNumberOfBytesWritten,
                              WIND::LPOVERLAPPED lpOverlapped)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering WriteFile" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::BOOL res = fptrWriteFile(hFile, lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, lpOverlapped);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving WriteFile" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::DWORD WINAPI myResumeThread(WIND::HANDLE hThread)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering ResumeThread" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::DWORD res = fptrResumeThread(hThread);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving ResumeThread" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::RPC_STATUS WINAPI myRpcMgmtEpEltInqNextA(WIND::RPC_EP_INQ_HANDLE InquiryContext, WIND::RPC_IF_ID __RPC_FAR* IfId,
                                               WIND::RPC_BINDING_HANDLE __RPC_FAR* Binding, WIND::UUID __RPC_FAR* ObjectUuid,
                                               __deref_opt_out WIND::RPC_CSTR __RPC_FAR* Annotation)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering RpcMgmtEpEltInqNextA" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::RPC_STATUS res = fptrRpcMgmtEpEltInqNextA(InquiryContext, IfId, Binding, ObjectUuid, Annotation);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving RpcMgmtEpEltInqNextA" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::DWORD WINAPI myMsgWaitForMultipleObjectsEx(WIND::DWORD nCount, __in_ecount_opt(nCount) CONST WIND::HANDLE* pHandles,
                                                 WIND::DWORD dwMilliseconds, WIND::DWORD dwWakeMask, WIND::DWORD dwFlags)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering MsgWaitForMultipleObjectsEx" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::DWORD res = fptrMsgWaitForMultipleObjectsEx(nCount, pHandles, dwMilliseconds, dwWakeMask, dwFlags);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving MsgWaitForMultipleObjectsEx" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

long WINAPI myBroadcastSystemMessageW(WIND::DWORD flags, WIND::LPDWORD lpInfo, WIND::UINT Msg, WIND::WPARAM wParam,
                                      WIND::LPARAM lParam)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering BroadcastSystemMessageW" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    long res = fptrBroadcastSystemMessageW(flags, lpInfo, Msg, wParam, lParam);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving BroadcastSystemMessageW" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::BOOL WINAPI myConnectNamedPipe(WIND::HANDLE hNamedPipe, WIND::LPOVERLAPPED lpOverlapped)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering ConnectNamedPipe" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::BOOL res = fptrConnectNamedPipe(hNamedPipe, lpOverlapped);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving ConnectNamedPipe" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

VOID WINAPI myFreeLibraryAndExitThread(WIND::HMODULE hLibModule, WIND::DWORD dwExitCode)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering FreeLibraryAndExitThread" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    fptrFreeLibraryAndExitThread(hLibModule, dwExitCode);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving FreeLibraryAndExitThread" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);
}

VOID WINAPI myInitializeCriticalSection(WIND::LPCRITICAL_SECTION lpCriticalSection)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering InitializeCriticalSection" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    fptrInitializeCriticalSection(lpCriticalSection);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving InitializeCriticalSection" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);
}

int WSAAPI myrecvfrom(WIND::SOCKET s, char FAR* buf, int len, int flags, struct sockaddr FAR* from, int FAR* fromlen)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering recvfrom" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    int res = fptrrecvfrom(s, buf, len, flags, from, fromlen);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving recvfrom" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::INT_PTR WINAPI myDialogBoxIndirectParamW(WIND::HINSTANCE hInstance, WIND::LPCDLGTEMPLATEW hDialogTemplate,
                                               WIND::HWND hWndParent, WIND::DLGPROC lpDialogFunc, WIND::LPARAM dwInitParam)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering DialogBoxIndirectParamW" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::INT_PTR res = fptrDialogBoxIndirectParamW(hInstance, hDialogTemplate, hWndParent, lpDialogFunc, dwInitParam);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving DialogBoxIndirectParamW" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::BOOL WINAPI myDeleteTimerQueue(WIND::HANDLE TimerQueue)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering DeleteTimerQueue" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::BOOL res = fptrDeleteTimerQueue(TimerQueue);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving DeleteTimerQueue" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::DWORD WINAPI myMsgWaitForMultipleObjects(WIND::DWORD nCount, __in_ecount_opt(nCount) CONST WIND::HANDLE* pHandles,
                                               WIND::BOOL fWaitAll, WIND::DWORD dwMilliseconds, WIND::DWORD dwWakeMask)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering MsgWaitForMultipleObjects" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::DWORD res = fptrMsgWaitForMultipleObjects(nCount, pHandles, fWaitAll, dwMilliseconds, dwWakeMask);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving MsgWaitForMultipleObjects" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::BOOL WINAPI myQueueUserWorkItem(WIND::LPTHREAD_START_ROUTINE Function, WIND::PVOID Context, WIND::ULONG Flags)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering QueueUserWorkItem" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::BOOL res = fptrQueueUserWorkItem(Function, Context, Flags);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving QueueUserWorkItem" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::HANDLE WINAPI myConnectToPrinterDlg(WIND::HWND hwnd, WIND::DWORD Flags)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering ConnectToPrinterDlg" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::HANDLE res = fptrConnectToPrinterDlg(hwnd, Flags);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving ConnectToPrinterDlg" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::BOOL WINAPI myDeviceIoControl(WIND::HANDLE hDevice, WIND::DWORD dwIoControlCode,
                                    __in_bcount_opt(nInBufferSize) WIND::LPVOID lpInBuffer, WIND::DWORD nInBufferSize,
                                    __out_bcount_part_opt(nOutBufferSize, *lpBytesReturned) WIND::LPVOID lpOutBuffer,
                                    WIND::DWORD nOutBufferSize, WIND::LPDWORD lpBytesReturned, WIND::LPOVERLAPPED lpOverlapped)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering DeviceIoControl" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::BOOL res = fptrDeviceIoControl(hDevice, dwIoControlCode, lpInBuffer, nInBufferSize, lpOutBuffer, nOutBufferSize,
                                         lpBytesReturned, lpOverlapped);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving DeviceIoControl" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::SOCKET WSAAPI myWSASocketA(int af, int type, int protocol, WIND::LPWSAPROTOCOL_INFOA lpProtocolInfo, WIND::GROUP g,
                                 WIND::DWORD dwFlags)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering WSASocketA" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::SOCKET res = fptrWSASocketA(af, type, protocol, lpProtocolInfo, g, dwFlags);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving WSASocketA" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

int WSAAPI myWSARecvFrom(WIND::SOCKET s, WIND::LPWSABUF lpBuffers, WIND::DWORD dwBufferCount, WIND::LPDWORD lpNumberOfBytesRecvd,
                         WIND::LPDWORD lpFlags, struct sockaddr FAR* lpFrom, WIND::LPINT lpFromlen,
                         WIND::LPWSAOVERLAPPED lpOverlapped, WIND::LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering WSARecvFrom" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    int res = fptrWSARecvFrom(s, lpBuffers, dwBufferCount, lpNumberOfBytesRecvd, lpFlags, lpFrom, lpFromlen, lpOverlapped,
                              lpCompletionRoutine);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving WSARecvFrom" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::RPC_STATUS WINAPI myRpcStringBindingComposeA(WIND::RPC_CSTR ObjUuid, WIND::RPC_CSTR Protseq, WIND::RPC_CSTR NetworkAddr,
                                                   WIND::RPC_CSTR Endpoint, WIND::RPC_CSTR Options,
                                                   __deref_opt_out WIND::RPC_CSTR __RPC_FAR* StringBinding)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering RpcStringBindingComposeA" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::RPC_STATUS res = fptrRpcStringBindingComposeA(ObjUuid, Protseq, NetworkAddr, Endpoint, Options, StringBinding);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving RpcStringBindingComposeA" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::WSAEVENT WSAAPI myWSACreateEvent()

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering WSACreateEvent" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::WSAEVENT res = fptrWSACreateEvent();

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving WSACreateEvent" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

VOID WINAPI myExitProcess(WIND::UINT uExitCode)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering ExitProcess" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    fptrExitProcess(uExitCode);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving ExitProcess" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);
}

int WSAAPI myconnect(WIND::SOCKET s, const struct sockaddr FAR* name, int namelen)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering connect" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    int res = fptrconnect(s, name, namelen);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving connect" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::BOOL WINAPI myGetMessageW(WIND::LPMSG lpMsg, WIND::HWND hWnd, WIND::UINT wMsgFilterMin, WIND::UINT wMsgFilterMax)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering GetMessageW" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::BOOL res = fptrGetMessageW(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving GetMessageW" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::BOOL WSAAPI myWSAResetEvent(WIND::WSAEVENT hEvent)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering WSAResetEvent" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::BOOL res = fptrWSAResetEvent(hEvent);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving WSAResetEvent" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::RPC_STATUS WINAPI myRpcStringBindingComposeW(WIND::RPC_WSTR ObjUuid, WIND::RPC_WSTR Protseq, WIND::RPC_WSTR NetworkAddr,
                                                   WIND::RPC_WSTR Endpoint, WIND::RPC_WSTR Options,
                                                   __deref_opt_out WIND::RPC_WSTR __RPC_FAR* StringBinding)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering RpcStringBindingComposeW" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::RPC_STATUS res = fptrRpcStringBindingComposeW(ObjUuid, Protseq, NetworkAddr, Endpoint, Options, StringBinding);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving RpcStringBindingComposeW" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::BOOL APIENTRY myPrintDlgA(WIND::LPPRINTDLGA lpDlg)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering PrintDlgA" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::BOOL res = fptrPrintDlgA(lpDlg);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving PrintDlgA" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::HANDLE WINAPI myOpenWaitableTimerA(WIND::DWORD dwDesiredAccess, WIND::BOOL bInheritHandle, WIND::LPCSTR lpTimerName)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering OpenWaitableTimerA" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::HANDLE res = fptrOpenWaitableTimerA(dwDesiredAccess, bInheritHandle, lpTimerName);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving OpenWaitableTimerA" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::HHOOK WINAPI mySetWindowsHookExW(int idHook, WIND::HOOKPROC lpfn, WIND::HINSTANCE hmod, WIND::DWORD dwThreadId)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering SetWindowsHookExW" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::HHOOK res = fptrSetWindowsHookExW(idHook, lpfn, hmod, dwThreadId);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving SetWindowsHookExW" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

int WSAAPI myWSASend(WIND::SOCKET s, WIND::LPWSABUF lpBuffers, WIND::DWORD dwBufferCount, WIND::LPDWORD lpNumberOfBytesSent,
                     WIND::DWORD dwFlags, WIND::LPWSAOVERLAPPED lpOverlapped,
                     WIND::LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering WSASend" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    int res = fptrWSASend(s, lpBuffers, dwBufferCount, lpNumberOfBytesSent, dwFlags, lpOverlapped, lpCompletionRoutine);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving WSASend" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::HANDLE WINAPI myOpenWaitableTimerW(WIND::DWORD dwDesiredAccess, WIND::BOOL bInheritHandle, WIND::LPCWSTR lpTimerName)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering OpenWaitableTimerW" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::HANDLE res = fptrOpenWaitableTimerW(dwDesiredAccess, bInheritHandle, lpTimerName);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving OpenWaitableTimerW" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::BOOL WINAPI myDeleteTimerQueueTimer(WIND::HANDLE TimerQueue, WIND::HANDLE Timer, WIND::HANDLE CompletionEvent)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering DeleteTimerQueueTimer" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::BOOL res = fptrDeleteTimerQueueTimer(TimerQueue, Timer, CompletionEvent);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving DeleteTimerQueueTimer" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::RPC_STATUS WINAPI myRpcNsBindingImportDone(WIND::RPC_NS_HANDLE __RPC_FAR* ImportContext)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering RpcNsBindingImportDone" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::RPC_STATUS res = fptrRpcNsBindingImportDone(ImportContext);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving RpcNsBindingImportDone" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::BOOL WINAPI myReadConsoleInputW(WIND::HANDLE hConsoleInput, WIND::PINPUT_RECORD lpBuffer, WIND::DWORD nLength,
                                      WIND::LPDWORD lpNumberOfEventsRead)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering ReadConsoleInputW" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::BOOL res = fptrReadConsoleInputW(hConsoleInput, lpBuffer, nLength, lpNumberOfEventsRead);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving ReadConsoleInputW" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::HANDLE WINAPI myCreateMutexA(WIND::LPSECURITY_ATTRIBUTES lpMutexAttributes, WIND::BOOL bInitialOwner, WIND::LPCSTR lpName)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering CreateMutexA" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::HANDLE res = fptrCreateMutexA(lpMutexAttributes, bInitialOwner, lpName);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving CreateMutexA" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::BOOL WINAPI myRegisterWaitForSingleObject(WIND::PHANDLE phNewWaitObject, WIND::HANDLE hObject,
                                                WIND::WAITORTIMERCALLBACK Callback, WIND::PVOID Context,
                                                WIND::ULONG dwMilliseconds, WIND::ULONG dwFlags)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering RegisterWaitForSingleObject" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::BOOL res = fptrRegisterWaitForSingleObject(phNewWaitObject, hObject, Callback, Context, dwMilliseconds, dwFlags);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving RegisterWaitForSingleObject" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::RPC_STATUS WINAPI myRpcMgmtInqIfIds(WIND::RPC_BINDING_HANDLE Binding,
                                          WIND::RPC_IF_ID_VECTOR __RPC_FAR* __RPC_FAR* IfIdVector)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering RpcMgmtInqIfIds" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::RPC_STATUS res = fptrRpcMgmtInqIfIds(Binding, IfIdVector);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving RpcMgmtInqIfIds" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::HANDLE WINAPI myCreateSemaphoreA(WIND::LPSECURITY_ATTRIBUTES lpSemaphoreAttributes, WIND::LONG lInitialCount,
                                       WIND::LONG lMaximumCount, WIND::LPCSTR lpName)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering CreateSemaphoreA" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::HANDLE res = fptrCreateSemaphoreA(lpSemaphoreAttributes, lInitialCount, lMaximumCount, lpName);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving CreateSemaphoreA" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::HANDLE WINAPI myCreateThread(WIND::LPSECURITY_ATTRIBUTES lpThreadAttributes, WIND::SIZE_T dwStackSize,
                                   WIND::LPTHREAD_START_ROUTINE lpStartAddress, WIND::LPVOID lpParameter,
                                   WIND::DWORD dwCreationFlags, WIND::LPDWORD lpThreadId)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering CreateThread" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::HANDLE res =
        fptrCreateThread(lpThreadAttributes, dwStackSize, lpStartAddress, lpParameter, dwCreationFlags, lpThreadId);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving CreateThread" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

long WINAPI myBroadcastSystemMessageExW(WIND::DWORD flags, WIND::LPDWORD lpInfo, WIND::UINT Msg, WIND::WPARAM wParam,
                                        WIND::LPARAM lParam, WIND::PBSMINFO pbsmInfo)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering BroadcastSystemMessageExW" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    long res = fptrBroadcastSystemMessageExW(flags, lpInfo, Msg, wParam, lParam, pbsmInfo);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving BroadcastSystemMessageExW" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::BOOL WINAPI myCreateProcessAsUserA(WIND::HANDLE hToken, WIND::LPCSTR lpApplicationName, WIND::LPSTR lpCommandLine,
                                         WIND::LPSECURITY_ATTRIBUTES lpProcessAttributes,
                                         WIND::LPSECURITY_ATTRIBUTES lpThreadAttributes, WIND::BOOL bInheritHandles,
                                         WIND::DWORD dwCreationFlags, WIND::LPVOID lpEnvironment, WIND::LPCSTR lpCurrentDirectory,
                                         WIND::LPSTARTUPINFOA lpStartupInfo, WIND::LPPROCESS_INFORMATION lpProcessInformation)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering CreateProcessAsUserA" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::BOOL res = fptrCreateProcessAsUserA(hToken, lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes,
                                              bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo,
                                              lpProcessInformation);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving CreateProcessAsUserA" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::HANDLE WINAPI myCreateSemaphoreW(WIND::LPSECURITY_ATTRIBUTES lpSemaphoreAttributes, WIND::LONG lInitialCount,
                                       WIND::LONG lMaximumCount, WIND::LPCWSTR lpName)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering CreateSemaphoreW" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::HANDLE res = fptrCreateSemaphoreW(lpSemaphoreAttributes, lInitialCount, lMaximumCount, lpName);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving CreateSemaphoreW" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::HANDLE WINAPI myCreateMutexW(WIND::LPSECURITY_ATTRIBUTES lpMutexAttributes, WIND::BOOL bInitialOwner, WIND::LPCWSTR lpName)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering CreateMutexW" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::HANDLE res = fptrCreateMutexW(lpMutexAttributes, bInitialOwner, lpName);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving CreateMutexW" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::RPC_STATUS myRpcMgmtWaitServerListen()

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering RpcMgmtWaitServerListen" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::RPC_STATUS res = fptrRpcMgmtWaitServerListen();

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving RpcMgmtWaitServerListen" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

VOID WINAPI myExitThread(WIND::DWORD dwExitCode)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering ExitThread" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    fptrExitThread(dwExitCode);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving ExitThread" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);
}

int WSAAPI myrecv(WIND::SOCKET s, char FAR* buf, int len, int flags)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering recv" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    int res = fptrrecv(s, buf, len, flags);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving recv" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::BOOL WINAPI myCreateProcessAsUserW(WIND::HANDLE hToken, WIND::LPCWSTR lpApplicationName, WIND::LPWSTR lpCommandLine,
                                         WIND::LPSECURITY_ATTRIBUTES lpProcessAttributes,
                                         WIND::LPSECURITY_ATTRIBUTES lpThreadAttributes, WIND::BOOL bInheritHandles,
                                         WIND::DWORD dwCreationFlags, WIND::LPVOID lpEnvironment,
                                         WIND::LPCWSTR lpCurrentDirectory, WIND::LPSTARTUPINFOW lpStartupInfo,
                                         WIND::LPPROCESS_INFORMATION lpProcessInformation)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering CreateProcessAsUserW" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::BOOL res = fptrCreateProcessAsUserW(hToken, lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes,
                                              bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo,
                                              lpProcessInformation);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving CreateProcessAsUserW" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

VOID WINAPI myRtlEnterCriticalSection(WIND::LPCRITICAL_SECTION lpCriticalSection)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering RtlEnterCriticalSection" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    fptrRtlEnterCriticalSection(lpCriticalSection);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving RtlEnterCriticalSection" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);
}

WIND::BOOL WINAPI myReadConsoleA(WIND::HANDLE hConsoleInput, WIND::LPVOID lpBuffer, WIND::DWORD nNumberOfCharsToRead,
                                 WIND::LPDWORD lpNumberOfCharsRead, WIND::LPVOID lpReserved)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering ReadConsoleA" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::BOOL res = fptrReadConsoleA(hConsoleInput, lpBuffer, nNumberOfCharsToRead, lpNumberOfCharsRead, lpReserved);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving ReadConsoleA" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

VOID WINAPI myRtlDeleteCriticalSection(WIND::LPCRITICAL_SECTION lpCriticalSection)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering RtlDeleteCriticalSection" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    fptrRtlDeleteCriticalSection(lpCriticalSection);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving RtlDeleteCriticalSection" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);
}

WIND::DWORD WINAPI myWaitForMultipleObjectsEx(WIND::DWORD nCount, __in_ecount(nCount) CONST WIND::HANDLE* lpHandles,
                                              WIND::BOOL bWaitAll, WIND::DWORD dwMilliseconds, WIND::BOOL bAlertable)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering WaitForMultipleObjectsEx" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::DWORD res = fptrWaitForMultipleObjectsEx(nCount, lpHandles, bWaitAll, dwMilliseconds, bAlertable);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving WaitForMultipleObjectsEx" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::BOOL WINAPI myTerminateProcess(WIND::HANDLE hProcess, WIND::UINT uExitCode)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering TerminateProcess" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::BOOL res = fptrTerminateProcess(hProcess, uExitCode);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving TerminateProcess" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::BOOL WINAPI myFindCloseChangeNotification(WIND::HANDLE hChangeHandle)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering FindCloseChangeNotification" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::BOOL res = fptrFindCloseChangeNotification(hChangeHandle);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving FindCloseChangeNotification" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::HANDLE WINAPI myFindFirstPrinterChangeNotification(WIND::HANDLE hPrinter, WIND::DWORD fdwFlags, WIND::DWORD fdwOptions,
                                                         WIND::LPVOID pPrinterNotifyOptions)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering FindFirstPrinterChangeNotification" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::HANDLE res = fptrFindFirstPrinterChangeNotification(hPrinter, fdwFlags, fdwOptions, pPrinterNotifyOptions);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving FindFirstPrinterChangeNotification" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::DWORD WINAPI myWaitForInputIdle(WIND::HANDLE hProcess, WIND::DWORD dwMilliseconds)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering WaitForInputIdle" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::DWORD res = fptrWaitForInputIdle(hProcess, dwMilliseconds);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving WaitForInputIdle" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

unsigned char* WINAPI myNdrNsSendReceive(WIND::PMIDL_STUB_MESSAGE pStubMsg, unsigned char* pBufferEnd,
                                         WIND::RPC_BINDING_HANDLE* pAutoHandle)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering NdrNsSendReceive" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    unsigned char* res = fptrNdrNsSendReceive(pStubMsg, pBufferEnd, pAutoHandle);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving NdrNsSendReceive" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::BOOL WINAPI myFindClosePrinterChangeNotification(WIND::HANDLE hChange)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering FindClosePrinterChangeNotification" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::BOOL res = fptrFindClosePrinterChangeNotification(hChange);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving FindClosePrinterChangeNotification" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::INT_PTR WINAPI myDialogBoxIndirectParamA(WIND::HINSTANCE hInstance, WIND::LPCDLGTEMPLATEA hDialogTemplate,
                                               WIND::HWND hWndParent, WIND::DLGPROC lpDialogFunc, WIND::LPARAM dwInitParam)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering DialogBoxIndirectParamA" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::INT_PTR res = fptrDialogBoxIndirectParamA(hInstance, hDialogTemplate, hWndParent, lpDialogFunc, dwInitParam);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving DialogBoxIndirectParamA" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::BOOL WINAPI myReadConsoleW(WIND::HANDLE hConsoleInput, WIND::LPVOID lpBuffer, WIND::DWORD nNumberOfCharsToRead,
                                 WIND::LPDWORD lpNumberOfCharsRead, WIND::LPVOID lpReserved)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering ReadConsoleW" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::BOOL res = fptrReadConsoleW(hConsoleInput, lpBuffer, nNumberOfCharsToRead, lpNumberOfCharsRead, lpReserved);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving ReadConsoleW" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::BOOL WINAPI myGetMessageA(WIND::LPMSG lpMsg, WIND::HWND hWnd, WIND::UINT wMsgFilterMin, WIND::UINT wMsgFilterMax)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering GetMessageA" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::BOOL res = fptrGetMessageA(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving GetMessageA" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::BOOL WINAPI mySetWaitableTimer(WIND::HANDLE hTimer, const WIND::LARGE_INTEGER* lpDueTime, WIND::LONG lPeriod,
                                     WIND::PTIMERAPCROUTINE pfnCompletionRoutine, WIND::LPVOID lpArgToCompletionRoutine,
                                     WIND::BOOL fResume)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering SetWaitableTimer" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::BOOL res = fptrSetWaitableTimer(hTimer, lpDueTime, lPeriod, pfnCompletionRoutine, lpArgToCompletionRoutine, fResume);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving SetWaitableTimer" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::DWORD WINAPI mySleepEx(WIND::DWORD dwMilliseconds, WIND::BOOL bAlertable)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering SleepEx" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::DWORD res = fptrSleepEx(dwMilliseconds, bAlertable);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving SleepEx" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::HMODULE WINAPI myLoadLibraryExW(WIND::LPCWSTR lpLibFileName, __reserved WIND::HANDLE hFile, unsigned long dwFlags)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering LoadLibraryExW" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::HMODULE res = fptrLoadLibraryExW(lpLibFileName, hFile, dwFlags);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        PIN_LockClient();

        globalCounter++;

        if (globalCounter >= 100 && attachCycles >= KnobStressDetachReAttach.Value() && doLoopPred != 0)
        {
            //eventhough this is not an error - print to cerr (in order to see it on the screen)
            cerr << "exiting from application!" << endl;
            doLoopPred = 0;
        }

        PIN_UnlockClient();

        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving LoadLibraryExW" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::BOOL WINAPI myCallNamedPipeW(WIND::LPCWSTR lpNamedPipeName, __in_bcount_opt(nInBufferSize) WIND::LPVOID lpInBuffer,
                                   WIND::DWORD nInBufferSize,
                                   __out_bcount_part_opt(nOutBufferSize, *lpBytesRead) WIND::LPVOID lpOutBuffer,
                                   WIND::DWORD nOutBufferSize, WIND::LPDWORD lpBytesRead, WIND::DWORD nTimeOut)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering CallNamedPipeW" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::BOOL res =
        fptrCallNamedPipeW(lpNamedPipeName, lpInBuffer, nInBufferSize, lpOutBuffer, nOutBufferSize, lpBytesRead, nTimeOut);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving CallNamedPipeW" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::BOOL WINAPI mySendNotifyMessageA(WIND::HWND hWnd, WIND::UINT Msg, WIND::WPARAM wParam, WIND::LPARAM lParam)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering SendNotifyMessageA" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::BOOL res = fptrSendNotifyMessageA(hWnd, Msg, wParam, lParam);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving SendNotifyMessageA" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::BOOL WINAPI myInitializeCriticalSectionAndSpinCount(WIND::LPCRITICAL_SECTION lpCriticalSection, WIND::DWORD dwSpinCount)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering InitializeCriticalSectionAndSpinCount" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::BOOL res = fptrInitializeCriticalSectionAndSpinCount(lpCriticalSection, dwSpinCount);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving InitializeCriticalSectionAndSpinCount" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::BOOL APIENTRY myPrintDlgW(WIND::LPPRINTDLGW lpDlg)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering PrintDlgW" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::BOOL res = fptrPrintDlgW(lpDlg);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving PrintDlgW" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::RPC_STATUS WINAPI myRpcMgmtEpEltInqNextW(WIND::RPC_EP_INQ_HANDLE InquiryContext, WIND::RPC_IF_ID __RPC_FAR* IfId,
                                               WIND::RPC_BINDING_HANDLE __RPC_FAR* Binding, WIND::UUID __RPC_FAR* ObjectUuid,
                                               __deref_opt_out WIND::RPC_WSTR __RPC_FAR* Annotation)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering RpcMgmtEpEltInqNextW" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::RPC_STATUS res = fptrRpcMgmtEpEltInqNextW(InquiryContext, IfId, Binding, ObjectUuid, Annotation);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving RpcMgmtEpEltInqNextW" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::RPC_STATUS WINAPI myRpcMgmtEpEltInqDone(WIND::RPC_EP_INQ_HANDLE __RPC_FAR* InquiryContext)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering RpcMgmtEpEltInqDone" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::RPC_STATUS res = fptrRpcMgmtEpEltInqDone(InquiryContext);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving RpcMgmtEpEltInqDone" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::BOOL WINAPI myPostMessageA(WIND::HWND hWnd, WIND::UINT Msg, WIND::WPARAM wParam, WIND::LPARAM lParam)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering PostMessageA" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::BOOL res = fptrPostMessageA(hWnd, Msg, wParam, lParam);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving PostMessageA" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::BOOL WINAPI myTerminateThread(WIND::HANDLE hThread, WIND::DWORD dwExitCode)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering TerminateThread" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::BOOL res = fptrTerminateThread(hThread, dwExitCode);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving TerminateThread" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::HMODULE WINAPI myLoadLibraryW(WIND::LPCWSTR lpLibFileName)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering LoadLibraryW" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::HMODULE res = fptrLoadLibraryW(lpLibFileName);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);

        PIN_LockClient();

        globalCounter++;

        if (globalCounter >= 100 && attachCycles >= KnobStressDetachReAttach.Value() && doLoopPred != 0)
        {
            //eventhough this is not an error - print to cerr (in order to see it on the screen)
            cerr << "success -  exiting from application!" << endl;
            doLoopPred = 0;
        }

        PIN_UnlockClient();

        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving LoadLibraryW" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::HANDLE WINAPI myCreateMailslotW(WIND::LPCWSTR lpName, WIND::DWORD nMaxMessageSize, WIND::DWORD lReadTimeout,
                                      WIND::LPSECURITY_ATTRIBUTES lpSecurityAttributes)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering CreateMailslotW" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::HANDLE res = fptrCreateMailslotW(lpName, nMaxMessageSize, lReadTimeout, lpSecurityAttributes);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving CreateMailslotW" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::BOOL WINAPI mySetEvent(WIND::HANDLE hEvent)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering SetEvent" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::BOOL res = fptrSetEvent(hEvent);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving SetEvent" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::BOOL WINAPI myReadConsoleInputA(WIND::HANDLE hConsoleInput, WIND::PINPUT_RECORD lpBuffer, WIND::DWORD nLength,
                                      WIND::LPDWORD lpNumberOfEventsRead)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering ReadConsoleInputA" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::BOOL res = fptrReadConsoleInputA(hConsoleInput, lpBuffer, nLength, lpNumberOfEventsRead);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving ReadConsoleInputA" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::BOOL WINAPI myCreateProcessWithLogonW(WIND::LPCWSTR lpUsername, WIND::LPCWSTR lpDomain, WIND::LPCWSTR lpPassword,
                                            WIND::DWORD dwLogonFlags, WIND::LPCWSTR lpApplicationName, WIND::LPWSTR lpCommandLine,
                                            WIND::DWORD dwCreationFlags, WIND::LPVOID lpEnvironment,
                                            WIND::LPCWSTR lpCurrentDirectory, WIND::LPSTARTUPINFOW lpStartupInfo,
                                            WIND::LPPROCESS_INFORMATION lpProcessInformation)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering CreateProcessWithLogonW" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::BOOL res =
        fptrCreateProcessWithLogonW(lpUsername, lpDomain, lpPassword, dwLogonFlags, lpApplicationName, lpCommandLine,
                                    dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving CreateProcessWithLogonW" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::BOOL WINAPI myCallNamedPipeA(WIND::LPCSTR lpNamedPipeName, __in_bcount_opt(nInBufferSize) WIND::LPVOID lpInBuffer,
                                   WIND::DWORD nInBufferSize,
                                   __out_bcount_part_opt(nOutBufferSize, *lpBytesRead) WIND::LPVOID lpOutBuffer,
                                   WIND::DWORD nOutBufferSize, WIND::LPDWORD lpBytesRead, WIND::DWORD nTimeOut)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering CallNamedPipeA" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::BOOL res =
        fptrCallNamedPipeA(lpNamedPipeName, lpInBuffer, nInBufferSize, lpOutBuffer, nOutBufferSize, lpBytesRead, nTimeOut);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving CallNamedPipeA" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::BOOL WINAPI mySendNotifyMessageW(WIND::HWND hWnd, WIND::UINT Msg, WIND::WPARAM wParam, WIND::LPARAM lParam)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering SendNotifyMessageW" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::BOOL res = fptrSendNotifyMessageW(hWnd, Msg, wParam, lParam);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving SendNotifyMessageW" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::HANDLE WINAPI myCreateMailslotA(WIND::LPCSTR lpName, WIND::DWORD nMaxMessageSize, WIND::DWORD lReadTimeout,
                                      WIND::LPSECURITY_ATTRIBUTES lpSecurityAttributes)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering CreateMailslotA" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::HANDLE res = fptrCreateMailslotA(lpName, nMaxMessageSize, lReadTimeout, lpSecurityAttributes);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving CreateMailslotA" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

int WSAAPI myselect(int nfds, WIND::fd_set FAR* readfds, WIND::fd_set FAR* writefds, WIND::fd_set FAR* exceptfds,
                    const struct timeval FAR* timeout)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering select" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    int res = fptrselect(nfds, readfds, writefds, exceptfds, timeout);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving select" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::HANDLE WINAPI myCreateRemoteThread(WIND::HANDLE hProcess, WIND::LPSECURITY_ATTRIBUTES lpThreadAttributes,
                                         WIND::SIZE_T dwStackSize, WIND::LPTHREAD_START_ROUTINE lpStartAddress,
                                         WIND::LPVOID lpParameter, WIND::DWORD dwCreationFlags, WIND::LPDWORD lpThreadId)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering CreateRemoteThread" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::HANDLE res = fptrCreateRemoteThread(hProcess, lpThreadAttributes, dwStackSize, lpStartAddress, lpParameter,
                                              dwCreationFlags, lpThreadId);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving CreateRemoteThread" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::HHOOK WINAPI mySetWindowsHookExA(int idHook, WIND::HOOKPROC lpfn, WIND::HINSTANCE hmod, WIND::DWORD dwThreadId)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering SetWindowsHookExA" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::HHOOK res = fptrSetWindowsHookExA(idHook, lpfn, hmod, dwThreadId);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving SetWindowsHookExA" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

long WINAPI myBroadcastSystemMessageExA(WIND::DWORD flags, WIND::LPDWORD lpInfo, WIND::UINT Msg, WIND::WPARAM wParam,
                                        WIND::LPARAM lParam, WIND::PBSMINFO pbsmInfo)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering BroadcastSystemMessageExA" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    long res = fptrBroadcastSystemMessageExA(flags, lpInfo, Msg, wParam, lParam, pbsmInfo);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving BroadcastSystemMessageExA" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::HANDLE WINAPI myOpenProcess(WIND::DWORD dwDesiredAccess, WIND::BOOL bInheritHandle, WIND::DWORD dwProcessId)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering OpenProcess" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::HANDLE res = fptrOpenProcess(dwDesiredAccess, bInheritHandle, dwProcessId);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving OpenProcess" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::DWORD WSAAPI myWSAWaitForMultipleEvents(WIND::DWORD cEvents, const WIND::WSAEVENT FAR* lphEvents, WIND::BOOL fWaitAll,
                                              WIND::DWORD dwTimeout, WIND::BOOL fAlertable)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering WSAWaitForMultipleEvents" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::DWORD res = fptrWSAWaitForMultipleEvents(cEvents, lphEvents, fWaitAll, dwTimeout, fAlertable);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving WSAWaitForMultipleEvents" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::RPC_STATUS WINAPI myRpcMgmtEpEltInqBegin(WIND::RPC_BINDING_HANDLE EpBinding, unsigned long InquiryType,
                                               WIND::RPC_IF_ID __RPC_FAR* IfId, unsigned long VersOption,
                                               WIND::UUID __RPC_FAR* ObjectUuid,
                                               WIND::RPC_EP_INQ_HANDLE __RPC_FAR* InquiryContext)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering RpcMgmtEpEltInqBegin" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::RPC_STATUS res = fptrRpcMgmtEpEltInqBegin(EpBinding, InquiryType, IfId, VersOption, ObjectUuid, InquiryContext);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving RpcMgmtEpEltInqBegin" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::BOOL WINAPI myWriteFileGather(WIND::HANDLE hFile, WIND::FILE_SEGMENT_ELEMENT** aSegmentArray,
                                    WIND::DWORD nNumberOfBytesToWrite, __reserved WIND::LPDWORD lpReserved,
                                    WIND::LPOVERLAPPED lpOverlapped)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering WriteFileGather" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::BOOL res = fptrWriteFileGather(hFile, aSegmentArray, nNumberOfBytesToWrite, lpReserved, lpOverlapped);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving WriteFileGather" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::DWORD WINAPI myWaitForMultipleObjects(WIND::DWORD nCount, __in_ecount(nCount) CONST WIND::HANDLE* lpHandles,
                                            WIND::BOOL bWaitAll, WIND::DWORD dwMilliseconds)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering WaitForMultipleObjects" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::DWORD res = fptrWaitForMultipleObjects(nCount, lpHandles, bWaitAll, dwMilliseconds);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving WaitForMultipleObjects" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

unsigned char* WINAPI myNdrSendReceive(WIND::PMIDL_STUB_MESSAGE pStubMsg, unsigned char* pBufferEnd)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering NdrSendReceive" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    unsigned char* res = fptrNdrSendReceive(pStubMsg, pBufferEnd);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving NdrSendReceive" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::HANDLE WINAPI myCreateWaitableTimerW(WIND::LPSECURITY_ATTRIBUTES lpTimerAttributes, WIND::BOOL bManualReset,
                                           WIND::LPCWSTR lpTimerName)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering CreateWaitableTimerW" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::HANDLE res = fptrCreateWaitableTimerW(lpTimerAttributes, bManualReset, lpTimerName);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving CreateWaitableTimerW" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::BOOL WINAPI myWaitMessage()

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering WaitMessage" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::BOOL res = fptrWaitMessage();

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving WaitMessage" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::DWORD WINAPI mySetCriticalSectionSpinCount(WIND::LPCRITICAL_SECTION lpCriticalSection, WIND::DWORD dwSpinCount)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering SetCriticalSectionSpinCount" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::DWORD res = fptrSetCriticalSectionSpinCount(lpCriticalSection, dwSpinCount);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving SetCriticalSectionSpinCount" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::BOOL WINAPI myWaitNamedPipeW(WIND::LPCWSTR lpNamedPipeName, WIND::DWORD nTimeOut)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering WaitNamedPipeW" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::BOOL res = fptrWaitNamedPipeW(lpNamedPipeName, nTimeOut);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving WaitNamedPipeW" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

int WSAAPI mysendto(WIND::SOCKET s, const char FAR* buf, int len, int flags, const struct sockaddr FAR* to, int tolen)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering sendto" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    int res = fptrsendto(s, buf, len, flags, to, tolen);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving sendto" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::BOOL WINAPI myWaitNamedPipeA(WIND::LPCSTR lpNamedPipeName, WIND::DWORD nTimeOut)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering WaitNamedPipeA" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::BOOL res = fptrWaitNamedPipeA(lpNamedPipeName, nTimeOut);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving WaitNamedPipeA" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::BOOL WINAPI myResetEvent(WIND::HANDLE hEvent)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering ResetEvent" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::BOOL res = fptrResetEvent(hEvent);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving ResetEvent" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::BOOL WINAPI myCreateTimerQueueTimer(WIND::PHANDLE phNewTimer, WIND::HANDLE TimerQueue, WIND::WAITORTIMERCALLBACK Callback,
                                          WIND::PVOID Parameter, WIND::DWORD DueTime, WIND::DWORD Period, WIND::ULONG Flags)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering CreateTimerQueueTimer" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::BOOL res = fptrCreateTimerQueueTimer(phNewTimer, TimerQueue, Callback, Parameter, DueTime, Period, Flags);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving CreateTimerQueueTimer" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::HANDLE WINAPI myCreateWaitableTimerA(WIND::LPSECURITY_ATTRIBUTES lpTimerAttributes, WIND::BOOL bManualReset,
                                           WIND::LPCSTR lpTimerName)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering CreateWaitableTimerA" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::HANDLE res = fptrCreateWaitableTimerA(lpTimerAttributes, bManualReset, lpTimerName);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving CreateWaitableTimerA" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::BOOL WINAPI myDuplicateHandle(WIND::HANDLE hSourceProcessHandle, WIND::HANDLE hSourceHandle,
                                    WIND::HANDLE hTargetProcessHandle, WIND::LPHANDLE lpTargetHandle, WIND::DWORD dwDesiredAccess,
                                    WIND::BOOL bInheritHandle, WIND::DWORD dwOptions)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering DuplicateHandle" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::BOOL res = fptrDuplicateHandle(hSourceProcessHandle, hSourceHandle, hTargetProcessHandle, lpTargetHandle,
                                         dwDesiredAccess, bInheritHandle, dwOptions);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving DuplicateHandle" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

int WSAAPI myclosesocket(WIND::SOCKET s)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering closesocket" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    int res = fptrclosesocket(s);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving closesocket" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::FARPROC WINAPI myGetProcAddress(WIND::HMODULE hModule, WIND::LPCSTR lpProcName)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering GetProcAddress" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::FARPROC res = fptrGetProcAddress(hModule, lpProcName);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving GetProcAddress" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::SOCKET WSAAPI mysocket(int af, int type, int protocol)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering socket" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::SOCKET res = fptrsocket(af, type, protocol);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving socket" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::BOOL WSAAPI myWSASetEvent(WIND::WSAEVENT hEvent)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering WSASetEvent" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::BOOL res = fptrWSASetEvent(hEvent);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving WSASetEvent" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::HANDLE WINAPI myCreateEventW(WIND::LPSECURITY_ATTRIBUTES lpEventAttributes, WIND::BOOL bManualReset,
                                   WIND::BOOL bInitialState, WIND::LPCWSTR lpName)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering CreateEventW" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::HANDLE res = fptrCreateEventW(lpEventAttributes, bManualReset, bInitialState, lpName);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving CreateEventW" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::BOOL WINAPI myPostThreadMessageW(WIND::DWORD idThread, WIND::UINT Msg, WIND::WPARAM wParam, WIND::LPARAM lParam)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering PostThreadMessageW" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::BOOL res = fptrPostThreadMessageW(idThread, Msg, wParam, lParam);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving PostThreadMessageW" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

int WSAAPI myWSASendTo(WIND::SOCKET s, WIND::LPWSABUF lpBuffers, WIND::DWORD dwBufferCount, WIND::LPDWORD lpNumberOfBytesSent,
                       WIND::DWORD dwFlags, const struct sockaddr FAR* lpTo, int iTolen, WIND::LPWSAOVERLAPPED lpOverlapped,
                       WIND::LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering WSASendTo" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    int res =
        fptrWSASendTo(s, lpBuffers, dwBufferCount, lpNumberOfBytesSent, dwFlags, lpTo, iTolen, lpOverlapped, lpCompletionRoutine);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving WSASendTo" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::HANDLE WINAPI myCreateFileW(WIND::LPCWSTR lpFileName, WIND::DWORD dwDesiredAccess, WIND::DWORD dwShareMode,
                                  WIND::LPSECURITY_ATTRIBUTES lpSecurityAttributes, WIND::DWORD dwCreationDisposition,
                                  WIND::DWORD dwFlagsAndAttributes, WIND::HANDLE hTemplateFile)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering CreateFileW" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::HANDLE res = fptrCreateFileW(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition,
                                       dwFlagsAndAttributes, hTemplateFile);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving CreateFileW" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::HANDLE WINAPI myCreateEventA(WIND::LPSECURITY_ATTRIBUTES lpEventAttributes, WIND::BOOL bManualReset,
                                   WIND::BOOL bInitialState, WIND::LPCSTR lpName)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering CreateEventA" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::HANDLE res = fptrCreateEventA(lpEventAttributes, bManualReset, bInitialState, lpName);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving CreateEventA" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::RPC_STATUS WINAPI myRpcServerListen(unsigned int MinimumCallThreads, unsigned int MaxCalls, unsigned int DontWait)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering RpcServerListen" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::RPC_STATUS res = fptrRpcServerListen(MinimumCallThreads, MaxCalls, DontWait);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving RpcServerListen" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::HANDLE WINAPI myCreateFileA(WIND::LPCSTR lpFileName, WIND::DWORD dwDesiredAccess, WIND::DWORD dwShareMode,
                                  WIND::LPSECURITY_ATTRIBUTES lpSecurityAttributes, WIND::DWORD dwCreationDisposition,
                                  WIND::DWORD dwFlagsAndAttributes, WIND::HANDLE hTemplateFile)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering CreateFileA" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::HANDLE res = fptrCreateFileA(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition,
                                       dwFlagsAndAttributes, hTemplateFile);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving CreateFileA" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::BOOL WINAPI myDeleteTimerQueueEx(WIND::HANDLE TimerQueue, WIND::HANDLE CompletionEvent)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering DeleteTimerQueueEx" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::BOOL res = fptrDeleteTimerQueueEx(TimerQueue, CompletionEvent);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving DeleteTimerQueueEx" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::LRESULT WINAPI mySendMessageTimeoutW(WIND::HWND hWnd, WIND::UINT Msg, WIND::WPARAM wParam, WIND::LPARAM lParam,
                                           WIND::UINT fuFlags, WIND::UINT uTimeout, WIND::PDWORD_PTR lpdwResult)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering SendMessageTimeoutW" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::LRESULT res = fptrSendMessageTimeoutW(hWnd, Msg, wParam, lParam, fuFlags, uTimeout, lpdwResult);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving SendMessageTimeoutW" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::BOOL WINAPI myPostThreadMessageA(WIND::DWORD idThread, WIND::UINT Msg, WIND::WPARAM wParam, WIND::LPARAM lParam)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering PostThreadMessageA" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::BOOL res = fptrPostThreadMessageA(idThread, Msg, wParam, lParam);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving PostThreadMessageA" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::SOCKET WSAAPI myWSASocketW(int af, int type, int protocol, WIND::LPWSAPROTOCOL_INFOW lpProtocolInfo, WIND::GROUP g,
                                 WIND::DWORD dwFlags)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering WSASocketW" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::SOCKET res = fptrWSASocketW(af, type, protocol, lpProtocolInfo, g, dwFlags);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving WSASocketW" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::BOOL WSAAPI myWSAGetOverlappedResult(WIND::SOCKET s, WIND::LPWSAOVERLAPPED lpOverlapped, WIND::LPDWORD lpcbTransfer,
                                           WIND::BOOL fWait, WIND::LPDWORD lpdwFlags)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering WSAGetOverlappedResult" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::BOOL res = fptrWSAGetOverlappedResult(s, lpOverlapped, lpcbTransfer, fWait, lpdwFlags);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving WSAGetOverlappedResult" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

int WSAAPI myWSAConnect(WIND::SOCKET s, const struct sockaddr FAR* name, int namelen, WIND::LPWSABUF lpCallerData,
                        WIND::LPWSABUF lpCalleeData, WIND::LPQOS lpSQOS, WIND::LPQOS lpGQOS)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering WSAConnect" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    int res = fptrWSAConnect(s, name, namelen, lpCallerData, lpCalleeData, lpSQOS, lpGQOS);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving WSAConnect" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::HANDLE WINAPI myFindFirstChangeNotificationA(WIND::LPCSTR lpPathName, WIND::BOOL bWatchSubtree, WIND::DWORD dwNotifyFilter)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering FindFirstChangeNotificationA" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::HANDLE res = fptrFindFirstChangeNotificationA(lpPathName, bWatchSubtree, dwNotifyFilter);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving FindFirstChangeNotificationA" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::HANDLE WINAPI myCreateNamedPipeW(WIND::LPCWSTR lpName, WIND::DWORD dwOpenMode, WIND::DWORD dwPipeMode,
                                       WIND::DWORD nMaxInstances, WIND::DWORD nOutBufferSize, WIND::DWORD nInBufferSize,
                                       WIND::DWORD nDefaultTimeOut, WIND::LPSECURITY_ATTRIBUTES lpSecurityAttributes)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering CreateNamedPipeW" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::HANDLE res = fptrCreateNamedPipeW(lpName, dwOpenMode, dwPipeMode, nMaxInstances, nOutBufferSize, nInBufferSize,
                                            nDefaultTimeOut, lpSecurityAttributes);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving CreateNamedPipeW" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::SOCKET WSAAPI myaccept(WIND::SOCKET s, struct sockaddr FAR* addr, int FAR* addrlen)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering accept" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::SOCKET res = fptraccept(s, addr, addrlen);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving accept" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::HANDLE WINAPI myOpenSemaphoreA(WIND::DWORD dwDesiredAccess, WIND::BOOL bInheritHandle, WIND::LPCSTR lpName)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering OpenSemaphoreA" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::HANDLE res = fptrOpenSemaphoreA(dwDesiredAccess, bInheritHandle, lpName);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving OpenSemaphoreA" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::BOOL WINAPI myUnregisterWaitEx(WIND::HANDLE WaitHandle, WIND::HANDLE CompletionEvent)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering UnregisterWaitEx" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::BOOL res = fptrUnregisterWaitEx(WaitHandle, CompletionEvent);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving UnregisterWaitEx" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::RPC_STATUS WINAPI myRpcMgmtIsServerListening(WIND::RPC_BINDING_HANDLE Binding)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering RpcMgmtIsServerListening" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::RPC_STATUS res = fptrRpcMgmtIsServerListening(Binding);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving RpcMgmtIsServerListening" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::DWORD WINAPI myWaitForSingleObjectEx(WIND::HANDLE hHandle, WIND::DWORD dwMilliseconds, WIND::BOOL bAlertable)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering WaitForSingleObjectEx" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::DWORD res = fptrWaitForSingleObjectEx(hHandle, dwMilliseconds, bAlertable);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving WaitForSingleObjectEx" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::HANDLE WINAPI myCreateNamedPipeA(WIND::LPCSTR lpName, WIND::DWORD dwOpenMode, WIND::DWORD dwPipeMode,
                                       WIND::DWORD nMaxInstances, WIND::DWORD nOutBufferSize, WIND::DWORD nInBufferSize,
                                       WIND::DWORD nDefaultTimeOut, WIND::LPSECURITY_ATTRIBUTES lpSecurityAttributes)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering CreateNamedPipeA" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::HANDLE res = fptrCreateNamedPipeA(lpName, dwOpenMode, dwPipeMode, nMaxInstances, nOutBufferSize, nInBufferSize,
                                            nDefaultTimeOut, lpSecurityAttributes);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving CreateNamedPipeA" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::HANDLE WINAPI myFindFirstChangeNotificationW(WIND::LPCWSTR lpPathName, WIND::BOOL bWatchSubtree, WIND::DWORD dwNotifyFilter)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering FindFirstChangeNotificationW" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::HANDLE res = fptrFindFirstChangeNotificationW(lpPathName, bWatchSubtree, dwNotifyFilter);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving FindFirstChangeNotificationW" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

int WINAPI myMessageBoxExA(WIND::HWND hWnd, WIND::LPCSTR lpText, WIND::LPCSTR lpCaption, WIND::UINT uType, WIND::WORD wLanguageId)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering MessageBoxExA" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    int res = fptrMessageBoxExA(hWnd, lpText, lpCaption, uType, wLanguageId);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving MessageBoxExA" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::BOOL WINAPI myGetQueuedCompletionStatus(WIND::HANDLE CompletionPort, WIND::LPDWORD lpNumberOfBytesTransferred,
                                              WIND::PULONG_PTR lpCompletionKey, WIND::LPOVERLAPPED lpOverlapped,
                                              WIND::DWORD dwMilliseconds)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering GetQueuedCompletionStatus" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::BOOL res =
        fptrGetQueuedCompletionStatus(CompletionPort, lpNumberOfBytesTransferred, lpCompletionKey, lpOverlapped, dwMilliseconds);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving GetQueuedCompletionStatus" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::BOOL WINAPI myCancelWaitableTimer(WIND::HANDLE hTimer)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering CancelWaitableTimer" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::BOOL res = fptrCancelWaitableTimer(hTimer);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving CancelWaitableTimer" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

int WINAPI myMessageBoxW(WIND::HWND hWnd, WIND::LPCWSTR lpText, WIND::LPCWSTR lpCaption, WIND::UINT uType)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering MessageBoxW" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    int res = fptrMessageBoxW(hWnd, lpText, lpCaption, uType);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving MessageBoxW" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::SOCKET WSAAPI myWSAAccept(WIND::SOCKET s, struct sockaddr FAR* addr, WIND::LPINT addrlen,
                                WIND::LPCONDITIONPROC lpfnCondition, WIND::DWORD_PTR dwCallbackData)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering WSAAccept" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::SOCKET res = fptrWSAAccept(s, addr, addrlen, lpfnCondition, dwCallbackData);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving WSAAccept" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::BOOL WINAPI myUnregisterWait(WIND::HANDLE WaitHandle)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering UnregisterWait" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::BOOL res = fptrUnregisterWait(WaitHandle);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving UnregisterWait" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::BOOL WSAAPI myWSACloseEvent(WIND::WSAEVENT hEvent)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering WSACloseEvent" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::BOOL res = fptrWSACloseEvent(hEvent);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving WSACloseEvent" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::HANDLE WINAPI myCreateIoCompletionPort(WIND::HANDLE FileHandle, WIND::HANDLE ExistingCompletionPort,
                                             WIND::ULONG_PTR CompletionKey, WIND::DWORD NumberOfConcurrentThreads)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering CreateIoCompletionPort" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::HANDLE res = fptrCreateIoCompletionPort(FileHandle, ExistingCompletionPort, CompletionKey, NumberOfConcurrentThreads);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving CreateIoCompletionPort" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::INT_PTR WINAPI myDialogBoxParamW(WIND::HINSTANCE hInstance, WIND::LPCWSTR lpTemplateName, WIND::HWND hWndParent,
                                       WIND::DLGPROC lpDialogFunc, WIND::LPARAM dwInitParam)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering DialogBoxParamW" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::INT_PTR res = fptrDialogBoxParamW(hInstance, lpTemplateName, hWndParent, lpDialogFunc, dwInitParam);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving DialogBoxParamW" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

int WINAPI myMessageBoxA(WIND::HWND hWnd, WIND::LPCSTR lpText, WIND::LPCSTR lpCaption, WIND::UINT uType)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering MessageBoxA" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    int res = fptrMessageBoxA(hWnd, lpText, lpCaption, uType);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving MessageBoxA" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

int WINAPI myMessageBoxExW(WIND::HWND hWnd, WIND::LPCWSTR lpText, WIND::LPCWSTR lpCaption, WIND::UINT uType,
                           WIND::WORD wLanguageId)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering MessageBoxExW" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    int res = fptrMessageBoxExW(hWnd, lpText, lpCaption, uType, wLanguageId);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving MessageBoxExW" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::INT_PTR WINAPI myDialogBoxParamA(WIND::HINSTANCE hInstance, WIND::LPCSTR lpTemplateName, WIND::HWND hWndParent,
                                       WIND::DLGPROC lpDialogFunc, WIND::LPARAM dwInitParam)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering DialogBoxParamA" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::INT_PTR res = fptrDialogBoxParamA(hInstance, lpTemplateName, hWndParent, lpDialogFunc, dwInitParam);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving DialogBoxParamA" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

VOID WINAPI mySleep(WIND::DWORD dwMilliseconds)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering Sleep" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    fptrSleep(dwMilliseconds);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving Sleep" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);
}

WIND::RPC_STATUS WINAPI myRpcNsBindingLookupDone(WIND::RPC_NS_HANDLE __RPC_FAR* LookupContext)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering RpcNsBindingLookupDone" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::RPC_STATUS res = fptrRpcNsBindingLookupDone(LookupContext);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving RpcNsBindingLookupDone" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::HANDLE WINAPI myOpenMutexA(WIND::DWORD dwDesiredAccess, WIND::BOOL bInheritHandle, WIND::LPCSTR lpName)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering OpenMutexA" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::HANDLE res = fptrOpenMutexA(dwDesiredAccess, bInheritHandle, lpName);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving OpenMutexA" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::DWORD WINAPI mySuspendThread(WIND::HANDLE hThread)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering SuspendThread" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::DWORD res = fptrSuspendThread(hThread);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving SuspendThread" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

VOID WINAPI myRaiseException(WIND::DWORD dwExceptionCode, WIND::DWORD dwExceptionFlags, WIND::DWORD nNumberOfArguments,
                             __in_ecount_opt(nNumberOfArguments) CONST WIND::ULONG_PTR* lpArguments)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering RaiseException" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    fptrRaiseException(dwExceptionCode, dwExceptionFlags, nNumberOfArguments, lpArguments);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving RaiseException" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);
}

WIND::RPC_STATUS WINAPI myRpcNsBindingImportBeginW(unsigned long EntryNameSyntax, WIND::RPC_WSTR EntryName,
                                                   WIND::RPC_IF_HANDLE IfSpec, WIND::UUID __RPC_FAR* ObjUuid,
                                                   WIND::RPC_NS_HANDLE __RPC_FAR* ImportContext)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering RpcNsBindingImportBeginW" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::RPC_STATUS res = fptrRpcNsBindingImportBeginW(EntryNameSyntax, EntryName, IfSpec, ObjUuid, ImportContext);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving RpcNsBindingImportBeginW" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::BOOL WINAPI myReleaseSemaphore(WIND::HANDLE hSemaphore, WIND::LONG lReleaseCount, WIND::LPLONG lpPreviousCount)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering ReleaseSemaphore" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::BOOL res = fptrReleaseSemaphore(hSemaphore, lReleaseCount, lpPreviousCount);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving ReleaseSemaphore" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::HFILE WINAPI myOpenFile(WIND::LPCSTR lpFileName, WIND::LPOFSTRUCT lpReOpenBuff, WIND::UINT uStyle)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering OpenFile" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::HFILE res = fptrOpenFile(lpFileName, lpReOpenBuff, uStyle);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving OpenFile" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::BOOL WINAPI myReadFile(WIND::HANDLE hFile,
                             __out_bcount_part(nNumberOfBytesToRead, *lpNumberOfBytesRead) WIND::LPVOID lpBuffer,
                             WIND::DWORD nNumberOfBytesToRead, WIND::LPDWORD lpNumberOfBytesRead, WIND::LPOVERLAPPED lpOverlapped)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering ReadFile" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::BOOL res = fptrReadFile(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, lpOverlapped);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving ReadFile" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::BOOL WINAPI myWriteFileEx(WIND::HANDLE hFile, __in_bcount(nNumberOfBytesToWrite) WIND::LPCVOID lpBuffer,
                                WIND::DWORD nNumberOfBytesToWrite, WIND::LPOVERLAPPED lpOverlapped,
                                WIND::LPOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering WriteFileEx" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::BOOL res = fptrWriteFileEx(hFile, lpBuffer, nNumberOfBytesToWrite, lpOverlapped, lpCompletionRoutine);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving WriteFileEx" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::RPC_STATUS WINAPI myRpcNsBindingImportBeginA(unsigned long EntryNameSyntax, WIND::RPC_CSTR EntryName,
                                                   WIND::RPC_IF_HANDLE IfSpec, WIND::UUID __RPC_FAR* ObjUuid,
                                                   WIND::RPC_NS_HANDLE __RPC_FAR* ImportContext)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering RpcNsBindingImportBeginA" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::RPC_STATUS res = fptrRpcNsBindingImportBeginA(EntryNameSyntax, EntryName, IfSpec, ObjUuid, ImportContext);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving RpcNsBindingImportBeginA" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::BOOL WINAPI myPulseEvent(WIND::HANDLE hEvent)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering PulseEvent" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::BOOL res = fptrPulseEvent(hEvent);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving PulseEvent" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::RPC_STATUS WINAPI myRpcCancelThread(void* Thread)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering RpcCancelThread" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::RPC_STATUS res = fptrRpcCancelThread(Thread);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving RpcCancelThread" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::BOOL WINAPI myCloseHandle(WIND::HANDLE hObject)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering CloseHandle" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::BOOL res = fptrCloseHandle(hObject);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving CloseHandle" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::HANDLE WINAPI myOpenMutexW(WIND::DWORD dwDesiredAccess, WIND::BOOL bInheritHandle, WIND::LPCWSTR lpName)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering OpenMutexW" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::HANDLE res = fptrOpenMutexW(dwDesiredAccess, bInheritHandle, lpName);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving OpenMutexW" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::BOOL WINAPI myPostMessageW(WIND::HWND hWnd, WIND::UINT Msg, WIND::WPARAM wParam, WIND::LPARAM lParam)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering PostMessageW" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::BOOL res = fptrPostMessageW(hWnd, Msg, wParam, lParam);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving PostMessageW" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::RPC_STATUS WINAPI myRpcEpResolveBinding(WIND::RPC_BINDING_HANDLE Binding, WIND::RPC_IF_HANDLE IfSpec)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering RpcEpResolveBinding" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::RPC_STATUS res = fptrRpcEpResolveBinding(Binding, IfSpec);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving RpcEpResolveBinding" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::BOOL WINAPI myBindIoCompletionCallback(WIND::HANDLE FileHandle, WIND::LPOVERLAPPED_COMPLETION_ROUTINE Function,
                                             WIND::ULONG Flags)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering BindIoCompletionCallback" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::BOOL res = fptrBindIoCompletionCallback(FileHandle, Function, Flags);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving BindIoCompletionCallback" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::BOOL WINAPI myPeekMessageW(WIND::LPMSG lpMsg, WIND::HWND hWnd, WIND::UINT wMsgFilterMin, WIND::UINT wMsgFilterMax,
                                 WIND::UINT wRemoveMsg)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering PeekMessageW" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::BOOL res = fptrPeekMessageW(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving PeekMessageW" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::LRESULT WINAPI mySendMessageTimeoutA(WIND::HWND hWnd, WIND::UINT Msg, WIND::WPARAM wParam, WIND::LPARAM lParam,
                                           WIND::UINT fuFlags, WIND::UINT uTimeout, WIND::PDWORD_PTR lpdwResult)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering SendMessageTimeoutA" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::LRESULT res = fptrSendMessageTimeoutA(hWnd, Msg, wParam, lParam, fuFlags, uTimeout, lpdwResult);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving SendMessageTimeoutA" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::HANDLE WINAPI myOpenEventA(WIND::DWORD dwDesiredAccess, WIND::BOOL bInheritHandle, WIND::LPCSTR lpName)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering OpenEventA" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::HANDLE res = fptrOpenEventA(dwDesiredAccess, bInheritHandle, lpName);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving OpenEventA" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::BOOL WINAPI myCreateProcessA(WIND::LPCSTR lpApplicationName, WIND::LPSTR lpCommandLine,
                                   WIND::LPSECURITY_ATTRIBUTES lpProcessAttributes,
                                   WIND::LPSECURITY_ATTRIBUTES lpThreadAttributes, WIND::BOOL bInheritHandles,
                                   WIND::DWORD dwCreationFlags, WIND::LPVOID lpEnvironment, WIND::LPCSTR lpCurrentDirectory,
                                   WIND::LPSTARTUPINFOA lpStartupInfo, WIND::LPPROCESS_INFORMATION lpProcessInformation)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering CreateProcessA" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::BOOL res =
        fptrCreateProcessA(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles,
                           dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving CreateProcessA" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::HANDLE WINAPI myOpenEventW(WIND::DWORD dwDesiredAccess, WIND::BOOL bInheritHandle, WIND::LPCWSTR lpName)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering OpenEventW" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::HANDLE res = fptrOpenEventW(dwDesiredAccess, bInheritHandle, lpName);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving OpenEventW" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::BOOL WINAPI myPostQueuedCompletionStatus(WIND::HANDLE CompletionPort, WIND::DWORD dwNumberOfBytesTransferred,
                                               WIND::ULONG_PTR dwCompletionKey, WIND::LPOVERLAPPED lpOverlapped)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering PostQueuedCompletionStatus" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::BOOL res = fptrPostQueuedCompletionStatus(CompletionPort, dwNumberOfBytesTransferred, dwCompletionKey, lpOverlapped);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving PostQueuedCompletionStatus" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::BOOL WINAPI myCreateProcessW(WIND::LPCWSTR lpApplicationName, WIND::LPWSTR lpCommandLine,
                                   WIND::LPSECURITY_ATTRIBUTES lpProcessAttributes,
                                   WIND::LPSECURITY_ATTRIBUTES lpThreadAttributes, WIND::BOOL bInheritHandles,
                                   WIND::DWORD dwCreationFlags, WIND::LPVOID lpEnvironment, WIND::LPCWSTR lpCurrentDirectory,
                                   WIND::LPSTARTUPINFOW lpStartupInfo, WIND::LPPROCESS_INFORMATION lpProcessInformation)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering CreateProcessW" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::BOOL res =
        fptrCreateProcessW(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles,
                           dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving CreateProcessW" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::BOOL WINAPI myTransactNamedPipe(WIND::HANDLE hNamedPipe, __in_bcount_opt(nInBufferSize) WIND::LPVOID lpInBuffer,
                                      WIND::DWORD nInBufferSize,
                                      __out_bcount_part_opt(nOutBufferSize, *lpBytesRead) WIND::LPVOID lpOutBuffer,
                                      WIND::DWORD nOutBufferSize, WIND::LPDWORD lpBytesRead, WIND::LPOVERLAPPED lpOverlapped)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering TransactNamedPipe" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::BOOL res =
        fptrTransactNamedPipe(hNamedPipe, lpInBuffer, nInBufferSize, lpOutBuffer, nOutBufferSize, lpBytesRead, lpOverlapped);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving TransactNamedPipe" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::BOOL WINAPI myReadFileEx(WIND::HANDLE hFile, __out_bcount(nNumberOfBytesToRead) WIND::LPVOID lpBuffer,
                               WIND::DWORD nNumberOfBytesToRead, WIND::LPOVERLAPPED lpOverlapped,
                               WIND::LPOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering ReadFileEx" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::BOOL res = fptrReadFileEx(hFile, lpBuffer, nNumberOfBytesToRead, lpOverlapped, lpCompletionRoutine);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving ReadFileEx" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::HANDLE WINAPI myOpenSemaphoreW(WIND::DWORD dwDesiredAccess, WIND::BOOL bInheritHandle, WIND::LPCWSTR lpName)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering OpenSemaphoreW" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::HANDLE res = fptrOpenSemaphoreW(dwDesiredAccess, bInheritHandle, lpName);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving OpenSemaphoreW" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

WIND::BOOL WINAPI myReplyMessage(WIND::LRESULT lResult)

{
    WIND::DWORD dwBeforeError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Entering ReplyMessage" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwBeforeError);

    WIND::BOOL res = fptrReplyMessage(lResult);

    WIND::DWORD dwAfterError = WIND::GetLastError();

    if (!(WIND::TlsGetValue(dwIndexTls)))

    {
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);
        if (KnobDoPrint)
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Leaving ReplyMessage" << endl;
            OutFile.flush();
        }
        WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);
    }

    WIND::SetLastError(dwAfterError);

    return res;
}

//functions section footer

void ImgLoad(IMG img, void* v)

{
    // Called every time a new image is loaded
    // Used to locate routines we want to probe

#if defined(TARGET_IA32E) && __ICL == 1010
    // Workaround for test failure under ICC 10.1 - need to backup rbx.
    // For some reason, ICC 10.1 does not do it (a bug?).
    ADDRINT rbx_save;
    __asm mov rbx_save, rbx
#endif

                            WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)1);

    if ((IMG_Name(img).find("comdlg32.dll") != string::npos) || (IMG_Name(img).find("COMDLG32.DLL") != string::npos) ||
        (IMG_Name(img).find("COMDLG32.dll") != string::npos))

    {
        RTN rtnPrintDlgExW = RTN_FindByName(img, "PrintDlgExW");
        if (RTN_Valid(rtnPrintDlgExW) && RTN_IsSafeForProbedReplacement(rtnPrintDlgExW))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for PrintDlgExW at " << RTN_Address(rtnPrintDlgExW) << endl;
            OutFile.flush();
            AFUNPTR fptr    = (RTN_ReplaceProbed(rtnPrintDlgExW, AFUNPTR(myPrintDlgExW)));
            fptrPrintDlgExW = (WIND::HRESULT(APIENTRY*)(WIND::LPPRINTDLGEXW))fptr;
        }
    }

    if ((IMG_Name(img).find("rpcns4.dll") != string::npos) || (IMG_Name(img).find("RPCNS4.DLL") != string::npos) ||
        (IMG_Name(img).find("RPCNS4.dll") != string::npos))

    {
        RTN rtnRpcNsBindingLookupBeginW = RTN_FindByName(img, "RpcNsBindingLookupBeginW");
        if (RTN_Valid(rtnRpcNsBindingLookupBeginW) && RTN_IsSafeForProbedReplacement(rtnRpcNsBindingLookupBeginW))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for RpcNsBindingLookupBeginW at " << RTN_Address(rtnRpcNsBindingLookupBeginW) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnRpcNsBindingLookupBeginW, AFUNPTR(myRpcNsBindingLookupBeginW)));
            fptrRpcNsBindingLookupBeginW =
                (WIND::RPC_STATUS(WINAPI*)(unsigned long, WIND::RPC_WSTR, WIND::RPC_IF_HANDLE, WIND::UUID __RPC_FAR*,
                                           unsigned long, WIND::RPC_NS_HANDLE __RPC_FAR*))fptr;
        }
    }

    if ((IMG_Name(img).find("kernel32.dll") != string::npos) || (IMG_Name(img).find("KERNEL32.DLL") != string::npos) ||
        (IMG_Name(img).find("KERNEL32.dll") != string::npos))

    {
        RTN rtnGetStdHandle = RTN_FindByName(img, "GetStdHandle");
        if (RTN_Valid(rtnGetStdHandle) && RTN_IsSafeForProbedReplacement(rtnGetStdHandle))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for GetStdHandle at " << RTN_Address(rtnGetStdHandle) << endl;
            OutFile.flush();
            AFUNPTR fptr     = (RTN_ReplaceProbed(rtnGetStdHandle, AFUNPTR(myGetStdHandle)));
            fptrGetStdHandle = (WIND::HANDLE(WINAPI*)(WIND::DWORD))fptr;
        }
    }

    if ((IMG_Name(img).find("kernel32.dll") != string::npos) || (IMG_Name(img).find("KERNEL32.DLL") != string::npos) ||
        (IMG_Name(img).find("KERNEL32.dll") != string::npos))

    {
        RTN rtnReleaseMutex = RTN_FindByName(img, "ReleaseMutex");
        if (RTN_Valid(rtnReleaseMutex) && RTN_IsSafeForProbedReplacement(rtnReleaseMutex))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for ReleaseMutex at " << RTN_Address(rtnReleaseMutex) << endl;
            OutFile.flush();
            AFUNPTR fptr     = (RTN_ReplaceProbed(rtnReleaseMutex, AFUNPTR(myReleaseMutex)));
            fptrReleaseMutex = (WIND::BOOL(WINAPI*)(WIND::HANDLE))fptr;
        }
    }

    if ((IMG_Name(img).find("kernel32.dll") != string::npos) || (IMG_Name(img).find("KERNEL32.DLL") != string::npos) ||
        (IMG_Name(img).find("KERNEL32.dll") != string::npos))

    {
        RTN rtnGetOverlappedResult = RTN_FindByName(img, "GetOverlappedResult");
        if (RTN_Valid(rtnGetOverlappedResult) && RTN_IsSafeForProbedReplacement(rtnGetOverlappedResult))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for GetOverlappedResult at " << RTN_Address(rtnGetOverlappedResult) << endl;
            OutFile.flush();
            AFUNPTR fptr            = (RTN_ReplaceProbed(rtnGetOverlappedResult, AFUNPTR(myGetOverlappedResult)));
            fptrGetOverlappedResult = (WIND::BOOL(WINAPI*)(WIND::HANDLE, WIND::LPOVERLAPPED, WIND::LPDWORD, WIND::BOOL))fptr;
        }
    }

    if ((IMG_Name(img).find("kernel32.dll") != string::npos) || (IMG_Name(img).find("KERNEL32.DLL") != string::npos) ||
        (IMG_Name(img).find("KERNEL32.dll") != string::npos))

    {
        RTN rtnWaitForSingleObject = RTN_FindByName(img, "WaitForSingleObject");
        if (RTN_Valid(rtnWaitForSingleObject) && RTN_IsSafeForProbedReplacement(rtnWaitForSingleObject))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for WaitForSingleObject at " << RTN_Address(rtnWaitForSingleObject) << endl;
            OutFile.flush();
            AFUNPTR fptr            = (RTN_ReplaceProbed(rtnWaitForSingleObject, AFUNPTR(myWaitForSingleObject)));
            fptrWaitForSingleObject = (WIND::DWORD(WINAPI*)(WIND::HANDLE, WIND::DWORD))fptr;
        }
    }

    if ((IMG_Name(img).find("user32.dll") != string::npos) || (IMG_Name(img).find("USER32.DLL") != string::npos) ||
        (IMG_Name(img).find("USER32.dll") != string::npos))

    {
        RTN rtnBroadcastSystemMessageA = RTN_FindByName(img, "BroadcastSystemMessageA");
        if (RTN_Valid(rtnBroadcastSystemMessageA) && RTN_IsSafeForProbedReplacement(rtnBroadcastSystemMessageA))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for BroadcastSystemMessageA at " << RTN_Address(rtnBroadcastSystemMessageA) << endl;
            OutFile.flush();
            AFUNPTR fptr                = (RTN_ReplaceProbed(rtnBroadcastSystemMessageA, AFUNPTR(myBroadcastSystemMessageA)));
            fptrBroadcastSystemMessageA = (long(WINAPI*)(WIND::DWORD, WIND::LPDWORD, WIND::UINT, WIND::WPARAM, WIND::LPARAM))fptr;
        }
    }

    if ((IMG_Name(img).find("kernel32.dll") != string::npos) || (IMG_Name(img).find("KERNEL32.DLL") != string::npos) ||
        (IMG_Name(img).find("KERNEL32.dll") != string::npos))

    {
        RTN rtnSignalObjectAndWait = RTN_FindByName(img, "SignalObjectAndWait");
        if (RTN_Valid(rtnSignalObjectAndWait) && RTN_IsSafeForProbedReplacement(rtnSignalObjectAndWait))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for SignalObjectAndWait at " << RTN_Address(rtnSignalObjectAndWait) << endl;
            OutFile.flush();
            AFUNPTR fptr            = (RTN_ReplaceProbed(rtnSignalObjectAndWait, AFUNPTR(mySignalObjectAndWait)));
            fptrSignalObjectAndWait = (WIND::DWORD(WINAPI*)(WIND::HANDLE, WIND::HANDLE, WIND::DWORD, WIND::BOOL))fptr;
        }
    }

    if ((IMG_Name(img).find("rpcns4.dll") != string::npos) || (IMG_Name(img).find("RPCNS4.DLL") != string::npos) ||
        (IMG_Name(img).find("RPCNS4.dll") != string::npos))

    {
        RTN rtnRpcNsBindingLookupBeginA = RTN_FindByName(img, "RpcNsBindingLookupBeginA");
        if (RTN_Valid(rtnRpcNsBindingLookupBeginA) && RTN_IsSafeForProbedReplacement(rtnRpcNsBindingLookupBeginA))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for RpcNsBindingLookupBeginA at " << RTN_Address(rtnRpcNsBindingLookupBeginA) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnRpcNsBindingLookupBeginA, AFUNPTR(myRpcNsBindingLookupBeginA)));
            fptrRpcNsBindingLookupBeginA =
                (WIND::RPC_STATUS(WINAPI*)(unsigned long, WIND::RPC_CSTR, WIND::RPC_IF_HANDLE, WIND::UUID __RPC_FAR*,
                                           unsigned long, WIND::RPC_NS_HANDLE __RPC_FAR*))fptr;
        }
    }

    if ((IMG_Name(img).find("comdlg32.dll") != string::npos) || (IMG_Name(img).find("COMDLG32.DLL") != string::npos) ||
        (IMG_Name(img).find("COMDLG32.dll") != string::npos))

    {
        RTN rtnPrintDlgExA = RTN_FindByName(img, "PrintDlgExA");
        if (RTN_Valid(rtnPrintDlgExA) && RTN_IsSafeForProbedReplacement(rtnPrintDlgExA))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for PrintDlgExA at " << RTN_Address(rtnPrintDlgExA) << endl;
            OutFile.flush();
            AFUNPTR fptr    = (RTN_ReplaceProbed(rtnPrintDlgExA, AFUNPTR(myPrintDlgExA)));
            fptrPrintDlgExA = (WIND::HRESULT(APIENTRY*)(WIND::LPPRINTDLGEXA))fptr;
        }
    }

    if ((IMG_Name(img).find("kernel32.dll") != string::npos) || (IMG_Name(img).find("KERNEL32.DLL") != string::npos) ||
        (IMG_Name(img).find("KERNEL32.dll") != string::npos))

    {
        RTN rtnCreateTimerQueue = RTN_FindByName(img, "CreateTimerQueue");
        if (RTN_Valid(rtnCreateTimerQueue) && RTN_IsSafeForProbedReplacement(rtnCreateTimerQueue))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for CreateTimerQueue at " << RTN_Address(rtnCreateTimerQueue) << endl;
            OutFile.flush();
            AFUNPTR fptr         = (RTN_ReplaceProbed(rtnCreateTimerQueue, AFUNPTR(myCreateTimerQueue)));
            fptrCreateTimerQueue = (WIND::HANDLE(WINAPI*)())fptr;
        }
    }

    if ((IMG_Name(img).find("rpcns4.dll") != string::npos) || (IMG_Name(img).find("RPCNS4.DLL") != string::npos) ||
        (IMG_Name(img).find("RPCNS4.dll") != string::npos))

    {
        RTN rtnRpcNsBindingImportNext = RTN_FindByName(img, "RpcNsBindingImportNext");
        if (RTN_Valid(rtnRpcNsBindingImportNext) && RTN_IsSafeForProbedReplacement(rtnRpcNsBindingImportNext))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for RpcNsBindingImportNext at " << RTN_Address(rtnRpcNsBindingImportNext) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnRpcNsBindingImportNext, AFUNPTR(myRpcNsBindingImportNext)));
            fptrRpcNsBindingImportNext =
                (WIND::RPC_STATUS(WINAPI*)(WIND::RPC_NS_HANDLE, WIND::RPC_BINDING_HANDLE __RPC_FAR*))fptr;
        }
    }

    if ((IMG_Name(img).find("kernel32.dll") != string::npos) || (IMG_Name(img).find("KERNEL32.DLL") != string::npos) ||
        (IMG_Name(img).find("KERNEL32.dll") != string::npos))

    {
        RTN rtnQueueUserAPC = RTN_FindByName(img, "QueueUserAPC");
        if (RTN_Valid(rtnQueueUserAPC) && RTN_IsSafeForProbedReplacement(rtnQueueUserAPC))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for QueueUserAPC at " << RTN_Address(rtnQueueUserAPC) << endl;
            OutFile.flush();
            AFUNPTR fptr     = (RTN_ReplaceProbed(rtnQueueUserAPC, AFUNPTR(myQueueUserAPC)));
            fptrQueueUserAPC = (WIND::DWORD(WINAPI*)(WIND::PAPCFUNC, WIND::HANDLE, WIND::ULONG_PTR))fptr;
        }
    }

    if ((IMG_Name(img).find("rpcns4.dll") != string::npos) || (IMG_Name(img).find("RPCNS4.DLL") != string::npos) ||
        (IMG_Name(img).find("RPCNS4.dll") != string::npos))

    {
        RTN rtnRpcNsBindingLookupNext = RTN_FindByName(img, "RpcNsBindingLookupNext");
        if (RTN_Valid(rtnRpcNsBindingLookupNext) && RTN_IsSafeForProbedReplacement(rtnRpcNsBindingLookupNext))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for RpcNsBindingLookupNext at " << RTN_Address(rtnRpcNsBindingLookupNext) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnRpcNsBindingLookupNext, AFUNPTR(myRpcNsBindingLookupNext)));
            fptrRpcNsBindingLookupNext =
                (WIND::RPC_STATUS(WINAPI*)(WIND::RPC_NS_HANDLE, WIND::RPC_BINDING_VECTOR __RPC_FAR * __RPC_FAR*)) fptr;
        }
    }

    if ((IMG_Name(img).find("ws2_32.dll") != string::npos) || (IMG_Name(img).find("WS2_32.DLL") != string::npos) ||
        (IMG_Name(img).find("WS2_32.dll") != string::npos))

    {
        RTN rtnsend = RTN_FindByName(img, "send");
        if (RTN_Valid(rtnsend) && RTN_IsSafeForProbedReplacement(rtnsend))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for send at " << RTN_Address(rtnsend) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnsend, AFUNPTR(mysend)));
            fptrsend     = (int(WSAAPI*)(WIND::SOCKET, const char FAR*, int, int))fptr;
        }
    }

    if ((IMG_Name(img).find("user32.dll") != string::npos) || (IMG_Name(img).find("USER32.DLL") != string::npos) ||
        (IMG_Name(img).find("USER32.dll") != string::npos))

    {
        RTN rtnPeekMessageA = RTN_FindByName(img, "PeekMessageA");
        if (RTN_Valid(rtnPeekMessageA) && RTN_IsSafeForProbedReplacement(rtnPeekMessageA))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for PeekMessageA at " << RTN_Address(rtnPeekMessageA) << endl;
            OutFile.flush();
            AFUNPTR fptr     = (RTN_ReplaceProbed(rtnPeekMessageA, AFUNPTR(myPeekMessageA)));
            fptrPeekMessageA = (WIND::BOOL(WINAPI*)(WIND::LPMSG, WIND::HWND, WIND::UINT, WIND::UINT, WIND::UINT))fptr;
        }
    }

    if ((IMG_Name(img).find("ntdll.dll") != string::npos) || (IMG_Name(img).find("NTDLL.DLL") != string::npos) ||
        (IMG_Name(img).find("NTDLL.dll") != string::npos))

    {
        RTN rtnRtlLeaveCriticalSection = RTN_FindByName(img, "RtlLeaveCriticalSection");
        if (RTN_Valid(rtnRtlLeaveCriticalSection) && RTN_IsSafeForProbedReplacement(rtnRtlLeaveCriticalSection))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for RtlLeaveCriticalSection at " << RTN_Address(rtnRtlLeaveCriticalSection) << endl;
            OutFile.flush();
            AFUNPTR fptr                = (RTN_ReplaceProbed(rtnRtlLeaveCriticalSection, AFUNPTR(myRtlLeaveCriticalSection)));
            fptrRtlLeaveCriticalSection = (VOID(WINAPI*)(WIND::LPCRITICAL_SECTION))fptr;
        }
    }

    if ((IMG_Name(img).find("kernel32.dll") != string::npos) || (IMG_Name(img).find("KERNEL32.DLL") != string::npos) ||
        (IMG_Name(img).find("KERNEL32.dll") != string::npos))

    {
        RTN rtnReadFileScatter = RTN_FindByName(img, "ReadFileScatter");
        if (RTN_Valid(rtnReadFileScatter) && RTN_IsSafeForProbedReplacement(rtnReadFileScatter))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for ReadFileScatter at " << RTN_Address(rtnReadFileScatter) << endl;
            OutFile.flush();
            AFUNPTR fptr        = (RTN_ReplaceProbed(rtnReadFileScatter, AFUNPTR(myReadFileScatter)));
            fptrReadFileScatter = (WIND::BOOL(WINAPI*)(WIND::HANDLE, WIND::FILE_SEGMENT_ELEMENT**, WIND::DWORD,
                                                       __reserved WIND::LPDWORD, WIND::LPOVERLAPPED))fptr;
        }
    }

    if ((IMG_Name(img).find("kernel32.dll") != string::npos) || (IMG_Name(img).find("KERNEL32.DLL") != string::npos) ||
        (IMG_Name(img).find("KERNEL32.dll") != string::npos))

    {
        RTN rtnWaitCommEvent = RTN_FindByName(img, "WaitCommEvent");
        if (RTN_Valid(rtnWaitCommEvent) && RTN_IsSafeForProbedReplacement(rtnWaitCommEvent))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for WaitCommEvent at " << RTN_Address(rtnWaitCommEvent) << endl;
            OutFile.flush();
            AFUNPTR fptr      = (RTN_ReplaceProbed(rtnWaitCommEvent, AFUNPTR(myWaitCommEvent)));
            fptrWaitCommEvent = (WIND::BOOL(WINAPI*)(WIND::HANDLE, WIND::LPDWORD, WIND::LPOVERLAPPED))fptr;
        }
    }

    if ((IMG_Name(img).find("user32.dll") != string::npos) || (IMG_Name(img).find("USER32.DLL") != string::npos) ||
        (IMG_Name(img).find("USER32.dll") != string::npos))

    {
        RTN rtnSendMessageW = RTN_FindByName(img, "SendMessageW");
        if (RTN_Valid(rtnSendMessageW) && RTN_IsSafeForProbedReplacement(rtnSendMessageW))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for SendMessageW at " << RTN_Address(rtnSendMessageW) << endl;
            OutFile.flush();
            AFUNPTR fptr     = (RTN_ReplaceProbed(rtnSendMessageW, AFUNPTR(mySendMessageW)));
            fptrSendMessageW = (WIND::LRESULT(WINAPI*)(WIND::HWND, WIND::UINT, WIND::WPARAM, WIND::LPARAM))fptr;
        }
    }

    if ((IMG_Name(img).find("ws2_32.dll") != string::npos) || (IMG_Name(img).find("WS2_32.DLL") != string::npos) ||
        (IMG_Name(img).find("WS2_32.dll") != string::npos))

    {
        RTN rtnWSARecv = RTN_FindByName(img, "WSARecv");
        if (RTN_Valid(rtnWSARecv) && RTN_IsSafeForProbedReplacement(rtnWSARecv))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for WSARecv at " << RTN_Address(rtnWSARecv) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnWSARecv, AFUNPTR(myWSARecv)));
            fptrWSARecv  = (int(WSAAPI*)(WIND::SOCKET, WIND::LPWSABUF, WIND::DWORD, WIND::LPDWORD, WIND::LPDWORD,
                                        WIND::LPWSAOVERLAPPED, WIND::LPWSAOVERLAPPED_COMPLETION_ROUTINE))fptr;
        }
    }

    if ((IMG_Name(img).find("user32.dll") != string::npos) || (IMG_Name(img).find("USER32.DLL") != string::npos) ||
        (IMG_Name(img).find("USER32.dll") != string::npos))

    {
        RTN rtnSendMessageA = RTN_FindByName(img, "SendMessageA");
        if (RTN_Valid(rtnSendMessageA) && RTN_IsSafeForProbedReplacement(rtnSendMessageA))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for SendMessageA at " << RTN_Address(rtnSendMessageA) << endl;
            OutFile.flush();
            AFUNPTR fptr     = (RTN_ReplaceProbed(rtnSendMessageA, AFUNPTR(mySendMessageA)));
            fptrSendMessageA = (WIND::LRESULT(WINAPI*)(WIND::HWND, WIND::UINT, WIND::WPARAM, WIND::LPARAM))fptr;
        }
    }

    if ((IMG_Name(img).find("kernel32.dll") != string::npos) || (IMG_Name(img).find("KERNEL32.DLL") != string::npos) ||
        (IMG_Name(img).find("KERNEL32.dll") != string::npos))

    {
        RTN rtnWriteFile = RTN_FindByName(img, "WriteFile");
        if (RTN_Valid(rtnWriteFile) && RTN_IsSafeForProbedReplacement(rtnWriteFile))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for WriteFile at " << RTN_Address(rtnWriteFile) << endl;
            OutFile.flush();
            AFUNPTR fptr  = (RTN_ReplaceProbed(rtnWriteFile, AFUNPTR(myWriteFile)));
            fptrWriteFile = (WIND::BOOL(WINAPI*)(WIND::HANDLE, __in_bcount(nNumberOfBytesToWrite) WIND::LPCVOID, WIND::DWORD,
                                                 WIND::LPDWORD, WIND::LPOVERLAPPED))fptr;
        }
    }

    if ((IMG_Name(img).find("kernel32.dll") != string::npos) || (IMG_Name(img).find("KERNEL32.DLL") != string::npos) ||
        (IMG_Name(img).find("KERNEL32.dll") != string::npos))

    {
        RTN rtnResumeThread = RTN_FindByName(img, "ResumeThread");
        if (RTN_Valid(rtnResumeThread) && RTN_IsSafeForProbedReplacement(rtnResumeThread))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for ResumeThread at " << RTN_Address(rtnResumeThread) << endl;
            OutFile.flush();
            AFUNPTR fptr     = (RTN_ReplaceProbed(rtnResumeThread, AFUNPTR(myResumeThread)));
            fptrResumeThread = (WIND::DWORD(WINAPI*)(WIND::HANDLE))fptr;
        }
    }

    if ((IMG_Name(img).find("rpcrt4.dll") != string::npos) || (IMG_Name(img).find("RPCRT4.DLL") != string::npos) ||
        (IMG_Name(img).find("RPCRT4.dll") != string::npos))

    {
        RTN rtnRpcMgmtEpEltInqNextA = RTN_FindByName(img, "RpcMgmtEpEltInqNextA");
        if (RTN_Valid(rtnRpcMgmtEpEltInqNextA) && RTN_IsSafeForProbedReplacement(rtnRpcMgmtEpEltInqNextA))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for RpcMgmtEpEltInqNextA at " << RTN_Address(rtnRpcMgmtEpEltInqNextA) << endl;
            OutFile.flush();
            AFUNPTR fptr             = (RTN_ReplaceProbed(rtnRpcMgmtEpEltInqNextA, AFUNPTR(myRpcMgmtEpEltInqNextA)));
            fptrRpcMgmtEpEltInqNextA = (WIND::RPC_STATUS(WINAPI*)(WIND::RPC_EP_INQ_HANDLE, WIND::RPC_IF_ID __RPC_FAR*,
                                                                  WIND::RPC_BINDING_HANDLE __RPC_FAR*, WIND::UUID __RPC_FAR*,
                                                                  __deref_opt_out WIND::RPC_CSTR __RPC_FAR*))fptr;
        }
    }

    if ((IMG_Name(img).find("user32.dll") != string::npos) || (IMG_Name(img).find("USER32.DLL") != string::npos) ||
        (IMG_Name(img).find("USER32.dll") != string::npos))

    {
        RTN rtnMsgWaitForMultipleObjectsEx = RTN_FindByName(img, "MsgWaitForMultipleObjectsEx");
        if (RTN_Valid(rtnMsgWaitForMultipleObjectsEx) && RTN_IsSafeForProbedReplacement(rtnMsgWaitForMultipleObjectsEx))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for MsgWaitForMultipleObjectsEx at " << RTN_Address(rtnMsgWaitForMultipleObjectsEx)
                    << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnMsgWaitForMultipleObjectsEx, AFUNPTR(myMsgWaitForMultipleObjectsEx)));
            fptrMsgWaitForMultipleObjectsEx = (WIND::DWORD(WINAPI*)(WIND::DWORD, __in_ecount_opt(nCount) CONST WIND::HANDLE*,
                                                                    WIND::DWORD, WIND::DWORD, WIND::DWORD))fptr;
        }
    }

    if ((IMG_Name(img).find("user32.dll") != string::npos) || (IMG_Name(img).find("USER32.DLL") != string::npos) ||
        (IMG_Name(img).find("USER32.dll") != string::npos))

    {
        RTN rtnBroadcastSystemMessageW = RTN_FindByName(img, "BroadcastSystemMessageW");
        if (RTN_Valid(rtnBroadcastSystemMessageW) && RTN_IsSafeForProbedReplacement(rtnBroadcastSystemMessageW))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for BroadcastSystemMessageW at " << RTN_Address(rtnBroadcastSystemMessageW) << endl;
            OutFile.flush();
            AFUNPTR fptr                = (RTN_ReplaceProbed(rtnBroadcastSystemMessageW, AFUNPTR(myBroadcastSystemMessageW)));
            fptrBroadcastSystemMessageW = (long(WINAPI*)(WIND::DWORD, WIND::LPDWORD, WIND::UINT, WIND::WPARAM, WIND::LPARAM))fptr;
        }
    }

    if ((IMG_Name(img).find("kernel32.dll") != string::npos) || (IMG_Name(img).find("KERNEL32.DLL") != string::npos) ||
        (IMG_Name(img).find("KERNEL32.dll") != string::npos))

    {
        RTN rtnConnectNamedPipe = RTN_FindByName(img, "ConnectNamedPipe");
        if (RTN_Valid(rtnConnectNamedPipe) && RTN_IsSafeForProbedReplacement(rtnConnectNamedPipe))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for ConnectNamedPipe at " << RTN_Address(rtnConnectNamedPipe) << endl;
            OutFile.flush();
            AFUNPTR fptr         = (RTN_ReplaceProbed(rtnConnectNamedPipe, AFUNPTR(myConnectNamedPipe)));
            fptrConnectNamedPipe = (WIND::BOOL(WINAPI*)(WIND::HANDLE, WIND::LPOVERLAPPED))fptr;
        }
    }

    if ((IMG_Name(img).find("kernel32.dll") != string::npos) || (IMG_Name(img).find("KERNEL32.DLL") != string::npos) ||
        (IMG_Name(img).find("KERNEL32.dll") != string::npos))

    {
        RTN rtnFreeLibraryAndExitThread = RTN_FindByName(img, "FreeLibraryAndExitThread");
        if (RTN_Valid(rtnFreeLibraryAndExitThread) && RTN_IsSafeForProbedReplacement(rtnFreeLibraryAndExitThread))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for FreeLibraryAndExitThread at " << RTN_Address(rtnFreeLibraryAndExitThread) << endl;
            OutFile.flush();
            AFUNPTR fptr                 = (RTN_ReplaceProbed(rtnFreeLibraryAndExitThread, AFUNPTR(myFreeLibraryAndExitThread)));
            fptrFreeLibraryAndExitThread = (VOID(WINAPI*)(WIND::HMODULE, WIND::DWORD))fptr;
        }
    }

    if ((IMG_Name(img).find("kernel32.dll") != string::npos) || (IMG_Name(img).find("KERNEL32.DLL") != string::npos) ||
        (IMG_Name(img).find("KERNEL32.dll") != string::npos))

    {
        RTN rtnInitializeCriticalSection = RTN_FindByName(img, "InitializeCriticalSection");
        if (RTN_Valid(rtnInitializeCriticalSection) && RTN_IsSafeForProbedReplacement(rtnInitializeCriticalSection))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for InitializeCriticalSection at " << RTN_Address(rtnInitializeCriticalSection) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnInitializeCriticalSection, AFUNPTR(myInitializeCriticalSection)));
            fptrInitializeCriticalSection = (VOID(WINAPI*)(WIND::LPCRITICAL_SECTION))fptr;
        }
    }

    if ((IMG_Name(img).find("ws2_32.dll") != string::npos) || (IMG_Name(img).find("WS2_32.DLL") != string::npos) ||
        (IMG_Name(img).find("WS2_32.dll") != string::npos))

    {
        RTN rtnrecvfrom = RTN_FindByName(img, "recvfrom");
        if (RTN_Valid(rtnrecvfrom) && RTN_IsSafeForProbedReplacement(rtnrecvfrom))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for recvfrom at " << RTN_Address(rtnrecvfrom) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnrecvfrom, AFUNPTR(myrecvfrom)));
            fptrrecvfrom = (int(WSAAPI*)(WIND::SOCKET, char FAR*, int, int, struct sockaddr FAR*, int FAR*))fptr;
        }
    }

    if ((IMG_Name(img).find("user32.dll") != string::npos) || (IMG_Name(img).find("USER32.DLL") != string::npos) ||
        (IMG_Name(img).find("USER32.dll") != string::npos))

    {
        RTN rtnDialogBoxIndirectParamW = RTN_FindByName(img, "DialogBoxIndirectParamW");
        if (RTN_Valid(rtnDialogBoxIndirectParamW) && RTN_IsSafeForProbedReplacement(rtnDialogBoxIndirectParamW))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for DialogBoxIndirectParamW at " << RTN_Address(rtnDialogBoxIndirectParamW) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnDialogBoxIndirectParamW, AFUNPTR(myDialogBoxIndirectParamW)));
            fptrDialogBoxIndirectParamW =
                (WIND::INT_PTR(WINAPI*)(WIND::HINSTANCE, WIND::LPCDLGTEMPLATEW, WIND::HWND, WIND::DLGPROC, WIND::LPARAM))fptr;
        }
    }

    if ((IMG_Name(img).find("kernel32.dll") != string::npos) || (IMG_Name(img).find("KERNEL32.DLL") != string::npos) ||
        (IMG_Name(img).find("KERNEL32.dll") != string::npos))

    {
        RTN rtnDeleteTimerQueue = RTN_FindByName(img, "DeleteTimerQueue");
        if (RTN_Valid(rtnDeleteTimerQueue) && RTN_IsSafeForProbedReplacement(rtnDeleteTimerQueue))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for DeleteTimerQueue at " << RTN_Address(rtnDeleteTimerQueue) << endl;
            OutFile.flush();
            AFUNPTR fptr         = (RTN_ReplaceProbed(rtnDeleteTimerQueue, AFUNPTR(myDeleteTimerQueue)));
            fptrDeleteTimerQueue = (WIND::BOOL(WINAPI*)(WIND::HANDLE))fptr;
        }
    }

    if ((IMG_Name(img).find("user32.dll") != string::npos) || (IMG_Name(img).find("USER32.DLL") != string::npos) ||
        (IMG_Name(img).find("USER32.dll") != string::npos))

    {
        RTN rtnMsgWaitForMultipleObjects = RTN_FindByName(img, "MsgWaitForMultipleObjects");
        if (RTN_Valid(rtnMsgWaitForMultipleObjects) && RTN_IsSafeForProbedReplacement(rtnMsgWaitForMultipleObjects))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for MsgWaitForMultipleObjects at " << RTN_Address(rtnMsgWaitForMultipleObjects) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnMsgWaitForMultipleObjects, AFUNPTR(myMsgWaitForMultipleObjects)));
            fptrMsgWaitForMultipleObjects = (WIND::DWORD(WINAPI*)(WIND::DWORD, __in_ecount_opt(nCount) CONST WIND::HANDLE*,
                                                                  WIND::BOOL, WIND::DWORD, WIND::DWORD))fptr;
        }
    }

    if ((IMG_Name(img).find("kernel32.dll") != string::npos) || (IMG_Name(img).find("KERNEL32.DLL") != string::npos) ||
        (IMG_Name(img).find("KERNEL32.dll") != string::npos))

    {
        RTN rtnQueueUserWorkItem = RTN_FindByName(img, "QueueUserWorkItem");
        if (RTN_Valid(rtnQueueUserWorkItem) && RTN_IsSafeForProbedReplacement(rtnQueueUserWorkItem))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for QueueUserWorkItem at " << RTN_Address(rtnQueueUserWorkItem) << endl;
            OutFile.flush();
            AFUNPTR fptr          = (RTN_ReplaceProbed(rtnQueueUserWorkItem, AFUNPTR(myQueueUserWorkItem)));
            fptrQueueUserWorkItem = (WIND::BOOL(WINAPI*)(WIND::LPTHREAD_START_ROUTINE, WIND::PVOID, WIND::ULONG))fptr;
        }
    }

    if ((IMG_Name(img).find("winspool.drv") != string::npos) || (IMG_Name(img).find("WINSPOOL.DRV") != string::npos) ||
        (IMG_Name(img).find("WINSPOOL.drv") != string::npos))

    {
        RTN rtnConnectToPrinterDlg = RTN_FindByName(img, "ConnectToPrinterDlg");
        if (RTN_Valid(rtnConnectToPrinterDlg) && RTN_IsSafeForProbedReplacement(rtnConnectToPrinterDlg))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for ConnectToPrinterDlg at " << RTN_Address(rtnConnectToPrinterDlg) << endl;
            OutFile.flush();
            AFUNPTR fptr            = (RTN_ReplaceProbed(rtnConnectToPrinterDlg, AFUNPTR(myConnectToPrinterDlg)));
            fptrConnectToPrinterDlg = (WIND::HANDLE(WINAPI*)(WIND::HWND, WIND::DWORD))fptr;
        }
    }

    if ((IMG_Name(img).find("kernel32.dll") != string::npos) || (IMG_Name(img).find("KERNEL32.DLL") != string::npos) ||
        (IMG_Name(img).find("KERNEL32.dll") != string::npos))

    {
        RTN rtnDeviceIoControl = RTN_FindByName(img, "DeviceIoControl");
        if (RTN_Valid(rtnDeviceIoControl) && RTN_IsSafeForProbedReplacement(rtnDeviceIoControl))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for DeviceIoControl at " << RTN_Address(rtnDeviceIoControl) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnDeviceIoControl, AFUNPTR(myDeviceIoControl)));
            fptrDeviceIoControl =
                (WIND::BOOL(WINAPI*)(WIND::HANDLE, WIND::DWORD, __in_bcount_opt(nInBufferSize) WIND::LPVOID, WIND::DWORD,
                                     __out_bcount_part_opt(nOutBufferSize, *lpBytesReturned) WIND::LPVOID, WIND::DWORD,
                                     WIND::LPDWORD, WIND::LPOVERLAPPED))fptr;
        }
    }

    if ((IMG_Name(img).find("ws2_32.dll") != string::npos) || (IMG_Name(img).find("WS2_32.DLL") != string::npos) ||
        (IMG_Name(img).find("WS2_32.dll") != string::npos))

    {
        RTN rtnWSASocketA = RTN_FindByName(img, "WSASocketA");
        if (RTN_Valid(rtnWSASocketA) && RTN_IsSafeForProbedReplacement(rtnWSASocketA))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for WSASocketA at " << RTN_Address(rtnWSASocketA) << endl;
            OutFile.flush();
            AFUNPTR fptr   = (RTN_ReplaceProbed(rtnWSASocketA, AFUNPTR(myWSASocketA)));
            fptrWSASocketA = (WIND::SOCKET(WSAAPI*)(int, int, int, WIND::LPWSAPROTOCOL_INFOA, WIND::GROUP, WIND::DWORD))fptr;
        }
    }

    if ((IMG_Name(img).find("ws2_32.dll") != string::npos) || (IMG_Name(img).find("WS2_32.DLL") != string::npos) ||
        (IMG_Name(img).find("WS2_32.dll") != string::npos))

    {
        RTN rtnWSARecvFrom = RTN_FindByName(img, "WSARecvFrom");
        if (RTN_Valid(rtnWSARecvFrom) && RTN_IsSafeForProbedReplacement(rtnWSARecvFrom))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for WSARecvFrom at " << RTN_Address(rtnWSARecvFrom) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnWSARecvFrom, AFUNPTR(myWSARecvFrom)));
            fptrWSARecvFrom =
                (int(WSAAPI*)(WIND::SOCKET, WIND::LPWSABUF, WIND::DWORD, WIND::LPDWORD, WIND::LPDWORD, struct sockaddr FAR*,
                              WIND::LPINT, WIND::LPWSAOVERLAPPED, WIND::LPWSAOVERLAPPED_COMPLETION_ROUTINE))fptr;
        }
    }

    if ((IMG_Name(img).find("rpcrt4.dll") != string::npos) || (IMG_Name(img).find("RPCRT4.DLL") != string::npos) ||
        (IMG_Name(img).find("RPCRT4.dll") != string::npos))

    {
        RTN rtnRpcStringBindingComposeA = RTN_FindByName(img, "RpcStringBindingComposeA");
        if (RTN_Valid(rtnRpcStringBindingComposeA) && RTN_IsSafeForProbedReplacement(rtnRpcStringBindingComposeA))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for RpcStringBindingComposeA at " << RTN_Address(rtnRpcStringBindingComposeA) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnRpcStringBindingComposeA, AFUNPTR(myRpcStringBindingComposeA)));
            fptrRpcStringBindingComposeA =
                (WIND::RPC_STATUS(WINAPI*)(WIND::RPC_CSTR, WIND::RPC_CSTR, WIND::RPC_CSTR, WIND::RPC_CSTR, WIND::RPC_CSTR,
                                           __deref_opt_out WIND::RPC_CSTR __RPC_FAR*))fptr;
        }
    }

    if ((IMG_Name(img).find("ws2_32.dll") != string::npos) || (IMG_Name(img).find("WS2_32.DLL") != string::npos) ||
        (IMG_Name(img).find("WS2_32.dll") != string::npos))

    {
        RTN rtnWSACreateEvent = RTN_FindByName(img, "WSACreateEvent");
        if (RTN_Valid(rtnWSACreateEvent) && RTN_IsSafeForProbedReplacement(rtnWSACreateEvent))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for WSACreateEvent at " << RTN_Address(rtnWSACreateEvent) << endl;
            OutFile.flush();
            AFUNPTR fptr       = (RTN_ReplaceProbed(rtnWSACreateEvent, AFUNPTR(myWSACreateEvent)));
            fptrWSACreateEvent = (WIND::WSAEVENT(WSAAPI*)())fptr;
        }
    }

    if ((IMG_Name(img).find("kernel32.dll") != string::npos) || (IMG_Name(img).find("KERNEL32.DLL") != string::npos) ||
        (IMG_Name(img).find("KERNEL32.dll") != string::npos))

    {
        RTN rtnExitProcess = RTN_FindByName(img, "ExitProcess");
        if (RTN_Valid(rtnExitProcess) && RTN_IsSafeForProbedReplacement(rtnExitProcess))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for ExitProcess at " << RTN_Address(rtnExitProcess) << endl;
            OutFile.flush();
            AFUNPTR fptr    = (RTN_ReplaceProbed(rtnExitProcess, AFUNPTR(myExitProcess)));
            fptrExitProcess = (VOID(WINAPI*)(WIND::UINT))fptr;
        }
    }

    if ((IMG_Name(img).find("ws2_32.dll") != string::npos) || (IMG_Name(img).find("WS2_32.DLL") != string::npos) ||
        (IMG_Name(img).find("WS2_32.dll") != string::npos))

    {
        RTN rtnconnect = RTN_FindByName(img, "connect");
        if (RTN_Valid(rtnconnect) && RTN_IsSafeForProbedReplacement(rtnconnect))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for connect at " << RTN_Address(rtnconnect) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnconnect, AFUNPTR(myconnect)));
            fptrconnect  = (int(WSAAPI*)(WIND::SOCKET, const struct sockaddr FAR*, int))fptr;
        }
    }

    if ((IMG_Name(img).find("user32.dll") != string::npos) || (IMG_Name(img).find("USER32.DLL") != string::npos) ||
        (IMG_Name(img).find("USER32.dll") != string::npos))

    {
        RTN rtnGetMessageW = RTN_FindByName(img, "GetMessageW");
        if (RTN_Valid(rtnGetMessageW) && RTN_IsSafeForProbedReplacement(rtnGetMessageW))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for GetMessageW at " << RTN_Address(rtnGetMessageW) << endl;
            OutFile.flush();
            AFUNPTR fptr    = (RTN_ReplaceProbed(rtnGetMessageW, AFUNPTR(myGetMessageW)));
            fptrGetMessageW = (WIND::BOOL(WINAPI*)(WIND::LPMSG, WIND::HWND, WIND::UINT, WIND::UINT))fptr;
        }
    }

    if ((IMG_Name(img).find("ws2_32.dll") != string::npos) || (IMG_Name(img).find("WS2_32.DLL") != string::npos) ||
        (IMG_Name(img).find("WS2_32.dll") != string::npos))

    {
        RTN rtnWSAResetEvent = RTN_FindByName(img, "WSAResetEvent");
        if (RTN_Valid(rtnWSAResetEvent) && RTN_IsSafeForProbedReplacement(rtnWSAResetEvent))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for WSAResetEvent at " << RTN_Address(rtnWSAResetEvent) << endl;
            OutFile.flush();
            AFUNPTR fptr      = (RTN_ReplaceProbed(rtnWSAResetEvent, AFUNPTR(myWSAResetEvent)));
            fptrWSAResetEvent = (WIND::BOOL(WSAAPI*)(WIND::WSAEVENT))fptr;
        }
    }

    if ((IMG_Name(img).find("rpcrt4.dll") != string::npos) || (IMG_Name(img).find("RPCRT4.DLL") != string::npos) ||
        (IMG_Name(img).find("RPCRT4.dll") != string::npos))

    {
        RTN rtnRpcStringBindingComposeW = RTN_FindByName(img, "RpcStringBindingComposeW");
        if (RTN_Valid(rtnRpcStringBindingComposeW) && RTN_IsSafeForProbedReplacement(rtnRpcStringBindingComposeW))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for RpcStringBindingComposeW at " << RTN_Address(rtnRpcStringBindingComposeW) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnRpcStringBindingComposeW, AFUNPTR(myRpcStringBindingComposeW)));
            fptrRpcStringBindingComposeW =
                (WIND::RPC_STATUS(WINAPI*)(WIND::RPC_WSTR, WIND::RPC_WSTR, WIND::RPC_WSTR, WIND::RPC_WSTR, WIND::RPC_WSTR,
                                           __deref_opt_out WIND::RPC_WSTR __RPC_FAR*))fptr;
        }
    }

    if ((IMG_Name(img).find("comdlg32.dll") != string::npos) || (IMG_Name(img).find("COMDLG32.DLL") != string::npos) ||
        (IMG_Name(img).find("COMDLG32.dll") != string::npos))

    {
        RTN rtnPrintDlgA = RTN_FindByName(img, "PrintDlgA");
        if (RTN_Valid(rtnPrintDlgA) && RTN_IsSafeForProbedReplacement(rtnPrintDlgA))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for PrintDlgA at " << RTN_Address(rtnPrintDlgA) << endl;
            OutFile.flush();
            AFUNPTR fptr  = (RTN_ReplaceProbed(rtnPrintDlgA, AFUNPTR(myPrintDlgA)));
            fptrPrintDlgA = (WIND::BOOL(APIENTRY*)(WIND::LPPRINTDLGA))fptr;
        }
    }

    if ((IMG_Name(img).find("kernel32.dll") != string::npos) || (IMG_Name(img).find("KERNEL32.DLL") != string::npos) ||
        (IMG_Name(img).find("KERNEL32.dll") != string::npos))

    {
        RTN rtnOpenWaitableTimerA = RTN_FindByName(img, "OpenWaitableTimerA");
        if (RTN_Valid(rtnOpenWaitableTimerA) && RTN_IsSafeForProbedReplacement(rtnOpenWaitableTimerA))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for OpenWaitableTimerA at " << RTN_Address(rtnOpenWaitableTimerA) << endl;
            OutFile.flush();
            AFUNPTR fptr           = (RTN_ReplaceProbed(rtnOpenWaitableTimerA, AFUNPTR(myOpenWaitableTimerA)));
            fptrOpenWaitableTimerA = (WIND::HANDLE(WINAPI*)(WIND::DWORD, WIND::BOOL, WIND::LPCSTR))fptr;
        }
    }

    if ((IMG_Name(img).find("user32.dll") != string::npos) || (IMG_Name(img).find("USER32.DLL") != string::npos) ||
        (IMG_Name(img).find("USER32.dll") != string::npos))

    {
        RTN rtnSetWindowsHookExW = RTN_FindByName(img, "SetWindowsHookExW");
        if (RTN_Valid(rtnSetWindowsHookExW) && RTN_IsSafeForProbedReplacement(rtnSetWindowsHookExW))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for SetWindowsHookExW at " << RTN_Address(rtnSetWindowsHookExW) << endl;
            OutFile.flush();
            AFUNPTR fptr          = (RTN_ReplaceProbed(rtnSetWindowsHookExW, AFUNPTR(mySetWindowsHookExW)));
            fptrSetWindowsHookExW = (WIND::HHOOK(WINAPI*)(int, WIND::HOOKPROC, WIND::HINSTANCE, WIND::DWORD))fptr;
        }
    }

    if ((IMG_Name(img).find("ws2_32.dll") != string::npos) || (IMG_Name(img).find("WS2_32.DLL") != string::npos) ||
        (IMG_Name(img).find("WS2_32.dll") != string::npos))

    {
        RTN rtnWSASend = RTN_FindByName(img, "WSASend");
        if (RTN_Valid(rtnWSASend) && RTN_IsSafeForProbedReplacement(rtnWSASend))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for WSASend at " << RTN_Address(rtnWSASend) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnWSASend, AFUNPTR(myWSASend)));
            fptrWSASend  = (int(WSAAPI*)(WIND::SOCKET, WIND::LPWSABUF, WIND::DWORD, WIND::LPDWORD, WIND::DWORD,
                                        WIND::LPWSAOVERLAPPED, WIND::LPWSAOVERLAPPED_COMPLETION_ROUTINE))fptr;
        }
    }

    if ((IMG_Name(img).find("kernel32.dll") != string::npos) || (IMG_Name(img).find("KERNEL32.DLL") != string::npos) ||
        (IMG_Name(img).find("KERNEL32.dll") != string::npos))

    {
        RTN rtnOpenWaitableTimerW = RTN_FindByName(img, "OpenWaitableTimerW");
        if (RTN_Valid(rtnOpenWaitableTimerW) && RTN_IsSafeForProbedReplacement(rtnOpenWaitableTimerW))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for OpenWaitableTimerW at " << RTN_Address(rtnOpenWaitableTimerW) << endl;
            OutFile.flush();
            AFUNPTR fptr           = (RTN_ReplaceProbed(rtnOpenWaitableTimerW, AFUNPTR(myOpenWaitableTimerW)));
            fptrOpenWaitableTimerW = (WIND::HANDLE(WINAPI*)(WIND::DWORD, WIND::BOOL, WIND::LPCWSTR))fptr;
        }
    }

    if ((IMG_Name(img).find("kernel32.dll") != string::npos) || (IMG_Name(img).find("KERNEL32.DLL") != string::npos) ||
        (IMG_Name(img).find("KERNEL32.dll") != string::npos))

    {
        RTN rtnDeleteTimerQueueTimer = RTN_FindByName(img, "DeleteTimerQueueTimer");
        if (RTN_Valid(rtnDeleteTimerQueueTimer) && RTN_IsSafeForProbedReplacement(rtnDeleteTimerQueueTimer))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for DeleteTimerQueueTimer at " << RTN_Address(rtnDeleteTimerQueueTimer) << endl;
            OutFile.flush();
            AFUNPTR fptr              = (RTN_ReplaceProbed(rtnDeleteTimerQueueTimer, AFUNPTR(myDeleteTimerQueueTimer)));
            fptrDeleteTimerQueueTimer = (WIND::BOOL(WINAPI*)(WIND::HANDLE, WIND::HANDLE, WIND::HANDLE))fptr;
        }
    }

    if ((IMG_Name(img).find("rpcns4.dll") != string::npos) || (IMG_Name(img).find("RPCNS4.DLL") != string::npos) ||
        (IMG_Name(img).find("RPCNS4.dll") != string::npos))

    {
        RTN rtnRpcNsBindingImportDone = RTN_FindByName(img, "RpcNsBindingImportDone");
        if (RTN_Valid(rtnRpcNsBindingImportDone) && RTN_IsSafeForProbedReplacement(rtnRpcNsBindingImportDone))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for RpcNsBindingImportDone at " << RTN_Address(rtnRpcNsBindingImportDone) << endl;
            OutFile.flush();
            AFUNPTR fptr               = (RTN_ReplaceProbed(rtnRpcNsBindingImportDone, AFUNPTR(myRpcNsBindingImportDone)));
            fptrRpcNsBindingImportDone = (WIND::RPC_STATUS(WINAPI*)(WIND::RPC_NS_HANDLE __RPC_FAR*))fptr;
        }
    }

    if ((IMG_Name(img).find("kernel32.dll") != string::npos) || (IMG_Name(img).find("KERNEL32.DLL") != string::npos) ||
        (IMG_Name(img).find("KERNEL32.dll") != string::npos))

    {
        RTN rtnReadConsoleInputW = RTN_FindByName(img, "ReadConsoleInputW");
        if (RTN_Valid(rtnReadConsoleInputW) && RTN_IsSafeForProbedReplacement(rtnReadConsoleInputW))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for ReadConsoleInputW at " << RTN_Address(rtnReadConsoleInputW) << endl;
            OutFile.flush();
            AFUNPTR fptr          = (RTN_ReplaceProbed(rtnReadConsoleInputW, AFUNPTR(myReadConsoleInputW)));
            fptrReadConsoleInputW = (WIND::BOOL(WINAPI*)(WIND::HANDLE, WIND::PINPUT_RECORD, WIND::DWORD, WIND::LPDWORD))fptr;
        }
    }

    if ((IMG_Name(img).find("kernel32.dll") != string::npos) || (IMG_Name(img).find("KERNEL32.DLL") != string::npos) ||
        (IMG_Name(img).find("KERNEL32.dll") != string::npos))

    {
        RTN rtnCreateMutexA = RTN_FindByName(img, "CreateMutexA");
        if (RTN_Valid(rtnCreateMutexA) && RTN_IsSafeForProbedReplacement(rtnCreateMutexA))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for CreateMutexA at " << RTN_Address(rtnCreateMutexA) << endl;
            OutFile.flush();
            AFUNPTR fptr     = (RTN_ReplaceProbed(rtnCreateMutexA, AFUNPTR(myCreateMutexA)));
            fptrCreateMutexA = (WIND::HANDLE(WINAPI*)(WIND::LPSECURITY_ATTRIBUTES, WIND::BOOL, WIND::LPCSTR))fptr;
        }
    }

    if ((IMG_Name(img).find("kernel32.dll") != string::npos) || (IMG_Name(img).find("KERNEL32.DLL") != string::npos) ||
        (IMG_Name(img).find("KERNEL32.dll") != string::npos))

    {
        RTN rtnRegisterWaitForSingleObject = RTN_FindByName(img, "RegisterWaitForSingleObject");
        if (RTN_Valid(rtnRegisterWaitForSingleObject) && RTN_IsSafeForProbedReplacement(rtnRegisterWaitForSingleObject))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for RegisterWaitForSingleObject at " << RTN_Address(rtnRegisterWaitForSingleObject)
                    << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnRegisterWaitForSingleObject, AFUNPTR(myRegisterWaitForSingleObject)));
            fptrRegisterWaitForSingleObject = (WIND::BOOL(WINAPI*)(WIND::PHANDLE, WIND::HANDLE, WIND::WAITORTIMERCALLBACK,
                                                                   WIND::PVOID, WIND::ULONG, WIND::ULONG))fptr;
        }
    }

    if ((IMG_Name(img).find("rpcrt4.dll") != string::npos) || (IMG_Name(img).find("RPCRT4.DLL") != string::npos) ||
        (IMG_Name(img).find("RPCRT4.dll") != string::npos))

    {
        RTN rtnRpcMgmtInqIfIds = RTN_FindByName(img, "RpcMgmtInqIfIds");
        if (RTN_Valid(rtnRpcMgmtInqIfIds) && RTN_IsSafeForProbedReplacement(rtnRpcMgmtInqIfIds))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for RpcMgmtInqIfIds at " << RTN_Address(rtnRpcMgmtInqIfIds) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnRpcMgmtInqIfIds, AFUNPTR(myRpcMgmtInqIfIds)));
            fptrRpcMgmtInqIfIds =
                (WIND::RPC_STATUS(WINAPI*)(WIND::RPC_BINDING_HANDLE, WIND::RPC_IF_ID_VECTOR __RPC_FAR * __RPC_FAR*)) fptr;
        }
    }

    if ((IMG_Name(img).find("kernel32.dll") != string::npos) || (IMG_Name(img).find("KERNEL32.DLL") != string::npos) ||
        (IMG_Name(img).find("KERNEL32.dll") != string::npos))

    {
        RTN rtnCreateSemaphoreA = RTN_FindByName(img, "CreateSemaphoreA");
        if (RTN_Valid(rtnCreateSemaphoreA) && RTN_IsSafeForProbedReplacement(rtnCreateSemaphoreA))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for CreateSemaphoreA at " << RTN_Address(rtnCreateSemaphoreA) << endl;
            OutFile.flush();
            AFUNPTR fptr         = (RTN_ReplaceProbed(rtnCreateSemaphoreA, AFUNPTR(myCreateSemaphoreA)));
            fptrCreateSemaphoreA = (WIND::HANDLE(WINAPI*)(WIND::LPSECURITY_ATTRIBUTES, WIND::LONG, WIND::LONG, WIND::LPCSTR))fptr;
        }
    }

    if ((IMG_Name(img).find("kernel32.dll") != string::npos) || (IMG_Name(img).find("KERNEL32.DLL") != string::npos) ||
        (IMG_Name(img).find("KERNEL32.dll") != string::npos))

    {
        RTN rtnCreateThread = RTN_FindByName(img, "CreateThread");
        if (RTN_Valid(rtnCreateThread) && RTN_IsSafeForProbedReplacement(rtnCreateThread))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for CreateThread at " << RTN_Address(rtnCreateThread) << endl;
            OutFile.flush();
            AFUNPTR fptr     = (RTN_ReplaceProbed(rtnCreateThread, AFUNPTR(myCreateThread)));
            fptrCreateThread = (WIND::HANDLE(WINAPI*)(WIND::LPSECURITY_ATTRIBUTES, WIND::SIZE_T, WIND::LPTHREAD_START_ROUTINE,
                                                      WIND::LPVOID, WIND::DWORD, WIND::LPDWORD))fptr;
        }
    }

    if ((IMG_Name(img).find("user32.dll") != string::npos) || (IMG_Name(img).find("USER32.DLL") != string::npos) ||
        (IMG_Name(img).find("USER32.dll") != string::npos))

    {
        RTN rtnBroadcastSystemMessageExW = RTN_FindByName(img, "BroadcastSystemMessageExW");
        if (RTN_Valid(rtnBroadcastSystemMessageExW) && RTN_IsSafeForProbedReplacement(rtnBroadcastSystemMessageExW))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for BroadcastSystemMessageExW at " << RTN_Address(rtnBroadcastSystemMessageExW) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnBroadcastSystemMessageExW, AFUNPTR(myBroadcastSystemMessageExW)));
            fptrBroadcastSystemMessageExW =
                (long(WINAPI*)(WIND::DWORD, WIND::LPDWORD, WIND::UINT, WIND::WPARAM, WIND::LPARAM, WIND::PBSMINFO))fptr;
        }
    }

    if ((IMG_Name(img).find("advapi32.dll") != string::npos) || (IMG_Name(img).find("ADVAPI32.DLL") != string::npos) ||
        (IMG_Name(img).find("ADVAPI32.dll") != string::npos))

    {
        RTN rtnCreateProcessAsUserA = RTN_FindByName(img, "CreateProcessAsUserA");
        if (RTN_Valid(rtnCreateProcessAsUserA) && RTN_IsSafeForProbedReplacement(rtnCreateProcessAsUserA))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for CreateProcessAsUserA at " << RTN_Address(rtnCreateProcessAsUserA) << endl;
            OutFile.flush();
            AFUNPTR fptr             = (RTN_ReplaceProbed(rtnCreateProcessAsUserA, AFUNPTR(myCreateProcessAsUserA)));
            fptrCreateProcessAsUserA = (WIND::BOOL(WINAPI*)(WIND::HANDLE, WIND::LPCSTR, WIND::LPSTR, WIND::LPSECURITY_ATTRIBUTES,
                                                            WIND::LPSECURITY_ATTRIBUTES, WIND::BOOL, WIND::DWORD, WIND::LPVOID,
                                                            WIND::LPCSTR, WIND::LPSTARTUPINFOA, WIND::LPPROCESS_INFORMATION))fptr;
        }
    }

    if ((IMG_Name(img).find("kernel32.dll") != string::npos) || (IMG_Name(img).find("KERNEL32.DLL") != string::npos) ||
        (IMG_Name(img).find("KERNEL32.dll") != string::npos))

    {
        RTN rtnCreateSemaphoreW = RTN_FindByName(img, "CreateSemaphoreW");
        if (RTN_Valid(rtnCreateSemaphoreW) && RTN_IsSafeForProbedReplacement(rtnCreateSemaphoreW))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for CreateSemaphoreW at " << RTN_Address(rtnCreateSemaphoreW) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnCreateSemaphoreW, AFUNPTR(myCreateSemaphoreW)));
            fptrCreateSemaphoreW =
                (WIND::HANDLE(WINAPI*)(WIND::LPSECURITY_ATTRIBUTES, WIND::LONG, WIND::LONG, WIND::LPCWSTR))fptr;
        }
    }

    if ((IMG_Name(img).find("kernel32.dll") != string::npos) || (IMG_Name(img).find("KERNEL32.DLL") != string::npos) ||
        (IMG_Name(img).find("KERNEL32.dll") != string::npos))

    {
        RTN rtnCreateMutexW = RTN_FindByName(img, "CreateMutexW");
        if (RTN_Valid(rtnCreateMutexW) && RTN_IsSafeForProbedReplacement(rtnCreateMutexW))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for CreateMutexW at " << RTN_Address(rtnCreateMutexW) << endl;
            OutFile.flush();
            AFUNPTR fptr     = (RTN_ReplaceProbed(rtnCreateMutexW, AFUNPTR(myCreateMutexW)));
            fptrCreateMutexW = (WIND::HANDLE(WINAPI*)(WIND::LPSECURITY_ATTRIBUTES, WIND::BOOL, WIND::LPCWSTR))fptr;
        }
    }

    if ((IMG_Name(img).find("rpcrt4.dll") != string::npos) || (IMG_Name(img).find("RPCRT4.DLL") != string::npos) ||
        (IMG_Name(img).find("RPCRT4.dll") != string::npos))

    {
        RTN rtnRpcMgmtWaitServerListen = RTN_FindByName(img, "RpcMgmtWaitServerListen");
        if (RTN_Valid(rtnRpcMgmtWaitServerListen) && RTN_IsSafeForProbedReplacement(rtnRpcMgmtWaitServerListen))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for RpcMgmtWaitServerListen at " << RTN_Address(rtnRpcMgmtWaitServerListen) << endl;
            OutFile.flush();
            AFUNPTR fptr                = (RTN_ReplaceProbed(rtnRpcMgmtWaitServerListen, AFUNPTR(myRpcMgmtWaitServerListen)));
            fptrRpcMgmtWaitServerListen = (WIND::RPC_STATUS(*)())fptr;
        }
    }

    if ((IMG_Name(img).find("kernel32.dll") != string::npos) || (IMG_Name(img).find("KERNEL32.DLL") != string::npos) ||
        (IMG_Name(img).find("KERNEL32.dll") != string::npos))

    {
        RTN rtnExitThread = RTN_FindByName(img, "ExitThread");
        if (RTN_Valid(rtnExitThread) && RTN_IsSafeForProbedReplacement(rtnExitThread))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for ExitThread at " << RTN_Address(rtnExitThread) << endl;
            OutFile.flush();
            AFUNPTR fptr   = (RTN_ReplaceProbed(rtnExitThread, AFUNPTR(myExitThread)));
            fptrExitThread = (VOID(WINAPI*)(WIND::DWORD))fptr;
        }
    }

    if ((IMG_Name(img).find("ws2_32.dll") != string::npos) || (IMG_Name(img).find("WS2_32.DLL") != string::npos) ||
        (IMG_Name(img).find("WS2_32.dll") != string::npos))

    {
        RTN rtnrecv = RTN_FindByName(img, "recv");
        if (RTN_Valid(rtnrecv) && RTN_IsSafeForProbedReplacement(rtnrecv))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for recv at " << RTN_Address(rtnrecv) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnrecv, AFUNPTR(myrecv)));
            fptrrecv     = (int(WSAAPI*)(WIND::SOCKET, char FAR*, int, int))fptr;
        }
    }

    if ((IMG_Name(img).find("advapi32.dll") != string::npos) || (IMG_Name(img).find("ADVAPI32.DLL") != string::npos) ||
        (IMG_Name(img).find("ADVAPI32.dll") != string::npos))

    {
        RTN rtnCreateProcessAsUserW = RTN_FindByName(img, "CreateProcessAsUserW");
        if (RTN_Valid(rtnCreateProcessAsUserW) && RTN_IsSafeForProbedReplacement(rtnCreateProcessAsUserW))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for CreateProcessAsUserW at " << RTN_Address(rtnCreateProcessAsUserW) << endl;
            OutFile.flush();
            AFUNPTR fptr             = (RTN_ReplaceProbed(rtnCreateProcessAsUserW, AFUNPTR(myCreateProcessAsUserW)));
            fptrCreateProcessAsUserW = (WIND::BOOL(WINAPI*)(
                WIND::HANDLE, WIND::LPCWSTR, WIND::LPWSTR, WIND::LPSECURITY_ATTRIBUTES, WIND::LPSECURITY_ATTRIBUTES, WIND::BOOL,
                WIND::DWORD, WIND::LPVOID, WIND::LPCWSTR, WIND::LPSTARTUPINFOW, WIND::LPPROCESS_INFORMATION))fptr;
        }
    }

    if ((IMG_Name(img).find("ntdll.dll") != string::npos) || (IMG_Name(img).find("NTDLL.DLL") != string::npos) ||
        (IMG_Name(img).find("NTDLL.dll") != string::npos))

    {
        RTN rtnRtlEnterCriticalSection = RTN_FindByName(img, "RtlEnterCriticalSection");
        if (RTN_Valid(rtnRtlEnterCriticalSection) && RTN_IsSafeForProbedReplacement(rtnRtlEnterCriticalSection))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for RtlEnterCriticalSection at " << RTN_Address(rtnRtlEnterCriticalSection) << endl;
            OutFile.flush();
            AFUNPTR fptr                = (RTN_ReplaceProbed(rtnRtlEnterCriticalSection, AFUNPTR(myRtlEnterCriticalSection)));
            fptrRtlEnterCriticalSection = (VOID(WINAPI*)(WIND::LPCRITICAL_SECTION))fptr;
        }
    }

    if ((IMG_Name(img).find("kernel32.dll") != string::npos) || (IMG_Name(img).find("KERNEL32.DLL") != string::npos) ||
        (IMG_Name(img).find("KERNEL32.dll") != string::npos))

    {
        RTN rtnReadConsoleA = RTN_FindByName(img, "ReadConsoleA");
        if (RTN_Valid(rtnReadConsoleA) && RTN_IsSafeForProbedReplacement(rtnReadConsoleA))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for ReadConsoleA at " << RTN_Address(rtnReadConsoleA) << endl;
            OutFile.flush();
            AFUNPTR fptr     = (RTN_ReplaceProbed(rtnReadConsoleA, AFUNPTR(myReadConsoleA)));
            fptrReadConsoleA = (WIND::BOOL(WINAPI*)(WIND::HANDLE, WIND::LPVOID, WIND::DWORD, WIND::LPDWORD, WIND::LPVOID))fptr;
        }
    }

    if ((IMG_Name(img).find("ntdll.dll") != string::npos) || (IMG_Name(img).find("NTDLL.DLL") != string::npos) ||
        (IMG_Name(img).find("NTDLL.dll") != string::npos))

    {
        RTN rtnRtlDeleteCriticalSection = RTN_FindByName(img, "RtlDeleteCriticalSection");
        if (RTN_Valid(rtnRtlDeleteCriticalSection) && RTN_IsSafeForProbedReplacement(rtnRtlDeleteCriticalSection))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for RtlDeleteCriticalSection at " << RTN_Address(rtnRtlDeleteCriticalSection) << endl;
            OutFile.flush();
            AFUNPTR fptr                 = (RTN_ReplaceProbed(rtnRtlDeleteCriticalSection, AFUNPTR(myRtlDeleteCriticalSection)));
            fptrRtlDeleteCriticalSection = (VOID(WINAPI*)(WIND::LPCRITICAL_SECTION))fptr;
        }
    }

    if ((IMG_Name(img).find("kernel32.dll") != string::npos) || (IMG_Name(img).find("KERNEL32.DLL") != string::npos) ||
        (IMG_Name(img).find("KERNEL32.dll") != string::npos))

    {
        RTN rtnWaitForMultipleObjectsEx = RTN_FindByName(img, "WaitForMultipleObjectsEx");
        if (RTN_Valid(rtnWaitForMultipleObjectsEx) && RTN_IsSafeForProbedReplacement(rtnWaitForMultipleObjectsEx))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for WaitForMultipleObjectsEx at " << RTN_Address(rtnWaitForMultipleObjectsEx) << endl;
            OutFile.flush();
            AFUNPTR fptr                 = (RTN_ReplaceProbed(rtnWaitForMultipleObjectsEx, AFUNPTR(myWaitForMultipleObjectsEx)));
            fptrWaitForMultipleObjectsEx = (WIND::DWORD(WINAPI*)(WIND::DWORD, __in_ecount(nCount) CONST WIND::HANDLE*, WIND::BOOL,
                                                                 WIND::DWORD, WIND::BOOL))fptr;
        }
    }

    if ((IMG_Name(img).find("kernel32.dll") != string::npos) || (IMG_Name(img).find("KERNEL32.DLL") != string::npos) ||
        (IMG_Name(img).find("KERNEL32.dll") != string::npos))

    {
        RTN rtnTerminateProcess = RTN_FindByName(img, "TerminateProcess");
        if (RTN_Valid(rtnTerminateProcess) && RTN_IsSafeForProbedReplacement(rtnTerminateProcess))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for TerminateProcess at " << RTN_Address(rtnTerminateProcess) << endl;
            OutFile.flush();
            AFUNPTR fptr         = (RTN_ReplaceProbed(rtnTerminateProcess, AFUNPTR(myTerminateProcess)));
            fptrTerminateProcess = (WIND::BOOL(WINAPI*)(WIND::HANDLE, WIND::UINT))fptr;
        }
    }

    if ((IMG_Name(img).find("kernel32.dll") != string::npos) || (IMG_Name(img).find("KERNEL32.DLL") != string::npos) ||
        (IMG_Name(img).find("KERNEL32.dll") != string::npos))

    {
        RTN rtnFindCloseChangeNotification = RTN_FindByName(img, "FindCloseChangeNotification");
        if (RTN_Valid(rtnFindCloseChangeNotification) && RTN_IsSafeForProbedReplacement(rtnFindCloseChangeNotification))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for FindCloseChangeNotification at " << RTN_Address(rtnFindCloseChangeNotification)
                    << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnFindCloseChangeNotification, AFUNPTR(myFindCloseChangeNotification)));
            fptrFindCloseChangeNotification = (WIND::BOOL(WINAPI*)(WIND::HANDLE))fptr;
        }
    }

    if ((IMG_Name(img).find("winspool.drv") != string::npos) || (IMG_Name(img).find("WINSPOOL.DRV") != string::npos) ||
        (IMG_Name(img).find("WINSPOOL.drv") != string::npos))

    {
        RTN rtnFindFirstPrinterChangeNotification = RTN_FindByName(img, "FindFirstPrinterChangeNotification");
        if (RTN_Valid(rtnFindFirstPrinterChangeNotification) &&
            RTN_IsSafeForProbedReplacement(rtnFindFirstPrinterChangeNotification))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for FindFirstPrinterChangeNotification at "
                    << RTN_Address(rtnFindFirstPrinterChangeNotification) << endl;
            OutFile.flush();
            AFUNPTR fptr =
                (RTN_ReplaceProbed(rtnFindFirstPrinterChangeNotification, AFUNPTR(myFindFirstPrinterChangeNotification)));
            fptrFindFirstPrinterChangeNotification =
                (WIND::HANDLE(WINAPI*)(WIND::HANDLE, WIND::DWORD, WIND::DWORD, WIND::LPVOID))fptr;
        }
    }

    if ((IMG_Name(img).find("user32.dll") != string::npos) || (IMG_Name(img).find("USER32.DLL") != string::npos) ||
        (IMG_Name(img).find("USER32.dll") != string::npos))

    {
        RTN rtnWaitForInputIdle = RTN_FindByName(img, "WaitForInputIdle");
        if (RTN_Valid(rtnWaitForInputIdle) && RTN_IsSafeForProbedReplacement(rtnWaitForInputIdle))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for WaitForInputIdle at " << RTN_Address(rtnWaitForInputIdle) << endl;
            OutFile.flush();
            AFUNPTR fptr         = (RTN_ReplaceProbed(rtnWaitForInputIdle, AFUNPTR(myWaitForInputIdle)));
            fptrWaitForInputIdle = (WIND::DWORD(WINAPI*)(WIND::HANDLE, WIND::DWORD))fptr;
        }
    }

    if ((IMG_Name(img).find("rpcrt4.dll") != string::npos) || (IMG_Name(img).find("RPCRT4.DLL") != string::npos) ||
        (IMG_Name(img).find("RPCRT4.dll") != string::npos))

    {
        RTN rtnNdrNsSendReceive = RTN_FindByName(img, "NdrNsSendReceive");
        if (RTN_Valid(rtnNdrNsSendReceive) && RTN_IsSafeForProbedReplacement(rtnNdrNsSendReceive))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for NdrNsSendReceive at " << RTN_Address(rtnNdrNsSendReceive) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnNdrNsSendReceive, AFUNPTR(myNdrNsSendReceive)));
            fptrNdrNsSendReceive =
                (unsigned char*(WINAPI*)(WIND::PMIDL_STUB_MESSAGE, unsigned char*, WIND::RPC_BINDING_HANDLE*))fptr;
        }
    }

    if ((IMG_Name(img).find("winspool.drv") != string::npos) || (IMG_Name(img).find("WINSPOOL.DRV") != string::npos) ||
        (IMG_Name(img).find("WINSPOOL.drv") != string::npos))

    {
        RTN rtnFindClosePrinterChangeNotification = RTN_FindByName(img, "FindClosePrinterChangeNotification");
        if (RTN_Valid(rtnFindClosePrinterChangeNotification) &&
            RTN_IsSafeForProbedReplacement(rtnFindClosePrinterChangeNotification))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for FindClosePrinterChangeNotification at "
                    << RTN_Address(rtnFindClosePrinterChangeNotification) << endl;
            OutFile.flush();
            AFUNPTR fptr =
                (RTN_ReplaceProbed(rtnFindClosePrinterChangeNotification, AFUNPTR(myFindClosePrinterChangeNotification)));
            fptrFindClosePrinterChangeNotification = (WIND::BOOL(WINAPI*)(WIND::HANDLE))fptr;
        }
    }

    if ((IMG_Name(img).find("user32.dll") != string::npos) || (IMG_Name(img).find("USER32.DLL") != string::npos) ||
        (IMG_Name(img).find("USER32.dll") != string::npos))

    {
        RTN rtnDialogBoxIndirectParamA = RTN_FindByName(img, "DialogBoxIndirectParamA");
        if (RTN_Valid(rtnDialogBoxIndirectParamA) && RTN_IsSafeForProbedReplacement(rtnDialogBoxIndirectParamA))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for DialogBoxIndirectParamA at " << RTN_Address(rtnDialogBoxIndirectParamA) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnDialogBoxIndirectParamA, AFUNPTR(myDialogBoxIndirectParamA)));
            fptrDialogBoxIndirectParamA =
                (WIND::INT_PTR(WINAPI*)(WIND::HINSTANCE, WIND::LPCDLGTEMPLATEA, WIND::HWND, WIND::DLGPROC, WIND::LPARAM))fptr;
        }
    }

    if ((IMG_Name(img).find("kernel32.dll") != string::npos) || (IMG_Name(img).find("KERNEL32.DLL") != string::npos) ||
        (IMG_Name(img).find("KERNEL32.dll") != string::npos))

    {
        RTN rtnReadConsoleW = RTN_FindByName(img, "ReadConsoleW");
        if (RTN_Valid(rtnReadConsoleW) && RTN_IsSafeForProbedReplacement(rtnReadConsoleW))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for ReadConsoleW at " << RTN_Address(rtnReadConsoleW) << endl;
            OutFile.flush();
            AFUNPTR fptr     = (RTN_ReplaceProbed(rtnReadConsoleW, AFUNPTR(myReadConsoleW)));
            fptrReadConsoleW = (WIND::BOOL(WINAPI*)(WIND::HANDLE, WIND::LPVOID, WIND::DWORD, WIND::LPDWORD, WIND::LPVOID))fptr;
        }
    }

    if ((IMG_Name(img).find("user32.dll") != string::npos) || (IMG_Name(img).find("USER32.DLL") != string::npos) ||
        (IMG_Name(img).find("USER32.dll") != string::npos))

    {
        RTN rtnGetMessageA = RTN_FindByName(img, "GetMessageA");
        if (RTN_Valid(rtnGetMessageA) && RTN_IsSafeForProbedReplacement(rtnGetMessageA))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for GetMessageA at " << RTN_Address(rtnGetMessageA) << endl;
            OutFile.flush();
            AFUNPTR fptr    = (RTN_ReplaceProbed(rtnGetMessageA, AFUNPTR(myGetMessageA)));
            fptrGetMessageA = (WIND::BOOL(WINAPI*)(WIND::LPMSG, WIND::HWND, WIND::UINT, WIND::UINT))fptr;
        }
    }

    if ((IMG_Name(img).find("kernel32.dll") != string::npos) || (IMG_Name(img).find("KERNEL32.DLL") != string::npos) ||
        (IMG_Name(img).find("KERNEL32.dll") != string::npos))

    {
        RTN rtnSetWaitableTimer = RTN_FindByName(img, "SetWaitableTimer");
        if (RTN_Valid(rtnSetWaitableTimer) && RTN_IsSafeForProbedReplacement(rtnSetWaitableTimer))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for SetWaitableTimer at " << RTN_Address(rtnSetWaitableTimer) << endl;
            OutFile.flush();
            AFUNPTR fptr         = (RTN_ReplaceProbed(rtnSetWaitableTimer, AFUNPTR(mySetWaitableTimer)));
            fptrSetWaitableTimer = (WIND::BOOL(WINAPI*)(WIND::HANDLE, const WIND::LARGE_INTEGER*, WIND::LONG,
                                                        WIND::PTIMERAPCROUTINE, WIND::LPVOID, WIND::BOOL))fptr;
        }
    }

    if ((IMG_Name(img).find("kernel32.dll") != string::npos) || (IMG_Name(img).find("KERNEL32.DLL") != string::npos) ||
        (IMG_Name(img).find("KERNEL32.dll") != string::npos))

    {
        RTN rtnSleepEx = RTN_FindByName(img, "SleepEx");
        if (RTN_Valid(rtnSleepEx) && RTN_IsSafeForProbedReplacement(rtnSleepEx))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for SleepEx at " << RTN_Address(rtnSleepEx) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnSleepEx, AFUNPTR(mySleepEx)));
            fptrSleepEx  = (WIND::DWORD(WINAPI*)(WIND::DWORD, WIND::BOOL))fptr;
        }
    }

    if ((IMG_Name(img).find("kernel32.dll") != string::npos) || (IMG_Name(img).find("KERNEL32.DLL") != string::npos) ||
        (IMG_Name(img).find("KERNEL32.dll") != string::npos))

    {
        RTN rtnLoadLibraryExW = RTN_FindByName(img, "LoadLibraryExW");
        if (RTN_Valid(rtnLoadLibraryExW) && RTN_IsSafeForProbedReplacement(rtnLoadLibraryExW))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for LoadLibraryExW at " << RTN_Address(rtnLoadLibraryExW) << endl;
            OutFile.flush();
            AFUNPTR fptr       = (RTN_ReplaceProbed(rtnLoadLibraryExW, AFUNPTR(myLoadLibraryExW)));
            fptrLoadLibraryExW = (WIND::HMODULE(WINAPI*)(WIND::LPCWSTR, __reserved WIND::HANDLE, unsigned long))fptr;
        }
    }

    if ((IMG_Name(img).find("kernel32.dll") != string::npos) || (IMG_Name(img).find("KERNEL32.DLL") != string::npos) ||
        (IMG_Name(img).find("KERNEL32.dll") != string::npos))

    {
        RTN rtnCallNamedPipeW = RTN_FindByName(img, "CallNamedPipeW");
        if (RTN_Valid(rtnCallNamedPipeW) && RTN_IsSafeForProbedReplacement(rtnCallNamedPipeW))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for CallNamedPipeW at " << RTN_Address(rtnCallNamedPipeW) << endl;
            OutFile.flush();
            AFUNPTR fptr       = (RTN_ReplaceProbed(rtnCallNamedPipeW, AFUNPTR(myCallNamedPipeW)));
            fptrCallNamedPipeW = (WIND::BOOL(WINAPI*)(WIND::LPCWSTR, __in_bcount_opt(nInBufferSize) WIND::LPVOID, WIND::DWORD,
                                                      __out_bcount_part_opt(nOutBufferSize, *lpBytesRead) WIND::LPVOID,
                                                      WIND::DWORD, WIND::LPDWORD, WIND::DWORD))fptr;
        }
    }

    if ((IMG_Name(img).find("user32.dll") != string::npos) || (IMG_Name(img).find("USER32.DLL") != string::npos) ||
        (IMG_Name(img).find("USER32.dll") != string::npos))

    {
        RTN rtnSendNotifyMessageA = RTN_FindByName(img, "SendNotifyMessageA");
        if (RTN_Valid(rtnSendNotifyMessageA) && RTN_IsSafeForProbedReplacement(rtnSendNotifyMessageA))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for SendNotifyMessageA at " << RTN_Address(rtnSendNotifyMessageA) << endl;
            OutFile.flush();
            AFUNPTR fptr           = (RTN_ReplaceProbed(rtnSendNotifyMessageA, AFUNPTR(mySendNotifyMessageA)));
            fptrSendNotifyMessageA = (WIND::BOOL(WINAPI*)(WIND::HWND, WIND::UINT, WIND::WPARAM, WIND::LPARAM))fptr;
        }
    }

    if ((IMG_Name(img).find("kernel32.dll") != string::npos) || (IMG_Name(img).find("KERNEL32.DLL") != string::npos) ||
        (IMG_Name(img).find("KERNEL32.dll") != string::npos))

    {
        RTN rtnInitializeCriticalSectionAndSpinCount = RTN_FindByName(img, "InitializeCriticalSectionAndSpinCount");
        if (RTN_Valid(rtnInitializeCriticalSectionAndSpinCount) &&
            RTN_IsSafeForProbedReplacement(rtnInitializeCriticalSectionAndSpinCount))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for InitializeCriticalSectionAndSpinCount at "
                    << RTN_Address(rtnInitializeCriticalSectionAndSpinCount) << endl;
            OutFile.flush();
            AFUNPTR fptr =
                (RTN_ReplaceProbed(rtnInitializeCriticalSectionAndSpinCount, AFUNPTR(myInitializeCriticalSectionAndSpinCount)));
            fptrInitializeCriticalSectionAndSpinCount = (WIND::BOOL(WINAPI*)(WIND::LPCRITICAL_SECTION, WIND::DWORD))fptr;
        }
    }

    if ((IMG_Name(img).find("comdlg32.dll") != string::npos) || (IMG_Name(img).find("COMDLG32.DLL") != string::npos) ||
        (IMG_Name(img).find("COMDLG32.dll") != string::npos))

    {
        RTN rtnPrintDlgW = RTN_FindByName(img, "PrintDlgW");
        if (RTN_Valid(rtnPrintDlgW) && RTN_IsSafeForProbedReplacement(rtnPrintDlgW))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for PrintDlgW at " << RTN_Address(rtnPrintDlgW) << endl;
            OutFile.flush();
            AFUNPTR fptr  = (RTN_ReplaceProbed(rtnPrintDlgW, AFUNPTR(myPrintDlgW)));
            fptrPrintDlgW = (WIND::BOOL(APIENTRY*)(WIND::LPPRINTDLGW))fptr;
        }
    }

    if ((IMG_Name(img).find("rpcrt4.dll") != string::npos) || (IMG_Name(img).find("RPCRT4.DLL") != string::npos) ||
        (IMG_Name(img).find("RPCRT4.dll") != string::npos))

    {
        RTN rtnRpcMgmtEpEltInqNextW = RTN_FindByName(img, "RpcMgmtEpEltInqNextW");
        if (RTN_Valid(rtnRpcMgmtEpEltInqNextW) && RTN_IsSafeForProbedReplacement(rtnRpcMgmtEpEltInqNextW))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for RpcMgmtEpEltInqNextW at " << RTN_Address(rtnRpcMgmtEpEltInqNextW) << endl;
            OutFile.flush();
            AFUNPTR fptr             = (RTN_ReplaceProbed(rtnRpcMgmtEpEltInqNextW, AFUNPTR(myRpcMgmtEpEltInqNextW)));
            fptrRpcMgmtEpEltInqNextW = (WIND::RPC_STATUS(WINAPI*)(WIND::RPC_EP_INQ_HANDLE, WIND::RPC_IF_ID __RPC_FAR*,
                                                                  WIND::RPC_BINDING_HANDLE __RPC_FAR*, WIND::UUID __RPC_FAR*,
                                                                  __deref_opt_out WIND::RPC_WSTR __RPC_FAR*))fptr;
        }
    }

    if ((IMG_Name(img).find("rpcrt4.dll") != string::npos) || (IMG_Name(img).find("RPCRT4.DLL") != string::npos) ||
        (IMG_Name(img).find("RPCRT4.dll") != string::npos))

    {
        RTN rtnRpcMgmtEpEltInqDone = RTN_FindByName(img, "RpcMgmtEpEltInqDone");
        if (RTN_Valid(rtnRpcMgmtEpEltInqDone) && RTN_IsSafeForProbedReplacement(rtnRpcMgmtEpEltInqDone))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for RpcMgmtEpEltInqDone at " << RTN_Address(rtnRpcMgmtEpEltInqDone) << endl;
            OutFile.flush();
            AFUNPTR fptr            = (RTN_ReplaceProbed(rtnRpcMgmtEpEltInqDone, AFUNPTR(myRpcMgmtEpEltInqDone)));
            fptrRpcMgmtEpEltInqDone = (WIND::RPC_STATUS(WINAPI*)(WIND::RPC_EP_INQ_HANDLE __RPC_FAR*))fptr;
        }
    }

    if ((IMG_Name(img).find("user32.dll") != string::npos) || (IMG_Name(img).find("USER32.DLL") != string::npos) ||
        (IMG_Name(img).find("USER32.dll") != string::npos))

    {
        RTN rtnPostMessageA = RTN_FindByName(img, "PostMessageA");
        if (RTN_Valid(rtnPostMessageA) && RTN_IsSafeForProbedReplacement(rtnPostMessageA))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for PostMessageA at " << RTN_Address(rtnPostMessageA) << endl;
            OutFile.flush();
            AFUNPTR fptr     = (RTN_ReplaceProbed(rtnPostMessageA, AFUNPTR(myPostMessageA)));
            fptrPostMessageA = (WIND::BOOL(WINAPI*)(WIND::HWND, WIND::UINT, WIND::WPARAM, WIND::LPARAM))fptr;
        }
    }

    if ((IMG_Name(img).find("kernel32.dll") != string::npos) || (IMG_Name(img).find("KERNEL32.DLL") != string::npos) ||
        (IMG_Name(img).find("KERNEL32.dll") != string::npos))

    {
        RTN rtnTerminateThread = RTN_FindByName(img, "TerminateThread");
        if (RTN_Valid(rtnTerminateThread) && RTN_IsSafeForProbedReplacement(rtnTerminateThread))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for TerminateThread at " << RTN_Address(rtnTerminateThread) << endl;
            OutFile.flush();
            AFUNPTR fptr        = (RTN_ReplaceProbed(rtnTerminateThread, AFUNPTR(myTerminateThread)));
            fptrTerminateThread = (WIND::BOOL(WINAPI*)(WIND::HANDLE, WIND::DWORD))fptr;
        }
    }

    if ((IMG_Name(img).find("kernel32.dll") != string::npos) || (IMG_Name(img).find("KERNEL32.DLL") != string::npos) ||
        (IMG_Name(img).find("KERNEL32.dll") != string::npos))

    {
        RTN rtnLoadLibraryW = RTN_FindByName(img, "LoadLibraryW");
        if (RTN_Valid(rtnLoadLibraryW) && RTN_IsSafeForProbedReplacement(rtnLoadLibraryW))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for LoadLibraryW at " << RTN_Address(rtnLoadLibraryW) << endl;
            OutFile.flush();
            AFUNPTR fptr     = (RTN_ReplaceProbed(rtnLoadLibraryW, AFUNPTR(myLoadLibraryW)));
            fptrLoadLibraryW = (WIND::HMODULE(WINAPI*)(WIND::LPCWSTR))fptr;
        }
    }

    if ((IMG_Name(img).find("kernel32.dll") != string::npos) || (IMG_Name(img).find("KERNEL32.DLL") != string::npos) ||
        (IMG_Name(img).find("KERNEL32.dll") != string::npos))

    {
        RTN rtnCreateMailslotW = RTN_FindByName(img, "CreateMailslotW");
        if (RTN_Valid(rtnCreateMailslotW) && RTN_IsSafeForProbedReplacement(rtnCreateMailslotW))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for CreateMailslotW at " << RTN_Address(rtnCreateMailslotW) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnCreateMailslotW, AFUNPTR(myCreateMailslotW)));
            fptrCreateMailslotW =
                (WIND::HANDLE(WINAPI*)(WIND::LPCWSTR, WIND::DWORD, WIND::DWORD, WIND::LPSECURITY_ATTRIBUTES))fptr;
        }
    }

    if ((IMG_Name(img).find("kernel32.dll") != string::npos) || (IMG_Name(img).find("KERNEL32.DLL") != string::npos) ||
        (IMG_Name(img).find("KERNEL32.dll") != string::npos))

    {
        RTN rtnSetEvent = RTN_FindByName(img, "SetEvent");
        if (RTN_Valid(rtnSetEvent) && RTN_IsSafeForProbedReplacement(rtnSetEvent))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for SetEvent at " << RTN_Address(rtnSetEvent) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnSetEvent, AFUNPTR(mySetEvent)));
            fptrSetEvent = (WIND::BOOL(WINAPI*)(WIND::HANDLE))fptr;
        }
    }

    if ((IMG_Name(img).find("kernel32.dll") != string::npos) || (IMG_Name(img).find("KERNEL32.DLL") != string::npos) ||
        (IMG_Name(img).find("KERNEL32.dll") != string::npos))

    {
        RTN rtnReadConsoleInputA = RTN_FindByName(img, "ReadConsoleInputA");
        if (RTN_Valid(rtnReadConsoleInputA) && RTN_IsSafeForProbedReplacement(rtnReadConsoleInputA))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for ReadConsoleInputA at " << RTN_Address(rtnReadConsoleInputA) << endl;
            OutFile.flush();
            AFUNPTR fptr          = (RTN_ReplaceProbed(rtnReadConsoleInputA, AFUNPTR(myReadConsoleInputA)));
            fptrReadConsoleInputA = (WIND::BOOL(WINAPI*)(WIND::HANDLE, WIND::PINPUT_RECORD, WIND::DWORD, WIND::LPDWORD))fptr;
        }
    }

    if ((IMG_Name(img).find("advapi32.dll") != string::npos) || (IMG_Name(img).find("ADVAPI32.DLL") != string::npos) ||
        (IMG_Name(img).find("ADVAPI32.dll") != string::npos))

    {
        RTN rtnCreateProcessWithLogonW = RTN_FindByName(img, "CreateProcessWithLogonW");
        if (RTN_Valid(rtnCreateProcessWithLogonW) && RTN_IsSafeForProbedReplacement(rtnCreateProcessWithLogonW))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for CreateProcessWithLogonW at " << RTN_Address(rtnCreateProcessWithLogonW) << endl;
            OutFile.flush();
            AFUNPTR fptr                = (RTN_ReplaceProbed(rtnCreateProcessWithLogonW, AFUNPTR(myCreateProcessWithLogonW)));
            fptrCreateProcessWithLogonW = (WIND::BOOL(WINAPI*)(
                WIND::LPCWSTR, WIND::LPCWSTR, WIND::LPCWSTR, WIND::DWORD, WIND::LPCWSTR, WIND::LPWSTR, WIND::DWORD, WIND::LPVOID,
                WIND::LPCWSTR, WIND::LPSTARTUPINFOW, WIND::LPPROCESS_INFORMATION))fptr;
        }
    }

    if ((IMG_Name(img).find("kernel32.dll") != string::npos) || (IMG_Name(img).find("KERNEL32.DLL") != string::npos) ||
        (IMG_Name(img).find("KERNEL32.dll") != string::npos))

    {
        RTN rtnCallNamedPipeA = RTN_FindByName(img, "CallNamedPipeA");
        if (RTN_Valid(rtnCallNamedPipeA) && RTN_IsSafeForProbedReplacement(rtnCallNamedPipeA))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for CallNamedPipeA at " << RTN_Address(rtnCallNamedPipeA) << endl;
            OutFile.flush();
            AFUNPTR fptr       = (RTN_ReplaceProbed(rtnCallNamedPipeA, AFUNPTR(myCallNamedPipeA)));
            fptrCallNamedPipeA = (WIND::BOOL(WINAPI*)(WIND::LPCSTR, __in_bcount_opt(nInBufferSize) WIND::LPVOID, WIND::DWORD,
                                                      __out_bcount_part_opt(nOutBufferSize, *lpBytesRead) WIND::LPVOID,
                                                      WIND::DWORD, WIND::LPDWORD, WIND::DWORD))fptr;
        }
    }

    if ((IMG_Name(img).find("user32.dll") != string::npos) || (IMG_Name(img).find("USER32.DLL") != string::npos) ||
        (IMG_Name(img).find("USER32.dll") != string::npos))

    {
        RTN rtnSendNotifyMessageW = RTN_FindByName(img, "SendNotifyMessageW");
        if (RTN_Valid(rtnSendNotifyMessageW) && RTN_IsSafeForProbedReplacement(rtnSendNotifyMessageW))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for SendNotifyMessageW at " << RTN_Address(rtnSendNotifyMessageW) << endl;
            OutFile.flush();
            AFUNPTR fptr           = (RTN_ReplaceProbed(rtnSendNotifyMessageW, AFUNPTR(mySendNotifyMessageW)));
            fptrSendNotifyMessageW = (WIND::BOOL(WINAPI*)(WIND::HWND, WIND::UINT, WIND::WPARAM, WIND::LPARAM))fptr;
        }
    }

    if ((IMG_Name(img).find("kernel32.dll") != string::npos) || (IMG_Name(img).find("KERNEL32.DLL") != string::npos) ||
        (IMG_Name(img).find("KERNEL32.dll") != string::npos))

    {
        RTN rtnCreateMailslotA = RTN_FindByName(img, "CreateMailslotA");
        if (RTN_Valid(rtnCreateMailslotA) && RTN_IsSafeForProbedReplacement(rtnCreateMailslotA))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for CreateMailslotA at " << RTN_Address(rtnCreateMailslotA) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnCreateMailslotA, AFUNPTR(myCreateMailslotA)));
            fptrCreateMailslotA =
                (WIND::HANDLE(WINAPI*)(WIND::LPCSTR, WIND::DWORD, WIND::DWORD, WIND::LPSECURITY_ATTRIBUTES))fptr;
        }
    }

    if ((IMG_Name(img).find("ws2_32.dll") != string::npos) || (IMG_Name(img).find("WS2_32.DLL") != string::npos) ||
        (IMG_Name(img).find("WS2_32.dll") != string::npos))

    {
        RTN rtnselect = RTN_FindByName(img, "select");
        if (RTN_Valid(rtnselect) && RTN_IsSafeForProbedReplacement(rtnselect))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for select at " << RTN_Address(rtnselect) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnselect, AFUNPTR(myselect)));
            fptrselect =
                (int(WSAAPI*)(int, WIND::fd_set FAR*, WIND::fd_set FAR*, WIND::fd_set FAR*, const struct timeval FAR*))fptr;
        }
    }

    if ((IMG_Name(img).find("kernel32.dll") != string::npos) || (IMG_Name(img).find("KERNEL32.DLL") != string::npos) ||
        (IMG_Name(img).find("KERNEL32.dll") != string::npos))

    {
        RTN rtnCreateRemoteThread = RTN_FindByName(img, "CreateRemoteThread");
        if (RTN_Valid(rtnCreateRemoteThread) && RTN_IsSafeForProbedReplacement(rtnCreateRemoteThread))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for CreateRemoteThread at " << RTN_Address(rtnCreateRemoteThread) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnCreateRemoteThread, AFUNPTR(myCreateRemoteThread)));
            fptrCreateRemoteThread =
                (WIND::HANDLE(WINAPI*)(WIND::HANDLE, WIND::LPSECURITY_ATTRIBUTES, WIND::SIZE_T, WIND::LPTHREAD_START_ROUTINE,
                                       WIND::LPVOID, WIND::DWORD, WIND::LPDWORD))fptr;
        }
    }

    if ((IMG_Name(img).find("user32.dll") != string::npos) || (IMG_Name(img).find("USER32.DLL") != string::npos) ||
        (IMG_Name(img).find("USER32.dll") != string::npos))

    {
        RTN rtnSetWindowsHookExA = RTN_FindByName(img, "SetWindowsHookExA");
        if (RTN_Valid(rtnSetWindowsHookExA) && RTN_IsSafeForProbedReplacement(rtnSetWindowsHookExA))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for SetWindowsHookExA at " << RTN_Address(rtnSetWindowsHookExA) << endl;
            OutFile.flush();
            AFUNPTR fptr          = (RTN_ReplaceProbed(rtnSetWindowsHookExA, AFUNPTR(mySetWindowsHookExA)));
            fptrSetWindowsHookExA = (WIND::HHOOK(WINAPI*)(int, WIND::HOOKPROC, WIND::HINSTANCE, WIND::DWORD))fptr;
        }
    }

    if ((IMG_Name(img).find("user32.dll") != string::npos) || (IMG_Name(img).find("USER32.DLL") != string::npos) ||
        (IMG_Name(img).find("USER32.dll") != string::npos))

    {
        RTN rtnBroadcastSystemMessageExA = RTN_FindByName(img, "BroadcastSystemMessageExA");
        if (RTN_Valid(rtnBroadcastSystemMessageExA) && RTN_IsSafeForProbedReplacement(rtnBroadcastSystemMessageExA))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for BroadcastSystemMessageExA at " << RTN_Address(rtnBroadcastSystemMessageExA) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnBroadcastSystemMessageExA, AFUNPTR(myBroadcastSystemMessageExA)));
            fptrBroadcastSystemMessageExA =
                (long(WINAPI*)(WIND::DWORD, WIND::LPDWORD, WIND::UINT, WIND::WPARAM, WIND::LPARAM, WIND::PBSMINFO))fptr;
        }
    }

    if ((IMG_Name(img).find("kernel32.dll") != string::npos) || (IMG_Name(img).find("KERNEL32.DLL") != string::npos) ||
        (IMG_Name(img).find("KERNEL32.dll") != string::npos))

    {
        RTN rtnOpenProcess = RTN_FindByName(img, "OpenProcess");
        if (RTN_Valid(rtnOpenProcess) && RTN_IsSafeForProbedReplacement(rtnOpenProcess))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for OpenProcess at " << RTN_Address(rtnOpenProcess) << endl;
            OutFile.flush();
            AFUNPTR fptr    = (RTN_ReplaceProbed(rtnOpenProcess, AFUNPTR(myOpenProcess)));
            fptrOpenProcess = (WIND::HANDLE(WINAPI*)(WIND::DWORD, WIND::BOOL, WIND::DWORD))fptr;
        }
    }

    if ((IMG_Name(img).find("ws2_32.dll") != string::npos) || (IMG_Name(img).find("WS2_32.DLL") != string::npos) ||
        (IMG_Name(img).find("WS2_32.dll") != string::npos))

    {
        RTN rtnWSAWaitForMultipleEvents = RTN_FindByName(img, "WSAWaitForMultipleEvents");
        if (RTN_Valid(rtnWSAWaitForMultipleEvents) && RTN_IsSafeForProbedReplacement(rtnWSAWaitForMultipleEvents))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for WSAWaitForMultipleEvents at " << RTN_Address(rtnWSAWaitForMultipleEvents) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnWSAWaitForMultipleEvents, AFUNPTR(myWSAWaitForMultipleEvents)));
            fptrWSAWaitForMultipleEvents =
                (WIND::DWORD(WSAAPI*)(WIND::DWORD, const WIND::WSAEVENT FAR*, WIND::BOOL, WIND::DWORD, WIND::BOOL))fptr;
        }
    }

    if ((IMG_Name(img).find("rpcrt4.dll") != string::npos) || (IMG_Name(img).find("RPCRT4.DLL") != string::npos) ||
        (IMG_Name(img).find("RPCRT4.dll") != string::npos))

    {
        RTN rtnRpcMgmtEpEltInqBegin = RTN_FindByName(img, "RpcMgmtEpEltInqBegin");
        if (RTN_Valid(rtnRpcMgmtEpEltInqBegin) && RTN_IsSafeForProbedReplacement(rtnRpcMgmtEpEltInqBegin))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for RpcMgmtEpEltInqBegin at " << RTN_Address(rtnRpcMgmtEpEltInqBegin) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnRpcMgmtEpEltInqBegin, AFUNPTR(myRpcMgmtEpEltInqBegin)));
            fptrRpcMgmtEpEltInqBegin =
                (WIND::RPC_STATUS(WINAPI*)(WIND::RPC_BINDING_HANDLE, unsigned long, WIND::RPC_IF_ID __RPC_FAR*, unsigned long,
                                           WIND::UUID __RPC_FAR*, WIND::RPC_EP_INQ_HANDLE __RPC_FAR*))fptr;
        }
    }

    if ((IMG_Name(img).find("kernel32.dll") != string::npos) || (IMG_Name(img).find("KERNEL32.DLL") != string::npos) ||
        (IMG_Name(img).find("KERNEL32.dll") != string::npos))

    {
        RTN rtnWriteFileGather = RTN_FindByName(img, "WriteFileGather");
        if (RTN_Valid(rtnWriteFileGather) && RTN_IsSafeForProbedReplacement(rtnWriteFileGather))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for WriteFileGather at " << RTN_Address(rtnWriteFileGather) << endl;
            OutFile.flush();
            AFUNPTR fptr        = (RTN_ReplaceProbed(rtnWriteFileGather, AFUNPTR(myWriteFileGather)));
            fptrWriteFileGather = (WIND::BOOL(WINAPI*)(WIND::HANDLE, WIND::FILE_SEGMENT_ELEMENT**, WIND::DWORD,
                                                       __reserved WIND::LPDWORD, WIND::LPOVERLAPPED))fptr;
        }
    }

    if ((IMG_Name(img).find("kernel32.dll") != string::npos) || (IMG_Name(img).find("KERNEL32.DLL") != string::npos) ||
        (IMG_Name(img).find("KERNEL32.dll") != string::npos))

    {
        RTN rtnWaitForMultipleObjects = RTN_FindByName(img, "WaitForMultipleObjects");
        if (RTN_Valid(rtnWaitForMultipleObjects) && RTN_IsSafeForProbedReplacement(rtnWaitForMultipleObjects))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for WaitForMultipleObjects at " << RTN_Address(rtnWaitForMultipleObjects) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnWaitForMultipleObjects, AFUNPTR(myWaitForMultipleObjects)));
            fptrWaitForMultipleObjects =
                (WIND::DWORD(WINAPI*)(WIND::DWORD, __in_ecount(nCount) CONST WIND::HANDLE*, WIND::BOOL, WIND::DWORD))fptr;
        }
    }

    if ((IMG_Name(img).find("rpcrt4.dll") != string::npos) || (IMG_Name(img).find("RPCRT4.DLL") != string::npos) ||
        (IMG_Name(img).find("RPCRT4.dll") != string::npos))

    {
        RTN rtnNdrSendReceive = RTN_FindByName(img, "NdrSendReceive");
        if (RTN_Valid(rtnNdrSendReceive) && RTN_IsSafeForProbedReplacement(rtnNdrSendReceive))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for NdrSendReceive at " << RTN_Address(rtnNdrSendReceive) << endl;
            OutFile.flush();
            AFUNPTR fptr       = (RTN_ReplaceProbed(rtnNdrSendReceive, AFUNPTR(myNdrSendReceive)));
            fptrNdrSendReceive = (unsigned char*(WINAPI*)(WIND::PMIDL_STUB_MESSAGE, unsigned char*))fptr;
        }
    }

    if ((IMG_Name(img).find("kernel32.dll") != string::npos) || (IMG_Name(img).find("KERNEL32.DLL") != string::npos) ||
        (IMG_Name(img).find("KERNEL32.dll") != string::npos))

    {
        RTN rtnCreateWaitableTimerW = RTN_FindByName(img, "CreateWaitableTimerW");
        if (RTN_Valid(rtnCreateWaitableTimerW) && RTN_IsSafeForProbedReplacement(rtnCreateWaitableTimerW))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for CreateWaitableTimerW at " << RTN_Address(rtnCreateWaitableTimerW) << endl;
            OutFile.flush();
            AFUNPTR fptr             = (RTN_ReplaceProbed(rtnCreateWaitableTimerW, AFUNPTR(myCreateWaitableTimerW)));
            fptrCreateWaitableTimerW = (WIND::HANDLE(WINAPI*)(WIND::LPSECURITY_ATTRIBUTES, WIND::BOOL, WIND::LPCWSTR))fptr;
        }
    }

    if ((IMG_Name(img).find("user32.dll") != string::npos) || (IMG_Name(img).find("USER32.DLL") != string::npos) ||
        (IMG_Name(img).find("USER32.dll") != string::npos))

    {
        RTN rtnWaitMessage = RTN_FindByName(img, "WaitMessage");
        if (RTN_Valid(rtnWaitMessage) && RTN_IsSafeForProbedReplacement(rtnWaitMessage))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for WaitMessage at " << RTN_Address(rtnWaitMessage) << endl;
            OutFile.flush();
            AFUNPTR fptr    = (RTN_ReplaceProbed(rtnWaitMessage, AFUNPTR(myWaitMessage)));
            fptrWaitMessage = (WIND::BOOL(WINAPI*)())fptr;
        }
    }

    if ((IMG_Name(img).find("kernel32.dll") != string::npos) || (IMG_Name(img).find("KERNEL32.DLL") != string::npos) ||
        (IMG_Name(img).find("KERNEL32.dll") != string::npos))

    {
        RTN rtnSetCriticalSectionSpinCount = RTN_FindByName(img, "SetCriticalSectionSpinCount");
        if (RTN_Valid(rtnSetCriticalSectionSpinCount) && RTN_IsSafeForProbedReplacement(rtnSetCriticalSectionSpinCount))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for SetCriticalSectionSpinCount at " << RTN_Address(rtnSetCriticalSectionSpinCount)
                    << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnSetCriticalSectionSpinCount, AFUNPTR(mySetCriticalSectionSpinCount)));
            fptrSetCriticalSectionSpinCount = (WIND::DWORD(WINAPI*)(WIND::LPCRITICAL_SECTION, WIND::DWORD))fptr;
        }
    }

    if ((IMG_Name(img).find("kernel32.dll") != string::npos) || (IMG_Name(img).find("KERNEL32.DLL") != string::npos) ||
        (IMG_Name(img).find("KERNEL32.dll") != string::npos))

    {
        RTN rtnWaitNamedPipeW = RTN_FindByName(img, "WaitNamedPipeW");
        if (RTN_Valid(rtnWaitNamedPipeW) && RTN_IsSafeForProbedReplacement(rtnWaitNamedPipeW))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for WaitNamedPipeW at " << RTN_Address(rtnWaitNamedPipeW) << endl;
            OutFile.flush();
            AFUNPTR fptr       = (RTN_ReplaceProbed(rtnWaitNamedPipeW, AFUNPTR(myWaitNamedPipeW)));
            fptrWaitNamedPipeW = (WIND::BOOL(WINAPI*)(WIND::LPCWSTR, WIND::DWORD))fptr;
        }
    }

    if ((IMG_Name(img).find("ws2_32.dll") != string::npos) || (IMG_Name(img).find("WS2_32.DLL") != string::npos) ||
        (IMG_Name(img).find("WS2_32.dll") != string::npos))

    {
        RTN rtnsendto = RTN_FindByName(img, "sendto");
        if (RTN_Valid(rtnsendto) && RTN_IsSafeForProbedReplacement(rtnsendto))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for sendto at " << RTN_Address(rtnsendto) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnsendto, AFUNPTR(mysendto)));
            fptrsendto   = (int(WSAAPI*)(WIND::SOCKET, const char FAR*, int, int, const struct sockaddr FAR*, int))fptr;
        }
    }

    if ((IMG_Name(img).find("kernel32.dll") != string::npos) || (IMG_Name(img).find("KERNEL32.DLL") != string::npos) ||
        (IMG_Name(img).find("KERNEL32.dll") != string::npos))

    {
        RTN rtnWaitNamedPipeA = RTN_FindByName(img, "WaitNamedPipeA");
        if (RTN_Valid(rtnWaitNamedPipeA) && RTN_IsSafeForProbedReplacement(rtnWaitNamedPipeA))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for WaitNamedPipeA at " << RTN_Address(rtnWaitNamedPipeA) << endl;
            OutFile.flush();
            AFUNPTR fptr       = (RTN_ReplaceProbed(rtnWaitNamedPipeA, AFUNPTR(myWaitNamedPipeA)));
            fptrWaitNamedPipeA = (WIND::BOOL(WINAPI*)(WIND::LPCSTR, WIND::DWORD))fptr;
        }
    }

    if ((IMG_Name(img).find("kernel32.dll") != string::npos) || (IMG_Name(img).find("KERNEL32.DLL") != string::npos) ||
        (IMG_Name(img).find("KERNEL32.dll") != string::npos))

    {
        RTN rtnResetEvent = RTN_FindByName(img, "ResetEvent");
        if (RTN_Valid(rtnResetEvent) && RTN_IsSafeForProbedReplacement(rtnResetEvent))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for ResetEvent at " << RTN_Address(rtnResetEvent) << endl;
            OutFile.flush();
            AFUNPTR fptr   = (RTN_ReplaceProbed(rtnResetEvent, AFUNPTR(myResetEvent)));
            fptrResetEvent = (WIND::BOOL(WINAPI*)(WIND::HANDLE))fptr;
        }
    }

    if ((IMG_Name(img).find("kernel32.dll") != string::npos) || (IMG_Name(img).find("KERNEL32.DLL") != string::npos) ||
        (IMG_Name(img).find("KERNEL32.dll") != string::npos))

    {
        RTN rtnCreateTimerQueueTimer = RTN_FindByName(img, "CreateTimerQueueTimer");
        if (RTN_Valid(rtnCreateTimerQueueTimer) && RTN_IsSafeForProbedReplacement(rtnCreateTimerQueueTimer))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for CreateTimerQueueTimer at " << RTN_Address(rtnCreateTimerQueueTimer) << endl;
            OutFile.flush();
            AFUNPTR fptr              = (RTN_ReplaceProbed(rtnCreateTimerQueueTimer, AFUNPTR(myCreateTimerQueueTimer)));
            fptrCreateTimerQueueTimer = (WIND::BOOL(WINAPI*)(WIND::PHANDLE, WIND::HANDLE, WIND::WAITORTIMERCALLBACK, WIND::PVOID,
                                                             WIND::DWORD, WIND::DWORD, WIND::ULONG))fptr;
        }
    }

    if ((IMG_Name(img).find("kernel32.dll") != string::npos) || (IMG_Name(img).find("KERNEL32.DLL") != string::npos) ||
        (IMG_Name(img).find("KERNEL32.dll") != string::npos))

    {
        RTN rtnCreateWaitableTimerA = RTN_FindByName(img, "CreateWaitableTimerA");
        if (RTN_Valid(rtnCreateWaitableTimerA) && RTN_IsSafeForProbedReplacement(rtnCreateWaitableTimerA))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for CreateWaitableTimerA at " << RTN_Address(rtnCreateWaitableTimerA) << endl;
            OutFile.flush();
            AFUNPTR fptr             = (RTN_ReplaceProbed(rtnCreateWaitableTimerA, AFUNPTR(myCreateWaitableTimerA)));
            fptrCreateWaitableTimerA = (WIND::HANDLE(WINAPI*)(WIND::LPSECURITY_ATTRIBUTES, WIND::BOOL, WIND::LPCSTR))fptr;
        }
    }

    if ((IMG_Name(img).find("kernel32.dll") != string::npos) || (IMG_Name(img).find("KERNEL32.DLL") != string::npos) ||
        (IMG_Name(img).find("KERNEL32.dll") != string::npos))

    {
        RTN rtnDuplicateHandle = RTN_FindByName(img, "DuplicateHandle");
        if (RTN_Valid(rtnDuplicateHandle) && RTN_IsSafeForProbedReplacement(rtnDuplicateHandle))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for DuplicateHandle at " << RTN_Address(rtnDuplicateHandle) << endl;
            OutFile.flush();
            AFUNPTR fptr        = (RTN_ReplaceProbed(rtnDuplicateHandle, AFUNPTR(myDuplicateHandle)));
            fptrDuplicateHandle = (WIND::BOOL(WINAPI*)(WIND::HANDLE, WIND::HANDLE, WIND::HANDLE, WIND::LPHANDLE, WIND::DWORD,
                                                       WIND::BOOL, WIND::DWORD))fptr;
        }
    }

    if ((IMG_Name(img).find("ws2_32.dll") != string::npos) || (IMG_Name(img).find("WS2_32.DLL") != string::npos) ||
        (IMG_Name(img).find("WS2_32.dll") != string::npos))

    {
        RTN rtnclosesocket = RTN_FindByName(img, "closesocket");
        if (RTN_Valid(rtnclosesocket) && RTN_IsSafeForProbedReplacement(rtnclosesocket))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for closesocket at " << RTN_Address(rtnclosesocket) << endl;
            OutFile.flush();
            AFUNPTR fptr    = (RTN_ReplaceProbed(rtnclosesocket, AFUNPTR(myclosesocket)));
            fptrclosesocket = (int(WSAAPI*)(WIND::SOCKET))fptr;
        }
    }

    if ((IMG_Name(img).find("kernel32.dll") != string::npos) || (IMG_Name(img).find("KERNEL32.DLL") != string::npos) ||
        (IMG_Name(img).find("KERNEL32.dll") != string::npos))

    {
        RTN rtnGetProcAddress = RTN_FindByName(img, "GetProcAddress");
        if (RTN_Valid(rtnGetProcAddress) && RTN_IsSafeForProbedReplacement(rtnGetProcAddress))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for GetProcAddress at " << RTN_Address(rtnGetProcAddress) << endl;
            OutFile.flush();
            AFUNPTR fptr       = (RTN_ReplaceProbed(rtnGetProcAddress, AFUNPTR(myGetProcAddress)));
            fptrGetProcAddress = (WIND::FARPROC(WINAPI*)(WIND::HMODULE, WIND::LPCSTR))fptr;
        }
    }

    if ((IMG_Name(img).find("ws2_32.dll") != string::npos) || (IMG_Name(img).find("WS2_32.DLL") != string::npos) ||
        (IMG_Name(img).find("WS2_32.dll") != string::npos))

    {
        RTN rtnsocket = RTN_FindByName(img, "socket");
        if (RTN_Valid(rtnsocket) && RTN_IsSafeForProbedReplacement(rtnsocket))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for socket at " << RTN_Address(rtnsocket) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnsocket, AFUNPTR(mysocket)));
            fptrsocket   = (WIND::SOCKET(WSAAPI*)(int, int, int))fptr;
        }
    }

    if ((IMG_Name(img).find("ws2_32.dll") != string::npos) || (IMG_Name(img).find("WS2_32.DLL") != string::npos) ||
        (IMG_Name(img).find("WS2_32.dll") != string::npos))

    {
        RTN rtnWSASetEvent = RTN_FindByName(img, "WSASetEvent");
        if (RTN_Valid(rtnWSASetEvent) && RTN_IsSafeForProbedReplacement(rtnWSASetEvent))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for WSASetEvent at " << RTN_Address(rtnWSASetEvent) << endl;
            OutFile.flush();
            AFUNPTR fptr    = (RTN_ReplaceProbed(rtnWSASetEvent, AFUNPTR(myWSASetEvent)));
            fptrWSASetEvent = (WIND::BOOL(WSAAPI*)(WIND::WSAEVENT))fptr;
        }
    }

    if ((IMG_Name(img).find("kernel32.dll") != string::npos) || (IMG_Name(img).find("KERNEL32.DLL") != string::npos) ||
        (IMG_Name(img).find("KERNEL32.dll") != string::npos))

    {
        RTN rtnCreateEventW = RTN_FindByName(img, "CreateEventW");
        if (RTN_Valid(rtnCreateEventW) && RTN_IsSafeForProbedReplacement(rtnCreateEventW))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for CreateEventW at " << RTN_Address(rtnCreateEventW) << endl;
            OutFile.flush();
            AFUNPTR fptr     = (RTN_ReplaceProbed(rtnCreateEventW, AFUNPTR(myCreateEventW)));
            fptrCreateEventW = (WIND::HANDLE(WINAPI*)(WIND::LPSECURITY_ATTRIBUTES, WIND::BOOL, WIND::BOOL, WIND::LPCWSTR))fptr;
        }
    }

    if ((IMG_Name(img).find("user32.dll") != string::npos) || (IMG_Name(img).find("USER32.DLL") != string::npos) ||
        (IMG_Name(img).find("USER32.dll") != string::npos))

    {
        RTN rtnPostThreadMessageW = RTN_FindByName(img, "PostThreadMessageW");
        if (RTN_Valid(rtnPostThreadMessageW) && RTN_IsSafeForProbedReplacement(rtnPostThreadMessageW))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for PostThreadMessageW at " << RTN_Address(rtnPostThreadMessageW) << endl;
            OutFile.flush();
            AFUNPTR fptr           = (RTN_ReplaceProbed(rtnPostThreadMessageW, AFUNPTR(myPostThreadMessageW)));
            fptrPostThreadMessageW = (WIND::BOOL(WINAPI*)(WIND::DWORD, WIND::UINT, WIND::WPARAM, WIND::LPARAM))fptr;
        }
    }

    if ((IMG_Name(img).find("ws2_32.dll") != string::npos) || (IMG_Name(img).find("WS2_32.DLL") != string::npos) ||
        (IMG_Name(img).find("WS2_32.dll") != string::npos))

    {
        RTN rtnWSASendTo = RTN_FindByName(img, "WSASendTo");
        if (RTN_Valid(rtnWSASendTo) && RTN_IsSafeForProbedReplacement(rtnWSASendTo))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for WSASendTo at " << RTN_Address(rtnWSASendTo) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnWSASendTo, AFUNPTR(myWSASendTo)));
            fptrWSASendTo =
                (int(WSAAPI*)(WIND::SOCKET, WIND::LPWSABUF, WIND::DWORD, WIND::LPDWORD, WIND::DWORD, const struct sockaddr FAR*,
                              int, WIND::LPWSAOVERLAPPED, WIND::LPWSAOVERLAPPED_COMPLETION_ROUTINE))fptr;
        }
    }

    if ((IMG_Name(img).find("kernel32.dll") != string::npos) || (IMG_Name(img).find("KERNEL32.DLL") != string::npos) ||
        (IMG_Name(img).find("KERNEL32.dll") != string::npos))

    {
        RTN rtnCreateFileW = RTN_FindByName(img, "CreateFileW");
        if (RTN_Valid(rtnCreateFileW) && RTN_IsSafeForProbedReplacement(rtnCreateFileW))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for CreateFileW at " << RTN_Address(rtnCreateFileW) << endl;
            OutFile.flush();
            AFUNPTR fptr    = (RTN_ReplaceProbed(rtnCreateFileW, AFUNPTR(myCreateFileW)));
            fptrCreateFileW = (WIND::HANDLE(WINAPI*)(WIND::LPCWSTR, WIND::DWORD, WIND::DWORD, WIND::LPSECURITY_ATTRIBUTES,
                                                     WIND::DWORD, WIND::DWORD, WIND::HANDLE))fptr;
        }
    }

    if ((IMG_Name(img).find("kernel32.dll") != string::npos) || (IMG_Name(img).find("KERNEL32.DLL") != string::npos) ||
        (IMG_Name(img).find("KERNEL32.dll") != string::npos))

    {
        RTN rtnCreateEventA = RTN_FindByName(img, "CreateEventA");
        if (RTN_Valid(rtnCreateEventA) && RTN_IsSafeForProbedReplacement(rtnCreateEventA))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for CreateEventA at " << RTN_Address(rtnCreateEventA) << endl;
            OutFile.flush();
            AFUNPTR fptr     = (RTN_ReplaceProbed(rtnCreateEventA, AFUNPTR(myCreateEventA)));
            fptrCreateEventA = (WIND::HANDLE(WINAPI*)(WIND::LPSECURITY_ATTRIBUTES, WIND::BOOL, WIND::BOOL, WIND::LPCSTR))fptr;
        }
    }

    if ((IMG_Name(img).find("rpcrt4.dll") != string::npos) || (IMG_Name(img).find("RPCRT4.DLL") != string::npos) ||
        (IMG_Name(img).find("RPCRT4.dll") != string::npos))

    {
        RTN rtnRpcServerListen = RTN_FindByName(img, "RpcServerListen");
        if (RTN_Valid(rtnRpcServerListen) && RTN_IsSafeForProbedReplacement(rtnRpcServerListen))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for RpcServerListen at " << RTN_Address(rtnRpcServerListen) << endl;
            OutFile.flush();
            AFUNPTR fptr        = (RTN_ReplaceProbed(rtnRpcServerListen, AFUNPTR(myRpcServerListen)));
            fptrRpcServerListen = (WIND::RPC_STATUS(WINAPI*)(unsigned int, unsigned int, unsigned int))fptr;
        }
    }

    if ((IMG_Name(img).find("kernel32.dll") != string::npos) || (IMG_Name(img).find("KERNEL32.DLL") != string::npos) ||
        (IMG_Name(img).find("KERNEL32.dll") != string::npos))

    {
        RTN rtnCreateFileA = RTN_FindByName(img, "CreateFileA");
        if (RTN_Valid(rtnCreateFileA) && RTN_IsSafeForProbedReplacement(rtnCreateFileA))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for CreateFileA at " << RTN_Address(rtnCreateFileA) << endl;
            OutFile.flush();
            AFUNPTR fptr    = (RTN_ReplaceProbed(rtnCreateFileA, AFUNPTR(myCreateFileA)));
            fptrCreateFileA = (WIND::HANDLE(WINAPI*)(WIND::LPCSTR, WIND::DWORD, WIND::DWORD, WIND::LPSECURITY_ATTRIBUTES,
                                                     WIND::DWORD, WIND::DWORD, WIND::HANDLE))fptr;
        }
    }

    if ((IMG_Name(img).find("kernel32.dll") != string::npos) || (IMG_Name(img).find("KERNEL32.DLL") != string::npos) ||
        (IMG_Name(img).find("KERNEL32.dll") != string::npos))

    {
        RTN rtnDeleteTimerQueueEx = RTN_FindByName(img, "DeleteTimerQueueEx");
        if (RTN_Valid(rtnDeleteTimerQueueEx) && RTN_IsSafeForProbedReplacement(rtnDeleteTimerQueueEx))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for DeleteTimerQueueEx at " << RTN_Address(rtnDeleteTimerQueueEx) << endl;
            OutFile.flush();
            AFUNPTR fptr           = (RTN_ReplaceProbed(rtnDeleteTimerQueueEx, AFUNPTR(myDeleteTimerQueueEx)));
            fptrDeleteTimerQueueEx = (WIND::BOOL(WINAPI*)(WIND::HANDLE, WIND::HANDLE))fptr;
        }
    }

    if ((IMG_Name(img).find("user32.dll") != string::npos) || (IMG_Name(img).find("USER32.DLL") != string::npos) ||
        (IMG_Name(img).find("USER32.dll") != string::npos))

    {
        RTN rtnSendMessageTimeoutW = RTN_FindByName(img, "SendMessageTimeoutW");
        if (RTN_Valid(rtnSendMessageTimeoutW) && RTN_IsSafeForProbedReplacement(rtnSendMessageTimeoutW))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for SendMessageTimeoutW at " << RTN_Address(rtnSendMessageTimeoutW) << endl;
            OutFile.flush();
            AFUNPTR fptr            = (RTN_ReplaceProbed(rtnSendMessageTimeoutW, AFUNPTR(mySendMessageTimeoutW)));
            fptrSendMessageTimeoutW = (WIND::LRESULT(WINAPI*)(WIND::HWND, WIND::UINT, WIND::WPARAM, WIND::LPARAM, WIND::UINT,
                                                              WIND::UINT, WIND::PDWORD_PTR))fptr;
        }
    }

    if ((IMG_Name(img).find("user32.dll") != string::npos) || (IMG_Name(img).find("USER32.DLL") != string::npos) ||
        (IMG_Name(img).find("USER32.dll") != string::npos))

    {
        RTN rtnPostThreadMessageA = RTN_FindByName(img, "PostThreadMessageA");
        if (RTN_Valid(rtnPostThreadMessageA) && RTN_IsSafeForProbedReplacement(rtnPostThreadMessageA))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for PostThreadMessageA at " << RTN_Address(rtnPostThreadMessageA) << endl;
            OutFile.flush();
            AFUNPTR fptr           = (RTN_ReplaceProbed(rtnPostThreadMessageA, AFUNPTR(myPostThreadMessageA)));
            fptrPostThreadMessageA = (WIND::BOOL(WINAPI*)(WIND::DWORD, WIND::UINT, WIND::WPARAM, WIND::LPARAM))fptr;
        }
    }

    if ((IMG_Name(img).find("ws2_32.dll") != string::npos) || (IMG_Name(img).find("WS2_32.DLL") != string::npos) ||
        (IMG_Name(img).find("WS2_32.dll") != string::npos))

    {
        RTN rtnWSASocketW = RTN_FindByName(img, "WSASocketW");
        if (RTN_Valid(rtnWSASocketW) && RTN_IsSafeForProbedReplacement(rtnWSASocketW))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for WSASocketW at " << RTN_Address(rtnWSASocketW) << endl;
            OutFile.flush();
            AFUNPTR fptr   = (RTN_ReplaceProbed(rtnWSASocketW, AFUNPTR(myWSASocketW)));
            fptrWSASocketW = (WIND::SOCKET(WSAAPI*)(int, int, int, WIND::LPWSAPROTOCOL_INFOW, WIND::GROUP, WIND::DWORD))fptr;
        }
    }

    if ((IMG_Name(img).find("ws2_32.dll") != string::npos) || (IMG_Name(img).find("WS2_32.DLL") != string::npos) ||
        (IMG_Name(img).find("WS2_32.dll") != string::npos))

    {
        RTN rtnWSAGetOverlappedResult = RTN_FindByName(img, "WSAGetOverlappedResult");
        if (RTN_Valid(rtnWSAGetOverlappedResult) && RTN_IsSafeForProbedReplacement(rtnWSAGetOverlappedResult))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for WSAGetOverlappedResult at " << RTN_Address(rtnWSAGetOverlappedResult) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnWSAGetOverlappedResult, AFUNPTR(myWSAGetOverlappedResult)));
            fptrWSAGetOverlappedResult =
                (WIND::BOOL(WSAAPI*)(WIND::SOCKET, WIND::LPWSAOVERLAPPED, WIND::LPDWORD, WIND::BOOL, WIND::LPDWORD))fptr;
        }
    }

    if ((IMG_Name(img).find("ws2_32.dll") != string::npos) || (IMG_Name(img).find("WS2_32.DLL") != string::npos) ||
        (IMG_Name(img).find("WS2_32.dll") != string::npos))

    {
        RTN rtnWSAConnect = RTN_FindByName(img, "WSAConnect");
        if (RTN_Valid(rtnWSAConnect) && RTN_IsSafeForProbedReplacement(rtnWSAConnect))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for WSAConnect at " << RTN_Address(rtnWSAConnect) << endl;
            OutFile.flush();
            AFUNPTR fptr   = (RTN_ReplaceProbed(rtnWSAConnect, AFUNPTR(myWSAConnect)));
            fptrWSAConnect = (int(WSAAPI*)(WIND::SOCKET, const struct sockaddr FAR*, int, WIND::LPWSABUF, WIND::LPWSABUF,
                                           WIND::LPQOS, WIND::LPQOS))fptr;
        }
    }

    if ((IMG_Name(img).find("kernel32.dll") != string::npos) || (IMG_Name(img).find("KERNEL32.DLL") != string::npos) ||
        (IMG_Name(img).find("KERNEL32.dll") != string::npos))

    {
        RTN rtnFindFirstChangeNotificationA = RTN_FindByName(img, "FindFirstChangeNotificationA");
        if (RTN_Valid(rtnFindFirstChangeNotificationA) && RTN_IsSafeForProbedReplacement(rtnFindFirstChangeNotificationA))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for FindFirstChangeNotificationA at " << RTN_Address(rtnFindFirstChangeNotificationA)
                    << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnFindFirstChangeNotificationA, AFUNPTR(myFindFirstChangeNotificationA)));
            fptrFindFirstChangeNotificationA = (WIND::HANDLE(WINAPI*)(WIND::LPCSTR, WIND::BOOL, WIND::DWORD))fptr;
        }
    }

    if ((IMG_Name(img).find("kernel32.dll") != string::npos) || (IMG_Name(img).find("KERNEL32.DLL") != string::npos) ||
        (IMG_Name(img).find("KERNEL32.dll") != string::npos))

    {
        RTN rtnCreateNamedPipeW = RTN_FindByName(img, "CreateNamedPipeW");
        if (RTN_Valid(rtnCreateNamedPipeW) && RTN_IsSafeForProbedReplacement(rtnCreateNamedPipeW))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for CreateNamedPipeW at " << RTN_Address(rtnCreateNamedPipeW) << endl;
            OutFile.flush();
            AFUNPTR fptr         = (RTN_ReplaceProbed(rtnCreateNamedPipeW, AFUNPTR(myCreateNamedPipeW)));
            fptrCreateNamedPipeW = (WIND::HANDLE(WINAPI*)(WIND::LPCWSTR, WIND::DWORD, WIND::DWORD, WIND::DWORD, WIND::DWORD,
                                                          WIND::DWORD, WIND::DWORD, WIND::LPSECURITY_ATTRIBUTES))fptr;
        }
    }

    if ((IMG_Name(img).find("ws2_32.dll") != string::npos) || (IMG_Name(img).find("WS2_32.DLL") != string::npos) ||
        (IMG_Name(img).find("WS2_32.dll") != string::npos))

    {
        RTN rtnaccept = RTN_FindByName(img, "accept");
        if (RTN_Valid(rtnaccept) && RTN_IsSafeForProbedReplacement(rtnaccept))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for accept at " << RTN_Address(rtnaccept) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnaccept, AFUNPTR(myaccept)));
            fptraccept   = (WIND::SOCKET(WSAAPI*)(WIND::SOCKET, struct sockaddr FAR*, int FAR*))fptr;
        }
    }

    if ((IMG_Name(img).find("kernel32.dll") != string::npos) || (IMG_Name(img).find("KERNEL32.DLL") != string::npos) ||
        (IMG_Name(img).find("KERNEL32.dll") != string::npos))

    {
        RTN rtnOpenSemaphoreA = RTN_FindByName(img, "OpenSemaphoreA");
        if (RTN_Valid(rtnOpenSemaphoreA) && RTN_IsSafeForProbedReplacement(rtnOpenSemaphoreA))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for OpenSemaphoreA at " << RTN_Address(rtnOpenSemaphoreA) << endl;
            OutFile.flush();
            AFUNPTR fptr       = (RTN_ReplaceProbed(rtnOpenSemaphoreA, AFUNPTR(myOpenSemaphoreA)));
            fptrOpenSemaphoreA = (WIND::HANDLE(WINAPI*)(WIND::DWORD, WIND::BOOL, WIND::LPCSTR))fptr;
        }
    }

    if ((IMG_Name(img).find("kernel32.dll") != string::npos) || (IMG_Name(img).find("KERNEL32.DLL") != string::npos) ||
        (IMG_Name(img).find("KERNEL32.dll") != string::npos))

    {
        RTN rtnUnregisterWaitEx = RTN_FindByName(img, "UnregisterWaitEx");
        if (RTN_Valid(rtnUnregisterWaitEx) && RTN_IsSafeForProbedReplacement(rtnUnregisterWaitEx))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for UnregisterWaitEx at " << RTN_Address(rtnUnregisterWaitEx) << endl;
            OutFile.flush();
            AFUNPTR fptr         = (RTN_ReplaceProbed(rtnUnregisterWaitEx, AFUNPTR(myUnregisterWaitEx)));
            fptrUnregisterWaitEx = (WIND::BOOL(WINAPI*)(WIND::HANDLE, WIND::HANDLE))fptr;
        }
    }

    if ((IMG_Name(img).find("rpcrt4.dll") != string::npos) || (IMG_Name(img).find("RPCRT4.DLL") != string::npos) ||
        (IMG_Name(img).find("RPCRT4.dll") != string::npos))

    {
        RTN rtnRpcMgmtIsServerListening = RTN_FindByName(img, "RpcMgmtIsServerListening");
        if (RTN_Valid(rtnRpcMgmtIsServerListening) && RTN_IsSafeForProbedReplacement(rtnRpcMgmtIsServerListening))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for RpcMgmtIsServerListening at " << RTN_Address(rtnRpcMgmtIsServerListening) << endl;
            OutFile.flush();
            AFUNPTR fptr                 = (RTN_ReplaceProbed(rtnRpcMgmtIsServerListening, AFUNPTR(myRpcMgmtIsServerListening)));
            fptrRpcMgmtIsServerListening = (WIND::RPC_STATUS(WINAPI*)(WIND::RPC_BINDING_HANDLE))fptr;
        }
    }

    if ((IMG_Name(img).find("kernel32.dll") != string::npos) || (IMG_Name(img).find("KERNEL32.DLL") != string::npos) ||
        (IMG_Name(img).find("KERNEL32.dll") != string::npos))

    {
        RTN rtnWaitForSingleObjectEx = RTN_FindByName(img, "WaitForSingleObjectEx");
        if (RTN_Valid(rtnWaitForSingleObjectEx) && RTN_IsSafeForProbedReplacement(rtnWaitForSingleObjectEx))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for WaitForSingleObjectEx at " << RTN_Address(rtnWaitForSingleObjectEx) << endl;
            OutFile.flush();
            AFUNPTR fptr              = (RTN_ReplaceProbed(rtnWaitForSingleObjectEx, AFUNPTR(myWaitForSingleObjectEx)));
            fptrWaitForSingleObjectEx = (WIND::DWORD(WINAPI*)(WIND::HANDLE, WIND::DWORD, WIND::BOOL))fptr;
        }
    }

    if ((IMG_Name(img).find("kernel32.dll") != string::npos) || (IMG_Name(img).find("KERNEL32.DLL") != string::npos) ||
        (IMG_Name(img).find("KERNEL32.dll") != string::npos))

    {
        RTN rtnCreateNamedPipeA = RTN_FindByName(img, "CreateNamedPipeA");
        if (RTN_Valid(rtnCreateNamedPipeA) && RTN_IsSafeForProbedReplacement(rtnCreateNamedPipeA))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for CreateNamedPipeA at " << RTN_Address(rtnCreateNamedPipeA) << endl;
            OutFile.flush();
            AFUNPTR fptr         = (RTN_ReplaceProbed(rtnCreateNamedPipeA, AFUNPTR(myCreateNamedPipeA)));
            fptrCreateNamedPipeA = (WIND::HANDLE(WINAPI*)(WIND::LPCSTR, WIND::DWORD, WIND::DWORD, WIND::DWORD, WIND::DWORD,
                                                          WIND::DWORD, WIND::DWORD, WIND::LPSECURITY_ATTRIBUTES))fptr;
        }
    }

    if ((IMG_Name(img).find("kernel32.dll") != string::npos) || (IMG_Name(img).find("KERNEL32.DLL") != string::npos) ||
        (IMG_Name(img).find("KERNEL32.dll") != string::npos))

    {
        RTN rtnFindFirstChangeNotificationW = RTN_FindByName(img, "FindFirstChangeNotificationW");
        if (RTN_Valid(rtnFindFirstChangeNotificationW) && RTN_IsSafeForProbedReplacement(rtnFindFirstChangeNotificationW))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for FindFirstChangeNotificationW at " << RTN_Address(rtnFindFirstChangeNotificationW)
                    << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnFindFirstChangeNotificationW, AFUNPTR(myFindFirstChangeNotificationW)));
            fptrFindFirstChangeNotificationW = (WIND::HANDLE(WINAPI*)(WIND::LPCWSTR, WIND::BOOL, WIND::DWORD))fptr;
        }
    }

    if ((IMG_Name(img).find("user32.dll") != string::npos) || (IMG_Name(img).find("USER32.DLL") != string::npos) ||
        (IMG_Name(img).find("USER32.dll") != string::npos))

    {
        RTN rtnMessageBoxExA = RTN_FindByName(img, "MessageBoxExA");
        if (RTN_Valid(rtnMessageBoxExA) && RTN_IsSafeForProbedReplacement(rtnMessageBoxExA))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for MessageBoxExA at " << RTN_Address(rtnMessageBoxExA) << endl;
            OutFile.flush();
            AFUNPTR fptr      = (RTN_ReplaceProbed(rtnMessageBoxExA, AFUNPTR(myMessageBoxExA)));
            fptrMessageBoxExA = (int(WINAPI*)(WIND::HWND, WIND::LPCSTR, WIND::LPCSTR, WIND::UINT, WIND::WORD))fptr;
        }
    }

    if ((IMG_Name(img).find("kernel32.dll") != string::npos) || (IMG_Name(img).find("KERNEL32.DLL") != string::npos) ||
        (IMG_Name(img).find("KERNEL32.dll") != string::npos))

    {
        RTN rtnGetQueuedCompletionStatus = RTN_FindByName(img, "GetQueuedCompletionStatus");
        if (RTN_Valid(rtnGetQueuedCompletionStatus) && RTN_IsSafeForProbedReplacement(rtnGetQueuedCompletionStatus))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for GetQueuedCompletionStatus at " << RTN_Address(rtnGetQueuedCompletionStatus) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnGetQueuedCompletionStatus, AFUNPTR(myGetQueuedCompletionStatus)));
            fptrGetQueuedCompletionStatus =
                (WIND::BOOL(WINAPI*)(WIND::HANDLE, WIND::LPDWORD, WIND::PULONG_PTR, WIND::LPOVERLAPPED, WIND::DWORD))fptr;
        }
    }

    if ((IMG_Name(img).find("kernel32.dll") != string::npos) || (IMG_Name(img).find("KERNEL32.DLL") != string::npos) ||
        (IMG_Name(img).find("KERNEL32.dll") != string::npos))

    {
        RTN rtnCancelWaitableTimer = RTN_FindByName(img, "CancelWaitableTimer");
        if (RTN_Valid(rtnCancelWaitableTimer) && RTN_IsSafeForProbedReplacement(rtnCancelWaitableTimer))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for CancelWaitableTimer at " << RTN_Address(rtnCancelWaitableTimer) << endl;
            OutFile.flush();
            AFUNPTR fptr            = (RTN_ReplaceProbed(rtnCancelWaitableTimer, AFUNPTR(myCancelWaitableTimer)));
            fptrCancelWaitableTimer = (WIND::BOOL(WINAPI*)(WIND::HANDLE))fptr;
        }
    }

    if ((IMG_Name(img).find("user32.dll") != string::npos) || (IMG_Name(img).find("USER32.DLL") != string::npos) ||
        (IMG_Name(img).find("USER32.dll") != string::npos))

    {
        RTN rtnMessageBoxW = RTN_FindByName(img, "MessageBoxW");
        if (RTN_Valid(rtnMessageBoxW) && RTN_IsSafeForProbedReplacement(rtnMessageBoxW))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for MessageBoxW at " << RTN_Address(rtnMessageBoxW) << endl;
            OutFile.flush();
            AFUNPTR fptr    = (RTN_ReplaceProbed(rtnMessageBoxW, AFUNPTR(myMessageBoxW)));
            fptrMessageBoxW = (int(WINAPI*)(WIND::HWND, WIND::LPCWSTR, WIND::LPCWSTR, WIND::UINT))fptr;
        }
    }

    if ((IMG_Name(img).find("ws2_32.dll") != string::npos) || (IMG_Name(img).find("WS2_32.DLL") != string::npos) ||
        (IMG_Name(img).find("WS2_32.dll") != string::npos))

    {
        RTN rtnWSAAccept = RTN_FindByName(img, "WSAAccept");
        if (RTN_Valid(rtnWSAAccept) && RTN_IsSafeForProbedReplacement(rtnWSAAccept))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for WSAAccept at " << RTN_Address(rtnWSAAccept) << endl;
            OutFile.flush();
            AFUNPTR fptr  = (RTN_ReplaceProbed(rtnWSAAccept, AFUNPTR(myWSAAccept)));
            fptrWSAAccept = (WIND::SOCKET(WSAAPI*)(WIND::SOCKET, struct sockaddr FAR*, WIND::LPINT, WIND::LPCONDITIONPROC,
                                                   WIND::DWORD_PTR))fptr;
        }
    }

    if ((IMG_Name(img).find("kernel32.dll") != string::npos) || (IMG_Name(img).find("KERNEL32.DLL") != string::npos) ||
        (IMG_Name(img).find("KERNEL32.dll") != string::npos))

    {
        RTN rtnUnregisterWait = RTN_FindByName(img, "UnregisterWait");
        if (RTN_Valid(rtnUnregisterWait) && RTN_IsSafeForProbedReplacement(rtnUnregisterWait))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for UnregisterWait at " << RTN_Address(rtnUnregisterWait) << endl;
            OutFile.flush();
            AFUNPTR fptr       = (RTN_ReplaceProbed(rtnUnregisterWait, AFUNPTR(myUnregisterWait)));
            fptrUnregisterWait = (WIND::BOOL(WINAPI*)(WIND::HANDLE))fptr;
        }
    }

    if ((IMG_Name(img).find("ws2_32.dll") != string::npos) || (IMG_Name(img).find("WS2_32.DLL") != string::npos) ||
        (IMG_Name(img).find("WS2_32.dll") != string::npos))

    {
        RTN rtnWSACloseEvent = RTN_FindByName(img, "WSACloseEvent");
        if (RTN_Valid(rtnWSACloseEvent) && RTN_IsSafeForProbedReplacement(rtnWSACloseEvent))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for WSACloseEvent at " << RTN_Address(rtnWSACloseEvent) << endl;
            OutFile.flush();
            AFUNPTR fptr      = (RTN_ReplaceProbed(rtnWSACloseEvent, AFUNPTR(myWSACloseEvent)));
            fptrWSACloseEvent = (WIND::BOOL(WSAAPI*)(WIND::WSAEVENT))fptr;
        }
    }

    if ((IMG_Name(img).find("kernel32.dll") != string::npos) || (IMG_Name(img).find("KERNEL32.DLL") != string::npos) ||
        (IMG_Name(img).find("KERNEL32.dll") != string::npos))

    {
        RTN rtnCreateIoCompletionPort = RTN_FindByName(img, "CreateIoCompletionPort");
        if (RTN_Valid(rtnCreateIoCompletionPort) && RTN_IsSafeForProbedReplacement(rtnCreateIoCompletionPort))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for CreateIoCompletionPort at " << RTN_Address(rtnCreateIoCompletionPort) << endl;
            OutFile.flush();
            AFUNPTR fptr               = (RTN_ReplaceProbed(rtnCreateIoCompletionPort, AFUNPTR(myCreateIoCompletionPort)));
            fptrCreateIoCompletionPort = (WIND::HANDLE(WINAPI*)(WIND::HANDLE, WIND::HANDLE, WIND::ULONG_PTR, WIND::DWORD))fptr;
        }
    }

    if ((IMG_Name(img).find("user32.dll") != string::npos) || (IMG_Name(img).find("USER32.DLL") != string::npos) ||
        (IMG_Name(img).find("USER32.dll") != string::npos))

    {
        RTN rtnDialogBoxParamW = RTN_FindByName(img, "DialogBoxParamW");
        if (RTN_Valid(rtnDialogBoxParamW) && RTN_IsSafeForProbedReplacement(rtnDialogBoxParamW))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for DialogBoxParamW at " << RTN_Address(rtnDialogBoxParamW) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnDialogBoxParamW, AFUNPTR(myDialogBoxParamW)));
            fptrDialogBoxParamW =
                (WIND::INT_PTR(WINAPI*)(WIND::HINSTANCE, WIND::LPCWSTR, WIND::HWND, WIND::DLGPROC, WIND::LPARAM))fptr;
        }
    }

    if ((IMG_Name(img).find("user32.dll") != string::npos) || (IMG_Name(img).find("USER32.DLL") != string::npos) ||
        (IMG_Name(img).find("USER32.dll") != string::npos))

    {
        RTN rtnMessageBoxA = RTN_FindByName(img, "MessageBoxA");
        if (RTN_Valid(rtnMessageBoxA) && RTN_IsSafeForProbedReplacement(rtnMessageBoxA))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for MessageBoxA at " << RTN_Address(rtnMessageBoxA) << endl;
            OutFile.flush();
            AFUNPTR fptr    = (RTN_ReplaceProbed(rtnMessageBoxA, AFUNPTR(myMessageBoxA)));
            fptrMessageBoxA = (int(WINAPI*)(WIND::HWND, WIND::LPCSTR, WIND::LPCSTR, WIND::UINT))fptr;
        }
    }

    if ((IMG_Name(img).find("user32.dll") != string::npos) || (IMG_Name(img).find("USER32.DLL") != string::npos) ||
        (IMG_Name(img).find("USER32.dll") != string::npos))

    {
        RTN rtnMessageBoxExW = RTN_FindByName(img, "MessageBoxExW");
        if (RTN_Valid(rtnMessageBoxExW) && RTN_IsSafeForProbedReplacement(rtnMessageBoxExW))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for MessageBoxExW at " << RTN_Address(rtnMessageBoxExW) << endl;
            OutFile.flush();
            AFUNPTR fptr      = (RTN_ReplaceProbed(rtnMessageBoxExW, AFUNPTR(myMessageBoxExW)));
            fptrMessageBoxExW = (int(WINAPI*)(WIND::HWND, WIND::LPCWSTR, WIND::LPCWSTR, WIND::UINT, WIND::WORD))fptr;
        }
    }

    if ((IMG_Name(img).find("user32.dll") != string::npos) || (IMG_Name(img).find("USER32.DLL") != string::npos) ||
        (IMG_Name(img).find("USER32.dll") != string::npos))

    {
        RTN rtnDialogBoxParamA = RTN_FindByName(img, "DialogBoxParamA");
        if (RTN_Valid(rtnDialogBoxParamA) && RTN_IsSafeForProbedReplacement(rtnDialogBoxParamA))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for DialogBoxParamA at " << RTN_Address(rtnDialogBoxParamA) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnDialogBoxParamA, AFUNPTR(myDialogBoxParamA)));
            fptrDialogBoxParamA =
                (WIND::INT_PTR(WINAPI*)(WIND::HINSTANCE, WIND::LPCSTR, WIND::HWND, WIND::DLGPROC, WIND::LPARAM))fptr;
        }
    }

    if ((IMG_Name(img).find("kernel32.dll") != string::npos) || (IMG_Name(img).find("KERNEL32.DLL") != string::npos) ||
        (IMG_Name(img).find("KERNEL32.dll") != string::npos))

    {
        RTN rtnSleep = RTN_FindByName(img, "Sleep");
        if (RTN_Valid(rtnSleep) && RTN_IsSafeForProbedReplacement(rtnSleep))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for Sleep at " << RTN_Address(rtnSleep) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnSleep, AFUNPTR(mySleep)));
            fptrSleep    = (VOID(WINAPI*)(WIND::DWORD))fptr;
        }
    }

    if ((IMG_Name(img).find("rpcns4.dll") != string::npos) || (IMG_Name(img).find("RPCNS4.DLL") != string::npos) ||
        (IMG_Name(img).find("RPCNS4.dll") != string::npos))

    {
        RTN rtnRpcNsBindingLookupDone = RTN_FindByName(img, "RpcNsBindingLookupDone");
        if (RTN_Valid(rtnRpcNsBindingLookupDone) && RTN_IsSafeForProbedReplacement(rtnRpcNsBindingLookupDone))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for RpcNsBindingLookupDone at " << RTN_Address(rtnRpcNsBindingLookupDone) << endl;
            OutFile.flush();
            AFUNPTR fptr               = (RTN_ReplaceProbed(rtnRpcNsBindingLookupDone, AFUNPTR(myRpcNsBindingLookupDone)));
            fptrRpcNsBindingLookupDone = (WIND::RPC_STATUS(WINAPI*)(WIND::RPC_NS_HANDLE __RPC_FAR*))fptr;
        }
    }

    if ((IMG_Name(img).find("kernel32.dll") != string::npos) || (IMG_Name(img).find("KERNEL32.DLL") != string::npos) ||
        (IMG_Name(img).find("KERNEL32.dll") != string::npos))

    {
        RTN rtnOpenMutexA = RTN_FindByName(img, "OpenMutexA");
        if (RTN_Valid(rtnOpenMutexA) && RTN_IsSafeForProbedReplacement(rtnOpenMutexA))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for OpenMutexA at " << RTN_Address(rtnOpenMutexA) << endl;
            OutFile.flush();
            AFUNPTR fptr   = (RTN_ReplaceProbed(rtnOpenMutexA, AFUNPTR(myOpenMutexA)));
            fptrOpenMutexA = (WIND::HANDLE(WINAPI*)(WIND::DWORD, WIND::BOOL, WIND::LPCSTR))fptr;
        }
    }

    if ((IMG_Name(img).find("kernel32.dll") != string::npos) || (IMG_Name(img).find("KERNEL32.DLL") != string::npos) ||
        (IMG_Name(img).find("KERNEL32.dll") != string::npos))

    {
        RTN rtnSuspendThread = RTN_FindByName(img, "SuspendThread");
        if (RTN_Valid(rtnSuspendThread) && RTN_IsSafeForProbedReplacement(rtnSuspendThread))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for SuspendThread at " << RTN_Address(rtnSuspendThread) << endl;
            OutFile.flush();
            AFUNPTR fptr      = (RTN_ReplaceProbed(rtnSuspendThread, AFUNPTR(mySuspendThread)));
            fptrSuspendThread = (WIND::DWORD(WINAPI*)(WIND::HANDLE))fptr;
        }
    }

    if ((IMG_Name(img).find("kernel32.dll") != string::npos) || (IMG_Name(img).find("KERNEL32.DLL") != string::npos) ||
        (IMG_Name(img).find("KERNEL32.dll") != string::npos))

    {
        RTN rtnRaiseException = RTN_FindByName(img, "RaiseException");
        if (RTN_Valid(rtnRaiseException) && RTN_IsSafeForProbedReplacement(rtnRaiseException))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for RaiseException at " << RTN_Address(rtnRaiseException) << endl;
            OutFile.flush();
            AFUNPTR fptr       = (RTN_ReplaceProbed(rtnRaiseException, AFUNPTR(myRaiseException)));
            fptrRaiseException = (VOID(WINAPI*)(WIND::DWORD, WIND::DWORD, WIND::DWORD,
                                                __in_ecount_opt(nNumberOfArguments) CONST WIND::ULONG_PTR*))fptr;
        }
    }

    if ((IMG_Name(img).find("rpcns4.dll") != string::npos) || (IMG_Name(img).find("RPCNS4.DLL") != string::npos) ||
        (IMG_Name(img).find("RPCNS4.dll") != string::npos))

    {
        RTN rtnRpcNsBindingImportBeginW = RTN_FindByName(img, "RpcNsBindingImportBeginW");
        if (RTN_Valid(rtnRpcNsBindingImportBeginW) && RTN_IsSafeForProbedReplacement(rtnRpcNsBindingImportBeginW))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for RpcNsBindingImportBeginW at " << RTN_Address(rtnRpcNsBindingImportBeginW) << endl;
            OutFile.flush();
            AFUNPTR fptr                 = (RTN_ReplaceProbed(rtnRpcNsBindingImportBeginW, AFUNPTR(myRpcNsBindingImportBeginW)));
            fptrRpcNsBindingImportBeginW = (WIND::RPC_STATUS(WINAPI*)(unsigned long, WIND::RPC_WSTR, WIND::RPC_IF_HANDLE,
                                                                      WIND::UUID __RPC_FAR*, WIND::RPC_NS_HANDLE __RPC_FAR*))fptr;
        }
    }

    if ((IMG_Name(img).find("kernel32.dll") != string::npos) || (IMG_Name(img).find("KERNEL32.DLL") != string::npos) ||
        (IMG_Name(img).find("KERNEL32.dll") != string::npos))

    {
        RTN rtnReleaseSemaphore = RTN_FindByName(img, "ReleaseSemaphore");
        if (RTN_Valid(rtnReleaseSemaphore) && RTN_IsSafeForProbedReplacement(rtnReleaseSemaphore))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for ReleaseSemaphore at " << RTN_Address(rtnReleaseSemaphore) << endl;
            OutFile.flush();
            AFUNPTR fptr         = (RTN_ReplaceProbed(rtnReleaseSemaphore, AFUNPTR(myReleaseSemaphore)));
            fptrReleaseSemaphore = (WIND::BOOL(WINAPI*)(WIND::HANDLE, WIND::LONG, WIND::LPLONG))fptr;
        }
    }

    if ((IMG_Name(img).find("kernel32.dll") != string::npos) || (IMG_Name(img).find("KERNEL32.DLL") != string::npos) ||
        (IMG_Name(img).find("KERNEL32.dll") != string::npos))

    {
        RTN rtnOpenFile = RTN_FindByName(img, "OpenFile");
        if (RTN_Valid(rtnOpenFile) && RTN_IsSafeForProbedReplacement(rtnOpenFile))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for OpenFile at " << RTN_Address(rtnOpenFile) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnOpenFile, AFUNPTR(myOpenFile)));
            fptrOpenFile = (WIND::HFILE(WINAPI*)(WIND::LPCSTR, WIND::LPOFSTRUCT, WIND::UINT))fptr;
        }
    }

    if ((IMG_Name(img).find("kernel32.dll") != string::npos) || (IMG_Name(img).find("KERNEL32.DLL") != string::npos) ||
        (IMG_Name(img).find("KERNEL32.dll") != string::npos))

    {
        RTN rtnReadFile = RTN_FindByName(img, "ReadFile");
        if (RTN_Valid(rtnReadFile) && RTN_IsSafeForProbedReplacement(rtnReadFile))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for ReadFile at " << RTN_Address(rtnReadFile) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnReadFile, AFUNPTR(myReadFile)));
            fptrReadFile =
                (WIND::BOOL(WINAPI*)(WIND::HANDLE, __out_bcount_part(nNumberOfBytesToRead, *lpNumberOfBytesRead) WIND::LPVOID,
                                     WIND::DWORD, WIND::LPDWORD, WIND::LPOVERLAPPED))fptr;
        }
    }

    if ((IMG_Name(img).find("kernel32.dll") != string::npos) || (IMG_Name(img).find("KERNEL32.DLL") != string::npos) ||
        (IMG_Name(img).find("KERNEL32.dll") != string::npos))

    {
        RTN rtnWriteFileEx = RTN_FindByName(img, "WriteFileEx");
        if (RTN_Valid(rtnWriteFileEx) && RTN_IsSafeForProbedReplacement(rtnWriteFileEx))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for WriteFileEx at " << RTN_Address(rtnWriteFileEx) << endl;
            OutFile.flush();
            AFUNPTR fptr    = (RTN_ReplaceProbed(rtnWriteFileEx, AFUNPTR(myWriteFileEx)));
            fptrWriteFileEx = (WIND::BOOL(WINAPI*)(WIND::HANDLE, __in_bcount(nNumberOfBytesToWrite) WIND::LPCVOID, WIND::DWORD,
                                                   WIND::LPOVERLAPPED, WIND::LPOVERLAPPED_COMPLETION_ROUTINE))fptr;
        }
    }

    if ((IMG_Name(img).find("rpcns4.dll") != string::npos) || (IMG_Name(img).find("RPCNS4.DLL") != string::npos) ||
        (IMG_Name(img).find("RPCNS4.dll") != string::npos))

    {
        RTN rtnRpcNsBindingImportBeginA = RTN_FindByName(img, "RpcNsBindingImportBeginA");
        if (RTN_Valid(rtnRpcNsBindingImportBeginA) && RTN_IsSafeForProbedReplacement(rtnRpcNsBindingImportBeginA))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for RpcNsBindingImportBeginA at " << RTN_Address(rtnRpcNsBindingImportBeginA) << endl;
            OutFile.flush();
            AFUNPTR fptr                 = (RTN_ReplaceProbed(rtnRpcNsBindingImportBeginA, AFUNPTR(myRpcNsBindingImportBeginA)));
            fptrRpcNsBindingImportBeginA = (WIND::RPC_STATUS(WINAPI*)(unsigned long, WIND::RPC_CSTR, WIND::RPC_IF_HANDLE,
                                                                      WIND::UUID __RPC_FAR*, WIND::RPC_NS_HANDLE __RPC_FAR*))fptr;
        }
    }

    if ((IMG_Name(img).find("kernel32.dll") != string::npos) || (IMG_Name(img).find("KERNEL32.DLL") != string::npos) ||
        (IMG_Name(img).find("KERNEL32.dll") != string::npos))

    {
        RTN rtnPulseEvent = RTN_FindByName(img, "PulseEvent");
        if (RTN_Valid(rtnPulseEvent) && RTN_IsSafeForProbedReplacement(rtnPulseEvent))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for PulseEvent at " << RTN_Address(rtnPulseEvent) << endl;
            OutFile.flush();
            AFUNPTR fptr   = (RTN_ReplaceProbed(rtnPulseEvent, AFUNPTR(myPulseEvent)));
            fptrPulseEvent = (WIND::BOOL(WINAPI*)(WIND::HANDLE))fptr;
        }
    }

    if ((IMG_Name(img).find("rpcrt4.dll") != string::npos) || (IMG_Name(img).find("RPCRT4.DLL") != string::npos) ||
        (IMG_Name(img).find("RPCRT4.dll") != string::npos))

    {
        RTN rtnRpcCancelThread = RTN_FindByName(img, "RpcCancelThread");
        if (RTN_Valid(rtnRpcCancelThread) && RTN_IsSafeForProbedReplacement(rtnRpcCancelThread))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for RpcCancelThread at " << RTN_Address(rtnRpcCancelThread) << endl;
            OutFile.flush();
            AFUNPTR fptr        = (RTN_ReplaceProbed(rtnRpcCancelThread, AFUNPTR(myRpcCancelThread)));
            fptrRpcCancelThread = (WIND::RPC_STATUS(WINAPI*)(void*))fptr;
        }
    }

    if ((IMG_Name(img).find("kernel32.dll") != string::npos) || (IMG_Name(img).find("KERNEL32.DLL") != string::npos) ||
        (IMG_Name(img).find("KERNEL32.dll") != string::npos))

    {
        RTN rtnCloseHandle = RTN_FindByName(img, "CloseHandle");
        if (RTN_Valid(rtnCloseHandle) && RTN_IsSafeForProbedReplacement(rtnCloseHandle))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for CloseHandle at " << RTN_Address(rtnCloseHandle) << endl;
            OutFile.flush();
            AFUNPTR fptr    = (RTN_ReplaceProbed(rtnCloseHandle, AFUNPTR(myCloseHandle)));
            fptrCloseHandle = (WIND::BOOL(WINAPI*)(WIND::HANDLE))fptr;
        }
    }

    if ((IMG_Name(img).find("kernel32.dll") != string::npos) || (IMG_Name(img).find("KERNEL32.DLL") != string::npos) ||
        (IMG_Name(img).find("KERNEL32.dll") != string::npos))

    {
        RTN rtnOpenMutexW = RTN_FindByName(img, "OpenMutexW");
        if (RTN_Valid(rtnOpenMutexW) && RTN_IsSafeForProbedReplacement(rtnOpenMutexW))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for OpenMutexW at " << RTN_Address(rtnOpenMutexW) << endl;
            OutFile.flush();
            AFUNPTR fptr   = (RTN_ReplaceProbed(rtnOpenMutexW, AFUNPTR(myOpenMutexW)));
            fptrOpenMutexW = (WIND::HANDLE(WINAPI*)(WIND::DWORD, WIND::BOOL, WIND::LPCWSTR))fptr;
        }
    }

    if ((IMG_Name(img).find("user32.dll") != string::npos) || (IMG_Name(img).find("USER32.DLL") != string::npos) ||
        (IMG_Name(img).find("USER32.dll") != string::npos))

    {
        RTN rtnPostMessageW = RTN_FindByName(img, "PostMessageW");
        if (RTN_Valid(rtnPostMessageW) && RTN_IsSafeForProbedReplacement(rtnPostMessageW))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for PostMessageW at " << RTN_Address(rtnPostMessageW) << endl;
            OutFile.flush();
            AFUNPTR fptr     = (RTN_ReplaceProbed(rtnPostMessageW, AFUNPTR(myPostMessageW)));
            fptrPostMessageW = (WIND::BOOL(WINAPI*)(WIND::HWND, WIND::UINT, WIND::WPARAM, WIND::LPARAM))fptr;
        }
    }

    if ((IMG_Name(img).find("rpcrt4.dll") != string::npos) || (IMG_Name(img).find("RPCRT4.DLL") != string::npos) ||
        (IMG_Name(img).find("RPCRT4.dll") != string::npos))

    {
        RTN rtnRpcEpResolveBinding = RTN_FindByName(img, "RpcEpResolveBinding");
        if (RTN_Valid(rtnRpcEpResolveBinding) && RTN_IsSafeForProbedReplacement(rtnRpcEpResolveBinding))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for RpcEpResolveBinding at " << RTN_Address(rtnRpcEpResolveBinding) << endl;
            OutFile.flush();
            AFUNPTR fptr            = (RTN_ReplaceProbed(rtnRpcEpResolveBinding, AFUNPTR(myRpcEpResolveBinding)));
            fptrRpcEpResolveBinding = (WIND::RPC_STATUS(WINAPI*)(WIND::RPC_BINDING_HANDLE, WIND::RPC_IF_HANDLE))fptr;
        }
    }

    if ((IMG_Name(img).find("kernel32.dll") != string::npos) || (IMG_Name(img).find("KERNEL32.DLL") != string::npos) ||
        (IMG_Name(img).find("KERNEL32.dll") != string::npos))

    {
        RTN rtnBindIoCompletionCallback = RTN_FindByName(img, "BindIoCompletionCallback");
        if (RTN_Valid(rtnBindIoCompletionCallback) && RTN_IsSafeForProbedReplacement(rtnBindIoCompletionCallback))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for BindIoCompletionCallback at " << RTN_Address(rtnBindIoCompletionCallback) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnBindIoCompletionCallback, AFUNPTR(myBindIoCompletionCallback)));
            fptrBindIoCompletionCallback =
                (WIND::BOOL(WINAPI*)(WIND::HANDLE, WIND::LPOVERLAPPED_COMPLETION_ROUTINE, WIND::ULONG))fptr;
        }
    }

    if ((IMG_Name(img).find("user32.dll") != string::npos) || (IMG_Name(img).find("USER32.DLL") != string::npos) ||
        (IMG_Name(img).find("USER32.dll") != string::npos))

    {
        RTN rtnPeekMessageW = RTN_FindByName(img, "PeekMessageW");
        if (RTN_Valid(rtnPeekMessageW) && RTN_IsSafeForProbedReplacement(rtnPeekMessageW))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for PeekMessageW at " << RTN_Address(rtnPeekMessageW) << endl;
            OutFile.flush();
            AFUNPTR fptr     = (RTN_ReplaceProbed(rtnPeekMessageW, AFUNPTR(myPeekMessageW)));
            fptrPeekMessageW = (WIND::BOOL(WINAPI*)(WIND::LPMSG, WIND::HWND, WIND::UINT, WIND::UINT, WIND::UINT))fptr;
        }
    }

    if ((IMG_Name(img).find("user32.dll") != string::npos) || (IMG_Name(img).find("USER32.DLL") != string::npos) ||
        (IMG_Name(img).find("USER32.dll") != string::npos))

    {
        RTN rtnSendMessageTimeoutA = RTN_FindByName(img, "SendMessageTimeoutA");
        if (RTN_Valid(rtnSendMessageTimeoutA) && RTN_IsSafeForProbedReplacement(rtnSendMessageTimeoutA))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for SendMessageTimeoutA at " << RTN_Address(rtnSendMessageTimeoutA) << endl;
            OutFile.flush();
            AFUNPTR fptr            = (RTN_ReplaceProbed(rtnSendMessageTimeoutA, AFUNPTR(mySendMessageTimeoutA)));
            fptrSendMessageTimeoutA = (WIND::LRESULT(WINAPI*)(WIND::HWND, WIND::UINT, WIND::WPARAM, WIND::LPARAM, WIND::UINT,
                                                              WIND::UINT, WIND::PDWORD_PTR))fptr;
        }
    }

    if ((IMG_Name(img).find("kernel32.dll") != string::npos) || (IMG_Name(img).find("KERNEL32.DLL") != string::npos) ||
        (IMG_Name(img).find("KERNEL32.dll") != string::npos))

    {
        RTN rtnOpenEventA = RTN_FindByName(img, "OpenEventA");
        if (RTN_Valid(rtnOpenEventA) && RTN_IsSafeForProbedReplacement(rtnOpenEventA))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for OpenEventA at " << RTN_Address(rtnOpenEventA) << endl;
            OutFile.flush();
            AFUNPTR fptr   = (RTN_ReplaceProbed(rtnOpenEventA, AFUNPTR(myOpenEventA)));
            fptrOpenEventA = (WIND::HANDLE(WINAPI*)(WIND::DWORD, WIND::BOOL, WIND::LPCSTR))fptr;
        }
    }

    if ((IMG_Name(img).find("kernel32.dll") != string::npos) || (IMG_Name(img).find("KERNEL32.DLL") != string::npos) ||
        (IMG_Name(img).find("KERNEL32.dll") != string::npos))

    {
        RTN rtnCreateProcessA = RTN_FindByName(img, "CreateProcessA");
        if (RTN_Valid(rtnCreateProcessA) && RTN_IsSafeForProbedReplacement(rtnCreateProcessA))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for CreateProcessA at " << RTN_Address(rtnCreateProcessA) << endl;
            OutFile.flush();
            AFUNPTR fptr       = (RTN_ReplaceProbed(rtnCreateProcessA, AFUNPTR(myCreateProcessA)));
            fptrCreateProcessA = (WIND::BOOL(WINAPI*)(WIND::LPCSTR, WIND::LPSTR, WIND::LPSECURITY_ATTRIBUTES,
                                                      WIND::LPSECURITY_ATTRIBUTES, WIND::BOOL, WIND::DWORD, WIND::LPVOID,
                                                      WIND::LPCSTR, WIND::LPSTARTUPINFOA, WIND::LPPROCESS_INFORMATION))fptr;
        }
    }

    if ((IMG_Name(img).find("kernel32.dll") != string::npos) || (IMG_Name(img).find("KERNEL32.DLL") != string::npos) ||
        (IMG_Name(img).find("KERNEL32.dll") != string::npos))

    {
        RTN rtnOpenEventW = RTN_FindByName(img, "OpenEventW");
        if (RTN_Valid(rtnOpenEventW) && RTN_IsSafeForProbedReplacement(rtnOpenEventW))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for OpenEventW at " << RTN_Address(rtnOpenEventW) << endl;
            OutFile.flush();
            AFUNPTR fptr   = (RTN_ReplaceProbed(rtnOpenEventW, AFUNPTR(myOpenEventW)));
            fptrOpenEventW = (WIND::HANDLE(WINAPI*)(WIND::DWORD, WIND::BOOL, WIND::LPCWSTR))fptr;
        }
    }

    if ((IMG_Name(img).find("kernel32.dll") != string::npos) || (IMG_Name(img).find("KERNEL32.DLL") != string::npos) ||
        (IMG_Name(img).find("KERNEL32.dll") != string::npos))

    {
        RTN rtnPostQueuedCompletionStatus = RTN_FindByName(img, "PostQueuedCompletionStatus");
        if (RTN_Valid(rtnPostQueuedCompletionStatus) && RTN_IsSafeForProbedReplacement(rtnPostQueuedCompletionStatus))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for PostQueuedCompletionStatus at " << RTN_Address(rtnPostQueuedCompletionStatus) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnPostQueuedCompletionStatus, AFUNPTR(myPostQueuedCompletionStatus)));
            fptrPostQueuedCompletionStatus =
                (WIND::BOOL(WINAPI*)(WIND::HANDLE, WIND::DWORD, WIND::ULONG_PTR, WIND::LPOVERLAPPED))fptr;
        }
    }

    if ((IMG_Name(img).find("kernel32.dll") != string::npos) || (IMG_Name(img).find("KERNEL32.DLL") != string::npos) ||
        (IMG_Name(img).find("KERNEL32.dll") != string::npos))

    {
        RTN rtnCreateProcessW = RTN_FindByName(img, "CreateProcessW");
        if (RTN_Valid(rtnCreateProcessW) && RTN_IsSafeForProbedReplacement(rtnCreateProcessW))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for CreateProcessW at " << RTN_Address(rtnCreateProcessW) << endl;
            OutFile.flush();
            AFUNPTR fptr       = (RTN_ReplaceProbed(rtnCreateProcessW, AFUNPTR(myCreateProcessW)));
            fptrCreateProcessW = (WIND::BOOL(WINAPI*)(WIND::LPCWSTR, WIND::LPWSTR, WIND::LPSECURITY_ATTRIBUTES,
                                                      WIND::LPSECURITY_ATTRIBUTES, WIND::BOOL, WIND::DWORD, WIND::LPVOID,
                                                      WIND::LPCWSTR, WIND::LPSTARTUPINFOW, WIND::LPPROCESS_INFORMATION))fptr;
        }
    }

    if ((IMG_Name(img).find("kernel32.dll") != string::npos) || (IMG_Name(img).find("KERNEL32.DLL") != string::npos) ||
        (IMG_Name(img).find("KERNEL32.dll") != string::npos))

    {
        RTN rtnTransactNamedPipe = RTN_FindByName(img, "TransactNamedPipe");
        if (RTN_Valid(rtnTransactNamedPipe) && RTN_IsSafeForProbedReplacement(rtnTransactNamedPipe))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for TransactNamedPipe at " << RTN_Address(rtnTransactNamedPipe) << endl;
            OutFile.flush();
            AFUNPTR fptr          = (RTN_ReplaceProbed(rtnTransactNamedPipe, AFUNPTR(myTransactNamedPipe)));
            fptrTransactNamedPipe = (WIND::BOOL(WINAPI*)(WIND::HANDLE, __in_bcount_opt(nInBufferSize) WIND::LPVOID, WIND::DWORD,
                                                         __out_bcount_part_opt(nOutBufferSize, *lpBytesRead) WIND::LPVOID,
                                                         WIND::DWORD, WIND::LPDWORD, WIND::LPOVERLAPPED))fptr;
        }
    }

    if ((IMG_Name(img).find("kernel32.dll") != string::npos) || (IMG_Name(img).find("KERNEL32.DLL") != string::npos) ||
        (IMG_Name(img).find("KERNEL32.dll") != string::npos))

    {
        RTN rtnReadFileEx = RTN_FindByName(img, "ReadFileEx");
        if (RTN_Valid(rtnReadFileEx) && RTN_IsSafeForProbedReplacement(rtnReadFileEx))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for ReadFileEx at " << RTN_Address(rtnReadFileEx) << endl;
            OutFile.flush();
            AFUNPTR fptr   = (RTN_ReplaceProbed(rtnReadFileEx, AFUNPTR(myReadFileEx)));
            fptrReadFileEx = (WIND::BOOL(WINAPI*)(WIND::HANDLE, __out_bcount(nNumberOfBytesToRead) WIND::LPVOID, WIND::DWORD,
                                                  WIND::LPOVERLAPPED, WIND::LPOVERLAPPED_COMPLETION_ROUTINE))fptr;
        }
    }

    if ((IMG_Name(img).find("kernel32.dll") != string::npos) || (IMG_Name(img).find("KERNEL32.DLL") != string::npos) ||
        (IMG_Name(img).find("KERNEL32.dll") != string::npos))

    {
        RTN rtnOpenSemaphoreW = RTN_FindByName(img, "OpenSemaphoreW");
        if (RTN_Valid(rtnOpenSemaphoreW) && RTN_IsSafeForProbedReplacement(rtnOpenSemaphoreW))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for OpenSemaphoreW at " << RTN_Address(rtnOpenSemaphoreW) << endl;
            OutFile.flush();
            AFUNPTR fptr       = (RTN_ReplaceProbed(rtnOpenSemaphoreW, AFUNPTR(myOpenSemaphoreW)));
            fptrOpenSemaphoreW = (WIND::HANDLE(WINAPI*)(WIND::DWORD, WIND::BOOL, WIND::LPCWSTR))fptr;
        }
    }

    if ((IMG_Name(img).find("user32.dll") != string::npos) || (IMG_Name(img).find("USER32.DLL") != string::npos) ||
        (IMG_Name(img).find("USER32.dll") != string::npos))

    {
        RTN rtnReplyMessage = RTN_FindByName(img, "ReplyMessage");
        if (RTN_Valid(rtnReplyMessage) && RTN_IsSafeForProbedReplacement(rtnReplyMessage))
        {
            OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                    << "Inserting probe for ReplyMessage at " << RTN_Address(rtnReplyMessage) << endl;
            OutFile.flush();
            AFUNPTR fptr     = (RTN_ReplaceProbed(rtnReplyMessage, AFUNPTR(myReplyMessage)));
            fptrReplyMessage = (WIND::BOOL(WINAPI*)(WIND::LRESULT))fptr;
        }
    }

    {
        const string sFuncName("DoLoop");

        for (SYM sym = IMG_RegsymHead(img); SYM_Valid(sym); sym = SYM_Next(sym))
        {
            string undFuncName = PIN_UndecorateSymbolName(SYM_Name(sym), UNDECORATION_NAME_ONLY);
            if (undFuncName == sFuncName)
            {
                RTN rtnDoLoop = RTN_FindByAddress(IMG_LowAddress(img) + SYM_Value(sym));
                if (RTN_Valid(rtnDoLoop) && RTN_IsSafeForProbedReplacement(rtnDoLoop))
                {
                    OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
                            << "Inserting probe for DoLoop at " << RTN_Address(rtnDoLoop) << endl;
                    OutFile.flush();
                    //eventhough this is not an error - print to cerr (in order to see it on the screen)
                    cerr << "Inserting a probe in DoLoop() in " << IMG_Name(img) << endl;
                    RTN_ReplaceProbed(rtnDoLoop, AFUNPTR(rep_DoLoop));
                }
            }
        }
    }

    WIND::TlsSetValue(dwIndexTls, (WIND::LPVOID)0);

#if defined(TARGET_IA32E) && __ICL == 1010
    // Workaround for test failure under ICC 10.1 - restore rbx.
    __asm mov rbx, rbx_save
#endif
}

VOID AttachMain(VOID* v);

VOID DetachComplete(VOID* v)
{
    std::cerr << "detach completed---";

    PIN_AttachProbed(AttachMain, 0);
}

VOID AppStart(VOID* v)
{
    std::cerr << "application started, # attach cycles = " << attachCycles << "---";
    if (attachCycles < KnobStressDetachReAttach.Value())
    {
        attachCycles++;
        PIN_DetachProbed();
    }
}

VOID AttachMain(VOID* v)
{
    globalCounter = 0;
    doLoopPred    = 1;

    IMG_AddInstrumentFunction(ImgLoad, 0);

    PIN_AddApplicationStartFunction(AppStart, 0);

    PIN_AddDetachFunctionProbed(DetachComplete, 0);

    std::cerr << "attach completed---";
}

int main(int argc, char* argv[])

{
    PIN_InitSymbols();

    if (PIN_Init(argc, argv))
    {
        return Usage();
    }

    OutFile.open(KnobOutputFile.Value().c_str());
    OutFile << hex;
    OutFile.setf(ios::showbase);
    OutFile << CurrentTime() << " " << WIND::GetCurrentThreadId() << " "
            << "started!" << endl;
    OutFile.flush();

    //allocate space on tls for our use
    if ((dwIndexTls = WIND::TlsAlloc()) == WIND::tls_out_of_indexes)
    {
        OutFile << "TlsAlloc failed" << endl;
        return 1;
    }

    IMG_AddInstrumentFunction(ImgLoad, 0);

    PIN_AddApplicationStartFunction(AppStart, 0);

    PIN_AddDetachFunctionProbed(DetachComplete, 0);

    PIN_StartProgramProbed();

    return 0;
}
