#
# Copyright (C) 2010-2012 Intel Corporation.
# SPDX-License-Identifier: MIT
#

This folder contains components that allow you to write a custom IDE extension
to Visual Studio 2012 that inferfaces with the Pin Advanced Debugging Extensions.
Most users of the Pin Advanced Debugging Extensions will NOT need to use the
contents of this folder.  If you simply want to write a Pin tool that interfaces
with the Visual Studio debugger, follow the instruction in the Pin User Manual
chapter titled "The Pin Advanced Debugging Extensions".

If you do want to author your own Visual Studio IDE extension for the Pin
Advanced Debugging features, you should already be familiar with the Visual
Studio SDK.  The header file "pinadx-vsplugin.hpp" contains the additional
APIs that you will need for launching a debugger session under Pin and for
communicating between your IDE extension and the Pin debugging infrastructure.

This folder also contains two installer merge modules, which you can use to
install the core Pin debugger components into Visual Studio.  To use them, you
must write your own installer which references these merge modules.

    pinadx-vsplugin-ia32.msm -      Components for debugging 32-bit
                                     applications (required).

    pinadx-vsplugin-intel64.msm -   Components for debugging 64-bit
                                     applications (optional).
