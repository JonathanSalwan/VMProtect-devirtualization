/*
 * Copyright (C) 2021-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <iostream>
#include <iomanip>
#include <windows.h>

using namespace std;

/*
 * This application tests memory allocation and protection via broker.
 * It calls SetProcessMitigationPolicy, which restricts allocating pages with executable permissions, or
 * granting executable permissions to an existing page.
 * If this application finishes, it means Pin successfully allocated memory AND changed protection via broker.
 */
int main()
{
    const int sleepMiliSeconds = 10000;

    PROCESS_MITIGATION_DYNAMIC_CODE_POLICY processMitigationObject;
    processMitigationObject.Flags               = 0x0;
    processMitigationObject.ProhibitDynamicCode = 0x1;
    void* lpBuffer;
    size_t dwLength = sizeof(processMitigationObject);

    lpBuffer = static_cast< void* >(&processMitigationObject);
    bool res = SetProcessMitigationPolicy(ProcessDynamicCodePolicy, lpBuffer, dwLength);
    if (!res)
    {
        cerr << "SetProcessMitigationPolicy failed with 0x" << hex << GetLastError() << " exit status." << endl;
        exit(-2);
    }
    cout << "Going to sleep for " << sleepMiliSeconds << " miliseconds." << endl;
    Sleep(sleepMiliSeconds);

    return 0;
}
