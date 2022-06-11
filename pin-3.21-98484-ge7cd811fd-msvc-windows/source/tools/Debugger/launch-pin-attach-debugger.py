#
# Copyright (C) 2010-2018 Intel Corporation.
# SPDX-License-Identifier: MIT
#

# This is a helper utility that launches Pin with "-appdebug", gets the
# TCP port number that it is waiting on, and then launches a debugger so
# that it attaches to that port.
#
# We used to do this with shell commands in the makefile, but there was
# a Cygwin bug that caused this to hang periodically (see Mantis #1839).

import optparse
import os.path
import re
import subprocess
import sys
import time
import platform


def Main(argv):
    """
    Main entry point.

    @param argv:    Command line arguments.
    @type argv:     List of String.
    """

    # There appears to be a bug in the Cygwin version that prevents us from
    # reading the output file of a subprocess while that subprocess is running.
    #
    if sys.platform == 'cygwin':
        print("This script must be run with the Windows native Python")
        return 1

    loader_library_path = ""
    if (platform.system() == "Linux"):
        loader_library_path ="LD_LIBRARY_PATH"
    if (platform.system() == "Darwin"):
        loader_library_path ="DYLD_LIBRARY_PATH"
    
    # Parse options.
    #
    parser = optparse.OptionParser()
    parser.add_option("--pin", dest="pin", type="string", default="",
        help="Pathname to the Pin driver script (or executable).  Required.")
    parser.add_option("--pin-exe", dest="pinexe", type="string", default="",
        help="Pathname to the Pin executable.  Required.")
    parser.add_option("--pindb", dest="pindb", type="string", default="",
        help="Pathname to the PinDB executable.  Required.")
    parser.add_option("--pindb-libpath", dest="pindbLibPath", type="string", default="",
        help="Additional path(s) to append to " + loader_library_path + " when launching PinDB.  Separate paths with a ':'.")
    parser.add_option("--tool", dest="tool", type="string", default="",
        help="Pathname to the Pin tool.  Required.")
    parser.add_option("--app", dest="app", type="string", default="",
        help="Pathname to the application.  Required.")
    parser.add_option("--pin-out", dest="pinout", type="string", default="",
        help="Pathname to the output file which receives the Pin output.  Required.")
    parser.add_option("--pindb-in", dest="pindbin", type="string", default="",
        help="Pathname to the input file for PinDB.  Required.")
    parser.add_option("--pindb-out", dest="pindbout", type="string", default="",
        help="Pathname to the output file which receives the PinDB output.  Required.")
    parser.add_option("--cpu", dest="cpu", type="string", default="",
        help="Name of the CPU to test [ia32 | intel64].  Required.")
    parser.add_option("--timeout", dest="timeout", type="int", default=30,
        help="Time limit (seconds) before deciding that a command is hung.")

    (options, args) = parser.parse_args(args=argv)
    if not options.pin:
        print("Must specify --pin=PIN option")
        return 1
    if not options.pinexe:
        print("Must specify --pin-exe=PINEXE option")
        return 1
    if not options.pindb:
        print("Must specify --pindb=PINDB option")
        return 1
    if not options.tool:
        print("Must specify --tool=TOOL option")
        return 1
    if not options.app:
        print("Must specify --app=APP option")
        return 1
    if not options.pinout:
        print("Must specify --pin-out=PINOUT option")
        return 1
    if not options.pindbin:
        print("Must specify --pindb-in=PINDBIN option")
        return 1
    if not options.pindbout:
        print("Must specify --pindb-out=PINDBOUT option")
        return 1
    if not options.cpu:
        print("Must specify --cpu=CPU option")
        return 1
    if options.cpu not in ['ia32', 'intel64']:
        print("Invalid --cpu=CPU option")
        return 1

    # Launch Pin.
    #
    cmd = [os.path.normcase(options.pin)]
    cmd.append('-slow_asserts')
    if (platform.system() == "Windows"):
        cmd.append('-xyzzy')
        cmd.append('-late_injection')
    cmd.append('-appdebug')
    cmd.append('-t')
    cmd.append(os.path.normcase(options.tool))
    cmd.append('--')
    cmd.append(os.path.normcase(options.app))

    outFile = open(os.path.normcase(options.pinout), 'w')

    PrintCommand(cmd)
    try:
        subPin = subprocess.Popen(cmd, stdout=outFile)
    except (OSError):
        print("Unable to launch Pin")
        return 1

    # Pin waits for the debugger to attach.  Wait for it to print the
    # port number, and get that port number from the output file.
    #
    port = None
    startTime = time.time()
    rePort = re.compile('\s.*remote :?([0-9]+)$')
    while not port:
        # Check for timeout.
        #
        now = time.time()
        if (now - startTime) > options.timeout:
            print("Timed out waiting for port after " + str(options.timeout) + " seconds")
            KillProcess(subPin)
            return 1
        time.sleep(1)

        f = open(os.path.normcase(options.pinout), 'r')
        lines = f.readlines()
        f.close()
        for line in lines:
            m = rePort.match(line)
            if m:
                port = m.group(1)
                break

    # Launch PinDB.
    #
    cmd = [os.path.normcase(options.pindb)]
    cmd.append('--pin=' + os.path.normcase(options.pinexe))
    cmd.append('--timeout=' + str(options.timeout))
    cmd.append('--gdb-protocol=:' + port)
    cmd.append('--cpu=' + options.cpu)
    cmd.append('--noprompt')

    env = None
    if options.pindbLibPath:
        env = os.environ
        if loader_library_path in env:
            env[loader_library_path] += ':' + options.pindbLibPath
        else:
            env[loader_library_path] = options.pindbLibPath
        print("PinDB runs with this " + loader_library_path + ": " + env[loader_library_path])

    inFile = open(os.path.normcase(options.pindbin), 'r')
    outFile = open(os.path.normcase(options.pindbout), 'w')

    PrintCommand(cmd)
    try:
        subPinDB = subprocess.Popen(cmd, env=env, stdin=inFile, stdout=outFile)
    except (OSError):
        print("Unable to launch PinDB")
        KillProcess(subPin)
        return 1

    # If PinDB exits abnormally, it may have left the Pin process hanging, so try to kill it.
    #
    if subPinDB.wait() != 0:
        KillProcess(subPin)
        return subPinDB.returncode
    return subPin.wait()


def KillProcess(sub):
    """
    Kill a subprocess.

    @param sub:     The subprocess to kill.
    @type sub:      Popen object.
    """

    # The functions below are only available on Python 2.6 or later.
    #
    if sys.hexversion < 0x20600f0:
        return

    # First try to kill it gracefully, then terminate forcefully.
    #
    sub.terminate()
    time.sleep(1)
    sub.kill()


def PrintCommand(cmd):
    """
    Print an informational message about a command that is executed.

    @param cmd:     The command.
    @type cmd:      List of String.
    """

    mesg = "Launching: " + " ".join(cmd)
    print(mesg)


if __name__ == "__main__": sys.exit(Main(sys.argv[1:]))
