/*
 * Copyright (C) 2013-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#ifndef REGVALUE_UTILS_H
#define REGVALUE_UTILS_H

/////////////////////
// INCLUDES
/////////////////////

#include <string>
#include <ostream>
#include "pin.H"

using std::ostream;
using std::string;

/////////////////////
// GLOBAL VARIABLES
/////////////////////

// Booleans indicating the supported ISA extensions.
extern const bool hasAvxSupport;
extern const bool hasAvx512fSupport;

/////////////////////
// FUNCTION DECLARATIONS
/////////////////////

// Returns a string of the hex representation of the given "value" of length "size" bytes.
string Val2Str(const void* value, unsigned int size);

// Compare two values of length "size" bytes.
bool CompareValues(const void* value, const void* expected, unsigned int size, ostream& ost);

// Assign a PIN_REGISTER object with a new value.
void AssignNewPinRegisterValue(PIN_REGISTER* pinreg, const UINT64* newval, UINT qwords);

#endif // REGVALUE_UTILS_H
