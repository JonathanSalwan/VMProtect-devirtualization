#!/usr/bin/env python

#
# Copyright (C) 2019-2019 Intel Corporation.
# SPDX-License-Identifier: MIT
#


import os
import sys
import subprocess

# This script gets two arguments: a binary file, and a name of a function - and returns
# the size in bytes of the function which name matches the input.
# If no function with that name was found then the script returns with error.

def read_func_size_linux(bin, func_name):
    # Use nm --print-size. 
    # Output for functions has 4 columns: [ Address, Size, "T" for text segment, Name]
    
    command = "nm --print-size --radix=d {0} | c++filt --no-params | grep {1}".format(bin, func_name)
    popen_obj = subprocess.Popen(   command,
                                    shell=True,
                                    stdout=subprocess.PIPE,
                                    stderr=subprocess.PIPE
                                )
    (out, err) = popen_obj.communicate()
    if (popen_obj.returncode == 0):
        out = out.decode().rstrip().lstrip()
        for line in out.split('\n'):
            tokens = line.split()
            if len(tokens) != 4:
                print ("ERROR: unexpected input \"{0}\"".format(line))
                return None
            if tokens[3] != func_name: continue
            try:
                size = int(tokens[1])
                return size
            except ValueError:
                print ("ERROR: Failed to convert \"{0}\" to int".format(tokens[1]))
                return None
                
    if (err): print("ERROR {0}".format(err.decode().rstrip().lstrip()))
    print ("ERROR: Failed to find function \"{0}\" in {1}".format(func_name, bin))
    return None

def read_func_size_osx(bin, func_name):
    # Use nm --print-size. 
    # Output for functions has 4 columns: [ Address, Size, "T" for text segment, Name]
    # On MAC, the Size will appear as 0.
    # For inlined assembly functions, we insert another symbol right after the function with extension _endfunc,
    # for example, if we have function "bar" then using macros for MAC we will have another symbol "bar_endfunc".
    # The size if the difference between the addresses of the two symbols.

    osx_func_name = "_{0}".format(func_name)
    osx_func_name_endfunc = "{0}_endfunc".format(osx_func_name)
    
    command = "nm --print-size --radix=d {0} | c++filt --no-params | grep {1}".format(bin, osx_func_name)
    popen_obj = subprocess.Popen(   command,
                                    shell=True,
                                    stdout=subprocess.PIPE,
                                    stderr=subprocess.PIPE
                                )
    (out, err) = popen_obj.communicate()
    if (popen_obj.returncode == 0):
        addr_func = None
        addr_endfunc = None
        out = out.decode().rstrip().lstrip()
        for line in out.split('\n'):
            tokens = line.split()
            if len(tokens) != 4:
                print ("ERROR: unexpected input \"{0}\"".format(line))
                return None
            if tokens[3] == osx_func_name:
                try:
                    addr_func = int(tokens[0])
                except ValueError:
                    print ("ERROR: Failed to convert \"{0}\" to int".format(tokens[0]))
                    return None
            elif tokens[3] == osx_func_name_endfunc:
                try:
                    addr_endfunc = int(tokens[0])
                except ValueError:
                    print ("ERROR: Failed to convert \"{0}\" to int".format(tokens[0]))
                    return None
                    
        if (addr_func and addr_endfunc):
            return addr_endfunc - addr_func           
                
    if (err): print("ERROR {0}".format(err.decode().rstrip().lstrip()))
    print ("ERROR: Failed to find function \"{0}\" in {1}".format(bin, func_name))
    return None


def read_func_size_windows(bin, func_name):
    # Use dumpbin /DISASM:NOBYTES
    # This will print the disassembly of the binary with addresses.
    # We will be searching for a label of the func_name, and then RET instruction,
    # and calculate the function size as the number of bytes between the two addresses.
    
    command = "dumpbin /DISASM:NOBYTES {0}".format(bin)
    popen_obj = subprocess.Popen(   command,
                                    shell=True,
                                    stdout=subprocess.PIPE,
                                    stderr=subprocess.PIPE
                                )
    (out, err) = popen_obj.communicate()
    if (popen_obj.returncode == 0):
        out = out.decode("utf-8", "ignore")
        lines = out.split('\n')
        lines1 = ""
        for line in lines:
            line.strip()
            if line.startswith(("{0}:".format(func_name), "_{0}:".format(func_name))):
                # record the line with the first instruction after the function label
                idx = lines.index(line)+1
                lines1 = lines[idx:]
                break

        lines2 = None
        for line in lines1:
            if " ret" in line:
                # record the line with RET instruction
                idx = lines1.index(line)+1
                lines2 = lines1[:idx]
                break

        if (lines2):
            # address of the first instruction after the function label
            addr1 = lines2[0].lstrip().split(':')[0]
            # address of RET instruction in this function
            addr2 = lines2[-1].lstrip().split(':')[0]
            try:
                addr1 = int(addr1, 16)
            except ValueError:
                print ("ERROR: Failed to convert \"{0}\" to int".format(addr1))
                return None
            try:
                addr2 = int(addr2, 16)
            except ValueError:
                print ("ERROR: Failed to convert \"{0}\" to int".format(addr2))
                return None

            # assume size of RET instruction is 1
            return (addr2-addr1+1)
            
    if (err): print("ERROR {0}".format(err.decode().rstrip().lstrip()))
    print ("ERROR: Failed to find function \"{0}\" in {1}".format(func_name, bin))
    return None


def main():
    if (sys.platform != 'linux') and (sys.platform != 'darwin') and (sys.platform != 'win32'):
        print("This script does not support {0}".format(sys.platform))
        return 1

    if len(sys.argv) != 3:
        print("ERROR: Please provide 2 arguments: path to binary and name of function")
        return 1

    path_to_bin = sys.argv[1]
    func_name = sys.argv[2]

    if not os.path.exists(path_to_bin):
        print("ERROR: File does not exist [{0}]".format(path_to_bin))
        return 1

    if (sys.platform == 'linux'):
        size = read_func_size_linux(path_to_bin, func_name)
    elif (sys.platform == 'darwin'):
        size = read_func_size_osx(path_to_bin, func_name)
    elif (sys.platform == 'win32'):
        size = read_func_size_windows(path_to_bin, func_name)
        
    if size:
        print(size)
        return 0
        
    return 1

    
if __name__ == "__main__":
    exit(main())
