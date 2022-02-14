/*
 * Copyright (C) 2021-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
  this headerfile contains defines the (additional) types needed by @ref LEVEL_BASE
 */

#ifndef TYPES_BASE_PH
#define TYPES_BASE_PH

#define B_VERBOSE 1
#define B_BRIEF 0

typedef VOID (*AFUNPTR)();
typedef VOID* (*THREAD_STARTROUTINE)(void*);
typedef VOID (*DESTRUCTFUN)(void*);

static inline USIZE USIZE_INVALID() { return ~((USIZE)0); }

typedef UINT16 OPCODE;

static inline OPCODE OPCODE_INVALID() { return 0; }
static inline BOOL OPCODE_valid(OPCODE opc) { return opc != 0; }

#if defined(TARGET_LINUX) || defined(TARGET_MAC)
#define KNOB_ONLY_ON_UNIX "1"
#else
#define KNOB_ONLY_ON_UNIX "0"
#endif

#if defined(TARGET_LINUX) || defined(TARGET_WINDOWS)
#define KNOB_ONLY_ON_WINDOWS_AND_LINUX "1"
#else
#define KNOB_ONLY_ON_WINDOWS_AND_LINUX "0"
#endif

#if defined(TARGET_LINUX)
#define KNOB_ONLY_ON_LINUX_AND_BSD "1"
#else
#define KNOB_ONLY_ON_LINUX_AND_BSD "0"
#endif

#if defined(TARGET_LINUX) || defined(TARGET_WINDOWS)
#define KNOB_ONLY_ON_WINDOWS_LINUX_AND_BSD "1"
#else
#define KNOB_ONLY_ON_WINDOWS_LINUX_AND_BSD "0"
#endif

#if defined(TARGET_LINUX)
#define KNOB_ONLY_ON_LINUX "1"
#else
#define KNOB_ONLY_ON_LINUX "0"
#endif

#if defined(TARGET_MAC)
#define KNOB_ONLY_ON_MAC "1"
#else
#define KNOB_ONLY_ON_MAC "0"
#endif

#if defined(TARGET_WINDOWS)
#define KNOB_ONLY_ON_WINDOWS "1"
#else
#define KNOB_ONLY_ON_WINDOWS "0"
#endif

#if defined(TARGET_IA32E)
#define KNOB_ONLY_ON_IA32E "1"
#else
#define KNOB_ONLY_ON_IA32E "0"
#endif

#if defined(TARGET_IA32) || defined(TARGET_IA32E)
#define KNOB_ONLY_ON_IA32_IA32E "1"
#else
#define KNOB_ONLY_ON_IA32_IA32E "0"
#endif

#if defined(TARGET_IA32) || defined(TARGET_IA32E)
#define KNOB_ONLY_ON_X86 "1"
#else
#define KNOB_ONLY_ON_X86 "0"
#endif

#if (defined(TARGET_IA32) || defined(TARGET_IA32E)) && defined(TARGET_LINUX)
#define KNOB_ONLY_ON_IA32_IA32E_LINUX "1"
#else
#define KNOB_ONLY_ON_IA32_IA32E_LINUX "0"
#endif

#define HEX(val) 0x##val

#if defined(TARGET_MAC)
// on macOS* the section name must start with '__'
#define PIN_VERSION_SECNAME "__pinversion"
#define PIN_CLIENTINT_SECNAME "__pinclientint"
#else
#define PIN_VERSION_SECNAME ".pinversion"
#define PIN_CLIENTINT_SECNAME ".pinclientint"
#endif

class STAT;
class STAT_NORM;

/*! @ingroup PTRACE
  x
*/

#if !defined(TARGET_MAC)

typedef struct
{
  public:
    char* name;
    ADDRINT lm;
    ADDRINT l_ld;
    ADDRINT l_addr;
} LINUX_LOADER_IMAGE_INFO;

#endif

/*! @ingroup MISC_BASIC
 *  Timeout interval that never elapses.
 */
const UINT32 PIN_INFINITE_TIMEOUT = static_cast< UINT32 >(-1);

/*! @ingroup THREADS_INTERNAL
 * Maximum number of threads supported by PIN.
 */
const UINT32 PIN_MAX_THREADS = 2048;

#endif
