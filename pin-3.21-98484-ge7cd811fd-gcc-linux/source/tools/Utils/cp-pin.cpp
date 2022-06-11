/*
 * Copyright (C) 2006-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdlib.h>
#include <iostream>
#include <fstream>
using std::cerr;
using std::endl;
using std::ifstream;
using std::ios;
using std::ofstream;

int main(int argc, char** argv)
{
    if (argc != 3)
    {
        cerr << "Usage: " << argv[0] << " input-file output-file" << endl;
        exit(1);
    }
    char* ifn = argv[1];
    char* ofn = argv[2];

    ifstream* i = new ifstream(ifn, ios::in | ios::binary);

    if (!i)
    {
        cerr << "Could not open input file " << ifn << endl;
        exit(1);
    }

    ofstream* o = new ofstream(ofn, ios::out | ios::trunc | ios::binary);
    if (!o)
    {
        cerr << "Could not open output file " << ofn << endl;
        exit(1);
    }

    char ch;
    while (i->get(ch))
    {
        *o << ch;
    }
    i->close();
    o->close();
    //cerr << "Exiting..." << endl;
    exit(0);
    //return 0;
}
