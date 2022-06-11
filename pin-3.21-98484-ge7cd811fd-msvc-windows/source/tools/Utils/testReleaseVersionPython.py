#!/usr/bin/env python

#
# Copyright (C) 2014-2018 Intel Corporation.
# SPDX-License-Identifier: MIT
#


from os import uname
from sys import argv

if len(argv) != 2:
    print("Usage: \"python testReleaseVersionPython.py <version_to_compare>")
    print("  output is 1 if os_ver > version_to_compare")
    exit()

_, _, os_release, _, _  = uname();
os_release = tuple(map(int, (os_release.split("-")[0].split("."))));

compare_release = tuple(map(int, (argv[1].split("."))))

if ( os_release < compare_release ):
    print(1)
else:
    print(0)

