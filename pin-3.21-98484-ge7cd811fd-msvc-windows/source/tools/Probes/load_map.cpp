/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
  The tool checks that page permissions in application process are not changed
  after Pin writes a probe
*/

/* ===================================================================== */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
#include <linux/limits.h>
#include "pin.H"
#include <list>
#include "tool_macros.h"
using std::cerr;
using std::cout;
using std::dec;
using std::endl;
using std::hex;
using std::list;
using std::string;

/* Memory range */
struct RANGE_ATTR
{
    RANGE_ATTR(ADDRINT start, ADDRINT end, string attr) : _start(start), _end(end), _attr(attr) {}
    ADDRINT _start;
    ADDRINT _end;
    string _attr;
};

/* A list of memory ranges */
class RANGES
{
  public:
    /* Get memory attributes under the specified address */
    BOOL GetAttributes(ADDRINT addr, string* attr);
    VOID AddRange(ADDRINT start, ADDRINT end, string attr);

  private:
    list< RANGE_ATTR > _ranges;
};

VOID RANGES::AddRange(ADDRINT start, ADDRINT end, string attr) { _ranges.push_back(RANGE_ATTR(start, end, attr)); }

/* Find range and get its attriutes */
BOOL RANGES::GetAttributes(ADDRINT addr, string* attrStr)
{
    list< RANGE_ATTR >::iterator it = _ranges.begin();
    for (; it != _ranges.end(); it++)
    {
        if ((it->_start <= addr) && (it->_end > addr))
        {
            *attrStr = it->_attr;
            return TRUE;
        }
    }
    return FALSE;
}

#if defined(TARGET_LINUX)
#define MAX_NUM_OF_RANGES_PER_FILENAME 10

/* Read /proc/self/maps and fill fileMap with ranges */
VOID FillFileMap(const char* name, RANGES* fileMap)
{
    FILE* fp = fopen("/proc/self/maps", "r");
    char buff[1024];
    char attributes[MAX_NUM_OF_RANGES_PER_FILENAME][10];
    unsigned long mapl[MAX_NUM_OF_RANGES_PER_FILENAME], maph[MAX_NUM_OF_RANGES_PER_FILENAME];
    int nRange = 0;
    while (fgets(buff, 1024, fp) != NULL)
    {
        if (strstr(buff, name) != 0)
        {
            if (sscanf(buff, "%lx-%lx %s", &mapl[nRange], &maph[nRange], attributes[nRange]) != 3) continue;
            nRange++;
        }
    }
    fclose(fp);
    for (int i = 0; i < nRange; i++)
    {
        fileMap->AddRange(mapl[i], maph[i], attributes[i]);
    }
}

#elif defined(TARGET_MAC)

#include <mach/mach_vm.h>
#include <mach/mach_init.h>
#include <mach/vm_prot.h>

VOID FillFileMap(const char* name, RANGES* fileMap)
{
    mach_vm_address_t address      = 0;
    mach_vm_address_t last_address = 0;
    mach_vm_size_t size            = 0;
    while (1)
    {
        kern_return_t ret = KERN_SUCCESS;
        natural_t depth   = 1;
        vm_region_submap_info_data_64_t info;
        mach_msg_type_number_t count = VM_REGION_SUBMAP_INFO_COUNT_64;
        ret = mach_vm_region_recurse(mach_task_self(), &address, &size, &depth, (vm_region_recurse_info_t)&info, &count);
        if (ret != KERN_SUCCESS || (ADDRINT)address < (ADDRINT)last_address)
        {
            break;
        }
        char buf[4];
        sprintf(buf, "%c%c%c", (info.protection & VM_PROT_READ) ? 'r' : '-', (info.protection & VM_PROT_WRITE) ? 'w' : '-',
                (info.protection & VM_PROT_EXECUTE) ? 'x' : '-');
        fileMap->AddRange((unsigned long)address, (unsigned long)(address + size), buf);
        last_address = address;
        address += size;
    }
}

#endif

VOID ToolDoNothing() { cout << "Tool replacement - nothing to do" << endl; }

VOID ToolOne(size_t nBytes) { cout << "Tool replacement - print 1" << endl; }

BOOL PutProbeAndCheckAttributes(IMG img, const char* rtnName, AFUNPTR rtnReplacement)
{
    string shortName      = IMG_Name(img);
    string::size_type pos = shortName.rfind('/');
    if (pos != string::npos) shortName = shortName.substr(pos + 1);

    RANGES fileMapBeforeProbe;
    FillFileMap(shortName.c_str(), &fileMapBeforeProbe);

    RTN rtn = RTN_FindByName(img, rtnName);
    if (RTN_Valid(rtn) && RTN_IsSafeForProbedReplacement(rtn))
    {
        cout << "Looking at file " << shortName << endl;

        ADDRINT addr = RTN_Address(rtn);
        string origAttr;
        BOOL res = fileMapBeforeProbe.GetAttributes(addr, &origAttr);
        if (!res)
        {
            cerr << "Failed to read original page attributes from /proc/self/maps" << endl;
            cerr << "The bug is in the test" << endl;
            exit(-1);
        }

        RTN_ReplaceProbed(rtn, rtnReplacement);

        RANGES fileMapAfterProbe;
        FillFileMap(shortName.c_str(), &fileMapAfterProbe);
        string newAttr = "-cant-read-maps-file-";
        res            = fileMapAfterProbe.GetAttributes(addr, &newAttr);
        if (!res)
        {
            cerr << "Failed to read new page attributes from /proc/self/maps" << endl;
            cerr << "The bug is in the test" << endl;
            exit(-1);
        }
        if (newAttr != origAttr)
        {
            cout << "Original map was changes around address " << hex << addr << endl;
            cout << "Org attributes: " << origAttr << " New attributes " << newAttr << endl;
            exit(-1);
        }
        else
        {
            cout << "Original map was preserved around address " << hex << addr << endl;
            cout << "Attributes: " << newAttr << endl;
        }
        return TRUE;
    }
    return FALSE;
}

VOID ImageLoad(IMG img, VOID* arg)
{
    UINT32* numOfInstrumentedRtnsPtr = (UINT32*)arg;
    if (PutProbeAndCheckAttributes(img, C_MANGLE("do_nothing"), (AFUNPTR)ToolDoNothing))
    {
        (*numOfInstrumentedRtnsPtr)++;
        cout << dec << *numOfInstrumentedRtnsPtr << " routines were instrumented" << endl;
    }

    if (PutProbeAndCheckAttributes(img, C_MANGLE("one"), (AFUNPTR)ToolOne))
    {
        (*numOfInstrumentedRtnsPtr)++;
        cout << dec << *numOfInstrumentedRtnsPtr << " routines were instrumented" << endl;
    }
}

int main(INT32 argc, CHAR** argv)
{
    PIN_Init(argc, argv);
    PIN_InitSymbols();

    UINT32 numOfInstrumentedRtns = 0;
    IMG_AddInstrumentFunction(ImageLoad, (VOID*)&numOfInstrumentedRtns);

    // Never returns
    PIN_StartProgramProbed();

    return 0;
}
