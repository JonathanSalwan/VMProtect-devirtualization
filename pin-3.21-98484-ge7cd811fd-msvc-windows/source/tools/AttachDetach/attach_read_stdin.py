#!/usr/bin/env python

#
# Copyright (C) 2016-2018 Intel Corporation.
# SPDX-License-Identifier: MIT
#


import os
import sys
import subprocess
import time
import glob
import re
from subprocess import *

def main():
    appImage = sys.argv[1]
    print("Running app: " + appImage)
    app=Popen(appImage,stdin=PIPE,stdout=PIPE)
    pidString = app.stdout.readline().rstrip().decode()
    pat = re.compile("my pid: (\d+)")
    mat = pat.match(pidString)
    if not mat:
        print("Error in pid line: " + pidString)
        sys.exit(1)
    #while this app waits for input we are attaching Pin
    cmd = [sys.argv[2]]
    cmd.extend(['-pid'])
    cmd.extend([mat.group(1)])
    cmd.extend(sys.argv[3:])
    print("Running Pin: " + " ".join(cmd))
    Popen(cmd)
    print("Waiting for few seconds")
    time.sleep(5) #waiting for Pin to attach
    #sending input to app in order to make app finish running
    print("Continuing app")
    (stdoutdata, stderrdata) = app.communicate(b'start')
    app.wait()
    print("App exited")
    print(stdoutdata)

if __name__ == "__main__":
    main()

