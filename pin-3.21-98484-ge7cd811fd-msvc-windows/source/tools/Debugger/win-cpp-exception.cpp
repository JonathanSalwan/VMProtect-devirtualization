/*
 * Copyright (C) 2011-2011 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <iostream>

int main()
{
    try
    {
        throw(1);
    }
    catch (int e)
    {
        std::cout << "Caught my exception: " << e << std::endl;
    }
    return 0;
}
