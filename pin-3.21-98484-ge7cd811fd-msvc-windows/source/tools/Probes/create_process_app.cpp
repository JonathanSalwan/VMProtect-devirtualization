/*
 * Copyright (C) 2008-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

// Application that creates new process

#include <Windows.h>
#include <iostream>
#include <string>
using std::cout;
using std::endl;
using std::string;
//Wait for a process completion
//Verify it returned the expected exit code
bool WaitAndVerify(HANDLE process)
{
    if (WaitForSingleObject(process, INFINITE) == WAIT_FAILED)
    {
        cout << "WaitForSingleObject failed" << endl;
        return FALSE;
    }
    DWORD processExitCode;
    if (GetExitCodeProcess(process, &processExitCode) == FALSE)
    {
        cout << "GetExitCodeProcess Failed" << endl;
        return FALSE;
    }
    if (processExitCode != 0)
    {
        cout << "Got unexpected exit code" << endl;
        return FALSE;
    }
    return TRUE;
}

int main()
{
    char cmd1[] = "win_child_process.exe \"param1 param2\" param3"; //"dir create*";
    STARTUPINFO startupInfo;
    PROCESS_INFORMATION processInfo;
    memset(&startupInfo, 0, sizeof(startupInfo));
    startupInfo.cb = sizeof(STARTUPINFO);
    memset(&processInfo, 0, sizeof(processInfo));

    if (!CreateProcess(NULL, cmd1, NULL, NULL, TRUE, NULL, NULL, NULL, &startupInfo, &processInfo))
    {
        cout << "CreateProcess failed for " << string(cmd1) << endl;
        exit(0);
    }
    if (WaitAndVerify(processInfo.hProcess) == FALSE)
    {
        exit(1);
    }
    return 0;
}
