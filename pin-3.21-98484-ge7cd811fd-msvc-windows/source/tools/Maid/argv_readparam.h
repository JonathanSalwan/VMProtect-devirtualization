/*
 * Copyright (C) 2004-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#ifndef _ARGV_READPARAM_H_
#define _ARGV_READPARAM_H_

int argv_hasFlag(int argc, char* argv[], char param);
int argv_hasLongFlag(int argc, char* argv[], char* param);
int argv_getInt(int argc, char* argv[], char* param, int* ret);
int argv_getLong(int argc, char** argv, char* param, long* ret);
char* argv_getString(int argc, char* argv[], char const* param, char** mem);

#endif
