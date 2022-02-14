/*
 * Copyright (C) 2012-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#ifndef OS_SPECIFIC_H_
#define OS_SPECIFIC_H_

#include "utils.h"

/*!
 * Setup the environment variables needed for pin to run.
 * @param base_path The path of the kit
 */
void update_environment(char* base_path);

/*!
 * @brief Finds the driver name across links.
 * @param argv0 This executable path
 * @return Resolved path to the executable
 */
char* find_driver_name(char* argv0);

/*!
 * Builds the command line arguments to be passed to pin.
 * @param argc The original argc
 * @param base_path The path to the kit
 * @param argv The original argv array
 * @return A null terminated array with the required parameters to pin.
 */
char** build_child_argv(char* base_path, int argc, char** argv, int user_argc, char** user_argv);

#endif /* OS_SPECIFIC_H_ */
