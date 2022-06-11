/*
 * Copyright (C) 2007-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

// sse-unaligned-class2.cpp

// This tool that provides a class that forces every SSE operation that
// reads or writes memory to work from an aligned buffer. There is one
// buffer for loads and one for stores, per thread.

// This tries to align more types of references than the
// sse-unaligned-class.cpp

// FIXME: 2007-05-09 This realigns ALL SSE operations that are not emulated
// already without checking for alignment.  I should make this check for
// misalignment.

#include <cassert>
#include <cstdio>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstring>
#include <unistd.h>

#if defined(__GNUC__) && !defined(_WIN32)
#include <cstdint>
#endif

#include "pin.H"
extern "C"
{
#include "xed-interface.h"
    using std::cerr;
    using std::endl;
    using std::string;
}

#if defined(_WIN32)
#define uint8_t unsigned __int8
#define uint16_t unsigned __int16
#define uint32_t unsigned __int32
#define uint64_t unsigned __int64
#define int8_t __int8
#define int16_t __int16
#define int32_t __int32
#define int64_t __int64
#define uint_t unsigned int
#else
typedef unsigned int uint_t;
#endif

#if defined(__GNUC__)
// NOTE: macOS* XCODE2.4.1 gcc and Cgywin gcc 3.4.x only allow for 16b
// alignment!
#define SSE_ALIGN __attribute__((aligned(16)))
#else
#define SSE_ALIGN __declspec(align(16))
#endif

//////////////////////////////////////////////////////////////////////////
typedef uint8_t sse_aligned_buffer_t[16];

class thread_data_t
{
  public:
    thread_data_t() {}
    sse_aligned_buffer_t SSE_ALIGN read;
    sse_aligned_buffer_t SSE_ALIGN write;
};

//////////////////////////////////////////////////////////////////////////

class sse_aligner_t
{
  public:
    sse_aligner_t()
        : knob_output_file(KNOB_MODE_WRITEONCE, "pintool", "o", "sse-unaligned.out", "specify profile file name"),
          knob_pid(KNOB_MODE_WRITEONCE, "pintool", "i", "0", "append pid to output"),
          knob_loads(KNOB_MODE_WRITEONCE, "pintool", "align-sse-loads", "1", "align unaligned SSE loads"),
          knob_stores(KNOB_MODE_WRITEONCE, "pintool", "align-sse-stores", "1", "align unaligned SSE stores"),
          knob_verbose(KNOB_MODE_WRITEONCE, "pintool", "align-sse-verbose", "0", "make the sse aligner verbose")
    {
        num_threads    = 1;
        active_threads = 1;
        out            = 0;
        //NOTE: knob processing must happen in the activate() function.
    }

    ~sse_aligner_t()
    {
        if (out) out->close();
    }

    std::ofstream* out;
    KNOB< string > knob_output_file;
    KNOB< BOOL > knob_pid;
    KNOB< BOOL > knob_loads;
    KNOB< BOOL > knob_stores;
    KNOB< BOOL > knob_verbose;

    // key for accessing TLS storage in the threads. initialized once in main()
    TLS_KEY tls_key;

    uint32_t num_threads;
    uint32_t active_threads;
    bool realign_stores;
    bool realign_loads;
    bool verbose;

    void activate()
    {
        //FIXME: only one aligner at a time -- not changing output file
        //name based on a static count of the number of sse_aligner_t
        //objects.

        if (knob_verbose)
        {
            string filename = knob_output_file.Value();
            if (knob_pid)
            {
                filename += "." + decstr(getpid());
            }
            out = new std::ofstream(filename.c_str());
        }
        realign_stores = (knob_stores == 1);
        realign_loads  = (knob_loads == 1);
        verbose        = (knob_verbose == 1);

        // obtain  a key for TLS storage
        tls_key              = PIN_CreateThreadDataKey(0);
        thread_data_t* tdata = new thread_data_t;
        // remember the first thread's data for later
        PIN_SetThreadData(tls_key, tdata, PIN_ThreadId());

        PIN_AddThreadStartFunction(reinterpret_cast< THREAD_START_CALLBACK >(thread_begin), this);
        PIN_AddThreadFiniFunction(reinterpret_cast< THREAD_FINI_CALLBACK >(thread_end), this);

        TRACE_AddInstrumentFunction(reinterpret_cast< TRACE_INSTRUMENT_CALLBACK >(instrument_trace), this);
        if (verbose) *out << "sse aligner activated" << endl;
    }

    thread_data_t* get_tls(THREADID tid)
    {
        thread_data_t* tdata = static_cast< thread_data_t* >(PIN_GetThreadData(tls_key, tid));
        return tdata;
    }

    static void thread_begin(THREADID tid, CONTEXT* ctxt, INT32 flags, sse_aligner_t* pthis)
    {
        if (pthis->verbose) *(pthis->out) << "thead begin " << static_cast< uint32_t >(tid) << endl;
        // This function is locked no need for a Pin Lock here
        pthis->num_threads++;
        pthis->active_threads++;

        thread_data_t* tdata = new thread_data_t;
        // remember my pointer for later
        PIN_SetThreadData(pthis->tls_key, tdata, tid);
    }

    static void thread_end(THREADID tid, const CONTEXT* ctxt, INT32 code, sse_aligner_t* pthis)
    {
        thread_data_t* tdata = pthis->get_tls(tid);
        delete tdata;

        // This function is locked no need for a Pin Lock here
        pthis->active_threads--;
    }

    static void rewrite_instruction(INS ins, bool is_read, sse_aligner_t* pthis)
    {
        //fprintf(stderr,"Rewriting %p\n",(void*)INS_Address(ins));

        // Avoid aligning trivially aligned stuff
        const xed_decoded_inst_t* xedd = INS_XedDec(ins);
        if (xed_decoded_inst_get_memory_operand_length(xedd, 0) > sizeof(sse_aligned_buffer_t))
        {
            return;
        }
        xed_reg_enum_t breg1 = xed_decoded_inst_get_base_reg(xedd, 0);
        xed_reg_enum_t ireg  = xed_decoded_inst_get_index_reg(xedd, 0);
        INT64 disp           = 0;
        if (xed_decoded_inst_get_memory_displacement_width(xedd, 0)) disp = xed_decoded_inst_get_memory_displacement(xedd, 0);
        if (breg1 == XED_REG_INVALID && ireg == XED_REG_INVALID)
        {
            // displacement only... check its alignment
            if ((disp & 0xF) == 0) return;
        }
        else if (breg1 == XED_REG_RIP)
        {
            // rip-relative -- check alignment
            ADDRINT ip = INS_Address(ins);
            ADDRINT ea = ip + disp;
            if ((ea & 0xF) == 0) return;
        }
        if (pthis->verbose)
            *(pthis->out) << "REWRITE " << string(is_read ? "LOAD :" : "STORE:") << std::setw(16) << std::hex << INS_Address(ins)
                          << std::dec << " " << INS_Disassemble(ins) << std::endl;

        if (is_read)
        {
            // Loads -- we change the load to use G0 as the base register and
            // then add a "before" function that sets G0 and copies the data to
            // an aligned bufffer.
            INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(copy_to_aligned_load_buffer_and_return_pointer), IARG_MEMORYREAD_EA,
                           IARG_MEMORYREAD_SIZE, IARG_INST_PTR, IARG_THREAD_ID, IARG_PTR, pthis, IARG_RETURN_REGS, REG_INST_G0,
                           IARG_END);
        }
        else
        {
            // Stores -- we change the store to use G0 as a base register and
            // then add a "before" function to set G0 and an "after" function
            // that copies the data from the aligned buffer to where it was
            // supposed to go.
            // Since we can't ask for the MEMORYWRITE_EA at IPOINT_AFTER, we save
            // that in REG_INST_G1 at IPOINT_BEFORE and then use it at IPOINT_AFTER.
            INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(return_pointer_to_aligned_store_buffer), IARG_MEMORYWRITE_EA,
                           IARG_INST_PTR, IARG_THREAD_ID, IARG_PTR, pthis, IARG_REG_REFERENCE, REG_INST_G1, IARG_RETURN_REGS,
                           REG_INST_G0, IARG_END);
            INS_InsertCall(ins, IPOINT_AFTER, AFUNPTR(copy_from_aligned_store_buffer), IARG_REG_VALUE, REG_INST_G1,
                           IARG_MEMORYWRITE_SIZE, IARG_INST_PTR, IARG_THREAD_ID, IARG_PTR, pthis, IARG_END);
        }

        // Rewrite the memory operand (we assume there's only one) to use the address in REG_INST_G0
        INS_RewriteMemoryOperand(ins, 0, REG_INST_G0);
    }

    // Presumption here that SSE ops do not have RMW semantics, or more than one memory operand.
    static bool check_for_sse_memop(INS ins, bool& is_read, sse_aligner_t* pthis)
    {
        // return true if the instruction is SSEx and reads/writes memory
        xed_extension_enum_t extension = static_cast< xed_extension_enum_t >(INS_Extension(ins));
        if (extension == XED_EXTENSION_SSE || extension == XED_EXTENSION_SSE2 || extension == XED_EXTENSION_SSE3 ||
            extension == XED_EXTENSION_SSSE3 || extension == XED_EXTENSION_SSE4)
        {
            if (pthis->realign_loads && INS_IsMemoryRead(ins))
            {
                is_read = true;
                return true;
            }
            if (pthis->realign_stores && INS_IsMemoryWrite(ins))
            {
                is_read = false;
                return true;
            }
        }
        return false;
    }

    static void instrument_trace(TRACE trace, sse_aligner_t* pthis)
    {
        //sse_aligner_t* pthis = static_cast<sse_aligner_t*>(v);
        bool is_read = false;
        for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
            for (INS ins = BBL_InsHead(bbl); INS_Valid(ins); ins = INS_Next(ins))
                if (check_for_sse_memop(ins, is_read, pthis)) rewrite_instruction(ins, is_read, pthis);
    }

    static ADDRINT copy_to_aligned_load_buffer_and_return_pointer(ADDRINT load_addr, ADDRINT byte_len, ADDRINT ip, THREADID tid,
                                                                  sse_aligner_t* pthis)
    {
        // return the address to use for the SSEx operation
        thread_data_t* tdata = pthis->get_tls(tid);
        ADDRINT copied       = PIN_SafeCopy(tdata->read, reinterpret_cast< uint8_t* >(load_addr), byte_len);
        if (copied != byte_len)
        {
            // The copy failed, this happens if the data is accessing an unmapped page
            // to cause a similar fault we access the faulting data here...
            (void)*(reinterpret_cast< uint8_t* >(load_addr) + copied);
        }
        return reinterpret_cast< ADDRINT >(tdata->read);
    }

    static ADDRINT return_pointer_to_aligned_store_buffer(ADDRINT store_addr, ADDRINT ip, THREADID tid, sse_aligner_t* pthis,
                                                          ADDRINT* saved_ea)
    {
        // return the address to use for the SSEx operation
        thread_data_t* tdata = pthis->get_tls(tid);
        // Save the effective address, we can't ask for it at IPOINT_AFTER.
        *saved_ea = store_addr;
        return reinterpret_cast< ADDRINT >(tdata->write);
    }

    static void copy_from_aligned_store_buffer(ADDRINT store_addr, ADDRINT byte_len, ADDRINT ip, THREADID tid,
                                               sse_aligner_t* pthis)
    {
        thread_data_t* tdata = pthis->get_tls(tid);
        ADDRINT copied       = PIN_SafeCopy(reinterpret_cast< uint8_t* >(store_addr), tdata->write, byte_len);

        if (copied != byte_len)
        {
            // The copy failed, this happens if the data is accessing an unmapped page.
            // To cause a similar fault we access the faulting data here...
            *(reinterpret_cast< uint8_t* >(store_addr) + copied) = tdata->write[copied];
        }
    }

}; // class

int usage()
{
    cerr << "Usage: ..." << endl;
    cerr << KNOB_BASE::StringKnobSummary() << endl;
    return 1;
}

int main(int argc, char* argv[])
{
    static sse_aligner_t aligner; // must be before usage...

    PIN_InitSymbols();
    if (PIN_Init(argc, argv)) return usage();

    aligner.activate();

    // Never returns
    PIN_StartProgram();

    return 0;
}
