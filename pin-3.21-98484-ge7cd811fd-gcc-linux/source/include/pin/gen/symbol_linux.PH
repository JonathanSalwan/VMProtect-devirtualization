/*
 * Copyright (C) 2020-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#ifdef PIN_G_SYMBOL_LINUX_PH
#error duplicate inclusion of symbol_linux
#else
#define PIN_G_SYMBOL_LINUX_PH
/*! @file
*/
#if defined (TARGET_LINUX)
/*! @ingroup PIN_CONTROL
  Initialize symbol table code. Pin does not read symbols unless this is called.
  Must be called before @ref PIN_StartProgram

  @par Availability:
  \b Mode:  JIT & Probe\n
  \b O/S:   Linux, Windows & macOS*\n
  \b CPU:   All\n
*/
extern VOID PIN_InitSymbols();

/*! @ingroup PIN_CONTROL
  Initialize symbol table code with an explicitly specified mode of symbol support. \n
  Pin does not read symbols unless this is called. \n
  Must be called before @ref PIN_StartProgram. \n
  Alternative to PIN_InitSymbols() on Windows & Linux. \n
  On macOS* it is implemented as PIN_InitSymbols() \n

  @param[in]  mode  Explicit mode of symbol support. Valid values: \n
                      EXPORT_SYMBOLS           Only symbols taken from export table are provided.
                                               Line information is not available (Windows only). \n
                      DEBUG_SYMBOLS            Currently treated the same as DEBUG_OR_EXPORT_SYMBOLS
                                               (Windows only). \n
                      IFUNC_SYMBOLS            IFUNC symbols (Linux only). \n
                      DEBUG_OR_EXPORT_SYMBOLS  First debug symbols if available, otherwise export symbols.
                                               This mode is set by PIN_InitSymbols() (Windows only) \n
                    This parameter is ignored on operating systems other than Windows & Linux. \n

  @return     TRUE if the operation succeeded \n

  @par Availability:
  \b Mode:  JIT & Probe\n
  \b O/S:   Windows, Linux & macOS*\n
  \b CPU:   All\n
*/
extern BOOL PIN_InitSymbolsAlt(SYMBOL_INFO_MODE mode);

#endif // defined (TARGET_LINUX)
#endif // PIN_G_SYMBOL_LINUX_PH


