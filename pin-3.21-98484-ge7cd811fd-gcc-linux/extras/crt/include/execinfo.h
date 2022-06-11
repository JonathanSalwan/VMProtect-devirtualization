/*
 * Copyright (C) 2015-2016 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <sys/cdefs.h>

__BEGIN_DECLS

int backtrace(void **buffer, int size);
char **backtrace_symbols(void *const *buffer, int size);

__END_DECLS
