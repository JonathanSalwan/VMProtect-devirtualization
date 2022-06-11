/*
 * Copyright (C) 2008-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/* footprint.H
   
Measures the number of references to unique (16B default) chunks of
memory. The references can be code, data loads or data stores.

The output includes such things as a count of unique chunks that were just
loaded, just stored, just code fetches, both loaded and stored to, both
loaded-from and code-fetched from, etc. 7 valid combinations of 3 bits.

Whenever a reference to a chunk occurs, I OR on a bit indicating load,
store or code fetch.

With a small tweak, I can count references to the chunks if one wants to
know where all the action is, from a chunk referencing perspective.

optimization opportunity: do all the code fetches for a basic block at one time.
 */
#include "pin.H"
#include <map>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <fstream>
using std::cerr;
using std::cout;
using std::endl;
using std::map;
using std::string;

const unsigned int FOOTPRINT_LOAD  = 1;
const unsigned int FOOTPRINT_STORE = 2;
const unsigned int FOOTPRINT_CODE  = 4;

class footprint_thread_data_t
{
    map< ADDRINT, unsigned int > mem;
    UINT64 block_total[8]; // 8 combinations of load, store, code
  public:
    footprint_thread_data_t() {}

    void load(ADDRINT ea)
    {
        map< ADDRINT, unsigned int >::iterator it = mem.find(ea);
        if (it == mem.end())
        {
            mem[ea] = FOOTPRINT_LOAD;
        }
        else
        {
            mem[ea] = it->second | FOOTPRINT_LOAD;
        }
    }
    void store(ADDRINT ea)
    {
        map< ADDRINT, unsigned int >::iterator it = mem.find(ea);
        if (it == mem.end())
        {
            mem[ea] = FOOTPRINT_STORE;
        }
        else
        {
            mem[ea] = it->second | FOOTPRINT_STORE;
        }
    }
    void code(ADDRINT ea)
    {
        map< ADDRINT, unsigned int >::iterator it = mem.find(ea);
        if (it == mem.end())
        {
            mem[ea] = FOOTPRINT_CODE;
        }
        else
        {
            mem[ea] = it->second | FOOTPRINT_CODE;
        }
    }
    void summary(std::ofstream* out)
    {
        /*
          1 = load
          2 = store
          4 = code
          3 = load+store
          5 = load+code
          6 = store+code
          7 = load+store+code
          0 = nothing - error
         */
        const char* header[] = {/*0*/ "error",
                                /*1*/ "load",
                                /*2*/ "store",
                                /*3*/ "load+store",
                                /*4*/ "code",
                                /*5*/ "load+code",
                                /*6*/ "store+code",
                                /*7*/ "load+store+code"};

        for (unsigned int i = 0; i < 8; i++)
            block_total[i] = 0;

        map< ADDRINT, unsigned int >::iterator it = mem.begin();
        for (; it != mem.end(); it++)
        {
            block_total[it->second]++;
        }

        for (unsigned int i = 0; i < 8; i++)
        {
            *out << std::setw(30) << header[i] << "  " << std::setw(12) << block_total[i] << endl;
        }
    }

    void update_totals(UINT64* out_total)
    {
        for (unsigned int i = 0; i < 8; i++)
            out_total[i] += block_total[i];
    }
};

class footprint_t
{
    KNOB< string > knob_output_file;
    std::ofstream* out;
    TLS_KEY tls_key;
    unsigned int num_threads;
    static const unsigned int chunk_size = 16;
    footprint_thread_data_t* get_tls(THREADID tid)
    {
        footprint_thread_data_t* tdata = static_cast< footprint_thread_data_t* >(PIN_GetThreadData(tls_key, tid));
        return tdata;
    }

    void summary()
    {
        UINT64 block_total[8];
        for (unsigned int j = 0; j < 8; j++)
            block_total[j] = 0;
        for (unsigned int i = 0; i < num_threads; i++)
        {
            footprint_thread_data_t* tdata = get_tls(i);
            *out << "# FINI TID " << i << endl;
            tdata->summary(out);
            tdata->update_totals(block_total);
        }

        *out << "# FINI GLOBAL SUMMARY" << endl;
        const char* header[] = {/*0*/ "error",
                                /*1*/ "load",
                                /*2*/ "store",
                                /*3*/ "load+store",
                                /*4*/ "code",
                                /*5*/ "load+code",
                                /*6*/ "store+code",
                                /*7*/ "load+store+code"};

        for (unsigned int i = 0; i < 8; i++)
        {
            *out << std::setw(30) << header[i] << "  " << std::setw(12) << block_total[i] << endl;
        }
    }

