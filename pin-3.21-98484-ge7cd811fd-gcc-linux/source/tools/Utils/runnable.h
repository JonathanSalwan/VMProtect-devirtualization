/*
 * Copyright (C) 2008-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 *  Runnable and function objects. 
 */
#ifndef RUNNABLE_H
#define RUNNABLE_H

#include <string>
#include <iostream>
#if defined(TARGET_LINUX) || defined(TARGET_BSD)
#include <stdlib.h> /* gcc4.3.x required */
#endif
using namespace std;

/*!
 * Abstract interface of a runnable object.
 */
class RUNNABLE_OBJ
{
  public:
    virtual void Run() = 0;
    virtual ~RUNNABLE_OBJ() {}
};

/*!
 * Abstract function object.
 */
class FUNC_OBJ : public RUNNABLE_OBJ
{
  public:
    // Execute the function.
    // @return  <this> object that contains result of the function invocation.
    virtual FUNC_OBJ& Execute() = 0;

    // Execute the function and return to the caller even if the function threw an
    // exception.
    // The function is NOT thread-safe.
    // @return  <this> object that contains result of the function invocation.
    virtual FUNC_OBJ& ExecuteSafe();

    // Implementation of the RUNNABLE_OBJ::Run() function.
    void Run() { Execute(); }

    // Return boolean status of the last Execute() invocation.
    // @return  TRUE - the function succeeded and returned an expected result
    //          FALSE - the function failed or returned an unexpected result
    virtual bool Status() const = 0;

    // Return human-readable string representation of the status of the last
    // Execute() invocation.
    virtual string ErrorMessage() const
    {
        if (Status())
        {
            return "Success";
        }
        else
        {
            return "Failure";
        }
    }

    // Check the status of the last Execute() invocation. Print error message and
    // exit abnormally if the function failed.
    void AssertStatus()
    {
        if (!Status())
        {
            cerr << Name() << ": " << ErrorMessage() << endl;
            exit(1);
        }
    }

    // Return name of the function.
    virtual string Name() const = 0;

    // Create a copy of this object.
    virtual FUNC_OBJ* Clone() const = 0;

    // Virtual destructor
    virtual ~FUNC_OBJ() {}

  protected:
    // Handle exception.
    // @param[in]  exceptIp    address of the instruction that caused the exception
    // @return  <this> object that contains result of the exception handling.
    virtual FUNC_OBJ& HandleException(void* exceptIp) { return *this; }
};

/*!
 * Class that represents a position-independent function.
 */
class PI_FUNC : public FUNC_OBJ
{
  public:
    // Copy the function body into specified buffer.
    // @return  <this> object that represents the function in the new location.
    virtual PI_FUNC& Copy(void* buffer) = 0;

    // Base address of the function's code range.
    virtual void* Start() const = 0;

    // Size of the function's code range.
    virtual size_t Size() const = 0;

    // Max. size of the function's code range
    static const size_t MAX_SIZE = 8192;
};

#endif //RUNNABLE_H

/* ===================================================================== */
/* eof */
/* ===================================================================== */
