/*
 * Copyright (C) 2014-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#ifndef _CONTROLLER_EVENTS_H_
#define _CONTROLLER_EVENTS_H_

#include <string>
#include <map>
#include "pin.H"

using namespace std;

namespace CONTROLLER
{
typedef enum
{
    EVENT_INVALID = 0,
    EVENT_PRECOND,
    EVENT_START,
    EVENT_STOP,
    EVENT_THREADID,
    EVENT_WARMUP_START,
    EVENT_WARMUP_STOP,
    EVENT_PROLOG_START,
    EVENT_PROLOG_STOP,
    EVENT_EPILOG_START,
    EVENT_EPILOG_STOP,
    EVENT_STATS_RESET,
    EVENT_STATS_EMIT,
    EVENT_STATS_EMIT_RESET,
    EVENT_USER_0,
    EVENT_USER_1,
    EVENT_USER_2,
    EVENT_USER_3,
    EVENT_USER_4,
    EVENT_USER_5,
    EVENT_USER_6,
    EVENT_USER_7,
    EVENT_USER_8,
    EVENT_USER_9
} EVENT_TYPE;

//the events manager can be created only by the control object
class CONTROLLER_EVENTS
{
  public:
    EVENT_TYPE AddEvent(const string& event_name);
    string IDToString(EVENT_TYPE ev);
    EVENT_TYPE EventStringToType(const string& event_name);

  private:
    friend class CONTROL_MANAGER;
    CONTROLLER_EVENTS();
    map< string, EVENT_TYPE > _events;
    static const EVENT_TYPE _max_user_ev = EVENT_USER_9;

}; //class
} // namespace CONTROLLER
#endif
