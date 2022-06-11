/*
 * Copyright (C) 2014-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#if !defined(_EMX_CALL_STACK_H_)
#define _EMX_CALL_STACK_H_

#include <string>
#include <vector>
#include <map>
#include <set>
#include <list>
#include "pin.H"
using std::list;
using std::map;
using std::set;
using std::string;
using std::vector;

extern "C"
{
#include "xed-interface.h"
}

namespace CALLSTACK
{
class CallEntry
{
  private:
    ADDRINT _current_sp;
    ADDRINT _target;

  public:
    CallEntry() : _current_sp(0), _target(0) {}
    CallEntry(ADDRINT current_sp, ADDRINT target) : _current_sp(current_sp), _target(target) {}

    bool operator==(const CallEntry& a) const { return (_current_sp == a._current_sp); }
    ADDRINT sp() const { return _current_sp; }
    ADDRINT target() const { return _target; }
};

class CallStack
{
  public:
    // print the call stack, emit only 'depth' entries
    void emit_stack(UINT32 depth, vector< string >& out);

    // return the depth of the call stack
    UINT32 depth();

    // add the current_sp and the target to the top of the call stack
    void push_head(ADDRINT current_sp, ADDRINT target);

    // return the ip target of the latest call
    ADDRINT top_target();

    // return the ip target of call per depth
    ADDRINT depth_target(UINT32 depth);

    // capture the info for each ip in the call stack
    // see CallStackInfo
    void save_all_ips_info();

    void process_call(ADDRINT current_sp, ADDRINT target);
    void process_return(ADDRINT current_sp, ADDRINT ip);

    // print the call stack, emit only 'depth' entries
    void get_targets(list< ADDRINT >& out);

  private:
    typedef std::vector< CallEntry > CallVec;
    CallVec _call_vec;

    void create_entry(ADDRINT current_sp, ADDRINT target);
    void adjust_stack(ADDRINT current_sp);
};

typedef void (*CALL_STACK_HANDLER)(CONTEXT* ctxt, ADDRINT ip, THREADID tid, VOID* v);
class CallStackHandlerParams
{
  public:
    CallStackHandlerParams(CALL_STACK_HANDLER h, const string& func_name, void* v, ADDRINT func_ip = 0, BOOL name_handler = TRUE)
    {
        _handler       = h;
        _function_name = func_name;
        _name_handler  = name_handler;
        _args          = v;
        _first_ip      = 0;
    }

    CALL_STACK_HANDLER _handler;
    string _function_name;
    string _function_ip;
    BOOL _name_handler;
    void* _args;
    ADDRINT _first_ip; //the first ip of the function, used for recursive function call
};

// this struct holds the informations need for emitting the call stack
// we hold a map of ip->CallStackInfo so we will no
// generate info for the same ip more than once
typedef struct CallStackInfoStruct
{
    char* func_name;
    char* image_name;
    char* file_name;
    UINT32 rtn_id;
    INT32 line;
    INT32 column;
    CallStackInfoStruct() : func_name(0), image_name(0), file_name(0), rtn_id(0), line(0), column(0) {}
} CallStackInfo;

// a singleton class
class CallStackManager
{
  public:
    // return a pointer to an instance of the class
    static CallStackManager* get_instance();

    // return a copied CallStack of thread tid
    CallStack get_stack(THREADID tid);

    // activate the CallStackManager
    void activate();

    // fill in info with the information about the ip, see CallStackInfo
    // if the the info does not exists we generated it first
    void get_ip_info(ADDRINT ip, CallStackInfo& info);

    //register a callback the will be called when entering to function: func_name
    void on_function_enter(CALL_STACK_HANDLER handler, const string& func_name, void* v, BOOL use_ctxt);

    //register a callback the will be called when returning from function: func_name
    void on_function_exit(CALL_STACK_HANDLER handler, const string& func_name, void* v, BOOL use_ctxt);

    // Register a callback that will be called when entering function: function_ip
    // The parameters are: function handler
    //                     function ip address
    //                     parameter to the function
    //                     flag if the function uses PIN context
    void on_function_ip_enter(CALL_STACK_HANDLER handler, ADDRINT func_ip, void* v, BOOL use_ctxt);

    // Register a callback that will be called when exiting function: function_ip
    // The parameters are: function handler
    //                     function ip address
    //                     parameter to the function
    //                     flag if the function uses PIN context
    void on_function_ip_exit(CALL_STACK_HANDLER handler, ADDRINT func_ip, void* v, BOOL use_ctxt);

    //// internal use ////
    void on_call(THREADID tid, CONTEXT* ctxt, ADDRINT ip);
    void on_ret_fire(THREADID tid, CONTEXT* ctxt, ADDRINT ip);
    BOOL on_ret_should_fire(THREADID tid);
    BOOL NeedContext();
    BOOL TargetInteresting(ADDRINT ip);

  private:
    CallStackManager() : _activated(false), _use_ctxt(false), _depth_func_handlers_tid_vec(PIN_MAX_THREADS)
    {
        PIN_InitLock(&_lock);
    }
    static void thread_begin(THREADID tid, CONTEXT* ctxt, INT32 flags, void* v);
    void add_stack(THREADID tid, CallStack* call_stack);
    static void Img(IMG img, void* v);

    static CallStackManager* _instance;
    bool _activated;
    //a map to threadid -> CallStack*
    typedef std::map< THREADID, CallStack* > CallStackMap;
    CallStackMap _call_stack_map;
    PIN_LOCK _map_lock;

    //map of ip to its info(file, func, line, ...)
    //used to prevent collecting info about the same ip multiple times
    typedef std::map< ADDRINT, CallStackInfo > CallStackInfoMap;
    CallStackInfoMap _call_stack_info;
    PIN_LOCK _lock;
    BOOL _use_ctxt;

    vector< CallStackHandlerParams > _enter_func_handlers;
    vector< CallStackHandlerParams > _exit_func_handlers;

    //map of ip to a vector of handlers
    typedef vector< CallStackHandlerParams* > CallStackHandlerVec;
    typedef map< ADDRINT, CallStackHandlerVec > IpFuncHnadlersMap;
    IpFuncHnadlersMap _enter_func_handlers_map;

    //map of ip to a vector of handlers
    IpFuncHnadlersMap _exit_func_handlers_map;

    //map of stack depth to a vector of handlers
    typedef std::map< UINT32, CallStackHandlerVec > DepthFuncHandlersMap;
    typedef std::vector< DepthFuncHandlersMap > DepthFuncHandlersTidVec;
    //a vector with entry per thread
    DepthFuncHandlersTidVec _depth_func_handlers_tid_vec;

    //holds the ips that we have marked for exit, needed for recursive calls
    set< ADDRINT > _marked_ip_for_exit;
};
} // namespace CALLSTACK
#endif
