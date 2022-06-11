/*
 * Copyright (C) 2015-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <fstream>
#include <string>
#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <stdint.h>
using std::cerr;
using std::cout;
using std::endl;
using std::hex;
using std::ifstream;
using std::string;

int main()
{
    string line;
    ifstream maps("/proc/self/maps");
    const string vvar = "[vvar]";

    while (std::getline(maps, line))
    {
        size_t idx = line.find(vvar);
        if (string::npos != idx && line.substr(idx) == vvar)
        {
            idx = line.find("-");
            if (string::npos == idx)
            {
                cerr << "Failed to parse line '" << line << "'" << endl;
                return 1;
            }
            intptr_t vvarStart = strtoul(line.substr(0, idx).c_str(), NULL, 0x10);
            cout << hex << "0x" << vvarStart << ":0x" << (vvarStart + getpagesize()) << endl;
            return 0;
        }
    }
    return 0;
}