  public:
    footprint_t() : knob_output_file(KNOB_MODE_WRITEONCE, "pintool", "o", "footprint.out", "specify output file name")
    {
        num_threads      = 0;
        string file_name = knob_output_file.Value();
        out              = new std::ofstream(file_name.c_str());
    }

    void activate()
    {
        tls_key = PIN_CreateThreadDataKey(0);
        TRACE_AddInstrumentFunction(reinterpret_cast< TRACE_INSTRUMENT_CALLBACK >(instrument_trace), this);
        PIN_AddThreadStartFunction(reinterpret_cast< THREAD_START_CALLBACK >(thread_start), this);
        PIN_AddFiniFunction(reinterpret_cast< FINI_CALLBACK >(fini), this);
    }

    static ADDRINT mask(ADDRINT ea)
    {
        const ADDRINT mask = ~static_cast< ADDRINT >(chunk_size - 1);
        return ea & mask;
    }

    static void load(footprint_t* xthis, THREADID tid, ADDRINT memea, UINT32 length)
    {
        ADDRINT start                  = mask(memea);
        ADDRINT end                    = mask(memea + length - 1);
        footprint_thread_data_t* tdata = xthis->get_tls(tid);
        for (ADDRINT addr = start; addr <= end; addr += chunk_size)
        {
            tdata->load(addr);
        }
    }
    static void store(footprint_t* xthis, THREADID tid, ADDRINT memea, UINT32 length)
    {
        ADDRINT start                  = mask(memea);
        ADDRINT end                    = mask(memea + length - 1);
        footprint_thread_data_t* tdata = xthis->get_tls(tid);
        for (ADDRINT addr = start; addr <= end; addr += chunk_size)
        {
            tdata->store(addr);
        }
    }
    static void code(footprint_t* xthis, THREADID tid, ADDRINT memea, UINT32 length)
    {
        ADDRINT start                  = mask(memea);
        ADDRINT end                    = mask(memea + length - 1);
        footprint_thread_data_t* tdata = xthis->get_tls(tid);
        for (ADDRINT addr = start; addr <= end; addr += chunk_size)
        {
            tdata->code(addr);
        }
    }

    static void thread_start(THREADID tid, CONTEXT* ctxt, INT32 flags, footprint_t* xthis)
    {
        footprint_thread_data_t* tdata = new footprint_thread_data_t;
        PIN_SetThreadData(xthis->tls_key, tdata, tid);
        xthis->num_threads++;
    }

    void instrument_instruction(INS ins, ADDRINT pc, unsigned int ins_bytes)
    {
        // instrument the code reference
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)code, IARG_PTR, this, IARG_THREAD_ID, IARG_INST_PTR, IARG_UINT32, ins_bytes,
                       IARG_END);

        // instrument the load(s)
        if (INS_IsMemoryRead(ins) && INS_IsStandardMemop(ins))
        {
            INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)load, IARG_PTR, this, IARG_THREAD_ID, IARG_MEMORYREAD_EA,
                           IARG_MEMORYREAD_SIZE, IARG_END);
        }
        if (INS_HasMemoryRead2(ins) && INS_IsStandardMemop(ins))
        {
            INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)load, IARG_PTR, this, IARG_THREAD_ID, IARG_MEMORYREAD2_EA,
                           IARG_MEMORYREAD_SIZE, IARG_END);
        }
        // instrument the store
        if (INS_IsMemoryWrite(ins) && INS_IsStandardMemop(ins))
        {
            INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)store, IARG_PTR, this, IARG_THREAD_ID, IARG_MEMORYWRITE_EA,
                           IARG_MEMORYWRITE_SIZE, IARG_END);
        }
    }

    static void instrument_trace(TRACE trace, footprint_t* xthis)
    {
        ADDRINT pc = TRACE_Address(trace);
        for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
        {
            const INS head = BBL_InsHead(bbl);
            if (!INS_Valid(head)) continue;
            for (INS ins = head; INS_Valid(ins); ins = INS_Next(ins))
            {
                if (!INS_IsStandardMemop(ins)) continue;
                unsigned int instruction_size = INS_Size(ins);
                xthis->instrument_instruction(ins, pc, instruction_size);
                pc = pc + instruction_size;
            }
        }
    }

    static void fini(int, footprint_t* xthis)
    {
        *(xthis->out) << "# Chunk size " << xthis->chunk_size << " bytes " << endl;
        xthis->summary();
        *(xthis->out) << "# EOF" << endl;
        xthis->out->close();
    }
};
