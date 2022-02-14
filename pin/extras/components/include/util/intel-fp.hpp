/*
 * Copyright (C) 2010-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

// <COMPONENT>: util
// <FILE-TYPE>: component public header

#ifndef UTIL_INTEL_FP_HPP
#define UTIL_INTEL_FP_HPP

namespace UTIL
{
/*!
 * An 80-bit X87 data register padded out to 128-bits.
 */
union /*<POD>*/ X87REG_PADDED
{
    struct
    {
        UINT64 _significand; ///< The floating-point significand.
        UINT16 _exponent;    ///< The floating-point exponent, top bit is the sign bit.
        UINT16 _pad[3];
    } _fp;
    struct
    {
        UINT64 _lo; ///< Least significant part of value.
        UINT64 _hi; ///< Most significant part of value.
    } _raw;
};

/*!
 * A 128-bit XMM register value.
 */
union /*<POD>*/ XMMREG
{
    UINT8 _vec8[16];  ///< Vector of 16 8-bit elements.
    UINT16 _vec16[8]; ///< Vector of 8 16-bit elements.
    UINT32 _vec32[4]; ///< Vector of 4 32-bit elements.
    UINT64 _vec64[2]; ///< Vector of 2 64-bit elements.
};

/*!
 * The memory format written by the FXSAVE instruction for IA32.
 */
struct /*<POD>*/ FXSAVE_IA32
{
    UINT16 _fcw; ///< X87 control word.
    UINT16 _fsw; ///< X87 status word.
    UINT8 _ftw;  ///< Abridged X87 tag value.
    UINT8 _pad1;
    UINT16 _fop;   ///< Last X87 non-control instruction opcode.
    UINT32 _fpuip; ///< Last X87 non-control instruction address.
    UINT16 _cs;    ///< Last X87 non-control instruction CS selector.
    UINT16 _pad2;
    UINT32 _fpudp; ///< Last X87 non-control instruction operand address.
    UINT16 _ds;    ///< Last X87 non-control instruction operand DS selector.
    UINT16 _pad3;
    UINT32 _mxcsr;         ///< MXCSR control and status register.
    UINT32 _mxcsrmask;     ///< Mask of valid MXCSR bits.
    X87REG_PADDED _sts[8]; ///< X87 data registers in top-of-stack order.
    XMMREG _xmms[8];       ///< XMM registers.
    UINT8 _pad4[224];
};

/*!
 * The memory format written by the FXSAVE instruction for Intel64 (default operand size).
 */
struct /*<POD>*/ FXSAVE_INTEL64_DEFAULT
{
    UINT16 _fcw; ///< X87 control word.
    UINT16 _fsw; ///< X87 status word.
    UINT8 _ftw;  ///< Abridged X87 tag value.
    UINT8 _pad1;
    UINT16 _fop;   ///< Last X87 non-control instruction opcode.
    UINT32 _fpuip; ///< Last X87 non-control instruction segment offset.
    UINT16 _cs;    ///< Last X87 non-control instruction CS selector.
    UINT16 _pad2;
    UINT32 _fpudp; ///< Last X87 non-control instruction operand segment offset.
    UINT16 _ds;    ///< Last X87 non-control instruction operand DS selector.
    UINT16 _pad3;
    UINT32 _mxcsr;         ///< MXCSR control and status register.
    UINT32 _mxcsrmask;     ///< Mask of valid MXCSR bits.
    X87REG_PADDED _sts[8]; ///< X87 data registers in top-of-stack order.
    XMMREG _xmms[16];      ///< XMM registers.  (at most 16 registers, on 32 bit system only 8 are accessible).
    UINT8 _pad4[96];
};

/*!
 * The memory format written by the FXSAVE instruction for Intel64 (promoted operand size).
 */
struct /*<POD>*/ FXSAVE_INTEL64_PROMOTED
{
    UINT16 _fcw; ///< X87 control word.
    UINT16 _fsw; ///< X87 status word.
    UINT8 _ftw;  ///< Abridged X87 tag value.
    UINT8 _pad1;
    UINT16 _fop;           ///< Last X87 non-control instruction opcode.
    UINT64 _fpuip;         ///< Last X87 non-control instruction address.
    UINT64 _fpudp;         ///< Last X87 non-control operand address.
    UINT32 _mxcsr;         ///< MXCSR control and status register.
    UINT32 _mxcsrmask;     ///< Mask of valid MXCSR bits.
    X87REG_PADDED _sts[8]; ///< X87 data registers in top-of-stack order.
    XMMREG _xmms[16];      ///< XMM registers.
    UINT8 _pad4[96];
};

/*!
 * Convert an X87 tag register value from the 16-bit full form to its 8-bit
 * abridged form (as saved by the FXSAVE instruction).
 *
 *  @param[in] fullTag      The tag value in full form.
 *
 * @return  The tag value in abridged form.
 */
inline UINT8 GetX87AbridgedTag(UINT16 fullTag)
{
    const UINT16 empty = 3;

    UINT8 tags  = 0;
    UINT16 mask = 3;
    for (int i = 0; i < 8; i++)
    {
        UINT8 tag;
        if ((fullTag & mask) == (empty << 2 * i))
            tag = 0;
        else
            tag = 1;
        tags |= (tag << i);
        mask <<= 2;
    }

    return tags;
}

/*!
 * Get the 16-bit "full form" of X87 tag register value from an FXSAVE buffer.
 *
 *  @tparam FXSAVE      One of the three FXSAVE buffer types: FXSAVE_IA32,
 *                       FXSAVE_INTEL64_DEFAULT, or FXSAVE_INTEL64_PROMOTED.
 *  @param[in] fxsave   The FXSAVE buffer data.
 */
template< typename FXSAVE > UINT16 GetX87FullTag(const FXSAVE* fxsave)
{
    // This algorithm follows the algorithm in the IA-32 Intel Architecture Software Developer's
    // Manual, Volume 2A.  See the section on the FXSAVE instruction.

    UINT16 tags  = 0;
    UINT8 ftw    = fxsave->_ftw;
    unsigned tos = (fxsave->_fsw >> 11) & 0x7;

    const UINT16 valid   = 0;
    const UINT16 zero    = 1;
    const UINT16 special = 2;
    const UINT16 empty   = 3;
    const UINT64 jbit    = UINT64(1) << 63;

    for (unsigned i = 0; i < 8; i++)
    {
        UINT16 tag;

        if (!(ftw & 1))
        {
            tag = empty;
        }
        else
        {
            // The tag bits are in physical order, but the ST registers are in top-of-stack order.  Subtract the TOS
            // base to get the corresponding ST register.
            //
            unsigned streg           = (i - tos) & 0x7;
            const X87REG_PADDED* reg = &fxsave->_sts[streg];

            UINT16 exponent = reg->_fp._exponent & 0x7fff;
            if (exponent == 0x7fff)
            {
                tag = special;
            }
            else if (exponent == 0)
            {
                if (!reg->_fp._significand)
                {
                    tag = zero;
                }
                else
                {
                    tag = special;
                }
            }
            else
            {
                if (reg->_fp._significand & jbit)
                    tag = valid;
                else
                    tag = special;
            }
        }

        tags |= (tag << (2 * i));
        ftw >>= 1;
    }

    return tags;
}

} // namespace UTIL
#endif // file guard
