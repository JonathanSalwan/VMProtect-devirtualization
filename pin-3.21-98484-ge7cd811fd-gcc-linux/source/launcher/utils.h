/*
 * Copyright (C) 2012-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * utils.h
 */

#ifndef UTILS_H_
#define UTILS_H_

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

/*
 * Appends 3 constant null terminated path strings.
 * @param s1
 * @param s2
 * @param s3
 * @return The concatenated path string
 */
char* appendPath(const char* s1, const char* s2, const char* s3);

/*!
 * Checks the return value of libc calls and prints the correct error message on error.
 * @param r Return value
 * @param s Error message
 */
void check_retval(int retval, const char* str);

/*!
 * @brief Checks if the file exist and readable.
 * @param fn The file path
 * @return True if exist, readable and executable
 */
unsigned int check_file_exists(const char* fn);

/*!
 * @brief Checks for file properties and prints an error message is requirements are not met.
 * @param fn The file path
 */
void check_file(const char* fn);

/*!
 * Check if the file path is a directory
 * @param fn The file path
 * @return 1 if the file path is not a directory
 */
unsigned int check_not_directory(const char* fn);

/*!
 * Checks that a file exists in the directory and that it is not a directory.
 * @param fn The file path to be checked
 * @param dir The directory path to be checked in
 * @param buff A buffer to create the complete path
 * @return 1 if file exist and not a directory
 */
unsigned int check_file_in_dir(const char* fn, const char* dir, char* buff);

/*!
 * Searches for the given executable in the directories list at the PATH environment variable,
 * and returns the directory it was found in.
 * @param exename The executable path
 * @return The directory it was found in, or null if not found.
 */
char* search_in_path(const char* exename);

/*!
 * Finds the base path (containing directory) of the given executable.
 * @param filename The file path
 * @return The base path
 */
char* find_base_path(char* filename);

#endif /* UTILS_H_ */
