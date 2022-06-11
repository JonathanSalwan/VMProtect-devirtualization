/*
 * Copyright (C) 2014-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <sstream>
#include "alarm_manager.H"
#include "controller_events.H"
#include "parse_control.H"
#include "alarms.H"
#include "pin.H"

using namespace CONTROLLER;

map< string, ALARM_TYPE > ALARM_MANAGER::InitAlarms()
{
    map< string, ALARM_TYPE > alarm_map;
    alarm_map["icount"]        = ALARM_TYPE_ICOUNT;
    alarm_map["address"]       = ALARM_TYPE_ADDRESS;
    alarm_map["ssc"]           = ALARM_TYPE_SSC;
    alarm_map["itext"]         = ALARM_TYPE_ITEXT;
    alarm_map["int3"]          = ALARM_TYPE_INT3;
    alarm_map["isa_extension"] = ALARM_TYPE_ISA_EXTENSION;
    alarm_map["isa_category"]  = ALARM_TYPE_ISA_CATEGORY;
    alarm_map["interactive"]   = ALARM_TYPE_INTERACTIVE;
    alarm_map["enter_func"]    = ALARM_TYPE_ENTER_FUNC;
    alarm_map["exit_func"]     = ALARM_TYPE_EXIT_FUNC;
    alarm_map["cpuid"]         = ALARM_TYPE_CPUID;
    alarm_map["magic"]         = ALARM_TYPE_MAGIC;
    alarm_map["pcontrol"]      = ALARM_TYPE_PCONTROL;
    alarm_map["timeout"]       = ALARM_TYPE_TIMEOUT;
#if !defined(TARGET_WINDOWS)
    alarm_map["signal"] = ALARM_TYPE_SIGNAL;
#endif
    alarm_map["image_load"] = ALARM_TYPE_IMAGE_LOAD;

    return alarm_map;
}

ALARM_TYPE ALARM_MANAGER::GetAlarmType(const string& alarm_name)
{
    map< string, ALARM_TYPE >::iterator iter = _alarm_map.find(alarm_name);
    if (iter == _alarm_map.end())
    {
        ASSERT(FALSE, "Unsupported alarm: " + alarm_name);
    }
    return iter->second;
}

ALARM_MANAGER::ALARM_MANAGER(const string& control_str, CONTROL_CHAIN* control_chain, UINT32 id, BOOL late_handler,
                             BOOL vector_alarm, UINT32 vector_index)
{
    _bcast          = FALSE;
    _tid            = ALL_THREADS;
    _count          = 1;
    _global_count   = FALSE;
    _raw_alarm      = control_str;
    _control_chain  = control_chain;
    _uniform_type   = FALSE;
    _arm_next       = TRUE;
    _id             = id;
    _alarm_map      = InitAlarms();
    _late_handler   = late_handler;
    _vector_alarm   = vector_alarm;
    _vector_index   = vector_index;
    _uniform_count  = 0;
    _uniform_period = 0;
    _uniform_length = 0;

    vector< string > control_tokens;

    PARSER::SplitArgs(":", control_str, control_tokens);
    if (PARSER::UniformToken(control_tokens))
    {
        ParseUniform(control_tokens);
        ParseCommon(control_tokens);

        _ialarm = GenUniform();
    }
    else
    {
        ParseEventId(control_tokens);
        ParseAlarm(control_tokens);
        ParseCommon(control_tokens);

        // In vector alarms we do not create all the alarms
        // but the first one in order to solve performance issues
        // in iregions
        if (!_vector_alarm || _vector_index == 0)
            _ialarm = GenerateAlarm();
        else
            _ialarm = NULL;
    }
}

IALARM* ALARM_MANAGER::GenUniform()
{
    BOOL ctxt = _control_chain->NeedContext();
    //FIXME: add comment
    return new ALARM_ICOUNT("1", _tid, 1, ctxt, this);
}

IALARM* ALARM_MANAGER::GenAddress()
{
    string hex = "0x";
    BOOL ctxt  = _control_chain->NeedContext();
    if (_alarm_value.compare(0, 2, hex) == 0)
    {
        //this is a raw address
        return new ALARM_ADDRESS(_alarm_value, _tid, _count, ctxt, this);
    }

    if (_alarm_value.find("+", 0) == string::npos)
    {
        //this is a symbol
        return new ALARM_SYMBOL(_alarm_value, _tid, _count, ctxt, this);
    }

    else
    {
        vector< string > tokens;
        PARSER::SplitArgs("+", _alarm_value, tokens);
        return new ALARM_IMAGE(tokens[0], tokens[1], _tid, _count, ctxt, this);
    }
}

IALARM* ALARM_MANAGER::GenerateAlarm()
{
    BOOL ctxt = _control_chain->NeedContext();
    switch (_alarm_type)
    {
        case ALARM_TYPE_ICOUNT:
            return new ALARM_ICOUNT(_alarm_value, _tid, _count, ctxt, this);
        case ALARM_TYPE_ADDRESS:
            return GenAddress();
        case ALARM_TYPE_SSC:
            return new ALARM_SSC(_alarm_value, _tid, _count, ctxt, this);
        case ALARM_TYPE_ISA_EXTENSION:
            return new ALARM_ISA_EXTENSION(_alarm_value, _tid, _count, ctxt, this);
        case ALARM_TYPE_ISA_CATEGORY:
            return new ALARM_ISA_CATEGORY(_alarm_value, _tid, _count, ctxt, this);
        case ALARM_TYPE_ITEXT:
            return new ALARM_ITEXT(_alarm_value, _tid, _count, ctxt, this);
        case ALARM_TYPE_INT3:
            return new ALARM_INT3(_alarm_value, _tid, _count, ctxt, this);
        case ALARM_TYPE_INTERACTIVE:
            return new ALARM_INTERACTIVE(_tid, ctxt, this);
        case ALARM_TYPE_ENTER_FUNC:
            return new ALARM_ENTER_FUNC(_alarm_value, _tid, _count, ctxt, this);
        case ALARM_TYPE_EXIT_FUNC:
            return new ALARM_EXIT_FUNC(_alarm_value, _tid, _count, ctxt, this);

        case ALARM_TYPE_CPUID:
            return new ALARM_CPUID(_alarm_value, _tid, _count, ctxt, this);
        case ALARM_TYPE_MAGIC:
            return new ALARM_MAGIC(_alarm_value, _tid, _count, ctxt, this);
        case ALARM_TYPE_PCONTROL:
            return new ALARM_PCONTROL(_alarm_value, _tid, _count, ctxt, this);
        case ALARM_TYPE_TIMEOUT:
            return new ALARM_TIMEOUT(_alarm_value, _tid, _count, ctxt, this);
        case ALARM_TYPE_SIGNAL:
            return new ALARM_SIGNAL(_alarm_value, _tid, _count, ctxt, this);
        case ALARM_TYPE_IMAGE_LOAD:
            return new ALARM_IMAGE_LOAD(_alarm_value, _tid, _count, ctxt, this);

        default:
            ASSERT(FALSE, "Unexcpected alarm");
    }
    return NULL; //pacify the compiler
}

VOID ALARM_MANAGER::ParseUniform(vector< string >& control_tokens)
{
    ASSERT(control_tokens.size() > 3, "usage uniform:<period>:<length>:<count>[:<tid>]");

    UINT32 uniform_tokens_num = 4;
    _event_name               = "uniform";
    _event_type               = EVENT_START;
    _uniform_type             = TRUE;
    _arm_next                 = FALSE;

    _uniform_period = PARSER::StringToUint64(control_tokens[1]);
    _uniform_length = PARSER::StringToUint64(control_tokens[2]);
    _uniform_count  = PARSER::StringToUint64(control_tokens[3]);

    // Check if the next token is thread id
    if (control_tokens.size() > 4 && control_tokens[4].substr(0, 3) == "tid")
    {
        // We found thread id
        _tid               = PARSER::StringToUint32(control_tokens[4].substr(3, control_tokens[4].length()));
        uniform_tokens_num = 5;
    }

    control_tokens.erase(control_tokens.begin(), control_tokens.begin() + uniform_tokens_num);

    ASSERT(_uniform_period >= _uniform_length, "uniform period must be "
                                               "larger than uniform length");

    _control_chain->SetUniformAlarm(this);
}

VOID ALARM_MANAGER::ParseEventId(vector< string >& control_tokens)
{
    ASSERT(control_tokens.size() > 0, "Usage: no event");

    _event_name = control_tokens[0];
    _event_type = _control_chain->EventStringToType(_event_name);
    control_tokens.erase(control_tokens.begin());
}

VOID ALARM_MANAGER::ParseAlarm(vector< string >& control_tokens)
{
    _alarm_name = control_tokens[0];
    _alarm_type = GetAlarmType(_alarm_name);

    // Only icount alarm can be vector
    if (_alarm_type != ALARM_TYPE_ICOUNT && _vector_alarm)
    {
        ASSERT(FALSE, "Only icount alarm can have vector alarm flag set");
    }

    if (_alarm_type == ALARM_TYPE_INTERACTIVE)
    {
        control_tokens.erase(control_tokens.begin());
        return;
    }
    ASSERT(control_tokens.size() > 1, "Usage Error: Alarm with no value");
    control_tokens.erase(control_tokens.begin());

    _alarm_value = control_tokens[0];
    control_tokens.erase(control_tokens.begin());

    // Parse global icount value
    if (_global_count && _vector_alarm)
    {
        _icount_alarm_value = PARSER::StringToUint64(_alarm_value);
    }
}

VOID ALARM_MANAGER::ParseCommon(vector< string >& control_tokens)
{
    BOOL used_tid    = FALSE;
    BOOL used_count  = FALSE;
    BOOL used_global = FALSE;

    for (UINT32 i = 0; i < control_tokens.size(); i++)
    {
        string token      = control_tokens[i];
        BOOL found_tid    = PARSER::ParseTIDToken(token, &_tid);
        BOOL found_bcast  = PARSER::ParseBcastToken(token, &_bcast);
        BOOL found_count  = PARSER::ParseCountToken(token, &_count);
        BOOL found_global = PARSER::ParseGlobalToken(token, &_global_count);
        BOOL found_repeat = PARSER::ParseRepeatToken(token);

        // Check if we got illegal token
        // ParseCommon is called from the constructor of ALARM_MANAGER
        // and after the event id and alarm value tokens where parsed.
        // When we have reached then we parse all remaining tokens.
        ASSERT(found_tid || found_bcast || found_count || found_global || found_repeat,
               "Usage: redundant token: " + control_tokens[i]);

        if (found_tid) used_tid = TRUE;
        if (found_count) used_count = TRUE;
        if (found_global) used_global = TRUE;
    }

    // global used only with count token
    if (used_global)
    {
        ASSERT(used_count || _alarm_type == ALARM_TYPE_ICOUNT,
               "Usage: global token must be used only with count token or icount alarm type");
    }

    // tid and global tokens are illegal together
    ASSERT(!used_tid || !used_global, "Usage: can not use both tid and global tokens");
}

VOID ALARM_MANAGER::SetNextUniformEvent(THREADID tid)
{
    if (_event_type == EVENT_START)
    {
        _event_type = EVENT_STOP;
        _ialarm->SetCount(_uniform_length);
        _ialarm->Arm();
        return;
    }

    if (_event_type == EVENT_STOP)
    {
        //completed one uniform cycle
        _uniform_count--;

        if (_uniform_count == 0)
        {
            //completed all uniform cycles, we can now arm the next alarm
            //in the chain(if exists)
            _arm_next = TRUE;
        }
        else
        {
            _event_type = EVENT_START;
            //setting the icount for the region until we get to "start"
            _ialarm->SetCount(_uniform_period - _uniform_length);
            _ialarm->Arm();
        }
        return;
    }
}

VOID ALARM_MANAGER::Fire(CONTEXT* ctx, VOID* ip, THREADID tid)
{
    if (!Disarm(tid)) return;
    EVENT_TYPE ev = _event_type;
    BOOL bcast    = _bcast;

    //saving the current event type since SetNextEvent will modify it.
    //must do SetNextEvent before Fire since its side effects are needed in
    //the fire function
    if (_uniform_type && !IsUniformDone())
    {
        //for uniform we reset all the counters
        Disarm();
        bcast = TRUE;
        SetNextUniformEvent(tid);
    }

    _control_chain->Fire(ev, ctx, ip, tid, bcast, _id);

    // Handle vector alarms
    // Whenever there is a Fire event for an alarm which is a part of
    // vector alarms we reuse this alarm for other alarm managers.
    // This is done in order to solve performance issues.
    // Currently it is supported only for icount alarms.
    // We do the following actions:
    // 1. Get the next alarm manager in the vector from the chain
    // 2. Update the icount alarm with the event data from the alarm manager
    // 3. Connect the alarm manager to the original icount alarm
    // 4. Arm the icount alarm so that it will Fire on the next icount
    // 5. Remove the connection from current alarm manager which fired from the icount alarm
    if (_vector_alarm)
    {
        // Update the alarm with the data from next alarm manager and arm it
        UINT32 tid_for_next_alarm = _tid;
        if (_global_count) tid_for_next_alarm = 0;
        ALARM_MANAGER* next_alarm = _control_chain->GetNextAlaramManager(_vector_index, tid_for_next_alarm);
        if (next_alarm)
        {
            _ialarm->UpdateAlarm(next_alarm, next_alarm->_alarm_value);
            next_alarm->_ialarm = _ialarm;
            if (_global_count)
                next_alarm->ArmAll();
            else
                next_alarm->ArmTID(_tid);
            _ialarm = NULL;
        }
    }
}

// Late fire
VOID ALARM_MANAGER::LateFire(CONTEXT* ctx, VOID* ip, THREADID tid)
{
    _control_chain->LateFire(_event_type, ctx, ip, tid, _bcast, _id);
}

BOOL ALARM_MANAGER::HasStartEvent() { return (_event_type == EVENT_START); }

VOID ALARM_MANAGER::Print()
{
    cerr << "EVENT: " << _event_name << endl;
    cerr << "ALARM: " << _alarm_name << endl;
    cerr << "VALUE: " << _alarm_value << endl;
    cerr << "TID: " << _tid << endl;
    cerr << "BCAST:" << _bcast << endl;
    cerr << "COUNT:" << _count << endl;
    cerr << "GLOBAL COUNT:" << _global_count << endl;
}

BOOL ALARM_MANAGER::IsUniformDone() { return _uniform_count == 0; }

VOID ALARM_MANAGER::Disarm() { _ialarm->Disarm(); }

BOOL ALARM_MANAGER::Disarm(THREADID tid)
{
    if (_bcast || _global_count)
    {
        // If the alarm is global then there is a chance that another thread
        // already disarmed it
        if (_global_count)
        {
            // Check if this is a race condition and we did not reach
            // the needed global count
            if (_vector_alarm && _icount_alarm_value > _ialarm->GetGlobalCount())
            {
                return FALSE;
            }
            BOOL armed = _ialarm->DisarmGlobalArmed();
            return armed;
        }

        _ialarm->Disarm();
        return TRUE;
    }
    else
    {
        _ialarm->Disarm(tid);
        return TRUE;
    }
}

VOID ALARM_MANAGER::Activate()
{
    if (_tid == ALL_THREADS || _global_count)
    {
        ArmAll();
    }
    else
    {
        ArmTID(_tid);
    }
}
