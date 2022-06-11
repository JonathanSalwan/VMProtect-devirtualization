/*
 * Copyright (C) 2013-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#ifndef REGVALUE_TEST_UTILS_H
#define REGVALUE_TEST_UTILS_H

/////////////////////
// INCLUDES
/////////////////////

#include "pin.H"
#include "../Utils/regvalue_utils.h"
using std::vector;

/////////////////////
// GLOBAL VARIABLES
/////////////////////

// A knob for specifying whether x87 fpstate registers should be tested.
extern KNOB< bool > KnobTestSt;

// A knob for specifying whether partial registers should be tested.
extern KNOB< bool > KnobTestPartial;

// A knob for specifying whether the SIMD registers should be tested.
extern KNOB< bool > KnobTestSIMD;

/////////////////////
// FUNCTION DECLARATIONS
/////////////////////

///// Register Operations

// Get a vector with all the tested registers.
const vector< REG >& GetTestRegs();

// Get a REGSET with all the tested registers.
const REGSET& GetTestRegset();

// Print all the tested registers.
void PrintStoredRegisters(ostream& ost);

///// Value Queries

// Each tested register is stored in this utilities library. This function retrieves a pointer to it.
UINT8* GetRegval(REG reg);

// Get the expected value for a tested register. This is the value that the application assigns in the
// ChangeRegs function. These values are defined in regvalues.h.
const UINT8* GetAppRegisterValue(REG Reg);

// Get the tool assigned value for a tested register. These values are defined in regvalues.h.
const UINT8* GetToolRegisterValue(REG reg);

///// Test Logic

// Compare the stored values of all the tested registers (see the GetTestRegs() function above) with the expected
// application values defined in regvalues.h.
bool CheckAllExpectedValues(ostream& ost);

#endif // REGVALUE_TEST_UTILS_H
