/*
 * Copyright (C) 2007-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <iostream>
#include <fstream>
#include <cstring>
using std::endl;

int main(int argc, char* argv[])
{
    std::ofstream file;
    file.open("unix_unicode.out");
    //internationalization in Japanese (encoded in UTF-8)
    char i18n[] = {(char)0xE5, (char)0x9B, (char)0xBD, (char)0xE9, (char)0x9A,
                   (char)0x9B, (char)0xE5, (char)0x8C, (char)0x96, (char)0x00};
    if (argc == 1)
    {
        file << "not equal" << endl;
    }
    else
    {
        if (strcmp(argv[1], i18n) == 0)
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
