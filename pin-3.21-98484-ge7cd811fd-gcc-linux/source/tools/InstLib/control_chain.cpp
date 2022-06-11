/*
 * Copyright (C) 2014-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include "control_chain.H"
#include "parse_control.H"
#include "alarm_manager.H"
#include "controller_events.H"

using namespace std;
using namespace CONTROLLER;

UINT32 CONTROL_CHAIN::global_id = 0;

CONTROL_CHAIN::CONTROL_CHAIN(CONTROL_MANAGER* control_mngr, VOID* event_handler, BOOL vector_chain)
{
    _control_mngr  = control_mngr;
    _name          = "";
    _wait_for_id   = NO_WAIT;
    _repeat_token  = 1;
    _id            = global_id;
    _event_handler = event_handler;
    _block_fire    = FALSE;
    global_id++;
    _vector_chain = vector_chain;
    _vector_index = 0;

    memset(_repeat, 0, sizeof(_repeat));
}

VOID CONTROL_CHAIN::Fire(EVENT_TYPE eventID, CONTEXT* ctx, VOID* ip, THREADID tid, BOOL bcast, UINT32 alarm_id)
{
    // Check if fire is block for this chain
    if (_block_fire) return;

    //roll the event to the tool only if it is not a precondition
    if (eventID != EVENT_PRECOND)
    {
        _control_mngr->Fire(eventID, ctx, ip, tid, bcast, _event_handler, this);
    }

    // Check if fire is block for this chain before we arm the next event
    if (_block_fire) return;
    if (_alarms[alarm_id]->ArmNext())
    {
        ArmNextAlarm(alarm_id, tid, bcast);
    }
}

// Late fire event
VOID CONTROL_CHAIN::LateFire(EVENT_TYPE eventID, CONTEXT* ctx, VOID* ip, THREADID tid, BOOL bcast, UINT32 alarm_id)
{
    // Check if fire is block for this chain
    if (_block_fire) return;
    _control_mngr->LateFire(eventID, ctx, ip, tid, bcast);
}

VOID CONTROL_CHAIN::Activate()
{
    //activate the first alarm in the chain only if the chain does not "wait"
    if (_wait_for_id == NO_WAIT)
    {
        _alarms[0]->Activate();
    }
}

VOID CONTROL_CHAIN::ArmChain(UINT32 tid) { _alarms[0]->ArmTID(tid); }

VOID CONTROL_CHAIN::Arm(UINT32 tid, BOOL bcast, UINT32 alarm_id)
{
    if (bcast || _alarms[alarm_id]->HasGlobalCounter())
    {
        //if we are in broadcast arm all threads in the next alarm
        _alarms[alarm_id]->ArmAll();
    }
    else
    {
        _alarms[alarm_id]->ArmTID(tid);
    }
}

VOID CONTROL_CHAIN::Parse(const string& chain_str)
{
    vector< string > control_str;

    PARSER::SplitArgs(",", chain_str, control_str);
    for (UINT32 i = 0; i < control_str.size(); i++)
    {
        if (PARSER::ConfigToken(control_str[i]))
        {
            //parse all the config tokes(repeat,name,waitfor)
            PARSER::ParseConfigTokens(control_str[i], this);
        }
        else
        {
            //generate the alarm
            ALARM_MANAGER* alarm_mngr =
                new ALARM_MANAGER(control_str[i], this, i, _control_mngr->HasLateHandler(), _vector_chain, _vector_index);
            _alarms.push_back(alarm_mngr);
        }
    }
}

BOOL CONTROL_CHAIN::NeedContext() { return _control_mngr->PassContext(); }

BOOL CONTROL_CHAIN::NeedToRepeat(UINT32 tid)
{
    if (_repeat[tid] < _repeat_token || _repeat_token == REPEAT_INDEFINITELY)
    {
        return TRUE;
    }
    return FALSE;
}

VOID CONTROL_CHAIN::ArmNextAlarm(UINT32 alarm_id, UINT32 tid, BOOL bcast)
{
    UINT32 last_alarm_id = _alarms.size() - 1;
    if (alarm_id < last_alarm_id)
    {
        //we still have alarm after this one
        Arm(tid, bcast, alarm_id + 1);
    }
    else
    {
        //we are in the last alarm check the repeat
        _repeat[tid]++;
        if (NeedToRepeat(tid))
        {
            Arm(tid, bcast, 0);
        }
        else
        {
            ArmWaitingChains(tid);
        }
    }
}

VOID CONTROL_CHAIN::SetWaitFor(const string& chain_name)
{
    UINT32 id = _control_mngr->GetChainId(chain_name);
    SetWaitFor(id);
}

VOID CONTROL_CHAIN::SetWaitFor(UINT32 chain_id)
{
    CONTROL_CHAIN* chain = _control_mngr->ChainById(chain_id);
    if (chain == NULL)
    {
        stringstream s;
        s << "chain id " << chain_id << " does not exists";
        ASSERT(FALSE, s.str());
    }
    _wait_for_id = chain_id;
    chain->AddWaitingChain(this);
}

VOID CONTROL_CHAIN::AddWaitingChain(CONTROL_CHAIN* chain) { _waiting_chains.push_back(chain); }

VOID CONTROL_CHAIN::ArmWaitingChains(UINT32 tid)
{
    list< CONTROL_CHAIN* >::iterator iter = _waiting_chains.begin();
    for (; iter != _waiting_chains.end(); iter++)
    {
        CONTROL_CHAIN* chain = *iter;
        chain->ArmChain(tid);
    }
}

//print debug massages - only when the debug knob is used
VOID CONTROL_CHAIN::DebugPrint()
{
    for (UINT32 i = 0; i < _alarms.size(); i++)
    {
        _alarms[i]->Print();
    }
    cerr << "REPEAT: " << _repeat_token << endl;
    cerr << "NAME: " << _name << endl;
    cerr << "WAIT FOR: " << _wait_for_id << endl;
}

BOOL CONTROL_CHAIN::HasStartEvent()
{
    for (UINT32 i = 0; i < _alarms.size(); i++)
    {
        if (_alarms[i]->HasStartEvent())
        {
            return TRUE;
        }
    }
    return FALSE;
}

VOID CONTROL_CHAIN::SetUniformAlarm(ALARM_MANAGER* uniform_alarm)
{
    if (_control_mngr->_uniform_alarm)
    {
        ASSERT(FALSE, "Only one uniform control is allowed");
    }
    _control_mngr->_uniform_alarm = uniform_alarm;
}

EVENT_TYPE CONTROL_CHAIN::EventStringToType(const string& event_name) { return _control_mngr->EventStringToType(event_name); }

// Set late handler in all alarms
VOID CONTROL_CHAIN::SetLateHandler()
{
    for (UINT32 i = 0; i < _alarms.size(); i++)
    {
        _alarms[i]->SetLateHandler();
    }
}
