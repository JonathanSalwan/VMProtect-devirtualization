#!/usr/bin/env python

#
# Copyright (C) 2014-2018 Intel Corporation.
# SPDX-License-Identifier: MIT
#


# -*- python -*-

'''
This script is used as the client side of the SDE interactive controller.
It allows users to interactively trigger controller event inside an application running under SDE.
 
Example:
> SDE_KIT/sde -control start:interactive -interactive_file <name> -- <app>
> SDE_KIT/misc/cntrl_client.py <name>
'''

import os
import sys
import json
import socket

if len(sys.argv) != 2:
    print('Error Usage: python cntrl_client.py <file_name>')
    exit(1)
    
file_name = sys.argv[1]
if not os.path.exists(file_name):
    print('file file: ' + file_name + ' does not exists')
    print('Have you run SDE with interactive controller?') 
    exit(1)

try:
    sock = None
    with open(file_name, 'r') as f:
        content = json.load(f)
        if (not 'port' in content):
            raise Exception('Unable to find port in the file')
        port = int(content['port'])
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.connect(('localhost', port))
        sock.send("1")
except Exception as e:
    print('ERROR: failed sending signal to SDE - ' + e.message)
    if None != sock:
        sock.close()
    
    
