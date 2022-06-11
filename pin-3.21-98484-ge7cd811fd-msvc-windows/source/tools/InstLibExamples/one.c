/*
 * Copyright (C) 2014-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#ifdef TARGET_WINDOWS
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif
#if defined(__cplusplus)
extern "C"
#endif
    int EXPORT
    one()
{
    return 1;
}
