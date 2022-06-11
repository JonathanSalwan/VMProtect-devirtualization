/*
 * Copyright (C) 2004-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 *  This file contains a guard tool against inadvertent or adversary
 *  attempts to modify program text.
 *  Note that self-modifying code will not run under this tool, as its
 *  attempts to execute modified program text will be aborted.
 *
 *  The tool works by making a backup copy of all text pages belonging to
 *  the application (and shared libraries) before the first instruction of
 *  the application is executed. Before jitting of every trace, we check
 *  (compare against backup copy) that none of the instructions belonging
 *  to the trace have been tampered with. If a trace is used out of the
 *  code cache, no such checking is done - the assumption is that
 *  only original code needs to be checked.
 *
 *  In Fedora core2, the kernel creates some code that is not part of an
 *  image. To make this tool work, we would also have to find that code
 *
 */

#include <iostream>
#include <string>
#include <map>

#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

#include "pin.H"
using std::cerr;
using std::endl;
using std::flush;
using std::map;
using std::string;

/* ===================================================================== */
/* Commandline Switches */
/* ===================================================================== */

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 Usage()
{
    cerr << "\n"
            "This pin tool guards against runtime program text modifications\n"
            "such as buffer overflows.\n"
            "\n";

    cerr << KNOB_BASE::StringKnobSummary();
    cerr << endl;

    return 1;
}

/* ===================================================================== */

class SANDBOX
{
  private:
    // types
    typedef map< const char*, const char* > AddrMap;

    // constants
    static const ADDRINT Kilo     = 1024;
    static const ADDRINT Mega     = Kilo * Kilo;
    static const ADDRINT PageSize = 4 * Kilo;
    static const ADDRINT PageMask = PageSize - 1;

    // members
    AddrMap _pages;
    AddrMap _deferredRanges;

    // functions
    const char* Addr2Page(const char* addr)
    {
        return reinterpret_cast< const char* >(reinterpret_cast< ADDRINT >(addr) & ~PageMask);
    }
    ADDRINT Addr2Offset(const char* addr) { return reinterpret_cast< ADDRINT >(addr) & PageMask; }

    const char* AllocatePage(const char* page);
    VOID ProtectPage(const char* page);
    VOID RecordPage(const char* page);
    VOID RecordPageRange(const char* beginPage, const char* endPage);
    VOID RecordAddressRange(const char* beginAddr, const char* endAddr);

    // report error - print message and terminate the application
    VOID Error(string msg)
    {
        static string errorPrefix = "ERROR:\n";
        PrintMessage(errorPrefix + msg);
        PIN_ExitProcess(1);
    }

    // report warning - print message and continue running
    VOID Warning(string msg)
    {
        static string warningPrefix = "WARNING:\n";
        PrintMessage(warningPrefix + msg);
    }

    // print a message
    VOID PrintMessage(string msg);

  public:
    VOID RecordIns(INS ins);
    VOID CheckAddressRange(const char* beginAddr, const char* endAddr);
    VOID CheckAddressRangeDeferred(const char* beginAddr, const char* endAddr) { _deferredRanges[beginAddr] = endAddr; }
    VOID HandlePendingChecks(const char* beginAddr, const char* endAddr);
    VOID HandlePendingChecks();
};

static SANDBOX sandbox;

VOID SANDBOX::PrintMessage(string msg)
{
    if (msg.length() > 0 && msg[0] != '\n')
    {
        msg = "\n" + msg;
    }

    string::size_type pos = msg.length() > 1 ? msg.length() - 2 : 0;

    // prefix every line
    for (pos = msg.find_last_of('\n', pos); pos != string::npos;
         pos = (pos == 0 ? string::npos : msg.find_last_of('\n', pos - 1)))
    {
        if (pos == msg.length() - 1)
        {
            msg += "Fence: \n";
        }
        else
        {
            msg.insert(pos + 1, "Fence: ");
        }
    }

    cerr << msg << endl << flush;
    fflush(NULL);
}

