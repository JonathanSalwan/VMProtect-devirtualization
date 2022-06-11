/*
 * Copyright (C) 2021-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include "pin.H"
#include <iostream>
#include <fstream>
#include <unistd.h>
namespace WIND
{
#include <windows.h>
}

using std::cout;
using std::endl;
using std::string;

CRT_DLLIMPORT extern CHAR** environ;

KNOB< string > KnobParentEnvLog(KNOB_MODE_OVERWRITE, "pintool", "parentEnvLog", "ParentEnvLog.log", "");
KNOB< string > KnobChildEnvLog(KNOB_MODE_OVERWRITE, "pintool", "childEnvLog", "ChildEnvLog.log", "");
KNOB< string > KnobGrandChildEnvLog(KNOB_MODE_OVERWRITE, "pintool", "grandChildEnvLog", "GrandChildEnvLog.log", "");

struct
{
    INT32 generation          = -1;
    CONST CHAR* strGeneration = "";

    CONST CHAR* ENV1_NAME  = "PARENT";
    CONST CHAR* ENV1_VALUE = "ANAKIN_SKYWALKER";

    CONST CHAR* ENV2_NAME  = "CHILD";
    CONST CHAR* ENV2_VALUE = "LEIA_ORGANA";

    CONST CHAR* ENV3_NAME  = "GRANDCHILD";
    CONST CHAR* ENV3_VALUE = "BEN_SOLO";

    WIND::HANDLE outputMutex;

} static Data;

// use mutex to synchronize outputs from multiple instances of this tool
VOID MutexWriteToStdout(CHAR* msg);

// Check that envName exists\doesn't exists in current environment
VOID CheckEnv(CONST CHAR* envName, BOOL shouldExists, CONST CHAR* envExpectedValue = NULL);

// Delete a file
VOID RemoveFile(CONST CHAR* file);

// Print all envrionment variables to a file
VOID PrintEnvironmentVariables(CONST CHAR* fileName);

// Safetly print message
VOID MutexWriteToStdout(CHAR* msg);

// Make some final checks before existing
VOID Fini(INT32 code, VOID* v);

// Functionality per generation
VOID IAmParent();
VOID IAmChild();
VOID IAmGrandChild();

VOID MutexWriteToStdout(CHAR* msg)
{
    WIND::WaitForSingleObject(Data.outputMutex, INFINITE);

    printf(msg);

    fflush(stdout);

    WIND::ReleaseMutex(Data.outputMutex);
}

VOID CheckEnv(CONST CHAR* envName, BOOL shouldExists, CONST CHAR* envExpectedValue)
{
    CONST CHAR* envActualVal = getenv(envName);

    if (shouldExists)
    {
        ASSERT(envActualVal, "\nGeneration " + Data.strGeneration + ":Environment variable " + envName + "Doesn't exists.");
        ASSERTX(envExpectedValue);
        ASSERT(!strcmp(envActualVal, envExpectedValue), string("") + "\nGeneration " + Data.strGeneration +
                                                            ":Environment variable " + envName +
                                                            "Incorrect value."
                                                            " Expected: " +
                                                            envExpectedValue + ", Actual: " + envActualVal);
    }
    else
    {
        ASSERT(!envActualVal, "\nGeneration " + Data.strGeneration + ":Environment variable " + envName + " exists.");
    }
}

VOID RemoveFile(CONST CHAR* file)
{
    // If file exists
    if (access(file, F_OK) == 0)
        // Try to remove it
        ASSERT(!remove(file), "Failed to remove the file: " + file);
}

VOID PrintEnvironmentVariables(CONST CHAR* fileName)
{
    std::ofstream file;
    file.open(fileName);
    ASSERT(!file.fail(), "Failed open fail: " + fileName);
    for (CHAR** entry = environ; *entry; entry++)
        file << *entry << "\n";
    file.close();
}

VOID IAmGrandChild()
{
    MutexWriteToStdout("\nI am grandchild");

    RemoveFile(KnobGrandChildEnvLog.Value().c_str());

    CheckEnv(Data.ENV1_NAME, TRUE, Data.ENV1_VALUE);
    CheckEnv(Data.ENV2_NAME, TRUE, Data.ENV2_VALUE);

    CheckEnv(Data.ENV3_NAME, FALSE);
    setenv(Data.ENV3_NAME, Data.ENV3_VALUE, 1);
    CheckEnv(Data.ENV3_NAME, TRUE, Data.ENV3_VALUE);

    PrintEnvironmentVariables(KnobGrandChildEnvLog.Value().c_str());
}

VOID IAmChild()
{
    MutexWriteToStdout("\nI am child");

    RemoveFile(KnobChildEnvLog.Value().c_str());

    CheckEnv(Data.ENV1_NAME, TRUE, Data.ENV1_VALUE);

    CheckEnv(Data.ENV2_NAME, FALSE);
    setenv(Data.ENV2_NAME, Data.ENV2_VALUE, 1);
    CheckEnv(Data.ENV2_NAME, TRUE, Data.ENV2_VALUE);

    CheckEnv(Data.ENV3_NAME, FALSE);

    PrintEnvironmentVariables(KnobChildEnvLog.Value().c_str());
}

// Call only on parent process
VOID Fini(INT32 code, VOID* v)
{
    // Make sure that the other processes hasn't changed the parent's environment
    CheckEnv(Data.ENV1_NAME, TRUE, Data.ENV1_VALUE);
    CheckEnv(Data.ENV2_NAME, FALSE);
    CheckEnv(Data.ENV3_NAME, FALSE);

    MutexWriteToStdout("\nParent process finished successfully");
}

VOID IAmParent()
{
    MutexWriteToStdout("\nI am parent");

    PIN_AddFiniFunction(Fini, 0);

    RemoveFile(KnobParentEnvLog.Value().c_str());

    CheckEnv(Data.ENV1_NAME, FALSE);
    setenv(Data.ENV1_NAME, Data.ENV1_VALUE, 1);
    CheckEnv(Data.ENV1_NAME, TRUE, Data.ENV1_VALUE);
    unsetenv(Data.ENV1_NAME);
    CheckEnv(Data.ENV1_NAME, FALSE);
    setenv(Data.ENV1_NAME, Data.ENV1_VALUE, 1);
    CheckEnv(Data.ENV1_NAME, TRUE, Data.ENV1_VALUE);

    CheckEnv(Data.ENV2_NAME, FALSE);
    CheckEnv(Data.ENV3_NAME, FALSE);

    PrintEnvironmentVariables(KnobParentEnvLog.Value().c_str());
}

VOID FindGeneration(INT32 argc, CHAR** argv)
{
    Data.strGeneration = NULL;
    for (uint32_t i = 0; i < argc - 1; i++)
    {
        if (!strcmp(argv[i], "-currentGeneration"))
        {
            Data.strGeneration = argv[i + 1];
            break;
        }
    }

    ASSERT(Data.strGeneration, "\nCouldn't find arg: currentGeneration");

    CHAR* endptr    = NULL;
    Data.generation = strtol(Data.strGeneration, &endptr, 10);
    ASSERT(endptr != Data.strGeneration || !*endptr, "\nCouldn't parse arg: " + Data.strGeneration);

    ASSERTX(Data.generation <= 3 && Data.generation >= 1);
}

int main(INT32 argc, CHAR** argv)
{
    Data.outputMutex = WIND::CreateMutex(NULL, FALSE /* not initial owner*/, "pin_child_tool_output_mutex");
    ASSERT(Data.outputMutex, "\nfailed to create outputMutex");

    PIN_InitSymbols();
    PIN_Init(argc, argv);

    FindGeneration(argc, argv);

    switch (Data.generation)
    {
        case 1:
            IAmParent();
            break;
        case 2:
            IAmChild();
            break;
        case 3:
            IAmGrandChild();
            break;
        default:
            ASSERTX(FALSE);
            break;
    }

    PIN_StartProgram();
    return 0;
}
