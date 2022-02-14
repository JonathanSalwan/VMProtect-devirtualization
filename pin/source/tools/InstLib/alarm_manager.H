/*
 * Copyright (C) 2014-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#ifndef _ALARM_MANAGER_H_
#define _ALARM_MANAGER_H_

#include <string>
#include <map>
#include "pin.H"
#include "ialarm.H"
#include "parse_control.H"

using namespace std;

namespace CONTROLLER
{
typedef enum
{
    ALARM_TYPE_ICOUNT,
    ALARM_TYPE_ADDRESS,
    ALARM_TYPE_SSC,
    ALARM_TYPE_ISA_EXTENSION,
    ALARM_TYPE_ISA_CATEGORY,
    ALARM_TYPE_ITEXT,
    ALARM_TYPE_INT3,
    ALARM_TYPE_INTERACTIVE,
    ALARM_TYPE_ENTER_FUNC,
    ALARM_TYPE_EXIT_FUNC,
    ALARM_TYPE_CPUID,
    ALARM_TYPE_MAGIC,
    ALARM_TYPE_PCONTROL,
    ALARM_TYPE_TIMEOUT,
    ALARM_TYPE_SIGNAL,
    ALARM_TYPE_IMAGE_LOAD
} ALARM_TYPE;

class ALARM_MANAGER
{
  public:
    ALARM_MANAGER(const string& alarm_str, CONTROL_CHAIN* control_chain, UINT32 id, BOOL late_handler, BOOL vector_alarm,
                  UINT32 vector_index);

    //arm all threads in the alarm
    VOID ArmAll()
    {
        if (_ialarm) _ialarm->Arm();
    }

    VOID Activate();

    //arm the alarm for thread id
    VOID ArmTID(THREADID tid)
    {
        if (_ialarm) _ialarm->Arm(tid);
    }

    //return True if the alarm raises start event
    BOOL HasStartEvent();

    //rolls the event to the control chain
    VOID Fire(CONTEXT* ctx, VOID* ip, THREADID tid);
    VOID LateFire(CONTEXT* ctx, VOID* ip, THREADID tid);

    BOOL ArmNext() { return _arm_next; }

    THREADID GetTid() { return _tid; }

    BOOL IsUniformDone();

    //print the alarm - for debug
    VOID Print();

    UINT32 GetInsOrder() { return _control_chain->GetInsOrder(); }
    UINT32 GetLateInsOrder() { return _control_chain->GetLateInsOrder(); }

    INTERACTIVE_LISTENER* GetListener() { return _control_chain->GetListener(); }

    //return the type of the alarm from the manager
    ALARM_TYPE GetAlarmTypeFromManager() { return _alarm_type; }

    // APIs for alarm manager late handler
    BOOL HasLateHandler() { return _late_handler; }
    VOID SetLateHandler() { _late_handler = TRUE; }

    // Get event type
    EVENT_TYPE GetEventType() { return _event_type; }

    // API is this is global counter
    inline BOOL HasGlobalCounter() { return _global_count; }

    // Is this is vector alarm
    BOOL IsVectorAlarm() { return _vector_alarm; }

    CONTROL_CHAIN* GetControlChain() { return _control_chain; }

  private:
    //extract the event id
    VOID ParseEventId(vector< string >& control_tokens);

    //extract the alarm name and value
    VOID ParseAlarm(vector< string >& control_tokens);

    //parse common alarm configs(tid,bcast,count)
    VOID ParseCommon(vector< string >& control_tokens);

    //parse the "uniform" token
    VOID ParseUniform(vector< string >& control_tokens);
    VOID SetNextUniformEvent(THREADID tid);
    BOOL Disarm(THREADID tid);
    VOID Disarm();

    //generate address type alarm(address, symbol, image)
    IALARM* GenAddress();

    IALARM* GenUniform();

    //generate an alarm
    IALARM* GenerateAlarm();

    //define the mapping of alarm name to ALARM_TYPE
    map< string, ALARM_TYPE > InitAlarms();

    //return the type of the alarm
    ALARM_TYPE GetAlarmType(const string& alarm_name);

    //full string of the alarm before parsing
    string _raw_alarm;

    //the name of the event (start,stop, ...)
    string _event_name;

    //the type of the event
    EVENT_TYPE _event_type;

    //the name of the alarm(icount,ssc-mark,...)
    string _alarm_name;

    ALARM_TYPE _alarm_type;

    //the value of the alarm
    string _alarm_value;
    UINT64 _icount_alarm_value;

    //indicates whether we need to broadcast the event to all the threads
    BOOL _bcast;

    //the thread id of the alarm
    UINT32 _tid;

    //fire only after #count
    UINT64 _count;

    // global counter flag
    BOOL _global_count;

    UINT32 _id;

    BOOL _uniform_type;
    UINT64 _uniform_length;
    UINT64 _uniform_period;
    UINT64 _uniform_count;
    BOOL _arm_next;

    //the generic alarm object - all the concrete alarm inherits from this class
    IALARM* _ialarm;
    CONTROL_CHAIN* _control_chain;

    map< string, ALARM_TYPE > _alarm_map;

    BOOL _late_handler;

    // Vector data members
    BOOL _vector_alarm;   // This is a flag if current alarm is a part of alarm vector
    UINT32 _vector_index; // Index of this alarm in the alarm vector
};                        //class
} // namespace CONTROLLER
#endif
