#
# Copyright (C) 2008-2018 Intel Corporation.
# SPDX-License-Identifier: MIT
#

#
# This script compares a multi-line input file with a multi-line comparison file, where
# each line in the comparison file is a Python regular expression (re).  The input file
# matches if there is some sequence of lines that match (in order) the patterns from the
# comparison file.  The input file may have additional non-matching lines, so long as
# there is a matching line for each pattern.
#

import optparse
import sys
import re


def Main(argv):
    parser = optparse.OptionParser()
    parser.add_option("-p", "--pattern", action="store", type="string", dest="pattern", default="",
        help="Pattern file")
    parser.add_option("-c", "--compare", action="store", type="string", dest="compare", default="",
        help="Compare file")
    parser.add_option("-q", "--quiet", action="store_true", dest="quiet", default=False,
        help="By default, matching lines are printed to stdout.  This option supresses this.")
    global Opts
    (Opts, args) = parser.parse_args(args=argv)

    if not Opts.pattern:
        PrintError("Must specify pattern file")
        return 1
    if not Opts.compare:
        PrintError("Must specify compare file")
        return 1

    try:
        filePattern = open(Opts.pattern, 'r')
    except IOError:
        PrintError("Unable to read '" + Opts.pattern + "'")
        return 1
    try:
        fileCompare = open(Opts.compare, 'r')
    except IOError:
        PrintError("Unable to read '" + Opts.compare + "'")
        return 1

    if not CompareFile(filePattern, fileCompare):
        return 1
    return 0


def CompareFile(filePattern, fileCompare):
    compLines = []
    for line in fileCompare.readlines():
        compLines.append(line.rstrip('\r\n'))
    fileCompare.close()

    patterns = []
    for line in filePattern.readlines():
        line = line.rstrip('\r\n')
        pat = re.compile(line)
        patterns.append((pat,line))
    filePattern.close()

    # This is a simple greedy algorithm.  We should backtrack and look for other matches
    # if it fails.
    #
    while patterns:
        (pat, x) = patterns[0]
        while compLines and not pat.match(compLines[0]):
            compLines.pop(0)
        if not compLines:
            PrintNoMatch(patterns)
            return False
        if not Opts.quiet:
            print(">> " + compLines[0])
        compLines.pop(0)
        patterns.pop(0)

    return True


def PrintNoMatch(patList):
    if not Opts.quiet:
        print()
    PrintError("Remaining patterns do not match:")
    for (x, patStr) in patList:
        print(">> " + patStr)


def PrintError(s):
    print("Error: " + s)


if __name__ == "__main__": sys.exit(Main(sys.argv[1:]))
