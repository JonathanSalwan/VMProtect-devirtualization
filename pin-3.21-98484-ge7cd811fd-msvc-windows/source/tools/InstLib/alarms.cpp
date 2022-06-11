/*
 * Copyright (C) 2014-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include "alarms.H"
#include "alarm_manager.H"
#include "call-stack.H"
#include <iostream>
#include <string>

using namespace std;
using namespace CONTROLLER;
using namespace CALLSTACK;

//*****************************************************************************
VOID ALARM_ICOUNT::Activate() { TRACE_AddInstrumentFunction(Trace, this); }

VOID ALARM_ICOUNT::Trace(TRACE trace, VOID* v)
{
    ALARM_ICOUNT* icount_alarm = static_cast< ALARM_ICOUNT* >(v);
    for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
    {
        INS ins      = BBL_InsHead(bbl);
        UINT32 ninst = BBL_NumIns(bbl);
        InsertIfCall_Count(icount_alarm, ins, ninst);
        InsertThenCall_Fire(icount_alarm, ins);
    }
}

//*****************************************************************************

VOID ALARM_SSC::Activate() { TRACE_AddInstrumentFunction(Trace, this); }

VOID ALARM_SSC::Trace(TRACE trace, VOID* v)
{
    ALARM_SSC* ssc_alarm                = static_cast< ALARM_SSC* >(v);
    UINT32 h                            = Uint32FromString("0x" + ssc_alarm->_ssc);
    const UINT32 pattern_len            = 8;
    const unsigned int movebx_size      = 5;
    const unsigned int special_nop_size = 3;
    unsigned int ins_size = 0, next_ins_size = 0;
    EXCEPTION_INFO excep = EXCEPTION_INFO();
    INS ins, next_ins = INS_Invalid();
    //the template of ssc marker
    unsigned char ssc_marker[] = {0xbb, 0x00, 0x00, 0x00, 0x00, 0x64, 0x67, 0x90};
    for (int j = 0; j < 4; j++)
    {
        //fill in the ssc value
        ssc_marker[1 + j] = (h >> (j * 8)) & 0xff;
    }

    for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
    {
        ins = BBL_InsHead(bbl);
        if (INS_Valid(ins))
        {
            ins_size = INS_Size(ins);
            next_ins = INS_Next(ins);
        }
        while (INS_Valid(next_ins))
        {
            next_ins_size = INS_Size(next_ins);
            if (ins_size + next_ins_size == pattern_len)
            {
                unsigned char* pc;
                pc = reinterpret_cast< unsigned char* >(INS_Address(ins));

                unsigned char dst_buf[pattern_len];
                size_t copy_size = PIN_FetchCode(dst_buf, pc, pattern_len, &excep);
                if (copy_size == pattern_len && memcmp(ssc_marker, dst_buf, pattern_len) == 0)
                {
                    InsertIfCall_Count(ssc_alarm, ins, 1);
                    InsertThenCall_Fire(ssc_alarm, ins);

                    // Add late handler instrumentation if needed
                    Insert_LateInstrumentation(ssc_alarm, ins);
                }
            }
            ins      = next_ins;
            ins_size = next_ins_size;
            next_ins = INS_Next(next_ins);
        }

        //For the last instruction in the BBL, we want to check the next
        //instruction, the head of the next BBL.
        if (ins_size == movebx_size)
        {
            BBL next_bbl = BBL_Next(bbl);
            if (BBL_Valid(next_bbl))
            {
                next_ins = BBL_InsHead(next_bbl);
            }
            //If the head of the next BBL can potentailly be joint with the
            //current ins to an ssc_mark, or if this is the last instruction in
            //the trace.
            if (!INS_Valid(next_ins) || INS_Size(next_ins) == special_nop_size)
            {
                unsigned char* pc;
                pc = reinterpret_cast< unsigned char* >(INS_Address(ins));

                unsigned char dst_buf[pattern_len];
                size_t copy_size = PIN_FetchCode(dst_buf, pc, pattern_len, &excep);
                if (copy_size == pattern_len && memcmp(ssc_marker, dst_buf, pattern_len) == 0)
                {
                    InsertIfCall_Count(ssc_alarm, ins, 1);
                    InsertThenCall_Fire(ssc_alarm, ins);

                    // Add late handler instrumentation if needed
                    Insert_LateInstrumentation(ssc_alarm, ins);
                }
            }
        }
    }
}

//*****************************************************************************

VOID ALARM_ITEXT::Activate() { TRACE_AddInstrumentFunction(Trace, this); }

VOID ALARM_ITEXT::Trace(TRACE trace, VOID* v)
{
    ALARM_ITEXT* itext_alarm = static_cast< ALARM_ITEXT* >(v);
    UINT32 pattern_len       = itext_alarm->_itext.length();
    UINT32 pattern_bytes     = pattern_len / 2; //nibbels -> bytes

    EXCEPTION_INFO excep  = EXCEPTION_INFO();
    const size_t max_inst = 15;
    unsigned char pattern_duf[max_inst];
    PARSER::str2hex(itext_alarm->_itext.c_str(), pattern_duf, pattern_len);

    for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
    {
        for (INS ins = BBL_InsHead(bbl); INS_Valid(ins); ins = INS_Next(ins))
        {
            if (INS_Size(ins) == pattern_bytes)
            {
                unsigned char* pc;
                pc = reinterpret_cast< unsigned char* >(INS_Address(ins));

                unsigned char dst_buf[max_inst];
                size_t copy_size = PIN_FetchCode(dst_buf, pc, pattern_bytes, &excep);

                if (copy_size == pattern_bytes && memcmp(pattern_duf, dst_buf, pattern_bytes) == 0)
                {
                    InsertIfCall_Count(itext_alarm, ins, 1);
                    InsertThenCall_Fire(itext_alarm, ins);
                }
            }
        }
    }
}

//*****************************************************************************

VOID ALARM_INT3::Activate() { TRACE_AddInstrumentFunction(Trace, this); }

VOID ALARM_INT3::Trace(TRACE trace, VOID* v)
{
    for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
    {
        for (INS ins = BBL_InsHead(bbl); INS_Valid(ins); ins = INS_Next(ins))
        {
            xed_iclass_enum_t iclass;
            iclass = static_cast< xed_iclass_enum_t >(INS_Opcode(ins));
            if (iclass == XED_ICLASS_INT3)
            {
                ALARM_INT3* int3_alarm = static_cast< ALARM_INT3* >(v);
                InsertIfCall_Count(int3_alarm, ins, 1);
                InsertThenCall_Fire(int3_alarm, ins);

                INS_Delete(ins); // so no "int3" will be actually executed
            }
        }
    }
}

//*****************************************************************************

VOID ALARM_ISA_CATEGORY::Activate() { TRACE_AddInstrumentFunction(Trace, this); }

VOID ALARM_ISA_CATEGORY::Trace(TRACE trace, VOID* v)
{
    ALARM_ISA_CATEGORY* isa_ctg_alarm = static_cast< ALARM_ISA_CATEGORY* >(v);

    for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
    {
        for (INS ins = BBL_InsHead(bbl); INS_Valid(ins); ins = INS_Next(ins))
        {
            xed_category_enum_t category;
            category = static_cast< xed_category_enum_t >(INS_Category(ins));
            if (category == isa_ctg_alarm->_required_ctg)
            {
                InsertIfCall_Count(isa_ctg_alarm, ins, 1);
                InsertThenCall_Fire(isa_ctg_alarm, ins);

                // Add late handler instrumentation if needed
                Insert_LateInstrumentation(isa_ctg_alarm, ins);
            }
        }
    }
}

//*****************************************************************************

VOID ALARM_ISA_EXTENSION::Activate() { TRACE_AddInstrumentFunction(Trace, this); }

VOID ALARM_ISA_EXTENSION::Trace(TRACE trace, VOID* v)
{
    ALARM_ISA_EXTENSION* isa_ext_alarm = static_cast< ALARM_ISA_EXTENSION* >(v);

    for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
    {
        for (INS ins = BBL_InsHead(bbl); INS_Valid(ins); ins = INS_Next(ins))
        {
            xed_extension_enum_t extension;
            extension = static_cast< xed_extension_enum_t >(INS_Extension(ins));
            if (extension == isa_ext_alarm->_required_ext)
            {
                InsertIfCall_Count(isa_ext_alarm, ins, 1);
                InsertThenCall_Fire(isa_ext_alarm, ins);

                // Add late handler instrumentation if needed
                Insert_LateInstrumentation(isa_ext_alarm, ins);
            }
        }
    }
}

//*****************************************************************************

VOID ALARM_ADDRESS::Activate() { TRACE_AddInstrumentFunction(TraceAddress, this); }

//*****************************************************************************

VOID ALARM_SYMBOL::Activate()
{
    PIN_InitSymbols();
    //this is for finding the address of the required symbol
    IMG_AddInstrumentFunction(Img, this);
    TRACE_AddInstrumentFunction(TraceAddress, this);
}

VOID ALARM_SYMBOL::Img(IMG img, VOID* v)
{
    ALARM_SYMBOL* symbol_alarm = static_cast< ALARM_SYMBOL* >(v);

    for (SYM sym = IMG_RegsymHead(img); SYM_Valid(sym); sym = SYM_Next(sym))
    {
        string symbol = SYM_Name(sym);
        if (symbol == symbol_alarm->_symbol)
        {
            symbol_alarm->_address = SYM_Value(sym) + IMG_LoadOffset(img);
            return;
        }
    }
}

//*****************************************************************************

VOID ALARM_IMAGE::Activate()
{
    PIN_InitSymbols();
    //this is for finding the address of the required symbol
    IMG_AddInstrumentFunction(Img, this);
    TRACE_AddInstrumentFunction(TraceAddress, this);
}

VOID ALARM_IMAGE::Img(IMG img, VOID* v)
{
    ALARM_IMAGE* image_alarm = static_cast< ALARM_IMAGE* >(v);
    string img_name          = IMG_Name(img);
    bool found               = false;

    if (img_name == image_alarm->_image)
        found = true;
    else
    {
        // check against the base name of the image
        string::size_type pos = img_name.rfind('/');
#if defined(TARGET_WINDOWS)
        string::size_type wpos = img_name.rfind('\\');
        if (pos == string::npos)
            pos = wpos;
        else if (wpos != string::npos && wpos > pos)
            pos = wpos;
#endif
        if (pos != string::npos)
        {
            string basename = img_name.substr(pos + 1);
            if (basename == image_alarm->_image) found = true;
        }
    }

    if (found)
    {
        image_alarm->_address = IMG_LowAddress(img) + image_alarm->_offset;
        return;
    }
}

//*****************************************************************************

VOID ALARM_INTERACTIVE::Activate()
{
    _listener = _alarm_manager->GetListener();
    if (_listener == NULL)
    {
        PIN_ERROR("interactive controller must be used "
                  "with the knob -interactive_file <file name>\n\n");
    }

    TRACE_AddInstrumentFunction(Trace, this);
}

VOID ALARM_INTERACTIVE::Trace(TRACE trace, VOID* v)
{
    ALARM_INTERACTIVE* alarm = static_cast< ALARM_INTERACTIVE* >(v);

    for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
    {
        INS ins = BBL_InsHead(bbl);
        InsertInteractiveIf(alarm, ins);
        InsertThenCall_Fire(alarm, ins);
    }
}

VOID ALARM_INTERACTIVE::InsertInteractiveIf(ALARM_INTERACTIVE* alarm, INS ins)
{
    INS_InsertIfCall(ins, IPOINT_BEFORE, AFUNPTR(InteractiveShouldFire), IARG_FAST_ANALYSIS_CALL, IARG_CALL_ORDER,
                     alarm->GetInstrumentOrder(), IARG_ADDRINT, alarm, IARG_THREAD_ID, IARG_END);
}

ADDRINT PIN_FAST_ANALYSIS_CALL ALARM_INTERACTIVE::InteractiveShouldFire(ALARM_INTERACTIVE* alarm, UINT32 tid)
{
    UINT32 armed       = alarm->_armed[tid];
    UINT32 correct_tid = (alarm->_tid == tid) | (alarm->_tid == ALL_THREADS);

    // In order to make sure that this routine is inline we call
    // GetClearSignal that checks if we can clear signal
    // The actual modification by cmpxchgl is done in Fire routine
    return (armed & correct_tid & alarm->_listener->GetClearSignal());
}

//*****************************************************************************

VOID ALARM_ENTER_FUNC::Activate()
{
    CallStackManager* mngr = CallStackManager::get_instance();
    mngr->activate();

    //register the callback
    mngr->on_function_enter(OnFunctionStart, _func_name, this, _need_context);
}

VOID ALARM_ENTER_FUNC::OnFunctionStart(CONTEXT* ctxt, ADDRINT ip, THREADID tid, VOID* v)
{
    ALARM_ENTER_FUNC* alarm = static_cast< ALARM_ENTER_FUNC* >(v);
    if ((!alarm->HasGlobalCounter() && alarm->Count(alarm, tid, 1)) ||
        (alarm->HasGlobalCounter() && alarm->GlobalCount(alarm, tid, 1)))
    {
        alarm->_alarm_manager->Fire(ctxt, reinterpret_cast< VOID* >(ip), tid);
    }
}

//*****************************************************************************

VOID ALARM_EXIT_FUNC::Activate()
{
    CallStackManager* mngr = CallStackManager::get_instance();
    mngr->activate();

    //register the callback
    mngr->on_function_exit(OnFunctionEnd, _func_name, this, _need_context);
}

VOID ALARM_EXIT_FUNC::OnFunctionEnd(CONTEXT* ctxt, ADDRINT ip, THREADID tid, VOID* v)
{
    ALARM_EXIT_FUNC* alarm = static_cast< ALARM_EXIT_FUNC* >(v);
    if ((!alarm->HasGlobalCounter() && alarm->Count(alarm, tid, 1)) ||
        (alarm->HasGlobalCounter() && alarm->GlobalCount(alarm, tid, 1)))
    {
        alarm->_alarm_manager->Fire(ctxt, reinterpret_cast< VOID* >(ip), tid);
    }
}

//*****************************************************************************

VOID ALARM_CPUID::Activate() { TRACE_AddInstrumentFunction(Trace, this); }

VOID ALARM_CPUID::Trace(TRACE trace, VOID* v)
{
    ALARM_CPUID* alarm_cpuid = static_cast< ALARM_CPUID* >(v);

    for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
    {
        for (INS ins = BBL_InsHead(bbl); INS_Valid(ins); ins = INS_Next(ins))
        {
            xed_iclass_enum_t iclass;
            iclass = static_cast< xed_iclass_enum_t >(INS_Opcode(ins));
            if (iclass == XED_ICLASS_CPUID)
            {
                INS_InsertIfCall(ins, IPOINT_BEFORE, AFUNPTR(ShouldFire), IARG_FAST_ANALYSIS_CALL, IARG_CALL_ORDER,
                                 alarm_cpuid->GetInstrumentOrder(), IARG_ADDRINT, alarm_cpuid, IARG_REG_VALUE, REG_GAX,
                                 IARG_UINT32, alarm_cpuid->_val, IARG_THREAD_ID, IARG_END);

                InsertThenCall_Fire(alarm_cpuid, ins);
            }
        }
    }
}

ADDRINT PIN_FAST_ANALYSIS_CALL ALARM_CPUID::ShouldFire(IALARM* alarm, ADDRINT eax, UINT32 val, UINT32 tid)
{
    // if eax match val then return apply the count and tid checks
    if (eax == val)
    {
        if (!alarm->HasGlobalCounter())
            return alarm->Count(alarm, tid, 1);
        else
            return alarm->GlobalCount(alarm, tid, 1);
    }

    return 0;
}

//*****************************************************************************

ALARM_MAGIC::ALARM_MAGIC(const string& val_str, UINT32 tid, UINT32 count, BOOL need_ctxt, ALARM_MANAGER* manager)
    : IALARM(tid, count, need_ctxt, manager)
{
    vector< string > tokens;
    PARSER::SplitArgs(".", val_str, tokens);
    ASSERT(tokens.size() == 2, "MAGIC alarm needs two parameters separated by \".\"");

    _a = PARSER::StringToUint32(tokens[0]);
    _b = PARSER::StringToUint32(tokens[1]);

    Activate();
}

VOID ALARM_MAGIC::Activate() { TRACE_AddInstrumentFunction(Trace, this); }

VOID ALARM_MAGIC::Trace(TRACE trace, VOID* v)
{
    ALARM_MAGIC* alarm_magic = static_cast< ALARM_MAGIC* >(v);

    for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
    {
        for (INS ins = BBL_InsHead(bbl); INS_Valid(ins); ins = INS_Next(ins))
        {
            // Simics-style magic instruction: xchg bx, bx
            if (INS_IsXchg(ins) && INS_OperandReg(ins, 0) == REG_BX && INS_OperandReg(ins, 1) == REG_BX)
            {
                INS_InsertIfCall(ins, IPOINT_BEFORE, AFUNPTR(ShouldFire), IARG_FAST_ANALYSIS_CALL, IARG_CALL_ORDER,
                                 alarm_magic->GetInstrumentOrder(), IARG_ADDRINT, alarm_magic, IARG_REG_VALUE, REG_GAX,
#if defined(__i386)
                                 IARG_REG_VALUE, REG_GDX,
#else
                                 IARG_REG_VALUE, REG_GBX,
#endif
                                 IARG_REG_VALUE, REG_GCX, IARG_UINT32, alarm_magic->_a, IARG_UINT32, alarm_magic->_b,
                                 IARG_THREAD_ID, IARG_END);

                InsertThenCall_Fire(alarm_magic, ins);
            }
        }
    }
}

ADDRINT PIN_FAST_ANALYSIS_CALL ALARM_MAGIC::ShouldFire(IALARM* alarm, ADDRINT eax, ADDRINT ebx, ADDRINT ecx, UINT32 a, UINT32 b,
                                                       UINT32 tid)
{
    // if eax/ebx/ecx match SIM_CMD_MARKER/a/b then return apply the count and tid checks
    if (eax == SIM_CMD_MARKER && ebx == a && ecx == b)
    {
        if (!alarm->HasGlobalCounter())
            return alarm->Count(alarm, tid, 1);
        else
            return alarm->GlobalCount(alarm, tid, 1);
    }

    return 0;
}

//*****************************************************************************

VOID ALARM_PCONTROL::Activate() { RTN_AddInstrumentFunction(Rtn, this); }

VOID ALARM_PCONTROL::Rtn(RTN rtn, VOID* v)
{
    ALARM_PCONTROL* alarm_pcontrol = static_cast< ALARM_PCONTROL* >(v);
    std::string rtn_name           = RTN_Name(rtn);

    if (rtn_name.find("MPI_Pcontrol") != std::string::npos)
    {
        RTN_Open(rtn);
        RTN_InsertCall(rtn, IPOINT_BEFORE, AFUNPTR(OnMpiPcontrol), IARG_CALL_ORDER, alarm_pcontrol->GetInstrumentOrder(),
                       IARG_ADDRINT, alarm_pcontrol, IARG_CONTEXT, IARG_INST_PTR, IARG_FUNCARG_ENTRYPOINT_VALUE, 1,
                       IARG_THREAD_ID, IARG_END);
        RTN_Close(rtn);
    }
}

VOID ALARM_PCONTROL::OnMpiPcontrol(ALARM_PCONTROL* alarm, CONTEXT* ctxt, ADDRINT ip, ADDRINT region_app, UINT32 tid)
{
    UINT32 length = alarm->_region.size() + 1;
    ASSERT(length < 128, "MPI_Pcontrol region name should be <128 characters");
    char region_app_local[128];
    PIN_SafeCopy(region_app_local, (const VOID*)region_app, length);

    if (strcmp(region_app_local, alarm->_region.c_str()) == 0)
    {
        if ((!alarm->HasGlobalCounter() && alarm->Count(alarm, tid, 1)) ||
            (alarm->HasGlobalCounter() && alarm->GlobalCount(alarm, tid, 1)))
        {
            alarm->_alarm_manager->Fire(ctxt, reinterpret_cast< VOID* >(ip), tid);
        }
    }
}

//*****************************************************************************

VOID ALARM_TIMEOUT::Activate()
{
    TRACE_AddInstrumentFunction(Trace, this);
    PIN_SpawnInternalThread(WaitForTimeout, this, 0, NULL);
}

VOID ALARM_TIMEOUT::WaitForTimeout(VOID* v)
{
    ALARM_TIMEOUT* timeout_alarm = static_cast< ALARM_TIMEOUT* >(v);
    while (1)
    {
        // Wait 100 millisecond till the alarm is armed
        // Timeout alarm is not relevant to any specific thread so just pick 0.
        while (!timeout_alarm->_armed[0])
        {
            PIN_Sleep(100);
        }

        PIN_Sleep(timeout_alarm->_seconds_timeout * 1000);
        timeout_alarm->_timeout_passed = TRUE;
        break;
    }
}

VOID ALARM_TIMEOUT::Trace(TRACE trace, VOID* v)
{
    ALARM_TIMEOUT* timeout_alarm = static_cast< ALARM_TIMEOUT* >(v);
    for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
    {
        INS ins = BBL_InsHead(bbl);
        InsertIfCall_CheckTime(timeout_alarm, ins);
        InsertThenCall_Fire(timeout_alarm, ins);
    }
}

// Instrumentation of time checking
VOID ALARM_TIMEOUT::InsertIfCall_CheckTime(ALARM_TIMEOUT* alarm, INS ins)
{
    INS_InsertIfCall(ins, IPOINT_BEFORE, AFUNPTR(CheckTime), IARG_FAST_ANALYSIS_CALL, IARG_CALL_ORDER,
                     alarm->GetInstrumentOrder(), IARG_ADDRINT, alarm, IARG_THREAD_ID, IARG_END);
}

// Check if we have reached the timeout we need
ADDRINT PIN_FAST_ANALYSIS_CALL ALARM_TIMEOUT::CheckTime(ALARM_TIMEOUT* ialarm, UINT32 tid)
{
    BOOL armed       = ialarm->_armed[tid];
    BOOL correct_tid = (ialarm->_tid == tid) | (ialarm->_tid == ALL_THREADS);

    return armed & correct_tid & ialarm->_timeout_passed;
}

//*****************************************************************************

VOID ALARM_SIGNAL::Activate()
{
#if (!defined(TARGET_WINDOWS))
    PIN_AddContextChangeFunction(ContextChangeCallback, this);
#endif
}

#if (!defined(TARGET_WINDOWS))
VOID ALARM_SIGNAL::ContextChangeCallback(THREADID tid, CONTEXT_CHANGE_REASON reason, const CONTEXT* ctxtFrom, CONTEXT* ctxtTo,
                                         INT32 sig, VOID* v)
{
    // Check if we need to file the signal alarm now for the specific thread
    ALARM_SIGNAL* ialarm = static_cast< ALARM_SIGNAL* >(v);
    BOOL armed           = ialarm->_armed[tid];
    BOOL correct_tid     = (ialarm->_tid == tid) | (ialarm->_tid == ALL_THREADS);

    // Fire alarm if we got the correct signal
    if (armed & correct_tid && reason == CONTEXT_CHANGE_REASON_SIGNAL && sig == ialarm->_signal_id)
    {
        ADDRINT ip = PIN_GetContextReg(ctxtFrom, REG_INST_PTR);
        Fire(ialarm, (CONTEXT*)ctxtFrom, (VOID*)ip, tid);
    }
}
#endif

//*****************************************************************************

VOID ALARM_IMAGE_LOAD::Activate() { IMG_AddInstrumentFunction(ImageLoad, this); }

VOID ALARM_IMAGE_LOAD::ImageLoad(IMG img, VOID* v)
{
    // Check if we need to file the signal alarm now for the specific thread
    ALARM_IMAGE_LOAD* image_alarm = static_cast< ALARM_IMAGE_LOAD* >(v);
    const string image_name       = IMG_Name(img);

    // Fire alarm if we got the correct image
    string image_file_name = ExtractFileName(image_name);
    if (image_file_name.compare(image_alarm->_image_name) == 0)
    {
        Fire(image_alarm, NULL, 0, PIN_ThreadId());
    }
}

// Find the last directory separator and return everything after that, or the
// whole string if there is no separator.
// On Windows we allow either '/' or '\\', since both are accepted by
// the kernel.
string ALARM_IMAGE_LOAD::ExtractFileName(const string& fullpath)
{
    size_t max_slash = 0;
    BOOL found_slash = FALSE;

    // Handle Unix slash
    size_t unix_slash = fullpath.rfind('/');
    if (unix_slash != string::npos)
    {
        max_slash   = unix_slash;
        found_slash = TRUE;
    }

    // Handle Windows slash
#if defined(TARGET_WINDOWS)
    size_t windows_slash = fullpath.rfind('\\');
    if (windows_slash != string::npos && windows_slash > max_slash)
    {
        max_slash   = windows_slash;
        found_slash = TRUE;
    }
#endif

    if (found_slash) max_slash++;
    return fullpath.substr(max_slash);
}
