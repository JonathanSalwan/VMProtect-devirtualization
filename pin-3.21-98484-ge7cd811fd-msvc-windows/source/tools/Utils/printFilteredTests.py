#!/usr/bin/env python

#
# Copyright (C) 2020-2020 Intel Corporation.
# SPDX-License-Identifier: MIT
#


import os
import argparse
import subprocess as sp

# This script gets two arguments: a list of tests, and a list of unfiltered tests.

def print_filtered(tests, unfiltered_tests):
    before_filter = list(map(lambda x : x.replace(".wrap",""), unfiltered_tests.split()))
    after_filter = list(map(lambda x : x.replace(".wrap",""), tests.split()))
    filtered_tests = list(filter(lambda x: x not in after_filter, before_filter))
    
    testPrefix = os.environ.get('TEST_PREFIX')
    
    print("{} filtered tests: {}".format(len(filtered_tests), filtered_tests))
    
    if testPrefix != None:    # wrapper for generating Junit exist
        for test in filtered_tests:
            cmd = os.environ.get('TEST_PREFIX')
            cmd = cmd + " skip " + test + ".test"
            print("Executing: " + cmd)
            p = sp.Popen(cmd, shell=True, stdout = sp.PIPE, stderr = sp.PIPE, universal_newlines=True)
            try:
                stdout, stderr = p.communicate(timeout=60)   #60 sec to produce dummy skipped test 
            except sp.TimeoutExpired:
                p.kill()
                stdout, stderr = p.communicate()
            if (p.returncode != 0):
                print("fail to create skip test for " + test + ".test")
                print("stdout: " + stdout)
                print("stderr: " + stderr)
    
def parse_argv():
    parser = argparse.ArgumentParser(
        formatter_class=argparse.RawDescriptionHelpFormatter,
        description="Script to print information about filtered tests.\n")

    parser.add_argument('--tests', nargs='?', const='{}', help='The list of test to execute')
    parser.add_argument('--unfiltered_tests', nargs='?', const='{}', help='The list of test to execute prior to filtering')

    args = parser.parse_args()    
    
    return args

def main():
    args = parse_argv()
    print_filtered(args.tests, args.unfiltered_tests)
    return 0

    
if __name__ == "__main__":
    exit(main())
