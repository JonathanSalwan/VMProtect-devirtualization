/*
 * Copyright (C) 2020-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#ifdef PIN_G_REGSET_IA32_PH
#error duplicate inclusion of regset_ia32
#else
#define PIN_G_REGSET_IA32_PH
/*! @file
    x
*/
/*! @ingroup REG
  @brief REG represented by the first bit in the regset vector
*/
const REG REG_FirstInRegset = REG_RBASE;

/*! @ingroup REG
 *
 * REG represented by the last bit in the regset vector. Most of the code
 * assumes that REG_LAST is not an actual register, so we should not
 * include it in the set. We use REG_LAST-1 for the last registers.
 */
const REG REG_LastInRegset = REG(REG_LAST - 1);

/*! @ingroup REG
  A regset type that contains all registers
*/
typedef class REGISTER_SET< REG_FirstInRegset, REG_LastInRegset > REGSET;

/*! @ingroup REG
  @return TRUE if the specified reg is contained in the specified regset
*/
inline BOOL REGSET_Contains(const REGSET& regset, REG reg) { return regset.Contains(reg); }

/*! @ingroup REG
  Insert the specified reg into the specified regset
*/
inline VOID REGSET_Insert(REGSET& regset, REG reg) { regset.Insert(reg); }

/*! @ingroup REG
  Remove the specified reg from the specified regset
*/
inline VOID REGSET_Remove(REGSET& regset, REG reg) { regset.Remove(reg); }

/*! @ingroup REG
  Remove all registers from the specified regset
*/
inline VOID REGSET_Clear(REGSET& regset) { regset.Clear(); }

/*! @ingroup REG
  Insert all registers into the specified regset
*/
inline VOID REGSET_AddAll(REGSET& regset) { regset.InsertAll(); }

/*! @ingroup REG
  Pop the next register from the specified regset
  @return the popped register
*/
inline REG REGSET_PopNext(REGSET& regset) { return regset.PopNext(); }

/*! @ingroup REG
  @return the number of registers in the specified regset
*/
inline UINT32 REGSET_PopCount(const REGSET& regset) { return regset.PopCount(); }

/*! @ingroup REG
  @return TRUE if the number of registers in the specified regset is zero
*/
inline BOOL REGSET_PopCountIsZero(const REGSET& regset) { return regset.PopCountIsZero(); }

/*! @ingroup REG
  @return a string with the names of all registers is the specified regset
*/
inline std::string REGSET_StringShort(const REGSET& regset) { return regset.String(); }

#endif // PIN_G_REGSET_IA32_PH
