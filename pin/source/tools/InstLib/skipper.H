/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#ifndef SKIPPER_H
#define SKIPPER_H
namespace INSTLIB
{
/*! @defgroup SKIPPER
    This class allows one to skip (i.e. delete) a set of instructions.
    Currently, only "int3" skipping is supported.
*/

/*! @defgroup SKIP_INT3
  @ingroup SKIPPER
  Delete INT3 instruction on IA-32 and Intel(R) 64 architectures.
*/

/*! @ingroup SKIP_INT3
*/
class SKIP_INT3
{
  public:
    SKIP_INT3(const string& prefix, const string& knob_family)
        : _skipInt3(KNOB_MODE_WRITEONCE, knob_family, "skip_int3", "0", "Skip 'int 3' instructions.", prefix)
    {}

    bool IsActive() { return (_skipInt3); }

    /*! @ingroup SKIP_INT3
      Delete "int3" if the -skip_int3 knob is provided
      @return 1 if enabled, otherwise 0
    */
    INT32 CheckKnobs(VOID* val)
    {
        if (_skipInt3 == 0) return 0;
#if defined(TARGET_IA32) || defined(TARGET_IA32E)
        // Register Instruction to be called to instrument instructions
        TRACE_AddInstrumentFunction(instrument_trace, this);
#endif
        return 1;
    }

  private:
    static void instrument_trace(TRACE trace, void* pthis)
    {
#if defined(TARGET_IA32) || defined(TARGET_IA32E)
        for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
            for (INS ins = BBL_InsHead(bbl); INS_Valid(ins); ins = INS_Next(ins))
            {
                const xed_iclass_enum_t iclass = (xed_iclass_enum_t)INS_Opcode(ins);
                if (iclass == XED_ICLASS_INT3)
                {
                    INS_Delete(ins);
                }
            }
#endif
    }
    KNOB< BOOL > _skipInt3;
};

/*! @ingroup SKIPPER
*/
class SKIPPER
{
  public:
    /*! @ingroup SKIPPER
    */
    SKIPPER(const string& prefix = "", const string& knob_family = "pintool:control",
            const string& knob_family_description = "Skipper knobs")
        : _skipper_knob_family(knob_family, knob_family_description), _skip_int3(prefix, knob_family)
    {}
    /*! @ingroup SKIPPER
      Activate all the component controllers
    */
    INT32 CheckKnobs(VOID* val)
    {
        _val        = val;
        INT32 start = 0;
        start       = start + _skip_int3.CheckKnobs(this);
        return start;
    }
    bool INT3_skipped() { return _skip_int3.IsActive(); };

  private:
    KNOB_COMMENT _skipper_knob_family;
    VOID* _val;
    SKIP_INT3 _skip_int3;
};
} // namespace INSTLIB
#endif
