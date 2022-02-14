/*
 * Copyright (C) 2014-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include "ialarm.H"
#include "alarm_manager.H"
#include "parse_control.H"
#include "control_chain.H"
#include "alarms.H"
#include <iostream>

using namespace CONTROLLER;

set< ADDRINT > IALARM::_thread_first_ip;
ADDRINT IALARM::_threads_first_ip_vec[PIN_MAX_THREADS];

IALARM::IALARM(UINT32 tid, UINT64 count, BOOL need_ctxt, ALARM_MANAGER* manager)
{
    _tid                 = tid;
    _target_count._count = count;
    _need_context        = need_ctxt;
    _alarm_manager       = manager;
    memset(_thread_count, 0, sizeof(_thread_count));
    memset(_armed, 0, sizeof(_armed));
    _global_armed          = 0;
    _activate_late_handler = FALSE;
    _address               = 0;
    PIN_InitLock(&_lock);

    static BOOL thread_callback_added = FALSE;
    if (!thread_callback_added)
    {
        PIN_CALLBACK thread_start = PIN_AddThreadStartFunction(ThreadStart, this);
        // other tools working with the controller might do some initialization in their
        // thread start callback.
        // need to make sure we are been call AFTER all thread start callbacks were called.
        CALLBACK_SetExecutionOrder(thread_start, CALL_ORDER_LAST);
    }
}

VOID IALARM::InsertIfCall_Count(IALARM* alarm, INS ins, UINT32 ninst, IPOINT point)
{
    if (alarm->HasGlobalCounter())
    {
        // Add global count instrumentation
        INS_InsertIfCall(ins, point, AFUNPTR(GlobalCount), IARG_FAST_ANALYSIS_CALL, IARG_CALL_ORDER, alarm->GetInstrumentOrder(),
                         IARG_ADDRINT, alarm, IARG_THREAD_ID, IARG_UINT32, ninst, IARG_END);
    }
    else
    {
        // Add per thread count instrumentation
        INS_InsertIfCall(ins, point, AFUNPTR(Count), IARG_FAST_ANALYSIS_CALL, IARG_CALL_ORDER, alarm->GetInstrumentOrder(),
                         IARG_ADDRINT, alarm, IARG_THREAD_ID, IARG_UINT32, ninst, IARG_END);
    }
}

VOID IALARM::InsertThenCall_Fire(IALARM* alarm, INS ins, IPOINT point)
{
    if (alarm->_need_context)
    {
        INS_InsertThenCall(ins, point, AFUNPTR(Fire), IARG_CALL_ORDER, alarm->GetInstrumentOrder(), IARG_ADDRINT, alarm,
                           IARG_CONTEXT, IARG_INST_PTR, IARG_THREAD_ID, IARG_END);
    }
    else
    {
        INS_InsertThenCall(ins, point, AFUNPTR(Fire), IARG_CALL_ORDER, alarm->GetInstrumentOrder(), IARG_ADDRINT, alarm,
                           IARG_ADDRINT, static_cast< ADDRINT >(0), // pass a null as context,
                           IARG_INST_PTR, IARG_THREAD_ID, IARG_END);
    }
}

// Insert late file instrumentation
VOID IALARM::Insert_LateInstrumentation(IALARM* alarm, INS ins)
{
    // Check if late handler is set
    if (!alarm->_alarm_manager->HasLateHandler()) return;

    // Determine ipoint
    IPOINT ipoint = IPOINT_AFTER;
    if (INS_IsInterrupt(ins) || INS_IsSyscall(ins))
    {
        // We don't want the region of interest (in tracing)
        // to include these instructions. Since they close
        // the trace we can't take their next instruction,
        // therefore don't deliver the late handler at all.
        return;
    }

    if (INS_IsValidForIpointTakenBranch(ins))
    {
        ipoint = IPOINT_TAKEN_BRANCH;
    }

    // Add if-then analysis routines
    INS_InsertIfCall(ins, ipoint, AFUNPTR(ActivateLate), IARG_FAST_ANALYSIS_CALL, IARG_CALL_ORDER,
                     alarm->GetLateInstrumentOrder(), IARG_ADDRINT, alarm, IARG_THREAD_ID, IARG_END);
    INS_InsertThenCall(ins, ipoint, AFUNPTR(LateFire), IARG_CALL_ORDER, alarm->GetLateInstrumentOrder(), IARG_ADDRINT, alarm,
                       IARG_CONST_CONTEXT, IARG_INST_PTR, IARG_THREAD_ID, IARG_END);
}

// Thread specific Counter analysis routine
ADDRINT PIN_FAST_ANALYSIS_CALL IALARM::Count(IALARM* ialarm, UINT32 tid, UINT32 ninst)
{
    UINT32 armed       = ialarm->_armed[tid];
    UINT32 correct_tid = (ialarm->_tid == tid) | (ialarm->_tid == ALL_THREADS);

    UINT32 should_count = armed & correct_tid;

    //if we are not in the correct thread
    ialarm->_thread_count[tid]._count += ninst * (should_count);

    // Return if we have reached thread counter
    return should_count & (ialarm->_thread_count[tid]._count >= ialarm->_target_count._count);
}

// Global counter analysis routine
ADDRINT PIN_FAST_ANALYSIS_CALL IALARM::GlobalCount(IALARM* ialarm, UINT32 tid, UINT32 ninst)
{
    UINT32 should_count = ialarm->_global_armed;

    // Increment global counter
    ATOMIC::OPS::Increment< UINT64 >(&ialarm->_global_count._count, ninst * (should_count));

    // Return if we have global counter
    return should_count & (ialarm->_global_count._count >= ialarm->_target_count._count);
}

//we want to generate the context only when we really need it.
//that is way most of the code is in the If instrumentation.
//even if the If instrumentation is be not inlined.
VOID IALARM::Fire(IALARM* ialarm, CONTEXT* ctxt, VOID* ip, UINT32 tid)
{
    // Check if flags was not already modified by another thread
    // in interactive controller
    if (ialarm->_alarm_manager->GetAlarmTypeFromManager() == ALARM_TYPE_INTERACTIVE)
    {
        ALARM_INTERACTIVE* interactive_alarm = static_cast< ALARM_INTERACTIVE* >(ialarm);
        if (!interactive_alarm->GetListener()->CheckClearSignal()) return;
    }

    // Check if we need to activate late handler
    // We should not activate it whenever this is precondition event
    if (ialarm->_alarm_manager->HasLateHandler() && ialarm->_alarm_manager->GetEventType() != EVENT_PRECOND)
        ialarm->_activate_late_handler = TRUE;

    ialarm->_alarm_manager->Fire(ctxt, ip, tid);
}

// Late fire event
ADDRINT PIN_FAST_ANALYSIS_CALL IALARM::ActivateLate(IALARM* ialarm, UINT32 tid)
{
    BOOL correct_tid = (ialarm->_tid == tid) | (ialarm->_tid == ALL_THREADS);

    return ialarm->_activate_late_handler & correct_tid;
}
VOID IALARM::LateFire(IALARM* ialarm, CONTEXT* ctxt, VOID* ip, UINT32 tid)
{
    BOOL activate_late_fire = FALSE;

    // Check if the late handler flag is set under lock
    PIN_GetLock(&ialarm->_lock, 0);
    if (ialarm->_activate_late_handler)
    {
        ialarm->_activate_late_handler = FALSE;
        activate_late_fire             = TRUE;
    }
    PIN_ReleaseLock(&ialarm->_lock);

    // Activate late fire
    if (activate_late_fire) ialarm->_alarm_manager->LateFire(ctxt, ip, tid);
}

VOID IALARM::Arm()
{
    PIN_GetLock(&_lock, 0);
    memset(_armed, 1, sizeof(_armed));
    _global_armed = 1;
    PIN_ReleaseLock(&_lock);
}

VOID IALARM::Disarm(THREADID tid)
{
    _armed[tid] = 0;
    if (!_alarm_manager->IsVectorAlarm()) _thread_count[tid]._count = 0;
}

VOID IALARM::Disarm()
{
    PIN_GetLock(&_lock, 0);
    memset(_armed, 0, sizeof(_armed));
    memset(_thread_count, 0, sizeof(_thread_count));
    _global_count._count = 0;
    _global_armed        = 0;
    PIN_ReleaseLock(&_lock);
}

BOOL IALARM::HasGlobalCounter() { return _alarm_manager->HasGlobalCounter(); }

UINT32 IALARM::GetInstrumentOrder() { return _alarm_manager->GetInsOrder(); }

UINT32 IALARM::GetLateInstrumentOrder() { return _alarm_manager->GetLateInsOrder(); }

VOID IALARM::TraceAddress(TRACE trace, VOID* v)
{
    IALARM* ialarm     = static_cast< IALARM* >(v);
    ADDRINT trace_addr = TRACE_Address(trace);
    UINT32 trace_size  = TRACE_Size(trace);

    for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
    {
        // Check Target
        // Get the last instruction in the BBL
        INS ins = BBL_InsTail(bbl);

        // Handle direct branches or calls
        if (INS_IsDirectControlFlow(ins))
        {
            // Get the target and compare it to the address we need
            ADDRINT target = INS_DirectControlFlowTargetAddress(ins);
            if (target == ialarm->_address)
            {
                InsertIfCall_Count(ialarm, ins, 1, IPOINT_TAKEN_BRANCH);
                InsertThenCall_Fire(ialarm, ins, IPOINT_TAKEN_BRANCH);

                // Add late handler instrumentation if needed
                Insert_LateInstrumentation(ialarm, ins);
            }
        }

        // Handle indirect branches or calls
        else if (INS_IsIndirectControlFlow(ins) && !INS_IsXend(ins))
        {
            InsertIfCall_Target(ialarm, ins);
            InsertThenCall_Fire(ialarm, ins, IPOINT_TAKEN_BRANCH);

            // Add late handler instrumentation if needed
            Insert_LateInstrumentation(ialarm, ins);
        }

        // If the address is not inside the trace then no need to check the
        // instructions in the BBL
        if (ialarm->_address < trace_addr || ialarm->_address > trace_addr + trace_size) continue;

        // Handle all other instructions in the BBL which may be before
        // The instruction we are looking for
        for (INS ins = BBL_InsHead(bbl); INS_Valid(ins); ins = INS_Next(ins))
        {
            // Compare the address of instructions itself
            // This is to handle rare scenario of first ip in the thread
            if (INS_Address(ins) == ialarm->_address && _thread_first_ip.find(ialarm->_address) != _thread_first_ip.end())
            {
                IPOINT ipoint = IPOINT_AFTER;
                if (INS_IsValidForIpointTakenBranch(ins))
                {
                    ipoint = IPOINT_TAKEN_BRANCH;
                }
                InsertIfCall_FirstIp(ialarm, ins, ipoint);
                InsertThenCall_Fire(ialarm, ins, ipoint);

                // Add late handler instrumentation if needed
                Insert_LateInstrumentation(ialarm, ins);
            }

            // Only relevant for instructions will fall through path
            if (!INS_HasFallThrough(ins)) return;

            // Compare the address of the next instruction to check if we
            // encountered the address of the alarm
            if (INS_NextAddress(ins) == ialarm->_address)
            {
                InsertIfCall_Count(ialarm, ins, 1, IPOINT_AFTER);
                InsertThenCall_Fire(ialarm, ins, IPOINT_AFTER);

                // Add late handler instrumentation if needed
                Insert_LateInstrumentation(ialarm, ins);
            }
        }
    }
}

// Instrumentation of indirect branch checking
VOID IALARM::InsertIfCall_Target(IALARM* alarm, INS ins)
{
    if (alarm->HasGlobalCounter())
        INS_InsertIfCall(ins, IPOINT_TAKEN_BRANCH, AFUNPTR(CheckTargetGlobal), IARG_FAST_ANALYSIS_CALL, IARG_CALL_ORDER,
                         alarm->GetInstrumentOrder(), IARG_PTR, alarm, IARG_BRANCH_TARGET_ADDR, IARG_END);
    else
        INS_InsertIfCall(ins, IPOINT_TAKEN_BRANCH, AFUNPTR(CheckTarget), IARG_FAST_ANALYSIS_CALL, IARG_CALL_ORDER,
                         alarm->GetInstrumentOrder(), IARG_PTR, alarm, IARG_THREAD_ID, IARG_BRANCH_TARGET_ADDR, IARG_END);
}

// Instrumentation of first ip address checking
VOID IALARM::InsertIfCall_FirstIp(IALARM* alarm, INS ins, IPOINT point)
{
    if (alarm->HasGlobalCounter())
        INS_InsertIfCall(ins, point, AFUNPTR(CheckFirstIpGlobal), IARG_FAST_ANALYSIS_CALL, IARG_CALL_ORDER,
                         alarm->GetInstrumentOrder(), IARG_PTR, alarm, IARG_THREAD_ID, IARG_INST_PTR, IARG_END);
    else
        INS_InsertIfCall(ins, point, AFUNPTR(CheckFirstIp), IARG_FAST_ANALYSIS_CALL, IARG_CALL_ORDER, alarm->GetInstrumentOrder(),
                         IARG_PTR, alarm, IARG_THREAD_ID, IARG_INST_PTR, IARG_END);
}

// Check if we have reached the target we need for global counter
ADDRINT PIN_FAST_ANALYSIS_CALL IALARM::CheckTarget(IALARM* ialarm, THREADID tid, ADDRINT branch_target)
{
    UINT32 armed       = ialarm->_armed[tid];
    UINT32 correct_tid = (ialarm->_tid == tid) | (ialarm->_tid == ALL_THREADS);

    UINT32 should_count = armed & correct_tid & (ialarm->_address == branch_target);

    // Increment counter if needed
    ialarm->_thread_count[tid]._count += should_count;

    // Return if we have reached thread or global counter
    return should_count & (ialarm->_thread_count[tid]._count >= ialarm->_target_count._count);
}

// Check if we have reached the target we need
ADDRINT PIN_FAST_ANALYSIS_CALL IALARM::CheckTargetGlobal(IALARM* ialarm, ADDRINT branch_target)
{
    UINT32 should_count = ialarm->_global_armed & (ialarm->_address == branch_target);

    // Increment global counter
    ATOMIC::OPS::Increment< UINT64 >(&ialarm->_global_count._count, should_count);

    // Return if we have reached thread or global counter
    return should_count & (ialarm->_global_count._count >= ialarm->_target_count._count);
}

// Check if we have reached the target we need in the first ip of the thread
ADDRINT PIN_FAST_ANALYSIS_CALL IALARM::CheckFirstIp(IALARM* ialarm, THREADID tid, ADDRINT addr)
{
    UINT32 armed       = ialarm->_armed[tid];
    UINT32 correct_tid = (ialarm->_tid == tid) | (ialarm->_tid == ALL_THREADS);

    UINT32 should_count = armed & correct_tid & (addr == _threads_first_ip_vec[tid]);

    // Reset the vector value of this thread so that next time
    // we will not count it and comparison to first ip will not return true.
    _threads_first_ip_vec[tid] = 0;

    // Increment counter if needed
    ialarm->_thread_count[tid]._count += should_count;

    // Return if we have reached thread or global counter
    return should_count & (ialarm->_thread_count[tid]._count >= ialarm->_target_count._count);
}

// Check if we have reached the target we need in the first ip of the thread
// for global counting
ADDRINT PIN_FAST_ANALYSIS_CALL IALARM::CheckFirstIpGlobal(IALARM* ialarm, THREADID tid, ADDRINT addr)
{
    UINT32 should_count = ialarm->_global_armed & (addr == _threads_first_ip_vec[tid]);

    // Reset the vector value of this thread so that next time
    // we will not count it and comparison to first ip will not return true.
    _threads_first_ip_vec[tid] = 0;

    // Increment global counter
    ATOMIC::OPS::Increment< UINT64 >(&ialarm->_global_count._count, should_count);

    // Return if we have reached thread or global counter
    return should_count & (ialarm->_global_count._count >= ialarm->_target_count._count);
}

// Thread start callback
VOID IALARM::ThreadStart(THREADID tid, CONTEXT* ctxt, INT32 flags, VOID* v)
{
    ADDRINT first_ip = PIN_GetContextReg(ctxt, REG_INST_PTR);
    _thread_first_ip.insert(first_ip);
    _threads_first_ip_vec[tid] = first_ip;
    // this IP might be already instrumented, so we need to reset
    // the instrumentations on it's BB.
    PIN_RemoveInstrumentationInRange(first_ip, first_ip + 15);

    // Check if we need to fix thread id
    // This is possible if thread translation callback is set
    // This is only relevant if this is not a global counter because in global
    // counters we summarize all threads and there is not meaning to tid.
    IALARM* ialarm = static_cast< IALARM* >(v);
    if (!ialarm->HasGlobalCounter() &&
        ialarm->_alarm_manager->GetControlChain()->GetControlManager()->GetThreadTransCallback() != NULL)
    {
        THREADID trans_tid =
            (ialarm->_alarm_manager->GetControlChain()->GetControlManager()->GetThreadTransCallback())(tid, NULL);
        if (trans_tid != tid && ialarm->_tid == trans_tid)
        {
            ialarm->_armed[tid]       = TRUE;
            ialarm->_armed[trans_tid] = FALSE;
            ialarm->_tid              = tid;
        }
    }
}
