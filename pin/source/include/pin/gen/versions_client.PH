/*
 * Copyright (C) 2020-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#ifdef PIN_G_VERSIONS_CLIENT_PH
#error duplicate inclusion of versions_client
#else
#define PIN_G_VERSIONS_CLIENT_PH
/*! @file
 *
 * Pin client interfaces for trace versioning
 */
/*! @ingroup TRACE_VERSION
 *
 * Set the versioning for traces that can be reached from this basic
 * block. Overrides the versioning from previous basic blocks. It is an
 * error to set the versioning for the same basic block more than
 * once.
 *
 * @param[in]   bbl     BBL to mark for versioning
 * @param[in]   version make BBL have this version
 * 
 * @par Availability:
 * \b Mode:  JIT\n
 * \b O/S:   All\n
 * \b CPU:   All\n
 *
 */
extern VOID BBL_SetTargetVersion(BBL bbl, ADDRINT version);

/*! @ingroup TRACE_VERSION
 *
 * Returns the version for this trace. See @ref BBL_SetTargetVersion
 * 
 * @par Availability:
 * \b Mode:  JIT\n
 * \b O/S:   All\n
 * \b CPU:   All\n
 *
 */
extern ADDRINT TRACE_Version(TRACE trace);

/*! @ingroup TRACE_VERSION
 *
 *  Insert a dynamic test to switch between versions before \a ins. If the
 *  value in \a reg matches \a case_value, then continue execution at \a
 *  ins with version \a version. Switching to a new version will cause
 *  execution to continue at a new trace starting with \a ins. This API can
 *  be called multiple times for the same instruction, creating a
 *  switch/case construct to select the version.
 *
 * @param[in]   ins         Insert case test before this instruction
 * @param[in]   reg         Virtual register used to select case.
 * @param[in]   case_value  Go to new version if reg contains case_value
 * @param[in]   version     New version to use
 * @param[in]   ...         List of additional arguments (see @ref IARG_TYPE), terminated with IARG_END.
 *                          Currently, only IARG_CALL_ORDER is supported. For more information, see @ref CALL_ORDER.
 * 
 * @par Availability:
 * \b Mode:  JIT\n
 * \b O/S:   All\n
 * \b CPU:   IA-32 and Intel(R) 64 architectures\n
 *
 */
extern VOID INS_InsertVersionCase(INS ins, REG reg, INT32 case_value, ADDRINT version, ...);

#endif // PIN_G_VERSIONS_CLIENT_PH
