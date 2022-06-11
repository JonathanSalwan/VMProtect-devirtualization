/*
 * Copyright (C) 2007-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#ifdef PIN_G_DEBUG_PH
#error duplicate inclusion of debug
#else
#define PIN_G_DEBUG_PH
/*! @file
  PIN support for debugging information
 */
/*! @ingroup DEBUG_INFO
  Find the line number, file, and column number corresponding to a memory address.

  \b Linux:   Compile your program with -g to include line number information. Pin can only
              read dwarf4 information. If you using compiler which generates more advanced format,
              please use -gdwarf-4 flag to generate dwarf4 debugging information. 

  \b macOS*:  Compile your program with -g to include line number information. Pin can only
              read dwarf4 information. If you using compiler which generates more advanced format,
              please use -gdwarf-4 flag to generate dwarf4 debugging information.

  \b Windows: Executable/dll has to have program data base (.pdb) file in order to support this
              API.

  This function can be used in any thread, including any internal thread spawned by the tool.

  @note  In analysis routines, Client Lock should be obtained before calling this function\n

  @param[in]  address   The code address to lookup.
  @param[out] column    If column is non-zero, *column is set to the column number.
                         Column number 0 indicates that there is no valid column information.
  @param[out] line      If line is non-zero, *line is set to the line number.
                         Line number 0 indicates that there is no valid line information.
  @param[out] fileName  If fileName is non-zero, *fileName is set to the file name.
                         Empty string ("") indicates that there is no valid file name information.
                         File names are encoded in UTF8 (a superset of ASCII), this is supported
                         for Linux (only for locales encoded in UTF8) and Windows

  @note The pin client lock is obtained during the call of this API.

  @par Availability:
  \b Mode:  JIT & Probe\n
  \b O/S:   Linux, Windows & macOS*\n
  \b CPU:   All\n
 */
extern void PIN_GetSourceLocation(ADDRINT address, INT32* column, INT32* line, std::string* fileName);

#endif // PIN_G_DEBUG_PH