// allocate a new page
const char* SANDBOX::AllocatePage(const char* page)
{
    const char* pageFrameStart =
        reinterpret_cast< const char* >(mmap(0, PageSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0));

    if (pageFrameStart == MAP_FAILED)
    {
        Error("Can't get page frame for page " + hexstr(page) + "\n" + strerror(errno));
    }

    _pages[page] = pageFrameStart;

    return pageFrameStart;
}

// protect a page read-only
VOID SANDBOX::ProtectPage(const char* page)
{
    int result = mprotect(const_cast< char* >(page), PageSize, PROT_READ);

    if (result != 0)
    {
        Error("Can't read-protect page " + hexstr(page) + "\n" + strerror(errno));
    }
}

// record one page
VOID SANDBOX::RecordPage(const char* page)
{
    const char* pageFrameStart = _pages[page];

    if (pageFrameStart != NULL)
    {
        return; // already recorded
    }

    pageFrameStart = AllocatePage(page);
    memcpy(const_cast< char* >(pageFrameStart), page, PageSize);
    ProtectPage(pageFrameStart);
}

// record pages in given range
VOID SANDBOX::RecordPageRange(const char* beginPage, const char* endPage)
{
    for (const char* page = beginPage; page <= endPage; page += PageSize)
    {
        RecordPage(page);
    }
}

// record bytes in given address range
VOID SANDBOX::RecordAddressRange(const char* beginAddr, const char* endAddr)
{
    const char* beginPage = Addr2Page(beginAddr);
    const char* endPage   = Addr2Page(endAddr);

    RecordPageRange(beginPage, endPage);
}

// record the page(s) occupied by the instruction
VOID SANDBOX::RecordIns(INS ins)
{
    const ADDRINT beginAddr = INS_Address(ins);
    const ADDRINT endAddr   = beginAddr + INS_Size(ins) - 1;

    RecordAddressRange(reinterpret_cast< const char* >(beginAddr), reinterpret_cast< const char* >(endAddr));
}

/* ===================================================================== */
// check bytes in given address range
VOID SANDBOX::CheckAddressRange(const char* beginAddr, const char* endAddr)
{
    const char* beginPage = Addr2Page(beginAddr);
    const char* endPage   = Addr2Page(endAddr);

    for (const char* page = beginPage; page <= endPage; page += PageSize)
    {
        const char* pageEnd        = page + PageSize - 1;
        const char* beginCheckAddr = beginAddr > page ? beginAddr : page;
        const char* endCheckAddr   = endAddr < pageEnd ? endAddr : pageEnd;
        size_t size                = endCheckAddr - beginCheckAddr + 1;

        const char* pageFrameStart = _pages[page];
        if (pageFrameStart == NULL)
        {
            // note: this might also trigger if PIN has not found all the code
            // of a routine, so try to distinguish between the two:
            IMG img = IMG_FindByAddress(reinterpret_cast< ADDRINT >(beginAddr));
            if (IMG_Valid(img))
            {
                bool AddressInCodeSection = false;
                for (SEC sec = IMG_SecHead(img); SEC_Valid(sec); sec = SEC_Next(sec))
                {
                    ADDRINT secBegin = SEC_Address(sec);
                    ADDRINT secEnd   = secBegin + SEC_Size(sec);
                    if (reinterpret_cast< ADDRINT >(endCheckAddr) >= secBegin &&
                        reinterpret_cast< ADDRINT >(endCheckAddr) <= secEnd)
                    {
                        if (SEC_TYPE_EXEC == SEC_Type(sec))
                        {
                            // This code is part of a code section in a known image but Pin failed to parse it
                            // into a routine.
                            AddressInCodeSection = true;
                            break;
                        }
                    }
                }
                if (AddressInCodeSection)
                {
                    // The current block is part of a code section in a known image but Pin did not find a routine
                    // it belongs to in the image. We ignore the block on account that Pin simply failed to parse
                    // the image correctly.
                    Warning("Instruction address range " + hexstr(beginCheckAddr) + " - " + hexstr(endCheckAddr) +
                            " was not found\n"
                            "during initial code discovery.\n" +
                            "\n"
                            "However, it is part of a code section in a known image.\n"
                            "It's likely that Pin simply failed to parse the image correctly.\n");
                    continue;
                }
            }

            // The code block is either not part of an image or does not belong to a code section within an image.
            Warning("Instruction address range " + hexstr(beginCheckAddr) + " - " + hexstr(endCheckAddr) +
                    " was not found\n"
                    "during initial code discovery.\n" +
                    "\n"
                    "The application is trying to execute instructions which were not in\n"
                    "the original application.\n"
                    "This might be due to a buffer overflow.\n");
        }
        else
        {
            const char* pageFrameBeginCheckAddr = pageFrameStart + Addr2Offset(beginCheckAddr);

            if (memcmp(beginCheckAddr, pageFrameBeginCheckAddr, size) != 0)
            {
                Warning("Instruction address range " + hexstr(beginCheckAddr) + " - " + hexstr(endCheckAddr) +
                        " is corrupted.\n" +
                        "\n"
                        "The application code has been corrupted during execution.\n"
                        "This might be due to a buffer overflow.\n");
            }
        }
    }
}

