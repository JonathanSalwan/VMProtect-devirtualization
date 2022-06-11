/*
 * Copyright (C) 2014-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#ifndef _INIT_ALARM_H_
#define _INIT_ALARM_H_

#include "pin.H"
#include <set>

namespace CONTROLLER
{
class CONTROL_MANAGER;

class INIT_ALARM
{
  public:
    INIT_ALARM() : _manager(NULL), _disabled(FALSE) {}
    VOID Activate(CONTROL_MANAGER* manager);
    VOID Disable() { _disabled = TRUE; }

  private:
    CONTROL_MANAGER* _manager;
    ADDRINT _start_thread[PIN_MAX_THREADS];
    std::set< ADDRINT > _thread_first_ip;
    volatile BOOL _disabled;

    static VOID OnTrace(TRACE trace, VOID* vthis);
    static VOID ThreadStart(THREADID tid, CONTEXT* ctxt, INT32 flags, VOID* v);
    static inline ADDRINT ShouldStart(INIT_ALARM* init_alarm, THREADID tid)
    {
        return init_alarm->_start_thread[tid] & !init_alarm->_disabled;
    }
    static VOID Start(CONTEXT* ctxt, ADDRINT ip, THREADID tid, VOID* vthis);
};

} // namespace CONTROLLER
#endif
