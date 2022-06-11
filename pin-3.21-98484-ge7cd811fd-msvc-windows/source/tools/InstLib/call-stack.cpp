/*
 * Copyright (C) 2014-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <iostream>
#include <sstream>
#include <iomanip>
#include <string.h>
#include "call-stack.H"
using std::dec;
using std::endl;
using std::hex;
using std::left;
using std::list;
using std::map;
using std::ostringstream;
using std::right;
using std::set;
using std::setfill;
using std::setw;
using std::string;
using std::vector;

#if defined(TARGET_WINDOWS)
#define strdup _strdup
#endif

using namespace CALLSTACK;
static REG vreg;
KNOB_COMMENT _comment("pintool:call-stack", "Call Stack knobs");
KNOB< BOOL > _knob_source_location(KNOB_MODE_WRITEONCE, "pintool:call-stack", "callstack:source_locaion", "1",
                                   "Emit source location (file,line,column) ");

///////////////////////// Analysis Functions //////////////////////////////////
static void a_process_call(ADDRINT target, ADDRINT sp, CallStack* call_stack)
{
    ASSERTX(call_stack);
    call_stack->process_call(sp, target);
}

static void a_process_return(ADDRINT sp, ADDRINT ip, CallStack* call_stack)
{
    ASSERTX(call_stack);
    call_stack->process_return(sp, ip);
}

static void a_on_call(ADDRINT target, CallStackManager* mngr, THREADID tid, CONTEXT* ctxt)
{
    ASSERTX(mngr);
    mngr->on_call(tid, ctxt, target);
}

static ADDRINT a_target_interesting(ADDRINT target, CallStackManager* mngr)
{
    ASSERTX(mngr);
    return mngr->TargetInteresting(target);
}

static ADDRINT a_on_ret_should_fire(THREADID tid, CallStackManager* mngr)
{
    ASSERTX(mngr);
    return mngr->on_ret_should_fire(tid);
}

static void a_on_ret_fire(THREADID tid, CONTEXT* ctxt, ADDRINT ip, CallStackManager* mngr)
{
    ASSERTX(mngr);
    mngr->on_ret_fire(tid, ctxt, ip);
}
/////////////////////// End Analysis Functions ////////////////////////////////

static void i_trace(TRACE trace, void* v)
{
    CallStackManager* mngr = CallStackManager::get_instance();
    ASSERTX(mngr);
    IARGLIST args = IARGLIST_Alloc();
    if (mngr->NeedContext())
    {
        IARGLIST_AddArguments(args, IARG_CONTEXT, IARG_END);
    }
    else
    {
        // pass a null as context
        IARGLIST_AddArguments(args, IARG_ADDRINT, static_cast< ADDRINT >(0), IARG_END);
    }

    for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
    {
        INS tail = BBL_InsTail(bbl);

        //in the original implementation there were special handling for
        //push
        //ret
        //on ia-32 windows
        //if this ever become an issue we should add this handling

        // We need a special check for RTM instructions cause they are
        // defined as branch as well
#if defined(SUPPORT_RTM)
        xed_decoded_inst_t const* const xedd = INS_XedDec(tail);
        xed_iclass_enum_t iclass             = xed_decoded_inst_get_iclass(xedd);
        if (iclass == XED_ICLASS_XBEGIN || iclass == XED_ICLASS_XEND || iclass == XED_ICLASS_XABORT)
        {
            continue;
        }
#endif
        if (INS_IsDirectControlFlow(tail))
        {
            //check if direct or indirect call and take the target accordingly
            ADDRINT target = INS_DirectControlFlowTargetAddress(tail);
            INS_InsertCall(tail, IPOINT_BEFORE, (AFUNPTR)a_process_call, IARG_ADDRINT, target, IARG_REG_VALUE, REG_STACK_PTR,
                           IARG_REG_VALUE, vreg, IARG_END);
            if (mngr->TargetInteresting(target))
            {
                INS_InsertCall(tail, IPOINT_TAKEN_BRANCH, (AFUNPTR)a_on_call, IARG_ADDRINT, target, IARG_PTR, mngr,
                               IARG_THREAD_ID, IARG_IARGLIST, args, //context or null
                               IARG_END);
            }
        }
        if (INS_IsIndirectControlFlow(tail) && !INS_IsRet(tail))
        {
            INS_InsertCall(tail, IPOINT_TAKEN_BRANCH, (AFUNPTR)a_process_call, IARG_BRANCH_TARGET_ADDR, IARG_REG_VALUE,
                           REG_STACK_PTR, IARG_REG_VALUE, vreg, IARG_END);

            INS_InsertIfCall(tail, IPOINT_TAKEN_BRANCH, (AFUNPTR)a_target_interesting, IARG_BRANCH_TARGET_ADDR, IARG_PTR, mngr,
                             IARG_END);
            INS_InsertThenCall(tail, IPOINT_TAKEN_BRANCH, (AFUNPTR)a_on_call, IARG_BRANCH_TARGET_ADDR, IARG_PTR, mngr,
                               IARG_THREAD_ID, IARG_IARGLIST, args, //context or null
                               IARG_END);
        }

        if (INS_IsRet(tail))
        {
            INS_InsertCall(tail, IPOINT_BEFORE, (AFUNPTR)a_process_return, IARG_REG_VALUE, REG_STACK_PTR, IARG_INST_PTR,
                           IARG_REG_VALUE, vreg, IARG_END);

            INS_InsertIfCall(tail, IPOINT_TAKEN_BRANCH, (AFUNPTR)a_on_ret_should_fire, IARG_THREAD_ID, IARG_PTR, mngr, IARG_END);
            INS_InsertThenCall(tail, IPOINT_TAKEN_BRANCH, (AFUNPTR)a_on_ret_fire, IARG_THREAD_ID, IARG_IARGLIST,
                               args, //context or null
                               IARG_INST_PTR, IARG_PTR, mngr, IARG_END);
        }
    }
    IARGLIST_Free(args);
}

static string RemoveNamespace(const string& name)
{
    size_t pos = name.rfind(':');
    if (pos == string::npos)
    {
        return name;
    }
    string s = name.substr(pos + 1, string::npos);
    return s;
}

///////////////////////////////////////////////////////////////////////////////

void CallStack::create_entry(ADDRINT current_sp, ADDRINT target)
{
    // push entry -- note this is sp at the callsite
    CallEntry entry(current_sp, target);
    _call_vec.push_back(entry);
}

// roll back stack if we got here from a longjmp
// Note stack grows down and register stack grows up.
void CallStack::adjust_stack(ADDRINT current_sp)
{
    if (_call_vec.size() == 0) return;

    //original comment:
    //TIPP: I changed this from > to >= ...not sure it's right, but works better
    while (current_sp >= _call_vec.back().sp())
    {
        _call_vec.pop_back();
        if (_call_vec.size() == 0)
        {
            break;
        }
    }
}

// standard call
void CallStack::process_call(ADDRINT current_sp, ADDRINT target)
{
    // check if we got here from a longjmp.
    adjust_stack(current_sp);
    create_entry(current_sp, target);
}

// standard return
void CallStack::process_return(ADDRINT current_sp, ADDRINT ip)
{
    // original implementation had this:
    // on ia-32 windows to identify
    // push
    // ret
    // and ignore it, in order to process callstack correctly
    // add this back if needed

    // check if we got here from a longjmp.
    adjust_stack(current_sp);

    if (_call_vec.size() > 0)
    {
        //on windows we do not start the instrumentation at the beginning code.
        //this my lead to this scenario:
        //  call ...
        //  ret ...
        //  ret ...
        //so if the stack size is 0 we are ignoring this.

        _call_vec.pop_back();
    }
}

void CallStack::push_head(ADDRINT current_sp, ADDRINT target) { create_entry(current_sp, target); }

void CallStack::save_all_ips_info()
{
    CallStackInfo info;
    CallStackManager* mngr = CallStackManager::get_instance();
    ASSERTX(mngr);
    for (UINT32 i = 0; i < depth(); i++)
    {
        ADDRINT ip = _call_vec[i].target();
        mngr->get_ip_info(ip, info);
    }
}

ADDRINT CallStack::top_target() { return _call_vec.back().target(); }

// Get target of specific call stack depth
ADDRINT CallStack::depth_target(UINT32 depth)
{
    ASSERTX(_call_vec.size() > depth);
    return _call_vec[depth].target();
}

UINT32 CallStack::depth() { return _call_vec.size(); }

void CallStack::emit_stack(UINT32 depth, vector< string >& out)
{
    CallVec::reverse_iterator iter;
    string last;
    string pc;
    INT32 level;
    INT32 id;
    ostringstream o;
    CallStackInfo info;

    UINT32 width          = sizeof(ADDRINT) * 2; //bytes => nibbles
    BOOL _source_location = _knob_source_location;

    //emit the header
    o << "#  " << setw(width) << "IP";
    o << setw(20) << right << " FUNCTION ";
    o << setw(50) << " IMAGE NAME ";

    if (_source_location)
    {
        o << setw(120) << " FILE NAME:LINE:COLUMN";
    }
    o << "\n";
    out.push_back(o.str());

    //number of entires to print
    level = (depth > _call_vec.size()) ? _call_vec.size() - 1 : depth - 1;
    id    = 0;
    o.str("");
    //emit the call stack
    for (iter = _call_vec.rbegin(); iter != _call_vec.rend(); iter++)
    {
        CallStackManager* mngr = CallStackManager::get_instance();
        mngr->get_ip_info(iter->target(), info);

        o << right << dec << setw(2) << id << "# ";
        o << "0x" << hex << setw(width) << setfill('0') << iter->target() << "  ";
        o << setw(20) << setfill(' ') << left << info.func_name;
        o << setw(20) << info.image_name;
        if (_source_location && info.file_name)
        {
            o << " at " << info.file_name << ":" << dec << info.line;
            if (info.column)
            {
                o << ":" << info.column;
            }
        }

        o << endl;
        out.push_back(o.str());
        o.str("");

        level--;
        id++;
        if (level < 0)
        {
            break;
        }
    }
    out.push_back("\n");
}

void CallStack::get_targets(list< ADDRINT >& out)
{
    CallVec::reverse_iterator iter;
    for (iter = _call_vec.rbegin(); iter != _call_vec.rend(); iter++)
    {
        out.push_back(iter->target());
    }
}

///////////////////////////////////////////////////////////////////////////////

CallStackManager* CallStackManager::_instance = 0;

// Handle new thread
void CallStackManager::thread_begin(THREADID tid, CONTEXT* ctxt, INT32 flags, void* v)
{
    CallStack* call_stack = new CallStack();
    ASSERTX(call_stack);
    ASSERTX(v);
    ASSERTX(ctxt);
    CallStackManager* call_stack_manager = reinterpret_cast< CallStackManager* >(v);
    ASSERTX(call_stack_manager);
    call_stack_manager->add_stack(tid, call_stack);

    PIN_SetContextReg(ctxt, vreg, (ADDRINT)call_stack);
}

// Add the call stack of a new thread
void CallStackManager::add_stack(THREADID tid, CallStack* call_stack)
{
    ASSERTX(call_stack);
    PIN_GetLock(&_map_lock, tid + 1);
    _call_stack_map[tid] = call_stack;
    PIN_ReleaseLock(&_map_lock);
}

// Activate call stack manager if needed
void CallStackManager::activate()
{
    if (_activated)
    {
        return;
    }
    _activated = true;

    // Get virtual register and insturmentation routines
    vreg = PIN_ClaimToolRegister();
    PIN_AddThreadStartFunction(thread_begin, this);
    TRACE_AddInstrumentFunction(i_trace, this);
    IMG_AddInstrumentFunction(Img, this);
}

// Get call stack manager instance and create it if needed
CallStackManager* CallStackManager::get_instance()
{
    if (_instance != 0)
    {
        return _instance;
    }

    // Create new instance
    _instance = new CallStackManager();
    ASSERTX(_instance);
    return _instance;
}

// Get call stack of a specific IP
CallStack CallStackManager::get_stack(THREADID tid)
{
    PIN_GetLock(&_map_lock, tid + 1);
    CallStack* call_stack = _call_stack_map[tid];
    PIN_ReleaseLock(&_map_lock);
    ASSERTX(call_stack);
    return *call_stack; //copy const.
}

// Get call stack and source information for a specific IP
void CallStackManager::get_ip_info(ADDRINT ip, CallStackInfo& info)
{
    PIN_GetLock(&_lock, 0);

    // If we already have information for this IP then just return it
    CallStackInfoMap::iterator it = _call_stack_info.find(ip);

    if (it != _call_stack_info.end())
    {
        PIN_ReleaseLock(&_lock);
        info = it->second;
        return;
    }

    // We got here for new IP
    CallStackInfo curr_info;
    string curr_file_name;

    // Get routine and image information
    PIN_LockClient();
    curr_info.rtn_id    = RTN_Id(RTN_FindByAddress(ip));
    curr_info.func_name = strdup(RTN_FindNameByAddress(ip).c_str());
    IMG img             = IMG_FindByAddress(ip);

    // Get source location if neeed
    if (_knob_source_location)
    {
        PIN_GetSourceLocation(ip, &curr_info.column, &curr_info.line, &curr_file_name);
        if (curr_file_name.length() > 0) curr_info.file_name = strdup(curr_file_name.c_str());
    }

    PIN_UnlockClient();

    // Analyze image information
    string curr_image_name;
    if (IMG_Valid(img))
    {
        curr_image_name  = IMG_Name(img);
        ADDRINT img_addr = IMG_LowAddress(img);
        // The string contains image name and the offset of
        // the instruction
        curr_image_name += ":" + hexstr(ip - img_addr);
        curr_info.image_name = strdup(curr_image_name.c_str());
    }
    else
    {
        curr_info.image_name = (char*)("UNKNOWN IMAGE");
    }

    // Add new information to our database
    info                 = curr_info;
    _call_stack_info[ip] = curr_info;

    PIN_ReleaseLock(&_lock);
}

BOOL CallStackManager::NeedContext() { return _use_ctxt; }

BOOL CallStackManager::TargetInteresting(ADDRINT ip)
{
    if (_enter_func_handlers_map.find(ip) != _enter_func_handlers_map.end())
    {
        return TRUE;
    }
    if (_exit_func_handlers_map.find(ip) != _exit_func_handlers_map.end())
    {
        return TRUE;
    }
    return FALSE;
}

void CallStackManager::on_function_enter(CALL_STACK_HANDLER handler, const string& func_name, void* v, BOOL use_ctxt)
{
    CallStackHandlerParams params(handler, func_name, v);
    _enter_func_handlers.push_back(params);
    if (use_ctxt) _use_ctxt = true;
}

void CallStackManager::on_function_exit(CALL_STACK_HANDLER handler, const string& func_name, void* v, BOOL use_ctxt)
{
    CallStackHandlerParams params(handler, func_name, v);
    _exit_func_handlers.push_back(params);
    if (use_ctxt) _use_ctxt = true;
}

void CallStackManager::on_function_ip_enter(CALL_STACK_HANDLER handler, ADDRINT func_ip, void* v, BOOL use_ctxt)
{
    CallStackHandlerParams* params = new CallStackHandlerParams(handler, "", v, func_ip, FALSE);
    _enter_func_handlers_map[func_ip].push_back(params);
    if (use_ctxt) _use_ctxt = true;
}

void CallStackManager::on_function_ip_exit(CALL_STACK_HANDLER handler, ADDRINT func_ip, void* v, BOOL use_ctxt)
{
    CallStackHandlerParams* params = new CallStackHandlerParams(handler, "", v, func_ip, FALSE);
    _exit_func_handlers_map[func_ip].push_back(params);
    if (use_ctxt) _use_ctxt = true;
}

// iterate each function in the loaded image and check the following:
// 1. whether the function was registered in on_function_enter.
//    if so store the ip of the interesting function with the relevant
//    vector of handlers.
// 2. whether the function was registered in on_function_exit.
//    if so store the ip of the interesting function with the relevant
//    vector of handlers.
void CallStackManager::Img(IMG img, void* v)
{
    CallStackManager* mngr = static_cast< CallStackManager* >(v);

    for (SEC sec = IMG_SecHead(img); SEC_Valid(sec); sec = SEC_Next(sec))
    {
        for (RTN rtn = SEC_RtnHead(sec); RTN_Valid(rtn); rtn = RTN_Next(rtn))
        {
            string name = RTN_Name(rtn);
            //get undecorated function name(with namespace)
            name       = PIN_UndecorateSymbolName(name, UNDECORATION_NAME_ONLY);
            name       = RemoveNamespace(name);
            ADDRINT ip = RTN_Address(rtn);
            //check if the function has a callback registered for enter_function
            for (UINT32 i = 0; i < mngr->_enter_func_handlers.size(); i++)
            {
                if (mngr->_enter_func_handlers[i]._function_name == name && mngr->_enter_func_handlers[i]._name_handler)
                {
                    mngr->_enter_func_handlers[i]._first_ip = ip;
                    mngr->_enter_func_handlers_map[ip].push_back(&mngr->_enter_func_handlers[i]);
                }
            }

            //check if the function has a callback registered for exit_function
            for (UINT32 i = 0; i < mngr->_exit_func_handlers.size(); i++)
            {
                if (mngr->_exit_func_handlers[i]._function_name == name && mngr->_exit_func_handlers[i]._name_handler)
                {
                    mngr->_exit_func_handlers[i]._first_ip = ip;
                    mngr->_exit_func_handlers_map[ip].push_back(&mngr->_exit_func_handlers[i]);
                }
            }
        }
    }
}

// called after the execution of call/direct/indirect jump
//
// 1. check whether the target ip is present in the map of ip->handlers for enter to fuction
//    if so - call all the handlers.
// 2. check whether the target ip is present in the map of ip->handlers for exit to fuction
//    if so - record the depth of the stack at the entry point to the function
//    so later, when we roll back the call-stack beyond the recorded depth we will call the
//    registered handlers for on_function_exit
void CallStackManager::on_call(THREADID tid, CONTEXT* ctxt, ADDRINT ip)
{
    //call all enter_function handlers
    //iter holds a tuple of (ip, list of handlers to be called)
    IpFuncHnadlersMap::iterator iter;
    iter = _enter_func_handlers_map.find(ip);
    if (iter != _enter_func_handlers_map.end())
    {
        for (UINT32 i = 0; i < iter->second.size(); i++)
        {
            CallStackHandlerParams* params = iter->second[i];
            ASSERTX(params);
            params->_handler(ctxt, ip, tid, params->_args);
        }
    }

    // if we already seen this function down the stack so we we exit only on the
    // top most caller
    // e.g. A*->A->A
    // will stop on the first A
    if (_marked_ip_for_exit.find(ip) != _marked_ip_for_exit.end())
    {
        return;
    }

    //recored the stack depth if this is a requested exit functioniter = _exit_func_handlers_map.find(ip);
    iter = _exit_func_handlers_map.find(ip);
    if (iter != _exit_func_handlers_map.end())
    {
        UINT32 depth            = get_stack(tid).depth();
        DepthFuncHandlersMap& m = _depth_func_handlers_tid_vec[tid];
        m[depth]                = iter->second; //a vector of handlers
        _marked_ip_for_exit.insert(ip);
    }
}

// If instrumentation called after the execution of ret instruction,
// after the call-stack was roll back.
// iterate over all the recorded call-stack depth to check if we have rolled back
// the call-stack beyond the recorded depth.
// if so,  we return 1 so the  Then instrumentation will be called
BOOL CallStackManager::on_ret_should_fire(THREADID tid)
{
    BOOL was_found = FALSE;
    UINT32 depth   = get_stack(tid).depth();
    DepthFuncHandlersMap::iterator iter;
    DepthFuncHandlersMap& m = _depth_func_handlers_tid_vec[tid];

    iter = m.begin();
    //find all handlers that should be called based on the stack depth
    for (; iter != m.end(); iter++)
    {
        if (iter->first > depth)
        {
            was_found = TRUE;
            break;
        }
    }

    return was_found;
}

// Then analysis
// iterate over all the recorded call-stack depth to check if we have rolled back
// the call-stack beyond the recorded depth.
// if so:
//    1. we call all the registered handlers
//    2. remove the 'depth' entry so it will not be call again later.
void CallStackManager::on_ret_fire(THREADID tid, CONTEXT* ctxt, ADDRINT ip)
{
    UINT32 depth = get_stack(tid).depth();
    DepthFuncHandlersMap::iterator iter;
    DepthFuncHandlersMap::iterator earase_iter;
    DepthFuncHandlersMap& m = _depth_func_handlers_tid_vec[tid];

    iter = m.begin();
    //find all handlers that should be called based on the stack depth
    while (iter != m.end())
    {
        //we have rolled back the call-stack beyond the recorded depth
        if (iter->first > depth)
        {
            for (UINT32 i = 0; i < iter->second.size(); i++)
            {
                CallStackHandlerParams* params = iter->second[i];
                params->_handler(ctxt, ip, tid, params->_args);
                _marked_ip_for_exit.erase(params->_first_ip);
            }
            earase_iter = iter;
            iter++;
            m.erase(earase_iter);
        }
        else
        {
            iter++;
        }
    }
}
