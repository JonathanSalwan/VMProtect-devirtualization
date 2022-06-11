#!/usr/bin/env python

#
# Copyright (C) 2019-2019 Intel Corporation.
# SPDX-License-Identifier: MIT
#


import os
import sys
import subprocess

# This script run this command to retrieve information about Windows version:
# powershell (Get-Item 'HKLM:SOFTWARE\Microsoft\Windows NT\CurrentVersion')
#
# Example for output:
#
#    Hive: HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT
#  
#  
#  Name                           Property
#  ----                           --------
#  CurrentVersion                 SystemRoot                : C:\windows
#                                 BuildBranch               : rs3_release
#                                 BuildGUID                 : ffffffff-ffff-ffff-ffff-ffffffffffff
#                                 BuildLab                  : 16299.rs3_release_svc.180808-1748
#                                 BuildLabEx                : 16299.637.amd64fre.rs3_release_svc.180808-1748
#                                 CompositionEditionID      : Enterprise
#                                 CurrentBuild              : 16299
#                                 CurrentBuildNumber        : 16299
#                                 CurrentMajorVersionNumber : 10
#                                 CurrentMinorVersionNumber : 0
#                                 CurrentType               : Multiprocessor Free
#                                 CurrentVersion            : 6.3
#                                 EditionID                 : Enterprise
#                                 EditionSubstring          :
#                                 InstallationType          : Client
#                                 InstallDate               : 1540319630
#                                 ProductName               : Windows 10 Enterprise
#                                 ReleaseId                 : 1709
#                                 SoftwareType              : System
#                                 UBR                       : 1029
#                                 PathName                  : C:\windows
#                                 DigitalProductId          : {164, 0, 0, 0...}
#                                 DigitalProductId4         : {248, 4, 0, 0...}
#                                 ProductId                 : 00329-00000-00003-AA779
#                                 RegisteredOwner           : Intel User
#                                 RegisteredOrganization    : Intel Corporation
#                                 InstallTime               : 131847932301866431
#
# The script may get an additional parameter to query a specific property.

if (len(sys.argv) == 1):
    command = "powershell (Get-Item 'HKLM:SOFTWARE\Microsoft\Windows NT\CurrentVersion')"
else:
    command = "powershell (Get-Item 'HKLM:SOFTWARE\Microsoft\Windows NT\CurrentVersion').GetValue('{0}')".format(sys.argv[1])

popen_obj = subprocess.Popen(   command,
                                shell=True,
                                stdout=subprocess.PIPE,
                                stderr=subprocess.PIPE
                            )
(out, err) = popen_obj.communicate()

if (popen_obj.returncode != 0):
    print("ERROR")
    exit(1)
    
out = out.decode().rstrip().lstrip()
print(out)
exit(0)
