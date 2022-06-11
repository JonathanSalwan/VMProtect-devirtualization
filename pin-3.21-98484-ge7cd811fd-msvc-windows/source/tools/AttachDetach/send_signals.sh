#!/bin/sh

#
# Copyright (C) 2009-2016 Intel Corporation.
# SPDX-License-Identifier: MIT
#


pid=$1
signo=$2

if [ $# -lt 1 ]; then
  echo "The script runs for testing only, no parameters are specified, exiting.."
  exit 0
fi

echo "sending signal $2 to process $1 "

kill -0 $pid > /dev/null 2>&1
status=$?
if [ $status -eq 0 ]; then
	echo "Sending signal $signo"
	kill -s $signo $pid
	sleep 5
else
    echo "process $1 doesn't exist"
    exit 0
fi

