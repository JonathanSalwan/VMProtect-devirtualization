/*
 * Copyright (C) 2020-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#ifdef PIN_G_REG_PARTIAL_IA32_PH
#error duplicate inclusion of reg_partial_ia32
#else
#define PIN_G_REG_PARTIAL_IA32_PH
/*! @file
  This file contains REG primitives
 */
/*! @ingroup REG
  Return TRUE if reg is a lower 16-bit register
 */
inline BOOL REG_is_Half16(const REG reg)
{
    const REG_CLASS_BITS rh16Mask = (_REGCBIT(REG_CLASS_GRH16)) | (_REGCBIT(REG_CLASS_FLAGS16)) | (_REGCBIT(REG_CLASS_IP16)) |
                                    (_REGCBIT(REG_CLASS_PIN_GRH16));

    return ((_regClassBitMapTable[reg] & rh16Mask) != 0);
}

/*! @ingroup REG
  return the register width for all regs.
 */
inline REGWIDTH REG_Width(REG reg) { return (_regWidthTable[reg]); }

/*! @ingroup REG
    Return TRUE if reg is a lower 32-bit register, actually any 32 bit register
 */
inline BOOL REG_is_Half32(const REG reg) { return (REG_Width(reg) == REGWIDTH_32); }

/*! @ingroup REG
  Return TRUE if reg is a lower 8-bit register
 */
inline BOOL REG_is_Lower8(const REG reg)
{
    const REG_CLASS_BITS rl8Mask = (_REGCBIT(REG_CLASS_GRL8)) | (_REGCBIT(REG_CLASS_PIN_GRL8));
    return ((_regClassBitMapTable[reg] & rl8Mask) != 0);
}

/*! @ingroup REG
  Return TRUE if reg is a upper 8-bit register
 */
inline BOOL REG_is_Upper8(const REG reg)
{
    const REG_CLASS_BITS ru8Mask = (_REGCBIT(REG_CLASS_GRU8)) | (_REGCBIT(REG_CLASS_PIN_GRU8));
    return ((_regClassBitMapTable[reg] & ru8Mask) != 0);
}

/*! @ingroup REG
  Return TRUE if reg is a upper or lower 8-bit register
 */
inline BOOL REG_is_Any8(const REG reg)
{
    const REG_CLASS_BITS r8Mask =
        (_REGCBIT(REG_CLASS_GRU8)) | (_REGCBIT(REG_CLASS_PIN_GRU8)) | (_REGCBIT(REG_CLASS_GRL8)) | (_REGCBIT(REG_CLASS_PIN_GRL8));
    return ((_regClassBitMapTable[reg] & r8Mask) != 0);
}

/*! @ingroup REG
  Return TRUE if reg is a partial register
 */
extern BOOL REG_is_partialreg(const REG reg);

#endif // PIN_G_REG_PARTIAL_IA32_PH
