/*
 * Copyright (C) 2015-2017 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#ifndef _FLOAT_H_
#define _FLOAT_H_

/*
 * Floating point constants according to ISO C Standard:  5.2.4.2.2 for MSVC compiler.
 * Note that on GCC and clang this file is shipped with the compiler.
 */

#define DBL_DIG         15
#define DBL_MAX         1.7976931348623157E+308
#define DBL_MIN         2.2250738585072014E-308
#define DBL_EPSILON     2.2204460492503131e-016
#define DBL_MANT_DIG    53
#define DBL_MAX_10_EXP  308
#define DBL_MAX_EXP     1024
#define DBL_MIN_10_EXP  (-307)
#define DBL_MIN_EXP     (-1021)

#define FLT_DIG         6
#define FLT_MAX         3.402823466E+38F
#define FLT_MIN         1.17549435E-38F
#define FLT_EPSILON     1.192092896e-07F
#define FLT_MANT_DIG    24
#define FLT_MAX_10_EXP  38
#define FLT_MAX_EXP     128
#define FLT_MIN_10_EXP  (-37)
#define FLT_MIN_EXP     (-125)
#define FLT_RADIX       2
#define FLT_ROUNDS      1

#define LDBL_DIG        DBL_DIG
#define LDBL_EPSILON    DBL_EPSILON
#define LDBL_MANT_DIG   DBL_MANT_DIG
#define LDBL_MAX        DBL_MAX
#define LDBL_MAX_10_EXP DBL_MAX_10_EXP
#define LDBL_MAX_EXP    DBL_MAX_EXP
#define LDBL_MIN        DBL_MIN
#define LDBL_MIN_10_EXP DBL_MIN_10_EXP
#define LDBL_MIN_EXP    DBL_MIN_EXP

#endif // _FLOAT_H_

