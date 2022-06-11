/*
 * Copyright (C) 2004-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

// Multiple inclusion is an error in pin sources, but ok for pintool
// writers
#if !defined(PIN_H)
#define PIN_H

#include <string>
#include <cstdarg>
#include <vector>
#include <cstdint>

extern "C"
{
#include "xed-interface.h"
#include "xed-iclass-enum.h"
#include "xed-category-enum.h"
}
#include "pin_version.h"

#include "types_marker.h"
#include "pin-errtype.h"

#include "util/intel-fp.hpp"
#include "atomic.hpp"
#include "pinsync.hpp"

// BASE
// foundation
namespace LEVEL_BASE
{
#include "types_foundation.PH"
#include "types_base.PH"
#include "message.PH"
#include "util.PH"
#include "knob.PH"
#include "lock.PH"

#if defined(TARGET_IA32)
#include "fpstate_ia32.PH"
#endif
#if defined(TARGET_IA32E)
#include "fpstate_ia32e.PH"
#endif

#include "reginfo_ia32.PH"
#include "reg_ia32.PH"
#include "reg_partial_ia32.PH"
#include "base_ia32.PH"
#include "tls.PH"
#include "exception.PH"
#include "child_process.PH"
#include "sym_undecorate.PH"

#if defined(TARGET_WINDOWS)
#include "os_info_win.PH"
#include "sysfuncs_windows.PH"
#elif defined(TARGET_LINUX)
#include "sysfuncs_linux.PH"
#elif defined(TARGET_MAC)
#include "sysfuncs_mac.PH"
#else
#error "no target OS defined"
#endif

} // namespace LEVEL_BASE

// CORE
namespace LEVEL_CORE
{
using namespace LEVEL_BASE;

#include "types_core.PH"
#include "val.PH"
#include "sec.PH"
#include "img.PH"
#include "bbl.PH"
#include "edg.PH"
#include "ins.PH"
#include "regset.PH"

#include "ins_ia32.PH"
#include "regset_ia32.PH"

#if defined(TARGET_LINUX) || defined(TARGET_MAC)
#include "ins_syscall_ia32_unix.PH"
#elif defined(TARGET_WINDOWS)
#include "ins_syscall_ia32_windows.PH"
#else
#error "No target os"
#endif

#include "ins_api_xed_ia32.PH"
#include "ins_xed_ia32.PH"
#include "operand_ia32.PH"

} // namespace LEVEL_CORE

// vmapi
namespace LEVEL_VM
{
using namespace LEVEL_CORE;

#include "types_vmapi.PH"
#include "ins_iarg.PH"

} // namespace LEVEL_VM

// pinclient

namespace LEVEL_PINCLIENT
{
// This includes the subset of vm types that pintools must access
using namespace LEVEL_VM;

#include "pin_client.PH"
#include "versions_client.PH"
#include "pin_client_replay.PH"
#include "thread_client.PH"
#include "debugger_client.PH"
#include "stopped_thread_client.PH"
#include "deprecated_client.PH"
#include "opin_client.PH"
#include "tls_client.PH"
#include "child_process_client.PH"
#include "image.PH"
#include "debug.PH"
#include "internal_exception_client.PH"
#include "probe_instrument.PH"

#if defined(TARGET_LINUX)
#include "image_elf.PH"
#include "symbol_linux.PH"
#include "backtrace_u.PH"
#elif defined(TARGET_MAC)
#include "image_macho.PH"
#include "symbol_macho.PH"
#include "debug_macho.PH"
#include "backtrace_u.PH"
#elif defined(TARGET_WINDOWS)
#include "os_info_client_windows.PH"
#include "image_win.PH"
#include "symbol_win.PH"
#include "debug_win.PH"
#else
#error "Unknown OS"
#endif

} // namespace LEVEL_PINCLIENT

using namespace LEVEL_PINCLIENT;

#endif
