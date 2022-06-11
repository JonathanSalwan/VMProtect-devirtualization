/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#define UNICODE
#define _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_DEPRECATE

#include <Windows.h>
#include <Winsvc.h>
#include <string>
#include <iostream>
#include <fstream>

using std::cerr;
using std::cout;
using std::endl;
using std::ofstream;
using std::string;
using std::wstring;

static wchar_t* ServiceNameW        = L"ServiceAppForPinTesting";
static wchar_t* ServiceDisplayNameW = L"Service App For Pin Testing";
static wchar_t* BinaryNameW         = L"w_service_app1.exe";

// perform administrative actions, this class is used
// when the process runs as regular process (not as service)
class SERVICE_ADMIN
{
  public:
    static void Main(int argc, wchar_t* argv[]);

  private:
    BOOL Create();
    BOOL Delete();
    DWORD Start(DWORD argc, LPCTSTR* argv);
    BOOL Stop();

    VOID Usage();
};

// class to be used when the process runs as service
class SERVICE_MANAGER
{
  public:
    static void WINAPI Main(DWORD argc, LPTSTR* argv);
    static void WINAPI ControlHandler(DWORD opcode);
    static void StartServiceTimer(DWORD milliSec);
    static void StopService(); //never returns
    static void OpenFile();

  private:
    static DWORD WINAPI ServiceTimerProc(VOID* p);
    static void LaunchProcess(wstring cmdLine);

    static SERVICE_STATUS status;
    static SERVICE_STATUS_HANDLE statusHandle;
    static ofstream outFile;
};

SERVICE_STATUS SERVICE_MANAGER::status;
SERVICE_STATUS_HANDLE SERVICE_MANAGER::statusHandle;
ofstream SERVICE_MANAGER::outFile;

// main function

int wmain(int argc, wchar_t* argv[])
{
    if ((argc >= 2) && ((wcscmp(argv[1], L"-admin") == 0) || (wcscmp(argv[1], L"-help") == 0)))
    {
        SERVICE_ADMIN::Main(argc, argv);
    }
    else
    {
        SERVICE_MANAGER::OpenFile();
        SERVICE_TABLE_ENTRY dispatchData[] = {{ServiceNameW, SERVICE_MANAGER::Main}, {NULL, NULL}};
        StartServiceCtrlDispatcher(dispatchData);
    }
    return 0;
}

//manager impl

void SERVICE_MANAGER::LaunchProcess(wstring cmdLine)
{
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    memset(&si, 0, sizeof(si));
    si.cb = sizeof(STARTUPINFO);
    memset(&pi, 0, sizeof(pi));

    if (!CreateProcess(NULL, (LPWSTR)cmdLine.c_str(), NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi))
    {
        outFile << "Failed to CreateProcess, " << GetLastError() << endl;
    }
    return;
}

__declspec(dllexport) int DoLoop() { return 1; }

__declspec(dllexport) void ShortFunction1(DWORD h)
{
    volatile DWORD i = h;
    Sleep(1);
    if (i != 1)
    {
        exit(-1);
    }
}

__declspec(dllexport) void ShortFunction2(DWORD h)
{
    volatile DWORD i = h;
    Sleep(1);
    if (i != 2)
    {
        exit(-1);
    }
}

void WINAPI SERVICE_MANAGER::Main(DWORD argc, LPTSTR* argv)
{
    status.dwServiceType             = SERVICE_WIN32;
    status.dwCurrentState            = SERVICE_START_PENDING;
    status.dwControlsAccepted        = SERVICE_ACCEPT_STOP;
    status.dwWin32ExitCode           = 0;
    status.dwServiceSpecificExitCode = 0;
    status.dwCheckPoint              = 0;
    status.dwWaitHint                = 0;

    statusHandle = RegisterServiceCtrlHandler(ServiceNameW, SERVICE_MANAGER::ControlHandler);
    if (statusHandle == (SERVICE_STATUS_HANDLE)NULL)
    {
        outFile << "Failed to RegisterServiceCtrlHandler, " << GetLastError() << endl;
        return;
    }

    status.dwCurrentState = SERVICE_RUNNING;
    status.dwCheckPoint   = 0;
    status.dwWaitHint     = 0;

    if (!SetServiceStatus(statusHandle, &status))
    {
        outFile << "Failed to SetServiceStatus, " << GetLastError() << endl;
        return;
    }

    //Will stop the service after 300 sec.
    StartServiceTimer(300 * 1000);

    while (DoLoop())
    {
        ShortFunction1(1);
        ShortFunction2(2);

        static int i = 0;
        i++;
        i          = i % 0x10;
        LPVOID aaa = VirtualAlloc(0, i * 0x1000000, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
        VirtualFree(aaa, 0, MEM_RELEASE);
    }

    outFile << "Success! someone changed DoLoop on time" << endl;
    outFile.flush();
    StopService();
    return;
}

void WINAPI SERVICE_MANAGER::ControlHandler(DWORD opcode)
{
    switch (opcode)
    {
        case SERVICE_CONTROL_PAUSE:
            status.dwCurrentState = SERVICE_PAUSED;
            break;
        case SERVICE_CONTROL_CONTINUE:
            status.dwCurrentState = SERVICE_RUNNING;
            break;
        case SERVICE_CONTROL_STOP:
            status.dwWin32ExitCode = 0;
            status.dwCurrentState  = SERVICE_STOPPED;
            status.dwCheckPoint    = 0;
            status.dwWaitHint      = 0;
            outFile.flush();
            outFile.close();
            SetServiceStatus(statusHandle, &status);
            break;
        case SERVICE_CONTROL_INTERROGATE:
            break;
    }
    return;
}

DWORD WINAPI SERVICE_MANAGER::ServiceTimerProc(VOID* p)
{
    DWORD* pMilliSec = (DWORD*)p;
    Sleep(*pMilliSec);
    delete pMilliSec;
    //never returns
    outFile << "Failure! DoLoop was not changed on time" << endl;
    SERVICE_MANAGER::StopService();
    return 0;
}

void SERVICE_MANAGER::StartServiceTimer(DWORD milliSec)
{
    DWORD* pMilliSec    = new DWORD(milliSec);
    HANDLE threadHandle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ServiceTimerProc, pMilliSec, 0, NULL);
    if (threadHandle == NULL)
    {
        outFile << "Failed to CreateThread, " << GetLastError() << endl;
    }
    return;
}

