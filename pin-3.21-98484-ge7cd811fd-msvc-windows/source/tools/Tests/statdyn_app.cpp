/*
 * Copyright (C) 2011-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/* Description:
 * This executable creates two symbols: statdyn_app_staticFunction and statdyn_app_dynamicFunction.
 * The test tool then checks that statdyn_app_staticFunction is a static symbol (defined only in
 * the SYMTAB table) and that statdyn_app_dynamicFunction is a dynamic symbol (defined in the
 * DYNSYM table).
 * The main program itself actually does nothing, we only need to create the two symbols for the
 * tool. The call to statdyn_app_staticFunction is there to prevent an error during compilation due
 * to a static function which is not used.
 */

static bool statdyn_app_staticFunction() { return true; }

bool statdyn_app_dynamicFunction() { return true; }

int main()
{
    statdyn_app_staticFunction();
    return 0;
}
