/*
 * Copyright (C) 2021-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <list>
#include "pin.H"
using std::endl;
using std::hex;
using std::list;
using std::map;
using std::ofstream;
using std::ostream;
using std::string;
using std::stringstream;

class Logger
{
  public:
    BOOL m_isFile;
    BOOL m_init;
    ostream* m_stream;

    // indicated an output file value if KnobOutputFile != "", else stdout.
    KNOB< string > KnobOutputFile;

    void Init();

  public:
    /**
     * C'tor.
     */
    Logger();

    /**
     * Close and delete output stream.
     */
    VOID Close();

    /**
     * ostream getter.
     */
    ostream& operator<<(std::string);
};

static Logger out;

KNOB< BOOL > KnobToolProbeMode(KNOB_MODE_WRITEONCE, "pintool", "probe", "0", "invoke tool in probe mode");

STATIC VOID Fini(INT32 code, VOID* v) { out.Close(); }

STATIC VOID Image(IMG img, VOID* v)
{
    if (!IMG_IsMainExecutable(img))
    {
        out << IMG_Name(img) << std::endl;
    }
}

int main(int argc, char** argv)
{
    PIN_Init(argc, argv);
    PIN_InitSymbols();

    IMG_AddInstrumentFunction(Image, 0);
    PIN_AddFiniFunction(Fini, 0);

    // Never returns
    if (KnobToolProbeMode)
    {
        PIN_StartProgramProbed();
    }
    else
    {
        PIN_StartProgram();
    }

    return 0;
}

Logger::Logger()
    : m_isFile(FALSE), m_init(FALSE), m_stream(NULL),
      KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "rtld_dbg_iface_tool.out", "specify output file name")
{}

void Logger::Init()
{
    m_init            = TRUE;
    string outputFile = KnobOutputFile.Value();
    m_isFile          = !outputFile.empty();
    if (m_isFile)
    {
        m_stream = new std::ofstream(outputFile.c_str());
    }
    else
    {
        m_stream = &std::cout;
    }
}

VOID Logger::Close()
{
    if (m_isFile)
    {
        static_cast< ofstream* >(m_stream)->close();
        delete m_stream;
    }
}

std::ostream& Logger::operator<<(std::string str)
{
    if (!m_init) Init();
    *m_stream << str;
    return *m_stream;
}
