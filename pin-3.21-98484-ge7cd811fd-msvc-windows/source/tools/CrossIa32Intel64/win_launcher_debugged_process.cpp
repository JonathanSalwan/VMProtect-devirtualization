/*
 * Copyright (C) 2009-2021 Intel Corporation.
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
bool WaitAndVerify(HANDLE process, HANDLE thread, DWORD pid)
{
    DebugSetProcessKillOnExit(FALSE);
    DEBUG_EVENT lastEvent;
    DWORD continueStatus = DBG_CONTINUE;
    bool stopDebug       = false;
    while (1)
    {
        if (stopDebug)
        {
            break;
        }

        WaitForDebugEvent(&lastEvent, INFINITE);
        switch (lastEvent.dwDebugEventCode)
        {
            case EXIT_PROCESS_DEBUG_EVENT:
            {
                stopDebug = true;
                break;
            }
        }
        ContinueDebugEvent(lastEvent.dwProcessId, lastEvent.dwThreadId, continueStatus);
    }
    DebugActiveProcessStop(pid);

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

/*
 * Split string into 2 sub strings.
 * Example1: prefix = SplitString(input = "aaa bbb ccc", " ") ->
 *           prefix = "aaa", input = "bbb ccc"
 * Example2: prefix = SplitString(input = "aaa", " ") ->
 *           prefix = "aaa", input = ""
 *
 * [inout]  input  - input string. remove prefix and set the new string into input.
 * [in]     delimiter - delimiter 
 *                
 * return prefix string
 */
string SplitString(string* input, const string& delimiter = " ")
{
    string::size_type pos = input->find(delimiter);
    string substr         = input->substr(0, pos);
    if (pos != string::npos)
    {
        *input = input->substr(pos + 1);
    }
    else
    {
        *input = "";
    }
    return substr;
}

int main(int argc, char* argv[])
{
    string cmdLine = GetCommandLine();
    SplitString(&cmdLine);

    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    {
        memset(&si, 0, sizeof(si));
        si.cb = sizeof(STARTUPINFO);
        memset(&pi, 0, sizeof(pi));
        if (!CreateProcess(NULL, (LPSTR)cmdLine.c_str(), NULL, NULL, TRUE, DEBUG_ONLY_THIS_PROCESS, NULL, NULL, &si, &pi))
        {
            cout << "Couldn't create child process, command line = " << cmdLine << ", system error =  " << GetLastError() << endl;
            exit(-1);
        }
        if (WaitAndVerify(pi.hProcess, pi.hThread, pi.dwProcessId) == FALSE)
        {
            exit(-1);
        }
        cout << "First Child Process was created successfully!" << endl;
    }

    return 0;
}
