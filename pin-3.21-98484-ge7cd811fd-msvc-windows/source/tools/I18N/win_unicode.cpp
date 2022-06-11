/*
 * Copyright (C) 2007-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <iostream>
#include <fstream>

int wmain(int argc, wchar_t* argv[])
{
    std::ofstream file;
    file.open("win_unicode.out");
    //internationalization in Japanese (encoded in UTF-16)
    wchar_t internationalization[] = {0x56fd, 0x969B, 0x5316, 0x0000};

    if (argc == 1)
    {
        file << "not equal" << endl;
    }
    else
    {
        if (_wcsicmp(argv[1], internationalization) == 0)
        {
            file << "equal" << endl;
        }
        else
        {
            file << "not equal" << endl;
        }
    }
    file.close();
    return 0;
}
