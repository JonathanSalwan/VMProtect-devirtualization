#!/usr/bin/python

#
# Copyright (C) 2014-2018 Intel Corporation.
# SPDX-License-Identifier: MIT
#


import re, sys, os

line_regex = re.compile(r'(?P<path>.+), (?P<low>0x[0-9a-fA-F]+)-(?P<high>0x[0-9a-fA-F]+)')

tool_output = {}
app_output = {}

for line in open(sys.argv[1]).readlines():
    match = line_regex.match(line)
    if os.path.abspath(match.group(1)) not in tool_output:
        tool_output[os.path.abspath(match.group(1))] = []
    tool_output[os.path.abspath(match.group(1))].append((int(match.group(2), 16), int(match.group(3), 16)))

for line in open(sys.argv[2]).readlines():
    match = line_regex.match(line)
    if os.path.abspath(match.group(1)) not in app_output:
        app_output[os.path.abspath(match.group(1))] = []
    app_output[os.path.abspath(match.group(1))].append((int(match.group(2), 16), int(match.group(3), 16)))

for img, segments in list(app_output.items()):
    if img not in tool_output:
        print("An image visible to the app was not found by pin image load callbacks...")
        sys.exit(1)

    # iterate over the segments in the image
    for segment in segments:
        segment_found = False
        # iterate over the image regions (pin struct)
        for region in tool_output[img]:
            # Check if the segment is contained in one of
            # the regions. It should!
            if segment[0] >= region[0] and segment[1] <= region[1]:
                segment_found = True

        if not segment_found:
            print("Couldn't find text/data segment in any of the image regions!")
            for region in tool_output:
                # Check if the segment is contained in one of
                # the regions. It should!
                if segment[0] >= region[0] and segment[1] <= region[1]:
                    segment_found = True

        if not segment_found:
            print("Couldn't find text/data segment in any of the image regions!")
            for region in tool_output[img]:
                print("Image: {0}, region [{1}, {2}]".format(img, region[0], region[1]))
            print("Image: {0}, segment[{1}, {2}]".format(img, segment[0], segment[1]))
            sys.exit(1)

sys.exit(0)
