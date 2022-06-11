/*
 * Copyright (C) 2021-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * A application which recursively calls itself until a stop condition occurs.
 * Format: "win_create_process_recursive.exe -currentGeneration <int> -totalGenerations <int>"
 * It will call itself in a new process with the same argument except for currentGeneration which increases by 1.
 * In total (totalGenerations - currentGeneration + 1) processes will be created.
 * The stop condition is currentGeneration == totalGenerations.
 */

#include <Windows.h>
#include <iostream>
#include <sstream>
#include <string>
#include <type_traits>
using std::cout;
using std::endl;
using std::string;

static CONST CHAR* CURRENT_GENERATION_FLAG = "-currentGeneration";
static CONST CHAR* TOTAL_GENERATIONS_FLAG  = "-totalGenerations";

//Wait for a process completion
//Verify it returned the expected exit code
bool WaitAndVerify(HANDLE process)
{
    if (WaitForSingleObject(process, INFINITE) == WAIT_FAILED)
    {
        fprintf(stderr, "\nWaitForSingleObject Failed");
        return FALSE;
    }
    DWORD processExitCode;
    if (GetExitCodeProcess(process, &processExitCode) == FALSE)
    {
        fprintf(stderr, "\nGetExitCodeProcess Failed");
        return FALSE;
    }
    if (processExitCode != 0)
    {
        fprintf(stderr, "\nGot unexpected exit code [%d]", processExitCode);
        return FALSE;
    }
    return TRUE;
}

void PrintFormat()
{
    printf("\nexpected format: win_create_process_recursive %s <uint> %s <uint>", CURRENT_GENERATION_FLAG,
           TOTAL_GENERATIONS_FLAG);
}

int32_t FindIntArg(int argc, char** argv, const char* arg)
{
    char* argValue = NULL;
    for (uint32_t i = 0; i < argc - 1; i++)
    {
        if (!strcmp(argv[i], arg))
        {
            argValue = argv[i + 1];
            break;
        }
    }

    if (argValue == NULL)
    {
        fprintf(stderr, "\nCouldn't find arg: %s", arg);
        PrintFormat();
        exit(1);
    }

    char* endptr   = NULL;
    int32_t intArg = strtol(argValue, &endptr, 10);
    if (endptr == argValue || *endptr)
    {
        fprintf(stderr, "\nCouldn't parse arg: %s", argValue);
        PrintFormat();
        exit(1);
    }

    return intArg;
}

void RunProcess(const char* cmd)
{
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    memset(&si, 0, sizeof(si));
    si.cb = sizeof(STARTUPINFO);
    memset(&pi, 0, sizeof(pi));

    if (!CreateProcess(NULL, (LPSTR)cmd, NULL, NULL, TRUE, NULL, NULL, NULL, &si, &pi))
    {
        fprintf(stderr, "\nCouldn't run command: %s", cmd);
        exit(1);
    }

    if (WaitAndVerify(pi.hProcess) == FALSE) exit(1);
}

int main(int argc, char** argv)
{
    // Parse arguments
    int32_t currentGeneneration = FindIntArg(argc, argv, CURRENT_GENERATION_FLAG);
    int32_t totalGenerations    = FindIntArg(argc, argv, TOTAL_GENERATIONS_FLAG);

    printf("\nGeneration %d out of %d.", currentGeneneration, totalGenerations);

    if (currentGeneneration >= totalGenerations)
    {
        printf("\nReached final descendant.");
    }
    else
    {
        // Build child command line
        currentGeneneration++;
        std::stringstream ss;
        ss << argv[0];
        ss << " " << CURRENT_GENERATION_FLAG << " " << currentGeneneration;
        ss << " " << TOTAL_GENERATIONS_FLAG << " " << totalGenerations;

        // Run child process
        RunProcess(ss.str().c_str());
    }

    return 0;
}
