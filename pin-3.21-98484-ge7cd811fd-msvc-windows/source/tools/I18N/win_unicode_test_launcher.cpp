/*
 * Copyright (C) 2007-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#define _CRT_SECURE_NO_DEPRECATE

#define UNICODE
#include <Windows.h>
#include <direct.h>
#include <iostream>
#include <string>
using std::cout;
using std::endl;
using std::wcout;
using std::wstring;
// launch types:
// 1. make <compiler> <compile flags> <source file name> <exe name>
//    - copy <source file name> to <unicode source file name>
//    - compile <unicode source file name>, executable name will be <exe name>
//    - delete all files it created except from the executable
// 2. test <path>/pin <pin flags + [pin tool]> -- <exe name>
//    - copy <exe name> to <unicode exe name>
//    - launch Pin
//    - delete the files it created

int make(int argc, wchar_t* argv[], wchar_t* envp[]);
int test(int argc, wchar_t* argv[], wchar_t* envp[]);

int wmain(int argc, wchar_t* argv[], wchar_t* envp[])
{
    if (_wcsicmp(argv[1], L"make") == 0)
    {
        return make(argc - 2, &argv[2], envp);
    }
    if (_wcsicmp(argv[1], L"test") == 0)
    {
        return test(argc - 2, &argv[2], envp);
    }

    wcout << "Bad arguments to Windows Unicode test launcher" << endl;
    return 0;
}

//Internationalization in Japanese (encoded in UTF16)
static const wchar_t i18n[] = {0x56fd, 0x969B, 0x5316, 0x0000};
static const wstring i18nStr(i18n);

// <compiler> <compile flags> <source file name>
int make(int argc, wchar_t* argv[], wchar_t* envp[])
{
    wstring compilerLine = L"";
    wstring env          = L"";
    for (int i = 0; envp[i] != NULL; i++)
    {
        env += wstring(L"set ") + wstring(envp[i]) + wstring(L"&");
    }
    //Copy all env. vars to the compiler cmd
    compilerLine += env;

    //Build Compiler command line
    for (int i = 0; i < argc - 1; i++)
    {
        compilerLine += wstring(argv[i]) + wstring(L" ");
    }

    // "copy <source file name> <Unicode file name>"
    wstring sourceFullFileName = argv[argc - 1];

    // split exeFullFileName into dirName and exeName
    size_t namePos  = sourceFullFileName.rfind(L'/') + 1;
    wstring srcName = sourceFullFileName.substr(namePos);
    wstring dirName = sourceFullFileName.substr(0, namePos);

    wstring newSourceFileName = wstring(L"prefix_") + i18nStr + wstring(L"_") + srcName;
    wstring copyStr =
        wstring(L"cd ") + dirName + wstring(L"&copy ") + srcName + wstring(L" ") + newSourceFileName + wstring(L"&cd ..");

    _wsystem(copyStr.c_str());

    // "<compiler> <compile flags> <Unicode file name>"
    compilerLine += wstring(L" ") + newSourceFileName;
    _wsystem(compilerLine.c_str());

    // "del <unicode name>.*"
    wstring delStr = wstring(L"del ") + newSourceFileName;
    _wsystem(delStr.c_str());

    return 0;
}

// <path>/pin <pin flags + [pin tool]> -- <exe name>
int test(int argc, wchar_t* argv[], wchar_t* envp[])
{
    wstring cmdLine = L"";

    // "copy <exe name> <unicode exe name>"
    // "copy <pdb name> <unicode pdb name>"
    wstring exeFullFileName = argv[argc - 1];

    // split exeFullFileName into dirName and exeName
    size_t namePos  = exeFullFileName.rfind(L'/') + 1;
    wstring exeName = exeFullFileName.substr(namePos);
    wstring dirName = exeFullFileName.substr(0, namePos);
    //exeFullFileName.replace()

    // compose new exe name that contains unicode chars
    wstring newExeName = wstring(L"prefix_") + i18nStr + wstring(L"_") + exeName;

    // copy original exe to unicode exe
    wstring copyStr = wstring(L"cd ") + dirName + wstring(L"&copy ") + exeName + wstring(L" ") + newExeName + wstring(L"&cd ..");
    _wsystem(copyStr.c_str());

    // take care for pdb file
    // remove .exe and add .pdb
    wstring baseName = exeName.substr(0, exeName.rfind(L".exe"));
    wstring pdbName  = baseName + wstring(L".pdb");

    wstring newBaseName = newExeName.substr(0, newExeName.rfind(L".exe"));
    wstring newPdbName  = newBaseName + wstring(L".pdb");

    // copy original pdb file to unicode pdb
    copyStr = wstring(L"cd ") + dirName + wstring(L"&copy ") + pdbName + wstring(L" ") + newPdbName + wstring(L"&cd ..");
    _wsystem(copyStr.c_str());

    // Build command line
    for (int i = 0; i < argc - 1; i++)
    {
        if ((i > 0) && (_wcsicmp(L"-uni_param", argv[i - 1])) == 0)
        {
            wstring newUnicodeParamName = dirName + newExeName;
            cmdLine += newUnicodeParamName + wstring(L" ");
            continue;
        }
        cmdLine += wstring(argv[i]) + wstring(L" ");
    }
    cmdLine += dirName + newExeName + wstring(L" ") + i18nStr;

    int ret = 0;

    // Invoke Pin
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    memset(&si, 0, sizeof(si));
    si.cb = sizeof(STARTUPINFO);
    memset(&pi, 0, sizeof(pi));
    // command line: "<path>/pin.exe -- <unicode name>.exe <unicode param>"
    if (!CreateProcess(NULL, (LPWSTR)cmdLine.c_str(), NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi))
    {
        cout << "Couldn't invoke pin" << endl;
        ret = 1;
    }
    else
    {
        WaitForSingleObject(pi.hProcess, INFINITE);
    }

    // "del <unicode name>.*"
    wstring delStr = wstring(L"cd ") + dirName + wstring(L"&del ") + newBaseName + wstring(L".* ") + wstring(L"& cd ..");
    _wsystem(delStr.c_str());

    return ret;
}
