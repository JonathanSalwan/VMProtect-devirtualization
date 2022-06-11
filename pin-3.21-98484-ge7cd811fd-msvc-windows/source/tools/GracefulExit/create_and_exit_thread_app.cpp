/*
 * Copyright (C) 2015-2015 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <iostream>
#include "create_and_exit_thread_utils.h"

using std::cerr;
using std::endl;

int main()
{
    cerr << "Application main reached." << endl;
    return RETVAL_FAILURE_APP_COMPLETED;
}
