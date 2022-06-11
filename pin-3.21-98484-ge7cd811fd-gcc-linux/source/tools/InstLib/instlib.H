/*
 * Copyright (C) 2004-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#ifndef INSTLIB_H
#define INSTLIB_H

#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <sstream>

#define ISIMPOINT_MAX_THREADS 160

#include "filter.H"
#include "skipper.H"
#include "icount.H"
#include "follow_child.H"

extern "C"
{
#include "xed-interface.h"
}

#endif
