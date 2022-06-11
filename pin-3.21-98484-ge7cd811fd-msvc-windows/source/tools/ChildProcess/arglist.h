/*
 * Copyright (C) 2008-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#ifndef _ARGLIST_H_
#define _ARGLIST_H_

#include <string>
#include <list>

using namespace std;

class ARGUMENTS_LIST
{
  public:
    ARGUMENTS_LIST() : m_argv(0) {}
    ARGUMENTS_LIST(int argc, const char* const* argv);
    ~ARGUMENTS_LIST();

    void Add(const string& arg);
    int Argc() const;
    char** Argv();
    string String() const;

  private:
    void CleanArray();
    void BuildArray();

    list< string > m_argvStrList;
    char** m_argv;
};

#endif
