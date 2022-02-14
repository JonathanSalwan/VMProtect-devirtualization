/*
 * Copyright (C) 2020-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#ifdef PIN_G_PROBE_INSTRUMENT_IA32_PH
#error duplicate inclusion of probe_instrument_ia32
#else
#define PIN_G_PROBE_INSTRUMENT_IA32_PH
/*! @file
 DWARF specific instrumentation in probe mode
*/
#if defined(TARGET_LINUX) || defined(TARGET_MAC)
#endif //defined(TARGET_LINUX)
#endif // PIN_G_PROBE_INSTRUMENT_IA32_PH
