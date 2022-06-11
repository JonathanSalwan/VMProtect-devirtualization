/*
 * Copyright (C) 2021-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#ifdef PIN_G_INS_IARG_PH
#error duplicate inclusion of ins_iarg
#else
#define PIN_G_INS_IARG_PH
/*! @file
  This file contains functions that inspect instructions with regard to IARGs.
*/

/*! @ingroup INS_INSPECTION
 *
 * This function checks whether the IARG can be used with the instruction.
 *
 * @param[in]  ins      The instruction to inspect
 * @param[in]  argType  The IARG_TYPE enum to use with the instruction
 *
 @Return true if the specified IARG type can be used in the specified instruction, false otherwise.
 */
extern BOOL INS_IsValidForIarg(INS ins, IARG_TYPE argType);


#endif // PIN_G_INS_IARG_PH


