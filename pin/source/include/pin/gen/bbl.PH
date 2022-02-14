/*
 * Copyright (C) 2020-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#ifdef PIN_G_BBL_PH
#error duplicate inclusion of bbl
#else
#define PIN_G_BBL_PH
/*! @file
  This file contains @ref BBL primitives
 */

/*!  @ingroup BBL
  Number of INSs within a BBL.

  @par Availability:
  \b Mode:  JIT & Probe\n
  \b O/S:   Linux, Windows & macOS*\n
  \b CPU:   All\n
*/
extern UINT32 BBL_NumIns(BBL bbl);

#endif // PIN_G_BBL_PH