void SERVICE_MANAGER::StopService()
{
    wchar_t binPathArr[2048];
    GetModuleFileName(NULL, binPathArr, 2048);
    LPCTSTR binPath = binPathArr;
    wstring cmdLine = wstring(L"\"") + binPath + L"\"";
    cmdLine += L" -admin -stop";
    LaunchProcess(cmdLine);
}

void SERVICE_MANAGER::OpenFile()
{
    wchar_t binPathArr[2048];
    GetModuleFileName(NULL, binPathArr, 2048);

    size_t serviceExeNameLen       = wcslen(BinaryNameW);
    size_t servicePathNameLen      = wcslen(binPathArr) - serviceExeNameLen;
    binPathArr[servicePathNameLen] = L'\0';

    SetCurrentDirectory(binPathArr);

    wstring fileName = binPathArr;
    fileName += BinaryNameW;
    fileName += L".service.log";
    outFile.open(fileName.c_str());
}

//admin impl

void SERVICE_ADMIN::Main(int argc, wchar_t* argv[])
{
    SERVICE_ADMIN admin;

    if ((argc == 1) || ((argc == 2) && ((wcscmp(argv[1], L"-admin") == 0) || (wcscmp(argv[1], L"-help") == 0))))
    {
        return admin.Usage();
    }

    if (wcscmp(argv[2], L"-create") == 0)
    {
        if (admin.Create())
        {
            cerr << "Created service sucessfully!" << endl;
        }
        else
        {
            cerr << "Failed to Create service" << endl;
        }
    }
    else if (wcscmp(argv[2], L"-start") == 0)
    {
        DWORD pid = admin.Start((DWORD)(argc - 3), (LPCWSTR*)&argv[3]);
        if (pid != 0)
        {
            cerr << "Started service sucessfully!" << endl;
        }
        else
        {
            cerr << "Failed to start service" << endl;
        }

        char digitBuffer[64];
        cout << _itoa(pid, digitBuffer, 10);
    }
    else if (wcscmp(argv[2], L"-stop") == 0)
    {
        if (admin.Stop())
        {
            cerr << "Stopped service sucessfully!" << endl;
        }
        else
        {
            cerr << "Failed to stop service" << endl;
        }
    }
    else if (wcscmp(argv[2], L"-delete") == 0)
    {
        if (admin.Delete())
        {
            cerr << "Deleted service sucessfully!" << endl;
        }
        else
        {
            cerr << "Failed to delete service" << endl;
        }
    }
    else
    {
        admin.Usage();
    }

    return;
}

BOOL SERVICE_ADMIN::Create()
{
    SC_HANDLE manager;
    manager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (manager == NULL)
    {
        cerr << "Failed to OpenSCManager, " << GetLastError() << endl;
        return false;
    }

    wchar_t binPathArr[2048];
    GetModuleFileName(NULL, binPathArr, 2048);
    wstring path = binPathArr;
    path         = L"\"" + path + L"\"";

#if 0 // This code is intended for symbolic link substitution
    size_t slashLocation = path.find(L"\\");
    wstring drive = path.substr(0, slashLocation);
    path = path.substr(slashLocation);

    wchar_t targetPath[2048];
    QueryDosDevice(drive.c_str(), targetPath, 2048);
    path = wstring(targetPath) + path;
    if(path.find(L"\\??\\") == 0)
    {
        /* Remove \??\ */
        path = path.substr(4);
    }
    else if(path.find(L"\\DosDevices\\") == 0)
    {
        /* Remove \DosDevices\ */
        path = path.substr(12);
    }
#endif

    LPCTSTR binPath = path.c_str();

    SC_HANDLE service;
    service =
        CreateService(manager, ServiceNameW, ServiceDisplayNameW, SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS,
                      SERVICE_DEMAND_START, SERVICE_ERROR_NORMAL, binPath, NULL, NULL, NULL,
                      NULL, // log on as Local System, other options L"NT AUTHORITY\\LocalService" L"NT AUTHORITY\\NetworkService"
                      NULL); // no password
    if (service == NULL && (GetLastError() != ERROR_SERVICE_EXISTS))
    {
        cerr << "Failed to CreateService, " << GetLastError() << endl;
        CloseServiceHandle(manager);
        return false;
    }

    if (manager != NULL)
    {
        CloseServiceHandle(manager);
    }
    if (service != NULL)
    {
        CloseServiceHandle(service);
    }
    return true;
}

