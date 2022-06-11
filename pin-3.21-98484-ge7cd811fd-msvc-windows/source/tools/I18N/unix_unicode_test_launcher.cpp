/*
 * Copyright (C) 2007-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <sys/wait.h>
using std::cerr;
using std::cout;
using std::endl;
using std::string;

// launch types:
// 1. make <compiler> <compile flags> <source file name> <exe name>
//    - copy <source file name> to <unicode source file name>
//    - compile <unicode source file name>, executable name will be <exe name>
//    - delete all files it created except from the executable
// 2. test <path>/pin <pin flags + [pin tool]> -- <exe name>
//    - copy <exe name> to <unicode exe name>
//    - launch Pin
//    - delete the files it created

int make(int argc, char* argv[]);
int test(int argc, char* argv[], char* envp[]);

int main(int argc, char* argv[], char* envp[])
{
    if (strcmp(argv[1], "make") == 0)
    {
        return make(argc - 2, &argv[2]);
    }
    if (strcmp(argv[1], "test") == 0)
    {
        return test(argc - 2, &argv[2], envp);
    }

    cout << "Bad arguments to Linux Unicode test launcher" << endl;
    return 0;
}

//internationalization in Japanese (encoded in UTF-8)
static char i18n[] = {(char)0xE5, (char)0x9B, (char)0xBD, (char)0xE9, (char)0x9A,
                      (char)0x9B, (char)0xE5, (char)0x8C, (char)0x96, (char)0x00};
static string i18nStr(i18n);

// <compiler> <compile flags> <source file name>
int make(int argc, char* argv[])
{
    string compilerLine = "";

    //Build Compiler command line
    for (int i = 0; i < argc - 1; i++)
    {
        compilerLine += string(argv[i]) + string(" ");
    }

    // "cp <source file name> <Unicode file name>"
    string sourceFileName = argv[argc - 1];

    string newSourceFileName = string("prefix_") + i18nStr + string("_") + sourceFileName;
    string copyStr           = string("cp ") + sourceFileName + string(" ") + newSourceFileName;
    system(copyStr.c_str());

    // "<compiler> <compile flags> <Unicode file name>"
    compilerLine += string(" ") + newSourceFileName;
    system(compilerLine.c_str());

    // "del <unicode name>.*"
    string delStr = string("rm -f ") + newSourceFileName;

    system(delStr.c_str());

    return 0;
}

// <path>/pin <pin flags + [pin tool]> -- <exe name>
int test(int argc, char* argv[], char* envp[])
{
    int unicodeParamIndex = 0;
    char** exeParams      = new char*[argc + 2];

    string newUnicodeParamName;
    string exeName;
    string newExeName;

    string exeFullName  = argv[argc - 1];
    size_t namePos      = exeFullName.rfind("/") + 1;
    string exeDir       = exeFullName.substr(0, namePos);
    exeName             = exeFullName.substr(namePos);
    newExeName          = exeDir + string("prefix_") + i18nStr + string("_") + exeName;
    exeParams[argc - 1] = (char*)newExeName.c_str();
    exeParams[argc]     = (char*)i18nStr.c_str();
    exeParams[argc + 1] = NULL;
    // "cp <original name> <unicode name>"
    string copyStr = string("cp ") + exeFullName + string(" ") + newExeName;
    system(copyStr.c_str());

    // Build command line
    for (int i = 0; i < argc - 1; i++)
    {
        exeParams[i] = argv[i];
        if (strcmp("-uni_param", argv[i]) == 0)
        {
            unicodeParamIndex = i + 1;
        }
    }

    if (unicodeParamIndex != 0)
    {
        newUnicodeParamName          = newExeName;
        exeParams[unicodeParamIndex] = (char*)newUnicodeParamName.c_str();
    }

    int ret = 0;

    pid_t pid = fork();
    //child
    if (pid == 0)
    {
        //invoke pin
        int res = execve(exeParams[0], exeParams, envp);
        cerr << "failed to exec, status = " << res << endl;
        ret = 1;
    }
    if (pid < 0)
    {
        cout << "failed to fork, status = " << pid << endl;
        ret = 1;
    }

    int status = 0;
    wait(&status);
    string delStr = string("rm -f ") + newExeName;

    // "rm -f <unicode name>"
    system(delStr.c_str());
    return ret;
}
