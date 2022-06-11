/*
 * Copyright (C) 2006-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

// This little application tests passing arguments in probes mode.
//
#include <stdio.h>

void Bar(int a, int b, int c, int d) { printf("Bar: %d, %d, %d, %d\n", a, b, c, d); }

void Baz(int a) { printf("Baz: %d\n", a); }
