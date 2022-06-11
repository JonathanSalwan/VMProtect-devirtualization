#!/usr/bin/python

#
# Copyright (C) 2015-2018 Intel Corporation.
# SPDX-License-Identifier: MIT
#


# This scripts check all the required stuff for proper support of GNU indirect
# functions (ifunc).
# To properly support ifunc, both the toolchain (gcc + binutils) and the runtime
# (glibc or bionic) must support it.

import os
import sys
import subprocess
import shlex
from subprocess import Popen, PIPE, STDOUT, call

import re

def lexical_greater_or_equals(a, b):
    if (a[0] > b[0]):
        return True;
    if (a[0] < b[0]):
        return False;
    if (a[1] > b[1]):
        return True;
    if (a[1] < b[1]):
        return False;
    return (a[2] >= b[2])
    

if (len(sys.argv) < 3):
    print("Usage: " + sys.argv[0] + " <GCC path> <arch>")
    sys.exit(1)

gccpath = sys.argv[1]
if (sys.argv[2] == "intel64"):
    arch = "-m64"
elif (sys.argv[2] == "ia32"):
    arch = "-m32"
else:
    print("Unsupported arch " + sys.argv[2] + " must be ia32 or intel64")
    sys.exit(1)
     
p = subprocess.Popen([gccpath, arch, "-v"], stdout=PIPE, stderr=STDOUT)
gcc_out = p.communicate();
p.wait()

gcc_ver_re = re.compile(r'version (\d+)\.(\d+).(\d*)', re.MULTILINE)
gcc_ver = gcc_ver_re.search(gcc_out[0])
if (gcc_ver is None):
    print("Unexpected output string from gcc:\n" + glibc_ver_out)
    sys.exit(1)

gcc_major = int(gcc_ver.group(1))
gcc_middle = int(gcc_ver.group(2))
gcc_minor = 0
if (gcc_ver.group(3) != ""):
    gcc_minor = int(gcc_ver.group(3))

if (not lexical_greater_or_equals([gcc_major, gcc_middle, gcc_minor], [4, 5, 0])):
    print("gcc version doesn't support ifunc - must be at least 4.5.0")
    sys.exit(1)

p = subprocess.Popen([gccpath, arch, "-Wl,--version"], stdout=PIPE, stderr=STDOUT)
binutils_out = p.communicate();
p.wait()
version_re = re.compile(r'(\d+)\.(\d+)\.(\d+)', re.MULTILINE)
version = version_re.search(binutils_out[0])
if version is None:
    print("Unexpected output string from ld:\n" + binutils_out)
    sys.exit(1)

binutils_major = int(version.group(1))
binutils_middle = int(version.group(2))
binutils_minor = int(version.group(3))

if (not lexical_greater_or_equals([binutils_major, binutils_middle, binutils_minor], [2, 20, 1])):
    print("Binutils version doesn't support ifunc - must be at least 2.20.1")
    sys.exit(1)

p = subprocess.Popen([gccpath, arch, "-print-file-name=libc.so.6"], stdout=PIPE, stderr=STDOUT)
glibc_path = p.communicate();
p.wait()
glibc_path_stripped = glibc_path[0].strip()

if (not os.path.isfile(glibc_path_stripped)):
    print("glibc path not found: " + glibc_path)
    sys.exit(1)

p = subprocess.Popen([glibc_path_stripped], stdout=PIPE, stderr=STDOUT)
glibc_ver_out = p.communicate();
p.wait()

glibc_ver_re = re.compile(r'version (\d+)\.(\d+).(\d*)', re.MULTILINE)
glibc_ver = glibc_ver_re.search(glibc_ver_out[0])
if (glibc_ver is None):
    print("Unexpected output string from glibc:\n" + glibc_ver_out)
    sys.exit(1)

glibc_major = int(glibc_ver.group(1))
glibc_middle = int(glibc_ver.group(2))
glibc_minor = 0
if (glibc_ver.group(3) != ""):
    glibc_minor = int(glibc_ver.group(3))

if (not lexical_greater_or_equals([glibc_major, glibc_middle, glibc_minor], [2, 11, 1])):
    print("glibc version doesn't support ifunc - must be at least 2.11.1")
    sys.exit(1)

print("IFUNC_SUPPORTED")
sys.exit(0)
