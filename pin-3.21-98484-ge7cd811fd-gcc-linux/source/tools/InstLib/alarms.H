/*
 * Copyright (C) 2014-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#ifndef _ALARMS_H_
#define _ALARMS_H_

#include "ialarm.H"
#include "parse_control.H"
extern "C"
{
#include "xed-interface.h"
}

//in this file we define all standard alarms
//each alarm inherits form IALRM
namespace CONTROLLER
{
//*****************************************************************************
class ALARM_ICOUNT : public IALARM
{
  public:
    ALARM_ICOUNT(const string& icount_str, UINT32 tid, UINT32 count, BOOL need_ctxt, ALARM_MANAGER* manager)
        : IALARM(tid, count, need_ctxt, manager)
    {
        UINT64 icount = PARSER::StringToUint64(icount_str);
        //for icount alarm the affective count is the a combination
        //of the icount value and the count value
        SetCount(icount * count);
        Activate();
    }

    // Update another alarm manager with its value
    VOID UpdateAlarm(ALARM_MANAGER* alarm_manager, const string& icount_str)
    {
        _alarm_manager = alarm_manager;
        UINT64 icount  = PARSER::StringToUint64(icount_str);
        SetCount(icount);
    }

  private:
    VOID Activate();
    static VOID Trace(TRACE trace, VOID* v);
};

//*****************************************************************************

class ALARM_ADDRESS : public IALARM
{
  public:
    ALARM_ADDRESS(const string& address, UINT32 tid, UINT32 count, BOOL need_ctxt, ALARM_MANAGER* manager)
        : IALARM(tid, count, need_ctxt, manager)
    {
        _address = PARSER::StringToUint64(address);
        Activate();
    }

  private:
    VOID Activate();
};

//*****************************************************************************
class ALARM_SYMBOL : public IALARM
{
  public:
    ALARM_SYMBOL(const string& symbol, UINT32 tid, UINT32 count, BOOL need_ctxt, ALARM_MANAGER* manager)
        : IALARM(tid, count, need_ctxt, manager), _symbol(symbol)
    {
        Activate();
    }

  private:
    VOID Activate();
    static VOID Img(IMG img, VOID* v);

    string _symbol;
};

//*****************************************************************************

class ALARM_IMAGE : public IALARM
{
  public:
    ALARM_IMAGE(const string& image, const string& offset, UINT32 tid, UINT32 count, BOOL need_ctxt, ALARM_MANAGER* manager)
        : IALARM(tid, count, need_ctxt, manager), _image(image)
    {
        _offset = PARSER::StringToUint64(offset);
        Activate();
    }

  private:
    VOID Activate();
    static VOID Img(IMG img, VOID* v);

    string _image;
    UINT64 _offset;
};

//*****************************************************************************
class ALARM_SSC : public IALARM
{
  public:
    ALARM_SSC(const string& ssc, UINT32 tid, UINT32 count, BOOL need_ctxt, ALARM_MANAGER* manager)
        : IALARM(tid, count, need_ctxt, manager), _ssc(ssc)
    {
        Activate();
    }

  private:
    string _ssc;
    static const UINT32 _pattern_len = 8;

    VOID Activate();
    static VOID Trace(TRACE trace, VOID* v);
};

//*****************************************************************************
class ALARM_ITEXT : public IALARM
{
  public:
    ALARM_ITEXT(const string& itext, UINT32 tid, UINT32 count, BOOL need_ctxt, ALARM_MANAGER* manager)
        : IALARM(tid, count, need_ctxt, manager), _itext(itext)
    {
        Activate();
    }

  private:
    string _itext;
    VOID Activate();
    static VOID Trace(TRACE trace, VOID* v);
};

//*****************************************************************************
class ALARM_INT3 : public IALARM
{
  public:
    ALARM_INT3(const string& icount_str, UINT32 tid, UINT32 count, BOOL need_ctxt, ALARM_MANAGER* manager)
        : IALARM(tid, count, need_ctxt, manager)
    {
        UINT64 icount = PARSER::StringToUint64(icount_str);
        //the count is the a combination
        //of the icount value and the count value
        SetCount(icount * count);
        Activate();
    }

  private:
    VOID Activate();
    static VOID Trace(TRACE trace, VOID* v);
};

//*****************************************************************************

class ALARM_ISA_CATEGORY : public IALARM
{
  public:
    ALARM_ISA_CATEGORY(const string& isa_ctg, UINT32 tid, UINT32 count, BOOL need_ctxt, ALARM_MANAGER* manager)
        : IALARM(tid, count, need_ctxt, manager)
    {
        _isa_ctg      = isa_ctg;
        _required_ctg = str2xed_category_enum_t(_isa_ctg.c_str());
        Activate();
    }

  private:
    string _isa_ctg;
    xed_category_enum_t _required_ctg;
    VOID Activate();
    static VOID Trace(TRACE trace, VOID* v);
};

//*****************************************************************************
class ALARM_ISA_EXTENSION : public IALARM
{
  public:
    ALARM_ISA_EXTENSION(const string& isa_ext, UINT32 tid, UINT32 count, BOOL need_ctxt, ALARM_MANAGER* manager)
        : IALARM(tid, count, need_ctxt, manager)
    {
        _isa_ext      = isa_ext;
        _required_ext = str2xed_extension_enum_t(_isa_ext.c_str());
        Activate();
    }

  private:
    string _isa_ext;
    xed_extension_enum_t _required_ext;
    VOID Activate();
    static VOID Trace(TRACE trace, VOID* v);
};

//*****************************************************************************
class ALARM_INTERACTIVE : public IALARM
{
  public:
    ALARM_INTERACTIVE(UINT32 tid, BOOL need_ctxt, ALARM_MANAGER* manager) : IALARM(tid, 1, need_ctxt, manager) { Activate(); }
    INTERACTIVE_LISTENER* GetListener() { return _listener; }

  private:
    INTERACTIVE_LISTENER* _listener;

    VOID Activate();
    static VOID Trace(TRACE trace, VOID* v);

    //add if analysis function
    static VOID InsertInteractiveIf(ALARM_INTERACTIVE* alarm, INS ins);

    static ADDRINT PIN_FAST_ANALYSIS_CALL InteractiveShouldFire(ALARM_INTERACTIVE* alarm, UINT32 tid);
};

//*****************************************************************************
class ALARM_ENTER_FUNC : public IALARM
{
  public:
    ALARM_ENTER_FUNC(const string& func, UINT32 tid, UINT32 count, BOOL need_ctxt, ALARM_MANAGER* manager)
        : IALARM(tid, count, need_ctxt, manager)
    {
        _func_name = func;
        Activate();
    }

  private:
    string _func_name;

    VOID Activate();
    static VOID OnFunctionStart(CONTEXT* ctxt, ADDRINT ip, THREADID tid, VOID* v);
};

//*****************************************************************************

class ALARM_EXIT_FUNC : public IALARM
{
  public:
    ALARM_EXIT_FUNC(const string& func, UINT32 tid, UINT32 count, BOOL need_ctxt, ALARM_MANAGER* manager)
        : IALARM(tid, count, need_ctxt, manager)
    {
        _func_name = func;
        Activate();
    }

  private:
    string _func_name;

    VOID Activate();
    static VOID OnFunctionEnd(CONTEXT* ctxt, ADDRINT ip, THREADID tid, VOID* v);
};

//*****************************************************************************

class ALARM_CPUID : public IALARM
{
  public:
    ALARM_CPUID(const string& val_str, UINT32 tid, UINT32 count, BOOL need_ctxt, ALARM_MANAGER* manager)
        : IALARM(tid, count, need_ctxt, manager)
    {
        UINT32 val = PARSER::StringToUint32(val_str);
        ASSERT((val < 0x10000), "CPUID alarm should be 16 bits value");
        _val = 0x4711 | (val << 16);
        Activate();
    }

  private:
    UINT32 _val;

    VOID Activate();
    static VOID Trace(TRACE trace, VOID* v);

    static ADDRINT PIN_FAST_ANALYSIS_CALL ShouldFire(IALARM* alarm, ADDRINT eax, UINT32 val, UINT32 tid);
};

//*****************************************************************************

class ALARM_MAGIC : public IALARM
{
  public:
    ALARM_MAGIC(const string& val_str, UINT32 tid, UINT32 count, BOOL need_ctxt, ALARM_MANAGER* manager);

  private:
    static const UINT32 SIM_CMD_MARKER = 4;
    UINT32 _a, _b;

    VOID Activate();
    static VOID Trace(TRACE trace, VOID* v);

    static ADDRINT PIN_FAST_ANALYSIS_CALL ShouldFire(IALARM* alarm, ADDRINT eax, ADDRINT ebx, ADDRINT ecx, UINT32 a, UINT32 b,
                                                     UINT32 tid);
};

//*****************************************************************************

class ALARM_PCONTROL : public IALARM
{
  public:
    ALARM_PCONTROL(const string& region, UINT32 tid, UINT32 count, BOOL need_ctxt, ALARM_MANAGER* manager)
        : IALARM(tid, count, need_ctxt, manager)
    {
        _region = region;
        Activate();
    }

  private:
    string _region;

    VOID Activate();
    static VOID Rtn(RTN rtn, VOID* v);

    static VOID OnMpiPcontrol(ALARM_PCONTROL* alarm, CONTEXT* ctxt, ADDRINT ip, ADDRINT region_app, UINT32 tid);
};

//*****************************************************************************
class ALARM_TIMEOUT : public IALARM
{
  public:
    ALARM_TIMEOUT(const string& timeout_str, UINT32 tid, UINT32 count, BOOL need_ctxt, ALARM_MANAGER* manager)
        : IALARM(tid, count, need_ctxt, manager)
    {
        _timeout_passed  = FALSE;
        _seconds_timeout = PARSER::StringToUint32(timeout_str);

        Activate();
    }

  private:
    // Private timeout data members
    UINT32 _seconds_timeout;
    volatile BOOL _timeout_passed;

    VOID Activate();

    // Internal thread routine
    static VOID WaitForTimeout(VOID* v);

    // Instrumentation routines
    static VOID Trace(TRACE trace, VOID* v);
    static VOID InsertIfCall_CheckTime(ALARM_TIMEOUT* alarm, INS ins);

    // Analysis routines
    static ADDRINT PIN_FAST_ANALYSIS_CALL CheckTime(ALARM_TIMEOUT* ialarm, UINT32 tid);
};

//*****************************************************************************
class ALARM_SIGNAL : public IALARM
{
  public:
    ALARM_SIGNAL(const string& signal_str, UINT32 tid, UINT32 count, BOOL need_ctxt, ALARM_MANAGER* manager)
        : IALARM(tid, count, need_ctxt, manager)
    {
        _signal_id = PARSER::StringToUint32(signal_str);
        Activate();
    }

  private:
    // Private signal data members and methods
    INT32 _signal_id;

    VOID Activate();

#if (!defined(TARGET_WINDOWS))
    static VOID ContextChangeCallback(THREADID tid, CONTEXT_CHANGE_REASON reason, const CONTEXT* ctxtFrom, CONTEXT* ctxtTo,
                                      INT32 sig, VOID* v);
#endif
};

//*****************************************************************************
class ALARM_IMAGE_LOAD : public IALARM
{
  public:
    ALARM_IMAGE_LOAD(const string& image_name, UINT32 tid, UINT32 count, BOOL need_ctxt, ALARM_MANAGER* manager)
        : IALARM(tid, count, need_ctxt, manager)
    {
        _image_name = ExtractFileName(image_name);
        Activate();
    }

  private:
    // Private data members and methods
    string _image_name;

    VOID Activate();
    static VOID ImageLoad(IMG img, VOID* v);
    static string ExtractFileName(const string& fullpath);
};

//*****************************************************************************

} // namespace CONTROLLER
#endif
