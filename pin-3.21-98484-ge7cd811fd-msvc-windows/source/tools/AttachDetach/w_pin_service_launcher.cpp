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

static wchar_t* ServiceNameW        = L"PinLauncherService";
static wchar_t* ServiceDisplayNameW = L"Pin Launcher Service";
static wchar_t* BinaryNameW         = L"w_pin_service_launcher.exe";

// perform administrative actions, this class is used
// when the process runs as regular process (not as service)
class SERVICE_ADMIN
{
  public:
    static void Main(int argc, wchar_t* argv[]);

  private:
    BOOL Create();
    BOOL Delete();
    BOOL Start(DWORD argc, LPCTSTR* argv);
    BOOL Stop();

    void Usage();
};

// functions to be used when the process runs as service
class SERVICE_MANAGER
{
  public:
    static void WINAPI Main(DWORD argc, LPTSTR* argv);
    static void WINAPI ControlHandler(DWORD opcode);
    static void CreatePinProcessAndWait(DWORD argc, LPCWSTR* argv);
    static void StopService(); //never returns
    static void OpenFile();

  private:
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

void SERVICE_MANAGER::CreatePinProcessAndWait(DWORD argc, LPCWSTR* argv)
{
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    memset(&si, 0, sizeof(si));
    si.cb = sizeof(STARTUPINFO);
    memset(&pi, 0, sizeof(pi));

    wstring cmdLine = L"";
    for (DWORD i = 0; i < argc; i++)
    {
        cmdLine += wstring(L"\"") + argv[i] + L"\"";
        if (i < argc - 1)
        {
            cmdLine += L" ";
        }
    }

    if (!CreateProcess(NULL, (LPWSTR)cmdLine.c_str(), NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi))
    {
        outFile << "Failed to CreateProcess, " << GetLastError() << endl;
    }

    if (WaitForSingleObject(pi.hProcess, 60 * 1000) != WAIT_OBJECT_0)
    {
        outFile << "Pin didn't finish running after 60 sec. " << endl;
    }

    return;
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

    CreatePinProcessAndWait(argc - 1, (LPCWSTR*)&argv[1]);

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

void SERVICE_MANAGER::StopService()
{
    wchar_t binPathArr[2048];
    GetModuleFileName(NULL, binPathArr, 2048);
    LPCTSTR binPath = binPathArr;
    wstring cmdLine = wstring(L"\"") + binPath + L"\"";
    cmdLine += L" -admin -stop";
    LaunchProcess(cmdLine);
}

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
        if (admin.Start((DWORD)(argc - 3), (LPCWSTR*)&argv[3]))
        {
            cerr << "Started service sucessfully!" << endl;
        }
        else
        {
            cerr << "Failed to start service" << endl;
        }
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
    service = CreateService(manager, ServiceNameW, ServiceDisplayNameW, SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS,
                            SERVICE_DEMAND_START, SERVICE_ERROR_NORMAL, binPath, NULL, NULL, NULL,
                            NULL,  // log on as Local System
                            NULL); // no password
    if (service == NULL && (GetLastError() != ERROR_SERVICE_EXISTS))
    {
        cerr << "Failed to CreateService, " << GetLastError() << endl;
        CloseServiceHandle(manager);
        return false;
    }

    CloseServiceHandle(manager);
    CloseServiceHandle(service);
    return true;
}

BOOL SERVICE_ADMIN::Start(DWORD argc, LPCWSTR* argv)
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

    if (!StartService(service, argc, argv))
    {
        cerr << "Failed to StartService, " << GetLastError() << endl;
        CloseServiceHandle(manager);
        CloseServiceHandle(service);
        return false;
    }

    Sleep(12000);

    //maximum 8k
    SERVICE_STATUS_PROCESS* pServiceStatusProcess = (SERVICE_STATUS_PROCESS*)malloc(0x2000);
    pServiceStatusProcess->dwCurrentState         = SERVICE_START_PENDING;

    //let pin to do it's job, it has 60 seconds to complete
    int timer = 12;
    while ((timer != 0) && (pServiceStatusProcess->dwCurrentState != SERVICE_STOPPED))
    {
        timer--;
        DWORD bytesNeeded = 0;
        if (!QueryServiceStatusEx(service, SC_STATUS_PROCESS_INFO, (LPBYTE)pServiceStatusProcess, 0x2000, &bytesNeeded))
        {
            cout << "Failed to QueryServiceStatusEx, " << GetLastError() << endl;
            CloseServiceHandle(manager);
            CloseServiceHandle(service);
            free(pServiceStatusProcess);
            return false;
        }
        Sleep(5000);
    }

    if (pServiceStatusProcess->dwCurrentState != SERVICE_STOPPED)
    {
        cerr << "Service haven't stopped after 60 seconds" << endl;
        CloseServiceHandle(manager);
        CloseServiceHandle(service);
        free(pServiceStatusProcess);
        return false;
    }

    CloseServiceHandle(manager);
    CloseServiceHandle(service);
    free(pServiceStatusProcess);

    return true;
}

BOOL SERVICE_ADMIN::Stop()
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
        cout << "Failed to OpenService, " << GetLastError() << endl;
        CloseServiceHandle(manager);
        return false;
    }

    SERVICE_STATUS status;
    BOOL res = ControlService(service, SERVICE_CONTROL_STOP, &status);
    if (!res && (GetLastError() != ERROR_SERVICE_NOT_ACTIVE))
    {
        cout << "Failed to ControlService, " << GetLastError() << endl;
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
        cout << "Failed to OpenService, " << GetLastError() << endl;
        CloseServiceHandle(manager);
        return false;
    }

    if (!DeleteService(service))
    {
        cout << "Failed to DeleteService, " << GetLastError() << endl;
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
    cerr << "To start the service:  -admin -start <pin> <pin arguments>" << endl;
    cerr << "To start the service:  -admin -stop" << endl;
    cerr << "To delete the service: -admin -delete" << endl;
}