VOID SANDBOX::HandlePendingChecks(const char* beginAddr, const char* endAddr)
{
    AddrMap::iterator it  = _deferredRanges.begin();
    AddrMap::iterator end = _deferredRanges.end();
    while (it != end)
    {
        AddrMap::iterator tempit = it;
        ++it;
        if (tempit->first >= beginAddr && tempit->second <= endAddr)
        {
            CheckAddressRange(tempit->first, tempit->second);
            _deferredRanges.erase(tempit);
        }
    }
}

VOID SANDBOX::HandlePendingChecks()
{
    // No need to erase iterators from the list here because this function is only called once, just before exit.
    AddrMap::const_iterator it  = _deferredRanges.begin();
    AddrMap::const_iterator end = _deferredRanges.end();
    for (; it != end; ++it)
    {
        CheckAddressRange(it->first, it->second);
    }
}

/* ===================================================================== */

VOID Trace(TRACE trace, VOID* v)
{
    const INS beginIns      = BBL_InsHead(TRACE_BblHead(trace));
    const INS endIns        = BBL_InsTail(TRACE_BblTail(trace));
    const ADDRINT beginAddr = INS_Address(beginIns);
    const ADDRINT endAddr   = INS_Address(endIns) + INS_Size(endIns) - 1;

    // Handle the case of ifuncs:
    // Some code of an image may be executed while loading the image in order to resolve the ifuncs.
    // This happens before Pin gives the image load callback, therefore the tool still considers this
    // code undiscovered.
    IMG img = IMG_FindByAddress(beginAddr);
    if (!IMG_Valid(img))
    {
        sandbox.CheckAddressRangeDeferred(reinterpret_cast< const char* >(beginAddr), reinterpret_cast< const char* >(endAddr));
    }
    else
    {
        sandbox.CheckAddressRange(reinterpret_cast< const char* >(beginAddr), reinterpret_cast< const char* >(endAddr));
    }
}

/* ===================================================================== */

VOID Image(IMG img, VOID* v)
{
    for (SEC sec = IMG_SecHead(img); SEC_Valid(sec); sec = SEC_Next(sec))
    {
        for (RTN rtn = SEC_RtnHead(sec); RTN_Valid(rtn); rtn = RTN_Next(rtn))
        {
            RTN_Open(rtn);

            for (INS ins = RTN_InsHead(rtn); INS_Valid(ins); ins = INS_Next(ins))
            {
                sandbox.RecordIns(ins);
            }

            RTN_Close(rtn);
        }
    }
    sandbox.HandlePendingChecks(reinterpret_cast< const char* >(IMG_LowAddress(img)),
                                reinterpret_cast< const char* >(IMG_HighAddress(img)));
}

VOID Fini(INT32 code, VOID* v) { sandbox.HandlePendingChecks(); }

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */

int main(int argc, CHAR* argv[])
{
    PIN_InitSymbols();
    if (PIN_Init(argc, argv))
    {
        return Usage();
    }

    TRACE_AddInstrumentFunction(Trace, 0);
    IMG_AddInstrumentFunction(Image, 0);
    PIN_AddFiniFunction(Fini, 0);

    PIN_StartProgram(); // Never returns

    return 0;
}
