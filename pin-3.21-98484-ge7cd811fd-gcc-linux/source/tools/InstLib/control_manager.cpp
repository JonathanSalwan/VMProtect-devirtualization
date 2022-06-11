/*
 * Copyright (C) 2014-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <sstream>
#include "control_manager.H"
#include "control_chain.H"
#include "alarm_manager.H"
#include "controller_events.H"
#include "parse_control.H"

using namespace std;
using namespace CONTROLLER;

VOID CONTROL_MANAGER::InitKnobs()
{
    _control_knob_family = new KNOB_COMMENT(_control_family, _family_description);
    _control_knob =
        new KNOB< string >(KNOB_MODE_APPEND, _control_family, "control", "", "Define control events and conditions", _prefix);

    _control_log_knob = new KNOB< BOOL >(KNOB_MODE_WRITEONCE, _control_family, "controller-log", "0",
                                         "Emit the controller's events log", _prefix);

    _control_log_file_knob = new KNOB< string >(KNOB_MODE_WRITEONCE, _control_family, "controller-olog", "controller-log.txt",
                                                "file name to emit controller's event log", _prefix);

    _control_default_start = new KNOB< BOOL >(KNOB_MODE_WRITEONCE, _control_family, "controller-default-start", "1",
                                              "add default START event", _prefix);

    //this is a hidden internal knob - for debugging
    _control_debug_knob =
        new KNOB< BOOL >(KNOB_MODE_WRITEONCE, "supported", "controller-debug", "0", "debug the controller", _prefix);

    _control_skip =
        new KNOB< string >(KNOB_MODE_WRITEONCE, _control_family, "skip", "", "number of instructions to skip", _prefix);

    _control_length = new KNOB< string >(KNOB_MODE_WRITEONCE, _control_family, "length", "",
                                         "Number of instructions to execute before stopping", _prefix);

    _control_start_address = new KNOB< string >(KNOB_MODE_APPEND, _control_family, "start_address", "",
                                                "Address and count to trigger a start", _prefix);

    _control_stop_address = new KNOB< string >(KNOB_MODE_APPEND, _control_family, "stop_address", "",
                                               "Address and count to trigger a start", _prefix);

    _control_start_ssc = new KNOB< string >(KNOB_MODE_APPEND, _control_family, "start_ssc_mark", "",
                                            "Trigger a start on seeing specified SSC marker -- "
                                            "big-endian hex  without an 0x prefix",
                                            _prefix);

    _control_stop_ssc = new KNOB< string >(KNOB_MODE_APPEND, _control_family, "stop_ssc_mark", "",
                                           "Trigger a stop on seeing specified SSC marker -- "
                                           "big-endian hex  without an 0x prefix",
                                           _prefix);

    _control_start_itext = new KNOB< string >(KNOB_MODE_APPEND, _control_family, "start-itext", "",
                                              "Trigger a start on seeing specified instruction bytes "
                                              "(max 15Bytes=30nibbles)",
                                              _prefix);

    _control_stop_itext = new KNOB< string >(KNOB_MODE_APPEND, _control_family, "stop-itext", "",
                                             "Trigger a stop on seeing specified instruction bytes "
                                             "(max 15Bytes=30nibbles)",
                                             _prefix);

    _control_start_int3 = new KNOB< string >(KNOB_MODE_APPEND, _control_family, "start-int3", "",
                                             "Trigger a start on seeing an 'int 3' instruction,"
                                             "Requires a count argument.",
                                             _prefix);

    _control_stop_int3 = new KNOB< string >(KNOB_MODE_APPEND, _control_family, "stop-int3", "",
                                            "Trigger a stop on seeing an 'int 3' instruction,"
                                            "Requires a count argument.",
                                            _prefix);

    _control_start_isa_ext = new KNOB< string >(KNOB_MODE_APPEND, _control_family, "start_extension", "",
                                                "Trigger a start on seeing an instruction from "
                                                "this XED ISA extension",
                                                _prefix);

    _control_stop_isa_ext = new KNOB< string >(KNOB_MODE_APPEND, _control_family, "stop_extension", "",
                                               "Trigger a stop on seeing an instruction from "
                                               "this XED ISA extension",
                                               _prefix);

    _control_start_isa_ctg = new KNOB< string >(KNOB_MODE_APPEND, _control_family, "start_category", "",
                                                "Trigger a start on seeing an instruction from "
                                                "this XED ISA category",
                                                _prefix);

    _control_stop_isa_ctg = new KNOB< string >(KNOB_MODE_APPEND, _control_family, "stop_category", "",
                                               "Trigger a stop on seeing an instruction from "
                                               "this XED ISA category",
                                               _prefix);
#if !defined(TARGET_WINDOWS)
    _control_interactive = new KNOB< string >(KNOB_MODE_APPEND, _control_family, "interactive_file", "",
                                              "name of file used for interactive communication", _prefix);
#else
    _control_interactive = new KNOB< string >(KNOB_MODE_APPEND, _control_family, "interactive-event", "",
                                              "name of event used for interactive communication", _prefix);
#endif
}

CONTROL_MANAGER::CONTROL_MANAGER(const string prefix, const string family, const string description)
{
    // The call order must be before restore check point in Replay
    _call_order           = CALL_ORDER_FIRST - 50 + 2;
    _late_call_order      = CALL_ORDER_LAST + 20;
    _prefix               = prefix;
    _control_family       = family;
    _family_description   = description;
    _uniform_alarm        = NULL;
    _interactive_listener = NULL;
    _late_handler         = FALSE;
    _pass_context         = FALSE;

    //add knobs definitions(new and old)
    InitKnobs();

    //create the region controller
    CONTROL_ARGS args(_prefix, _control_family);
    _iregions = new CONTROL_IREGIONS(args, this);

    // External regions fields
    _external_region_chains        = NULL;
    _set_external_region_triggered = NULL;
    _external_region_param         = NULL;

    memset(_control_chain_thread_vec, 0, sizeof(ADDRINT) * PIN_MAX_THREADS);

    _region_info_callback  = NULL;
    _region_info_param     = NULL;
    _thread_trans_callback = NULL;
}

VOID CONTROL_MANAGER::Fini(INT32, VOID* v)
{
    CONTROL_MANAGER* mngr = static_cast< CONTROL_MANAGER* >(v);
    if (mngr->_control_log_knob->Value())
    {
        mngr->_out.close();
    }
}

VOID CONTROL_MANAGER::RegisterHandler(CONTROL_HANDLER ch, VOID* val, BOOL passContext, CONTROL_HANDLER late_ch)
{
    CONTROL_HANDLER_PARAMS params;
    params.handler     = ch;
    params.val         = val;
    params.passContext = passContext;
    _control_handler.push_back(params);

    // Add late handler if needed
    // The late handler was added to support precise controller handling in logger
    // Previously logger handled controller events only at the end of a Trace.
    // Since original handler is called in IPOINT_BEFORE we can not call PIN_ExecuteAt
    // So we have added a late to be called in IPOINT_AFTER and then we call
    // PIN_ExecuteAt which terminates a Trace and is presice.
    if (late_ch)
    {
        params.handler = late_ch;
        _late_control_handler.push_back(params);
        if (!_late_handler)
        {
            _late_handler = TRUE;
            // Set late handler flags in alarms
            SetLateHandler();
        }
    }
}

VOID CONTROL_MANAGER::Activate()
{
    CHAIN_EVENT_VECTOR* controlChains = NULL;

    // Activate IREGION class and get controller knobs from it
    CHAIN_EVENT_VECTOR* regionControlChains = NULL;
    UINT32 regionControllerNum              = 0;
    _iregions->Activate(_pass_context, &regionControlChains);
    if (regionControlChains)
    {
        regionControllerNum = regionControlChains->size();
    }

    UINT32 num_chains = _control_knob->NumberOfValues();

    // We do not allow using region controller with other controller knobs
    if (num_chains > regionControllerNum && regionControllerNum > 0)
    {
        ASSERT(FALSE, "Illegal usage of -control together with -regions:in ");
    }

    if (_control_log_knob->Value())
    {
        //open the controller event log
        string filename = _control_log_file_knob->Value();
        _out.open(filename.c_str());
        AddIcountInstrumentation();
    }

    //we need to know if context is needed
    _pass_context = ShouldPassContext();

    if (_control_interactive->Value() != "")
    {
        // We do not allow using region controller with interactive controller
        if (regionControllerNum > 0)
        {
            ASSERT(FALSE, "Illegal usage of interactive controller together with -regions:in ");
        }
        string name           = _control_interactive->Value();
        _interactive_listener = new INTERACTIVE_LISTENER(name);
        _interactive_listener->Active();
        if (_control_log_knob->Value())
        {
            _out << "Interactive Controller created " << name << endl;
        }
    }

    //iterate all the chains and parse them
    for (UINT32 i = 0; i < num_chains; i++)
    {
        string chain_str = _control_knob->Value(i);

        if (_control_log_knob->Value())
        {
            // Emit the controller knobs
            _out << "Normal Controller knob " << i << " : " << chain_str << endl;
        }

        CONTROL_CHAIN* chain = new CONTROL_CHAIN(this);
        _control_chain.push_back(chain);

        //parse the chain - creates all the required alarms
        chain->Parse(chain_str);

        if (_control_debug_knob->Value())
        {
            chain->DebugPrint();
        }

        chain->Activate();
    }

    // Check if we got chain handler from iregions or external regions
    UINT32 num_handler_chains = 0;
    BOOL vector_chain         = FALSE;
    if (regionControllerNum)
    {
        // Support region controller
        num_handler_chains = regionControllerNum;
        controlChains      = regionControlChains;
        vector_chain       = TRUE;
    }

    // Iterate all chains with handles and parse them
    for (UINT32 i = 0; i < num_handler_chains; i++)
    {
        string chain_str    = (*controlChains)[i].chain_str;
        VOID* event_handler = (*controlChains)[i].event_handler;
        THREADID tid        = (*controlChains)[i].tid;

        if (_control_log_knob->Value())
        {
            // Emit the controller knobs
            _out << "Regions Controller knob " << i << " : " << chain_str << endl;
        }

        CONTROL_CHAIN* chain = new CONTROL_CHAIN(this, event_handler, vector_chain);
        _control_chain.push_back(chain);

        // Add to vector chain of the thread
        if (vector_chain)
        {
            // Search global string in chain str
            // if found that use thread id 0
            THREADID vector_tid = tid;
            if (chain_str.find(":global") != std::string::npos)
            {
                vector_tid = 0;
            }

            // Add check to the vector
            if (_control_chain_thread_vec[vector_tid] == NULL) _control_chain_thread_vec[vector_tid] = new CONTROL_CHAIN_VECTOR;
            _control_chain_thread_vec[vector_tid]->push_back(chain);
            chain->SetVectorIndex(_control_chain_thread_vec[vector_tid]->size() - 1);
        }

        // parse the chain - creates all the required alarms
        chain->Parse(chain_str);

        if (_control_debug_knob->Value())
        {
            chain->DebugPrint();
        }

        chain->Activate();
    }

    //add translation of old controller knobs
    if (AddOldKnobs())
    {
        // We do not allow using old controller knobs with interactive controller
        if (regionControllerNum > 0)
        {
            ASSERT(FALSE, "Illegal usage of old controller knobs together with -regions:in or external regions ");
        }
    }

    //if non of the chains has start event we must add one
    if (_control_default_start->Value())
    {
        AddDefaultStart();
    }

    PIN_AddFiniFunction(Fini, this);
}

VOID CONTROL_MANAGER::Fire(EVENT_TYPE eventID, CONTEXT* ctxt, VOID* ip, THREADID tid, BOOL bcast, VOID* event_handler,
                           CONTROL_CHAIN* chain)
{
    if (_control_log_knob->Value())
    {
        //emit the controllers events log
        _out << "TID" << tid << ":  event: " << _events.IDToString(eventID) << " at icount: " << _icount[tid]
             << " ip: " << hexstr(ip) << " handler: " << hexstr(event_handler) << endl;
    }

    // Handle iregion if active
    if (IregionsActive())
    {
        _iregions->SetTriggeredRegion(tid, event_handler);
    }

    // Handle external regions if active
    if (ExternalRegionActive())
    {
        BOOL legal_event = _set_external_region_triggered(tid, eventID, event_handler, _external_region_param);

        // In case we found overlap region we should not fire the event
        if (!legal_event)
        {
            if (_control_log_knob->Value())
            {
                //emit the controllers events log
                _out << "TID" << tid << ":  ignore non legal event: " << _events.IDToString(eventID)
                     << " at icount: " << _icount[tid] << " ip: " << hexstr(ip) << " handler: " << hexstr(event_handler) << endl;
            }

            // Disable more events from this chain
            if (chain) chain->SetBlockFire();
            return;
        }
    }

    //call all registered control handlers
    list< CONTROL_HANDLER_PARAMS >::iterator iter = _control_handler.begin();
    for (; iter != _control_handler.end(); iter++)
    {
        iter->handler(eventID, iter->val, ctxt, ip, tid, bcast);
    }
}

// Late Fire event
VOID CONTROL_MANAGER::LateFire(EVENT_TYPE eventID, CONTEXT* ctxt, VOID* ip, THREADID tid, BOOL bcast)
{
    if (_control_log_knob->Value())
    {
        //emit the controllers events log
        _out << "TID" << tid << ":  late Fire event: " << _events.IDToString(eventID) << " at icount: " << _icount[tid]
             << " ip: " << hexstr(ip) << endl;
    }

    //call all registered control handlers
    list< CONTROL_HANDLER_PARAMS >::iterator iter = _late_control_handler.begin();
    for (; iter != _late_control_handler.end(); iter++)
    {
        iter->handler(eventID, iter->val, ctxt, ip, tid, bcast);
    }
}

//iterate all the control handlers to check if at least one of
//them needs context
BOOL CONTROL_MANAGER::ShouldPassContext()
{
    // If there is a late handler then we need context
    if (_late_handler) return TRUE;

    // Check all handlers
    list< CONTROL_HANDLER_PARAMS >::iterator it = _control_handler.begin();
    for (; it != _control_handler.end(); it++)
        if (it->passContext) return TRUE;
    return FALSE;
}

VOID CONTROL_MANAGER::AddIcountInstrumentation()
{
    memset(_icount, 0, sizeof(_icount));
    TRACE_AddInstrumentFunction(Trace, this);
}

VOID CONTROL_MANAGER::Trace(TRACE trace, VOID* v)
{
    for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
    {
        INS ins           = BBL_InsHead(bbl);
        INT32 ninst       = BBL_NumIns(bbl);
        UINT32 call_order = static_cast< CONTROL_MANAGER* >(v)->_call_order;
        INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(ICount), IARG_CALL_ORDER, call_order, IARG_ADDRINT, v, IARG_UINT32, ninst,
                       IARG_THREAD_ID, IARG_END);
    }
}

VOID CONTROL_MANAGER::ICount(CONTROL_MANAGER* control_manager, UINT32 nins, THREADID thread)
{
    control_manager->_icount[thread] += nins;
}

BOOL CONTROL_MANAGER::AddOldKnobs()
{
    // create a "chain" from old controller knobs
    // in case length knob is used we need to append it to the generated chain
    UINT32 added_length = 0;
    added_length += CreateOldOne(_control_skip->Value(), "start", "icount", TRUE);
    added_length += CreateOld(_control_start_address, "start", "address", TRUE);
    added_length += CreateOld(_control_stop_address, "stop", "address");
    added_length += CreateOld(_control_start_ssc, "start", "ssc", TRUE);
    added_length += CreateOld(_control_stop_ssc, "stop", "ssc");
    added_length += CreateOld(_control_start_itext, "start", "itext", TRUE);
    added_length += CreateOld(_control_stop_itext, "stop", "itext");
    added_length += CreateOld(_control_start_int3, "start", "int3", TRUE);
    added_length += CreateOld(_control_stop_int3, "stop", "int3");
    added_length += CreateOld(_control_start_isa_ctg, "start", "isa_category", TRUE);
    added_length += CreateOld(_control_stop_isa_ctg, "stop", "isa_category");
    added_length += CreateOld(_control_start_isa_ext, "start", "isa_extension", TRUE);
    added_length += CreateOld(_control_stop_isa_ext, "stop", "isa_extension");

    //length knob was used, but no start event was specified - so we did
    //not add this stop event to any other chain.
    if (_control_length->Value() != "")
    {
        if (added_length == 0)
        {
            CreateOldOne(_control_length->Value(), "stop", "icount");
        }
    }

    return added_length > 0;
}

// translate the old controller knob to the new chain syntax
// the knob is expected to be of type APPEND
// add_length indicates whether we need to append
// the length knob - in case it is used by the user
UINT32 CONTROL_MANAGER::CreateOld(KNOB< string >* knob, const string& control_event, const string& alarm, BOOL add_length)
{
    UINT32 length_added = 0;
    for (UINT32 i = 0; i < knob->NumberOfValues(); i++)
    {
        length_added += CreateOldOne(knob->Value(i), control_event, alarm, add_length);
    }
    return length_added;
}

UINT32 CONTROL_MANAGER::CreateOldOne(const string& raw_knob, const string& control_event, const string& alarm, BOOL add_length)
{
    UINT32 length_added = 0;
    if (raw_knob != "")
    {
        CONTROL_CHAIN* chain = new CONTROL_CHAIN(this);
        string value;
        string count_str = "";
        string tid_str   = "";
        BOOL repeat      = FALSE;
        PARSER::ParseOldConfigTokens(raw_knob, value, count_str, tid_str, repeat);
        string config_tokes = "";
        if (count_str != "")
        {
            config_tokes += ":count" + count_str;
        }
        if (tid_str != "")
        {
            config_tokes += ":" + tid_str;
        }
        string s = control_event + ":" + alarm + ":" + value + config_tokes;

        if (add_length && _control_length->Value() != "")
        {
            //length knob is used - append it
            s += ",stop:icount:" + _control_length->Value();
            length_added = 1;
        }

        if (repeat)
        {
            s += ",repeat";
        }
        chain->Parse(s);
        _legacy_control_chain.push_back(chain);
        if (_control_debug_knob->Value())
        {
            chain->DebugPrint();
        }
        chain->Activate();
    }
    return length_added;
}

//check if we have at least one start event
BOOL CONTROL_MANAGER::HasStartEvent()
{
    list< CONTROL_CHAIN* >::iterator iter = _control_chain.begin();

    for (; iter != _control_chain.end(); iter++)
    {
        CONTROL_CHAIN* chain = *iter;
        if (chain->HasStartEvent())
        {
            return TRUE;
        }
    }

    iter = _legacy_control_chain.begin();
    for (; iter != _legacy_control_chain.end(); iter++)
    {
        CONTROL_CHAIN* chain = *iter;
        if (chain->HasStartEvent())
        {
            return TRUE;
        }
    }

    if (_iregions->IsActive())
    {
        return TRUE;
    }

    if (ExternalRegionActive())
    {
        return TRUE;
    }

    return FALSE;
}

// Set late handler in chains and alarms
VOID CONTROL_MANAGER::SetLateHandler()
{
    list< CONTROL_CHAIN* >::iterator iter = _control_chain.begin();

    for (; iter != _control_chain.end(); iter++)
    {
        CONTROL_CHAIN* chain = *iter;
        chain->SetLateHandler();
    }

    iter = _legacy_control_chain.begin();
    for (; iter != _legacy_control_chain.end(); iter++)
    {
        CONTROL_CHAIN* chain = *iter;
        chain->SetLateHandler();
    }
}

VOID CONTROL_MANAGER::AddDefaultStart()
{
    //no start event was defined.
    if (HasStartEvent() == FALSE)
    {
        _init_alarm.Activate(this);
    }
}

//find the chain id by the given name
UINT32 CONTROL_MANAGER::GetChainId(const string& chain_name)
{
    list< CONTROL_CHAIN* >::iterator iter = _control_chain.begin();
    for (; iter != _control_chain.end(); iter++)
    {
        CONTROL_CHAIN* chain = *iter;
        if (chain->GetName() == chain_name)
        {
            return chain->GetId();
        }
    }
    ASSERT(FALSE, "could not find chain name: " + chain_name);
    return 0; //pacify the compiler
}

//find the chain pointer by the Id
CONTROL_CHAIN* CONTROL_MANAGER::ChainById(UINT32 chain_id)
{
    list< CONTROL_CHAIN* >::iterator iter = _control_chain.begin();
    for (; iter != _control_chain.end(); iter++)
    {
        CONTROL_CHAIN* chain = *iter;
        if (chain->GetId() == chain_id)
        {
            return chain;
        }
    }
    return NULL;
}

IREGION* CONTROL_MANAGER::CurrentIregion(THREADID tid) const { return _iregions->LastTriggeredRegion(tid); }

BOOL CONTROL_MANAGER::IregionsActive() const { return _iregions->IsActive(); }

BOOL CONTROL_MANAGER::UniformActive() { return _uniform_alarm != NULL; }

BOOL CONTROL_MANAGER::UniformIsDone()
{
    if (_uniform_alarm)
    {
        return _uniform_alarm->IsUniformDone();
    }
    return FALSE;
}

THREADID CONTROL_MANAGER::GetUniformTid() { return _uniform_alarm->GetTid(); }

EVENT_TYPE CONTROL_MANAGER::AddEvent(const string& event_name) { return _events.AddEvent(event_name); }

string CONTROL_MANAGER::EventToString(EVENT_TYPE id) { return _events.IDToString(id); }

EVENT_TYPE CONTROL_MANAGER::EventStringToType(const string& event_name) { return _events.EventStringToType(event_name); }

UINT32 CONTROL_MANAGER::GetInsOrder() { return _call_order; }

UINT32 CONTROL_MANAGER::GetLateInsOrder() { return _late_call_order; }

// Get specific alarm manager according to index
CONTROL_CHAIN* CONTROL_MANAGER::GetNextControlChain(UINT32 index, THREADID tid)
{
    ASSERT(_control_chain_thread_vec[tid], "Did not find control chain vector for the thread");

    // Add to vector chain of the thread
    if (index + 1 < _control_chain_thread_vec[tid]->size())
    {
        CONTROL_CHAIN_VECTOR* control_chain_vec = _control_chain_thread_vec[tid];
        return (*control_chain_vec)[index + 1];
    }

    return NULL;
}

// Add external region chains
VOID CONTROL_MANAGER::AddExternalRegionChains(CHAIN_EVENT_VECTOR* external_region_chains,
                                              SET_EXTERNAL_REGION_TRIGGERED set_external_region_triggered, VOID* param)
{
    ASSERT(_external_region_chains == NULL, "Illegal to add more than one external chain to controller");
    ASSERT(external_region_chains != NULL, "Set external region chain is NULL");
    ASSERT(set_external_region_triggered != NULL, "Set external region trigger callback is NULL");
    _external_region_chains        = external_region_chains;
    _set_external_region_triggered = set_external_region_triggered;
    _external_region_param         = param;

    // Get external chains number
    UINT32 ExternalRegionsChainsNum = _external_region_chains->size();
    if (ExternalRegionsChainsNum == 0) return;

    // Sanity checks
    if (_control_knob->NumberOfValues() > 0)
    {
        ASSERT(FALSE, "Illegal usage of -control together with external regions ");
    }
    if (_control_interactive->Value() != "")
    {
        ASSERT(FALSE, "Illegal usage of interactive controller together with  external regions ");
    }

    // Iterate all chains with handles and parse them
    for (UINT32 i = 0; i < ExternalRegionsChainsNum; i++)
    {
        string chain_str    = (*_external_region_chains)[i].chain_str;
        VOID* event_handler = (*_external_region_chains)[i].event_handler;

        if (_control_log_knob->Value())
        {
            // Emit the controller knobs
            _out << "External Regions Controller knob " << i << " : " << chain_str << endl;
        }

        CONTROL_CHAIN* chain = new CONTROL_CHAIN(this, event_handler);
        _control_chain.push_back(chain);

        // parse the chain - creates all the required alarms
        chain->Parse(chain_str);

        if (_control_debug_knob->Value())
        {
            chain->DebugPrint();
        }

        chain->Activate();
    }

    // Disable default controller
    _init_alarm.Disable();
}
