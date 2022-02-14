/*
 * Copyright (C) 2014-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#ifndef _CONTROL_MANAGER_H_
#define _CONTROL_MANAGER_H_

#include <iostream>
#include <fstream>
#include <list>
#include <map>
#include "pin.H"
#include "controller_events.H"
#include "init_alarm.H"
#include "interactive_listener.H"

//for backward compatibility with instlib
extern "C"
{
#include "xed-interface.h"
}

using namespace std;

namespace CONTROLLER
{
static const UINT32 ALL_THREADS = 0xFFFF;

typedef VOID (*CONTROL_HANDLER)(EVENT_TYPE, VOID*, CONTEXT*, VOID*, THREADID tid, BOOL bcast);

typedef struct
{
    CONTROL_HANDLER handler;
    VOID* val;
    bool passContext; // context requirement indicator
} CONTROL_HANDLER_PARAMS;

typedef struct
{
    string regionName;
    UINT32 regionId;
} CONTROL_REGION_INFO;

typedef CONTROL_REGION_INFO (*REGION_INFO_CALLBACK)(THREADID tid, VOID*);

// Set external region triggered callback
typedef BOOL (*SET_EXTERNAL_REGION_TRIGGERED)(THREADID tid, EVENT_TYPE event_type, VOID* event_handler, VOID* param);

// Thread translation callback
typedef THREADID (*THREAD_TRANS_CALLBACK)(THREADID tid, VOID*);

// This structure is used by iregions when creating
// new alarms inside the controller.
typedef struct
{
    string chain_str;
    VOID* event_handler;
    THREADID tid;
} CHAIN_EVENT;
typedef vector< CHAIN_EVENT > CHAIN_EVENT_VECTOR;

//forward decelerations
class CONTROL_CHAIN;
class ALARM_MANAGER;
class CONTROLLER_EVENTS;
class CONTROL_IREGIONS;
class IREGION;

typedef vector< CONTROL_CHAIN* > CONTROL_CHAIN_VECTOR;

class CONTROL_ARGS
{
  public:
    //this class is used only by the region controller
    CONTROL_ARGS(const string& prefix, string knob_family, UINT32 instrument_order = CALL_ORDER_DEFAULT)
        : _prefix(prefix), _knob_family(knob_family), _instrument_order(instrument_order)
    {}

    string get_prefix() const { return _prefix; }
    string get_knob_family() const { return _knob_family; }
    UINT32 get_instrument_order() const { return _instrument_order; }

  private:
    string _prefix;
    string _knob_family;
    UINT32 _instrument_order;
};

class CONTROL_MANAGER
{
  public:
    CONTROL_MANAGER(const string prefix = "", const string family = "pintool:control",
                    const string description = "Controller knobs");
    /* 
     * register tool handler to controller
     * ch - function to call when event triggers 
     * val - argument to pass for the called function
     * passContext - context's necessity in the called function
     * late_ch - function to call when event triggers after instruction and analysis routines execution
     */
    VOID RegisterHandler(CONTROL_HANDLER ch, VOID* val, BOOL passContext = FALSE, CONTROL_HANDLER late_ch = NULL);

    // activate the controller
    VOID Activate();

    //return the call order of the controller instrumentations
    UINT32 GetInsOrder();
    UINT32 GetLateInsOrder();

    //return the thread id of the uniform sampling
    THREADID GetUniformTid();

    //return True if uniform sampling is used
    BOOL UniformActive();

    //return True if uniform sampling id done
    BOOL UniformIsDone();

    //return the last triggered region
    IREGION* CurrentIregion(THREADID tid = 0) const;

    //return True if region control is active
    BOOL IregionsActive() const;

    //FIXME:I think this is not required since we have the focus tid knob
    BOOL StartTIDActive() { return FALSE; }

    EVENT_TYPE AddEvent(const string& event_name);
    string EventToString(EVENT_TYPE ev);
    EVENT_TYPE EventStringToType(const string& event_name);

    //adds a start event at the beginning of the run
    VOID AddDefaultStart();

    //trigger all registered control handlers
    //eventID - the Id of the event
    //tid     - the triggering thread
    //bcast   - whether this event affects all threads
    VOID Fire(EVENT_TYPE eventID, CONTEXT* ctx, VOID* ip, THREADID tid, BOOL bcast, VOID* event_handler = NULL,
              CONTROL_CHAIN* chain = NULL);

    //trigger all registered late control handlers
    //eventID - the Id of the event
    //tid     - the triggering thread
    //bcast   - whether this event affects all threads
    VOID LateFire(EVENT_TYPE eventID, CONTEXT* ctxt, VOID* ip, THREADID tid, BOOL bcast);

    // Return if the control manager has late handler
    BOOL HasLateHandler() { return _late_handler; }

    // Get specific alarm manager according to index
    CONTROL_CHAIN* GetNextControlChain(UINT32 index, THREADID tid);

    // Region name callback accessors
    REGION_INFO_CALLBACK GetRegionInfoCallback() { return _region_info_callback; }
    VOID* GetRegionInfoParam() { return _region_info_param; }
    VOID SetRegionInfoCallback(REGION_INFO_CALLBACK region_info_callback, VOID* region_info_param)
    {
        _region_info_callback = region_info_callback;
        _region_info_param    = region_info_param;
    }

    // Add external region chains
    // This is an external API for the controller that enables
    // other SDE tools to send chain strings to be activated by the controller
    //external_region_chains - external region chains vector
    //set_external_region_triggered - Pointer to a function that return is triggered event is legel
    //param - parameter handle
    VOID AddExternalRegionChains(CHAIN_EVENT_VECTOR* external_region_chains,
                                 SET_EXTERNAL_REGION_TRIGGERED set_external_region_triggered, VOID* param);

    // Thread translation callback
    VOID SetThreadTransCallback(THREAD_TRANS_CALLBACK thread_trans_callback) { _thread_trans_callback = thread_trans_callback; }
    THREAD_TRANS_CALLBACK GetThreadTransCallback() { return _thread_trans_callback; }

  private:
    //enable those classes to access private members
    friend class CONTROL_CHAIN;
    friend class CONTROL_IREGIONS;
    friend class INIT_ALARM;

    //add Icount instrumentation, used for logging controller events
    VOID AddIcountInstrumentation();

    static VOID Fini(INT32 i, VOID* v);
    static VOID Trace(TRACE trace, VOID* v);
    static VOID ICount(CONTROL_MANAGER* control_manager, UINT32 nins, THREADID thread);

    //return True is least one of the CONTROL_HANDLERS needs context.
    BOOL ShouldPassContext();

    //return True if one of the control chains has start event
    BOOL HasStartEvent();

    // Set late handler flags in chains and alarms
    VOID SetLateHandler();

    //translate all old controller knobs to the new "chain" format
    BOOL AddOldKnobs();

    //translate one old controller knob to the new chain format
    //the input is the knobs value
    UINT32 CreateOldOne(const string& value, const string& control_event, const string& alarm, BOOL add_length = FALSE);

    //translate one old controller knob to the new chain format
    //the knob is of type APPEND
    UINT32 CreateOld(KNOB< string >* knob, const string& control_event, const string& alarm, BOOL add_length = FALSE);

    //defines all controller knobs
    VOID InitKnobs();

    //return the int Id of the chain
    UINT32 GetChainId(const string& chain_name);

    //return a pointer to CHAIN with chain_id
    CONTROL_CHAIN* ChainById(UINT32 chain_id);

    BOOL PassContext() { return _pass_context; }

    INTERACTIVE_LISTENER* GetListener() { return _interactive_listener; }

    // Return is external region is active
    BOOL ExternalRegionActive() const { return _external_region_chains != NULL; }

    string _control_family;
    string _prefix;
    string _family_description;

    UINT32 _call_order;
    UINT32 _late_call_order;

    KNOB_COMMENT* _control_knob_family;
    KNOB< string >* _control_knob;
    KNOB< BOOL >* _control_debug_knob;
    KNOB< BOOL >* _control_log_knob;
    KNOB< string >* _control_log_file_knob;
    KNOB< BOOL >* _control_default_start;

    KNOB< string >* _control_skip;
    KNOB< string >* _control_length;
    KNOB< string >* _control_start_address;
    KNOB< string >* _control_stop_address;
    KNOB< string >* _control_start_ssc;
    KNOB< string >* _control_stop_ssc;
    KNOB< string >* _control_start_itext;
    KNOB< string >* _control_stop_itext;
    KNOB< string >* _control_start_int3;
    KNOB< string >* _control_stop_int3;
    KNOB< string >* _control_start_isa_ext;
    KNOB< string >* _control_stop_isa_ext;
    KNOB< string >* _control_start_isa_ctg;
    KNOB< string >* _control_stop_isa_ctg;
    KNOB< string >* _control_interactive;

    CONTROL_IREGIONS* _iregions;

    // Chains from external tool
    CHAIN_EVENT_VECTOR* _external_region_chains;
    SET_EXTERNAL_REGION_TRIGGERED _set_external_region_triggered;
    VOID* _external_region_param;

    //list of all control chains
    list< CONTROL_CHAIN* > _control_chain;

    //list of all chains translated from legacy knobs
    list< CONTROL_CHAIN* > _legacy_control_chain;

    //list of registered control handlers
    list< CONTROL_HANDLER_PARAMS > _control_handler;

    //list of late registered control handlers
    list< CONTROL_HANDLER_PARAMS > _late_control_handler;
    BOOL _late_handler;

    //the events manager
    CONTROLLER_EVENTS _events;

    BOOL _pass_context;
    UINT64 _icount[PIN_MAX_THREADS]; //for log generation
    ALARM_MANAGER* _uniform_alarm;
    ofstream _out;

    //this class is responsible for adding a default start event instrumentation
    INIT_ALARM _init_alarm;

    INTERACTIVE_LISTENER* _interactive_listener;

    CONTROL_CHAIN_VECTOR* _control_chain_thread_vec[PIN_MAX_THREADS];

    // Region info callback
    REGION_INFO_CALLBACK _region_info_callback;
    VOID* _region_info_param;

    // Thread translation callback
    THREAD_TRANS_CALLBACK _thread_trans_callback;
}; //class

} // namespace CONTROLLER
//must be here due to code dependency
#include "regions_control.H"
#endif
