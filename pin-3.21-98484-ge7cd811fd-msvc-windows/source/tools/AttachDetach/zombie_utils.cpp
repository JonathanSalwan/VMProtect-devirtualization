/*
 * Copyright (C) 2013-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <sys/types.h>
#include <unistd.h>
#include <string>
#include <sstream>
#include <fstream>
#include <string>
#include <ctype.h>
#include "zombie_utils.h"

using std::ifstream;
using std::ostringstream;
using std::string;

// Part of the message which Pin gives when it can't attach to the application since the
// main thread is a zombie thread
const string toFind = "The main thread of the application is a zombie thread. Pin can't attach to an application which its main "
                      "thread is a zombie thread";

EXPORT_SYM void NotifyTestType(TEST_TYPE exprType)
{
    // Pin sets an analysis function here to retrieve from the application
    // the type of the test.
    fprintf(stderr, "APP, type experiment is :%d\n", (int)exprType);
}

EXPORT_SYM void NotifyZombiePid(pid_t pid)
{
    // Pin sets an analysis function here to retrieve from the application
    // the pid of the main thread.
    fprintf(stderr, "APP, pid of the main thread: %d\n", (int)pid);
}

/*
 * Check if a thread is a zombie thread
 * @param[in] tid - thread system id
 */
bool isZombie(pid_t tid)
{
    string oneline;
    ostringstream ss;
    ss << ((int)tid);

    // Contains various information about the status of a process
    string filename = "/proc/" + ss.str() + "/status";
    ifstream inFile(filename.c_str());
    const string state = "State:";
    if (inFile.is_open())
    {
        while (inFile.good())
        {
            getline(inFile, oneline);
            size_t i;
            //Find the line which repesents the state of the thread
            for (i = 0; i < state.length(); ++i)
            {
                if (oneline[i] != state[i]) break;
            }
            if (i == state.length())
            {
                // Check if the thread is a zombie thread
                // The state is given by a sequence of characters.
                // The first character indicates the state of the process
                // 'Z' marks a zombie thread.
                for (size_t j = i; j < oneline.length(); ++j)
                {
                    if (isspace(oneline[j]))
                    {
                        continue;
                    }
                    // The current state of the process is zombie.
                    else if (oneline[j] == 'Z')
                    {
                        return true;
                    }
                    else
                    {
                        break;
                    }
                }
            }
        }
        inFile.close();
    }
    return false;
}

/*
 *  Check if Pin gave a message already that it can't attach to the application since the main thread is a zombie thread.
 *  @param[in] fileName - the message that Pin can't attach to the application will be redirected to this file.
 */
bool NotifyUserPinUnableToAttach(string fileName)
{
    string oneline;

    ifstream inFile(fileName.c_str());
    if (inFile.is_open())
    {
        while (inFile.good())
        {
            getline(inFile, oneline);
            if (oneline.find(toFind) != string::npos)
            {
                return true;
            }
        }
        inFile.close();
    }

    return false;
}