DWORD SERVICE_ADMIN::Start(DWORD argc, LPCWSTR* argv)
{
    SC_HANDLE manager;
    manager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (manager == NULL)
    {
        cerr << "Failed to OpenSCManager, " << GetLastError() << endl;
        return 0;
    }

    SC_HANDLE service;
    service = OpenService(manager, ServiceNameW, SERVICE_ALL_ACCESS);
    if (service == NULL)
    {
        cerr << "Failed to OpenService, " << GetLastError() << endl;
        CloseServiceHandle(manager);
        return 0;
    }

    if (!StartService(service, argc, argv))
    {
        cerr << "Failed to StartService, " << GetLastError() << endl;
        CloseServiceHandle(manager);
        CloseServiceHandle(service);
        return 0;
    }

    //maximum 8k
    SERVICE_STATUS_PROCESS* pServiceStatusProcess = (SERVICE_STATUS_PROCESS*)malloc(0x2000);
    pServiceStatusProcess->dwCurrentState         = SERVICE_START_PENDING;

    int timer = 60;
    while ((timer != 0) && (pServiceStatusProcess->dwCurrentState != SERVICE_RUNNING))
    {
        timer--;
        DWORD bytesNeeded = 0;
        if (!QueryServiceStatusEx(service, SC_STATUS_PROCESS_INFO, (LPBYTE)pServiceStatusProcess, 0x2000, &bytesNeeded))
        {
            cout << "Failed to QueryServiceStatusEx, " << GetLastError() << endl;
            CloseServiceHandle(manager);
            CloseServiceHandle(service);
            free(pServiceStatusProcess);
            return 0;
        }
        //Let the service start
        Sleep(1000);
    }

    if (pServiceStatusProcess->dwCurrentState != SERVICE_RUNNING)
    {
        cerr << "Failed, Service haven't started after 60 seconds" << endl;
        CloseServiceHandle(manager);
        CloseServiceHandle(service);
        free(pServiceStatusProcess);
        return 0;
    }

    CloseServiceHandle(manager);
    CloseServiceHandle(service);

    DWORD pid = pServiceStatusProcess->dwProcessId;
    free(pServiceStatusProcess);
    return pid;
}

BOOL SERVICE_ADMIN::Stop()
{
    SC_HANDLE manager;
    manager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (manager == NULL)
    {
        cerr << "Failed to OpenSCManager, " << GetLastError() << endl;
        return false;
    }

    SC_HANDLE service;
    service = OpenService(manager, ServiceNameW, SERVICE_ALL_ACCESS);
    if (service == NULL)
    {
        cerr << "Failed to OpenService, " << GetLastError() << endl;
        CloseServiceHandle(manager);
        return false;
    }

    SERVICE_STATUS status;
    BOOL res = ControlService(service, SERVICE_CONTROL_STOP, &status);
    if (!res && (GetLastError() != ERROR_SERVICE_NOT_ACTIVE))
    {
        cerr << "Failed to ControlService, " << GetLastError() << endl;
        CloseServiceHandle(manager);
        CloseServiceHandle(service);
        return false;
    }

    CloseServiceHandle(manager);
    CloseServiceHandle(service);
    return true;
}

BOOL SERVICE_ADMIN::Delete()
{
    SC_HANDLE manager;
    manager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (manager == NULL)
    {
        cout << "Failed to OpenSCManager, " << GetLastError() << endl;
        return false;
    }

    SC_HANDLE service;
    service = OpenService(manager, ServiceNameW, SERVICE_ALL_ACCESS);
    if (service == NULL)
    {
        cerr << "Failed to OpenService, " << GetLastError() << endl;
        CloseServiceHandle(manager);
        return false;
    }

    if (!DeleteService(service))
    {
        cerr << "Failed to DeleteService, " << GetLastError() << endl;
        CloseServiceHandle(manager);
        CloseServiceHandle(service);
        return false;
    }

    CloseServiceHandle(manager);
    CloseServiceHandle(service);
    return true;
}

void SERVICE_ADMIN::Usage()
{
    cerr << "Usage:" << endl;
    cerr << "To create the service: -admin -create" << endl;
    cerr << "To start the service:  -admin -start <service arguments>" << endl;
    cerr << "To start the service:  -admin -stop" << endl;
    cerr << "To delete the service: -admin -delete" << endl;
}
