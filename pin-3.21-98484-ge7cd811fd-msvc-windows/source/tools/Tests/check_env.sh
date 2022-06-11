#! /bin/bash

#
# Copyright (C) 2008-2014 Intel Corporation.
# SPDX-License-Identifier: MIT
#


# We want to check that pin does not change the environment in unexpected ways

# Ignore PPID since it changes with every process
# Ignore environment variables prefixed with PIN_
set | $* -v "^(PPID|PIN_|_=|SSH_CLIENT|SSH_CONNECTION|RANDOM|MFLAGS)"

