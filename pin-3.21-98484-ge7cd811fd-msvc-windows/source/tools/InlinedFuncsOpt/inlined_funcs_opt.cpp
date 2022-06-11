/*
 * Copyright (C) 2004-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <iostream>
#include <fstream>
#include "pin.H"
using std::ofstream;
ofstream OutFile;

extern "C" VOID IfProc1();
extern "C" VOID IfProc2();
extern "C" VOID IfProc3();
extern "C" VOID IfProc4();
extern "C" VOID IfProc5();
extern "C" VOID IfProc6();
extern "C" VOID IfProc7();
extern "C" BOOL IfProc8();
extern "C" BOOL IfProc9();
extern "C" BOOL IfProc10();
extern "C" BOOL IfProc11();
extern "C" BOOL IfProc12();
extern "C" BOOL IfProc13();
extern "C" BOOL IfProc14();
extern "C" BOOL IfProc15();
extern "C" BOOL IfProc16();
extern "C" BOOL IfProc17();
extern "C" BOOL IfProc18();
extern "C" BOOL IfProc19();
extern "C" BOOL IfProc20();
extern "C" BOOL IfProc21();
extern "C" BOOL IfProc22();
extern "C" BOOL IfProc23();
extern "C" BOOL IfProc24();
extern "C" BOOL IfProc25();
extern "C" BOOL IfProc26();
extern "C" BOOL IfProc27();
extern "C" BOOL IfProc28();
extern "C" BOOL IfProc29();
extern "C" BOOL IfProc30();
extern "C" BOOL IfProc31();
extern "C" BOOL IfProc32();
extern "C" BOOL IfProc33();
extern "C" BOOL IfProc34();
extern "C" BOOL IfProc35();
extern "C" BOOL IfProc36();
extern "C" BOOL IfProc37();
extern "C" BOOL IfProc38();
extern "C" BOOL IfProc39();
extern "C" BOOL IfProc40();
extern "C" unsigned int globVal[];
unsigned int globVal[2];

int numTimesThenProc1Called = 0;
void ThenProc1() { numTimesThenProc1Called++; }

int ifProc2Param = 0;

int numTimesThenProc2Called = 0;
void ThenProc2() { numTimesThenProc2Called++; }

int numTimesThenProc8Called = 0;
void ThenProc8() { numTimesThenProc8Called++; }

int numTimesThenProc6YesCalled = 0;
void ThenProc6Yes() { numTimesThenProc6YesCalled++; }
int numTimesThenProc6NoCalled = 0;
void ThenProc6No() { numTimesThenProc6NoCalled++; }

int numTimesThenProc7YesCalled = 0;
void ThenProc7Yes() { numTimesThenProc7YesCalled++; }
int numTimesThenProc7NoCalled = 0;
void ThenProc7No() { numTimesThenProc7NoCalled++; }

int numTimesThenProc10YesCalled = 0;
void ThenProc10Yes() { numTimesThenProc10YesCalled++; }
int numTimesThenProc10NoCalled = 0;
void ThenProc10No() { numTimesThenProc10NoCalled++; }

int numTimesThenProc11YesCalled = 0;
void ThenProc11Yes() { numTimesThenProc11YesCalled++; }
int numTimesThenProc11NoCalled = 0;
void ThenProc11No() { numTimesThenProc11NoCalled++; }

int numTimesThenProc12YesCalled = 0;
void ThenProc12Yes() { numTimesThenProc12YesCalled++; }
int numTimesThenProc12YesACalled = 0;
void ThenProc12YesA() { numTimesThenProc12YesACalled++; }
int numTimesThenProc12NoCalled = 0;
void ThenProc12No() { numTimesThenProc12NoCalled++; }

int numTimesThenProc13YesCalled = 0;
void ThenProc13Yes() { numTimesThenProc13YesCalled++; }
int numTimesThenProc13YesACalled = 0;
void ThenProc13YesA() { numTimesThenProc13YesACalled++; }
int numTimesThenProc13NoCalled = 0;
void ThenProc13No() { numTimesThenProc13NoCalled++; }

int numTimesThenProc14YesCalled = 0;
void ThenProc14Yes() { numTimesThenProc14YesCalled++; }

int numTimesThenProc14NoCalled = 0;
void ThenProc14No() { numTimesThenProc14NoCalled++; }

int numTimesThenProc15YesCalled = 0;
void ThenProc15Yes() { numTimesThenProc15YesCalled++; }
int numTimesThenProc15NoCalled = 0;
void ThenProc15No() { numTimesThenProc15NoCalled++; }
int numTimesThenProc16NoCalled = 0;
void ThenProc16No() { numTimesThenProc16NoCalled++; }

int numTimesThenProc16YesCalled = 0;
void ThenProc16Yes() { numTimesThenProc16YesCalled++; }
int numTimesThenProc17NoCalled = 0;
void ThenProc17No() { numTimesThenProc17NoCalled++; }
int numTimesThenProc17YesCalled = 0;
void ThenProc17Yes() { numTimesThenProc17YesCalled++; }

int numTimesThenProc18YesCalled = 0;
void ThenProc18Yes() { numTimesThenProc18YesCalled++; }
int numTimesThenProc18YesACalled = 0;
void ThenProc18YesA() { numTimesThenProc18YesACalled++; }
int numTimesThenProc18NoCalled = 0;
void ThenProc18No() { numTimesThenProc18NoCalled++; }

int numTimesThenProc19YesCalled = 0;
void ThenProc19Yes() { numTimesThenProc19YesCalled++; }
int numTimesThenProc19YesACalled = 0;
void ThenProc19YesA() { numTimesThenProc19YesACalled++; }
int numTimesThenProc19NoCalled = 0;
void ThenProc19No() { numTimesThenProc19NoCalled++; }

int numTimesThenProc20NoCalled = 0;
void ThenProc20No() { numTimesThenProc20NoCalled++; }

int numTimesThenProc20YesCalled = 0;
void ThenProc20Yes() { numTimesThenProc20YesCalled++; }
int numTimesThenProc21NoCalled = 0;
void ThenProc21No() { numTimesThenProc21NoCalled++; }
int numTimesThenProc21YesCalled = 0;
void ThenProc21Yes() { numTimesThenProc21YesCalled++; }

int numTimesThenProc22NoCalled = 0;
void ThenProc22No() { numTimesThenProc22NoCalled++; }
int numTimesThenProc22NoACalled = 0;
void ThenProc22NoA() { numTimesThenProc22NoACalled++; }

int numTimesThenProc22YesCalled = 0;
void ThenProc22Yes() { numTimesThenProc22YesCalled++; }
int numTimesThenProc23NoCalled = 0;
void ThenProc23No() { numTimesThenProc23NoCalled++; }
int numTimesThenProc23NoACalled = 0;
void ThenProc23NoA() { numTimesThenProc23NoACalled++; }
int numTimesThenProc23YesCalled = 0;
void ThenProc23Yes() { numTimesThenProc23YesCalled++; }

int numTimesThenProc24NoCalled = 0;
void ThenProc24No() { numTimesThenProc24NoCalled++; }
int numTimesThenProc24NoACalled = 0;
void ThenProc24NoA() { numTimesThenProc24NoACalled++; }

int numTimesThenProc24YesCalled = 0;
void ThenProc24Yes() { numTimesThenProc24YesCalled++; }
int numTimesThenProc25NoCalled = 0;
void ThenProc25No() { numTimesThenProc25NoCalled++; }
int numTimesThenProc25NoACalled = 0;
void ThenProc25NoA() { numTimesThenProc25NoACalled++; }
int numTimesThenProc25YesCalled = 0;
void ThenProc25Yes() { numTimesThenProc25YesCalled++; }

int numTimesThenProc26NoCalled = 0;
void ThenProc26No() { numTimesThenProc26NoCalled++; }

int numTimesThenProc26YesCalled = 0;
void ThenProc26Yes() { numTimesThenProc26YesCalled++; }
int numTimesThenProc27NoCalled = 0;
void ThenProc27No() { numTimesThenProc27NoCalled++; }
int numTimesThenProc27YesCalled = 0;
void ThenProc27Yes() { numTimesThenProc27YesCalled++; }

int numTimesThenProc28NoCalled = 0;
void ThenProc28No() { numTimesThenProc28NoCalled++; }

int numTimesThenProc28YesCalled = 0;
void ThenProc28Yes() { numTimesThenProc28YesCalled++; }
int numTimesThenProc29NoCalled = 0;
void ThenProc29No() { numTimesThenProc29NoCalled++; }
int numTimesThenProc29YesCalled = 0;
void ThenProc29Yes() { numTimesThenProc29YesCalled++; }

int numTimesThenProc30NoCalled = 0;
void ThenProc30No() { numTimesThenProc30NoCalled++; }

int numTimesThenProc30YesCalled = 0;
void ThenProc30Yes() { numTimesThenProc30YesCalled++; }
int numTimesThenProc31NoCalled = 0;
void ThenProc31No() { numTimesThenProc31NoCalled++; }
int numTimesThenProc31YesCalled = 0;
void ThenProc31Yes() { numTimesThenProc31YesCalled++; }

int numTimesThenProc32NoCalled = 0;
void ThenProc32No() { numTimesThenProc32NoCalled++; }
int numTimesThenProc32YesCalled = 0;
void ThenProc32Yes() { numTimesThenProc32YesCalled++; }
int numTimesThenProc33NoCalled = 0;
void ThenProc33No() { numTimesThenProc33NoCalled++; }
int numTimesThenProc33YesCalled = 0;
void ThenProc33Yes() { numTimesThenProc33YesCalled++; }

int numTimesThenProc34NoCalled = 0;
void ThenProc34No() { numTimesThenProc34NoCalled++; }
int numTimesThenProc34YesCalled = 0;
void ThenProc34Yes() { numTimesThenProc34YesCalled++; }
int numTimesThenProc35NoCalled = 0;
void ThenProc35No() { numTimesThenProc35NoCalled++; }
int numTimesThenProc35YesCalled = 0;
void ThenProc35Yes() { numTimesThenProc35YesCalled++; }

int numTimesThenProc36NoCalled = 0;
void ThenProc36No() { numTimesThenProc36NoCalled++; }
int numTimesThenProc36YesCalled = 0;
void ThenProc36Yes() { numTimesThenProc36YesCalled++; }
int numTimesThenProc37NoCalled = 0;
void ThenProc37No() { numTimesThenProc37NoCalled++; }
int numTimesThenProc37YesCalled = 0;
void ThenProc37Yes() { numTimesThenProc37YesCalled++; }

int numTimesThenProc38NoCalled = 0;
void ThenProc38No() { numTimesThenProc38NoCalled++; }
int numTimesThenProc38YesCalled = 0;
void ThenProc38Yes() { numTimesThenProc38YesCalled++; }
int numTimesThenProc39NoCalled = 0;
void ThenProc39No() { numTimesThenProc39NoCalled++; }
int numTimesThenProc39YesCalled = 0;
void ThenProc39Yes() { numTimesThenProc39YesCalled++; }
int numTimesThenProc40NoCalled = 0;
void ThenProc40No() { numTimesThenProc40NoCalled++; }

int instrumented = 0;
VOID Instruction(INS ins, VOID* v)
{
    if (instrumented < 1)
    {
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)IfProc1, IARG_END);
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)IfProc2, IARG_END);
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)IfProc3, IARG_END);
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)IfProc4, IARG_END);
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)IfProc5, IARG_END);
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)IfProc6, IARG_END);
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)IfProc7, IARG_END);

        INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)IfProc6, IARG_FAST_ANALYSIS_CALL, IARG_ADDRINT, (ADDRINT)1, IARG_ADDRINT,
                         (ADDRINT)1, IARG_END);
        INS_InsertThenCall(ins, IPOINT_BEFORE, AFUNPTR(ThenProc6Yes), IARG_END);
        INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)IfProc6, IARG_FAST_ANALYSIS_CALL, IARG_ADDRINT, (ADDRINT)2, IARG_ADDRINT,
                         (ADDRINT)1, IARG_END);
        INS_InsertThenCall(ins, IPOINT_BEFORE, AFUNPTR(ThenProc6No), IARG_END);
        INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)IfProc7, IARG_FAST_ANALYSIS_CALL, IARG_ADDRINT, (ADDRINT)1, IARG_ADDRINT,
                         (ADDRINT)1, IARG_END);
        INS_InsertThenCall(ins, IPOINT_BEFORE, AFUNPTR(ThenProc7Yes), IARG_END);
        INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)IfProc7, IARG_FAST_ANALYSIS_CALL, IARG_ADDRINT, (ADDRINT)2, IARG_ADDRINT,
                         (ADDRINT)1, IARG_END);
        INS_InsertThenCall(ins, IPOINT_BEFORE, AFUNPTR(ThenProc7No), IARG_END);
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)IfProc10, IARG_END);
        INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)IfProc10, IARG_FAST_ANALYSIS_CALL, IARG_ADDRINT, (ADDRINT)1, IARG_ADDRINT,
                         (ADDRINT)2, IARG_END);
        INS_InsertThenCall(ins, IPOINT_BEFORE, AFUNPTR(ThenProc10No), IARG_END);
        INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)IfProc10, IARG_FAST_ANALYSIS_CALL, IARG_ADDRINT, (ADDRINT)2, IARG_ADDRINT,
                         (ADDRINT)1, IARG_END);
        INS_InsertThenCall(ins, IPOINT_BEFORE, AFUNPTR(ThenProc10Yes), IARG_END);

        INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)IfProc11, IARG_FAST_ANALYSIS_CALL, IARG_ADDRINT, (ADDRINT)1, IARG_ADDRINT,
                         (ADDRINT)2, IARG_END);
        INS_InsertThenCall(ins, IPOINT_BEFORE, AFUNPTR(ThenProc11No), IARG_END);
        INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)IfProc11, IARG_FAST_ANALYSIS_CALL, IARG_ADDRINT, (ADDRINT)2, IARG_ADDRINT,
                         (ADDRINT)1, IARG_END);
        INS_InsertThenCall(ins, IPOINT_BEFORE, AFUNPTR(ThenProc11Yes), IARG_END);

        INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)IfProc12, IARG_FAST_ANALYSIS_CALL, IARG_ADDRINT, (ADDRINT)1, IARG_ADDRINT,
                         (ADDRINT)2, IARG_END);
        INS_InsertThenCall(ins, IPOINT_BEFORE, AFUNPTR(ThenProc12No), IARG_END);
        INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)IfProc12, IARG_FAST_ANALYSIS_CALL, IARG_ADDRINT, (ADDRINT)2, IARG_ADDRINT,
                         (ADDRINT)1, IARG_END);
        INS_InsertThenCall(ins, IPOINT_BEFORE, AFUNPTR(ThenProc12Yes), IARG_END);
        INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)IfProc12, IARG_FAST_ANALYSIS_CALL, IARG_ADDRINT, (ADDRINT)2, IARG_ADDRINT,
                         (ADDRINT)2, IARG_END);
        INS_InsertThenCall(ins, IPOINT_BEFORE, AFUNPTR(ThenProc12YesA), IARG_END);

        INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)IfProc13, IARG_FAST_ANALYSIS_CALL, IARG_ADDRINT, (ADDRINT)1, IARG_ADDRINT,
                         (ADDRINT)2, IARG_END);
        INS_InsertThenCall(ins, IPOINT_BEFORE, AFUNPTR(ThenProc13No), IARG_END);
        INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)IfProc13, IARG_FAST_ANALYSIS_CALL, IARG_ADDRINT, (ADDRINT)2, IARG_ADDRINT,
                         (ADDRINT)1, IARG_END);
        INS_InsertThenCall(ins, IPOINT_BEFORE, AFUNPTR(ThenProc13Yes), IARG_END);
        INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)IfProc13, IARG_FAST_ANALYSIS_CALL, IARG_ADDRINT, (ADDRINT)2, IARG_ADDRINT,
                         (ADDRINT)2, IARG_END);
        INS_InsertThenCall(ins, IPOINT_BEFORE, AFUNPTR(ThenProc13YesA), IARG_END);

        INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)IfProc14, IARG_FAST_ANALYSIS_CALL, IARG_ADDRINT, (ADDRINT)2, IARG_ADDRINT,
                         (ADDRINT)1, IARG_END);
        INS_InsertThenCall(ins, IPOINT_BEFORE, AFUNPTR(ThenProc14No), IARG_END);
        INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)IfProc14, IARG_FAST_ANALYSIS_CALL, IARG_ADDRINT, (ADDRINT)1, IARG_ADDRINT,
                         (ADDRINT)2, IARG_END);
        INS_InsertThenCall(ins, IPOINT_BEFORE, AFUNPTR(ThenProc14Yes), IARG_END);

        INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)IfProc15, IARG_FAST_ANALYSIS_CALL, IARG_ADDRINT, (ADDRINT)2, IARG_ADDRINT,
                         (ADDRINT)1, IARG_END);
        INS_InsertThenCall(ins, IPOINT_BEFORE, AFUNPTR(ThenProc15No), IARG_END);
        INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)IfProc15, IARG_FAST_ANALYSIS_CALL, IARG_ADDRINT, (ADDRINT)1, IARG_ADDRINT,
                         (ADDRINT)2, IARG_END);
        INS_InsertThenCall(ins, IPOINT_BEFORE, AFUNPTR(ThenProc15Yes), IARG_END);

        INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)IfProc16, IARG_FAST_ANALYSIS_CALL, IARG_ADDRINT, (ADDRINT)1, IARG_ADDRINT,
                         (ADDRINT)2, IARG_END);
        INS_InsertThenCall(ins, IPOINT_BEFORE, AFUNPTR(ThenProc16No), IARG_END);
        INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)IfProc16, IARG_FAST_ANALYSIS_CALL, IARG_ADDRINT, (ADDRINT)2, IARG_ADDRINT,
                         (ADDRINT)1, IARG_END);
        INS_InsertThenCall(ins, IPOINT_BEFORE, AFUNPTR(ThenProc16Yes), IARG_END);

        INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)IfProc17, IARG_FAST_ANALYSIS_CALL, IARG_ADDRINT, (ADDRINT)1, IARG_ADDRINT,
                         (ADDRINT)2, IARG_END);
        INS_InsertThenCall(ins, IPOINT_BEFORE, AFUNPTR(ThenProc17No), IARG_END);
        INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)IfProc17, IARG_FAST_ANALYSIS_CALL, IARG_ADDRINT, (ADDRINT)2, IARG_ADDRINT,
                         (ADDRINT)1, IARG_END);
        INS_InsertThenCall(ins, IPOINT_BEFORE, AFUNPTR(ThenProc17Yes), IARG_END);

        INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)IfProc18, IARG_FAST_ANALYSIS_CALL, IARG_ADDRINT, (ADDRINT)1, IARG_ADDRINT,
                         (ADDRINT)2, IARG_END);
        INS_InsertThenCall(ins, IPOINT_BEFORE, AFUNPTR(ThenProc18No), IARG_END);
        INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)IfProc18, IARG_FAST_ANALYSIS_CALL, IARG_ADDRINT, (ADDRINT)2, IARG_ADDRINT,
                         (ADDRINT)1, IARG_END);
        INS_InsertThenCall(ins, IPOINT_BEFORE, AFUNPTR(ThenProc18Yes), IARG_END);
        INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)IfProc18, IARG_FAST_ANALYSIS_CALL, IARG_ADDRINT, (ADDRINT)2, IARG_ADDRINT,
                         (ADDRINT)2, IARG_END);
        INS_InsertThenCall(ins, IPOINT_BEFORE, AFUNPTR(ThenProc18YesA), IARG_END);

        INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)IfProc19, IARG_FAST_ANALYSIS_CALL, IARG_ADDRINT, (ADDRINT)1, IARG_ADDRINT,
                         (ADDRINT)2, IARG_END);
        INS_InsertThenCall(ins, IPOINT_BEFORE, AFUNPTR(ThenProc19No), IARG_END);
        INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)IfProc19, IARG_FAST_ANALYSIS_CALL, IARG_ADDRINT, (ADDRINT)2, IARG_ADDRINT,
                         (ADDRINT)1, IARG_END);
        INS_InsertThenCall(ins, IPOINT_BEFORE, AFUNPTR(ThenProc19Yes), IARG_END);
        INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)IfProc19, IARG_FAST_ANALYSIS_CALL, IARG_ADDRINT, (ADDRINT)2, IARG_ADDRINT,
                         (ADDRINT)2, IARG_END);
        INS_InsertThenCall(ins, IPOINT_BEFORE, AFUNPTR(ThenProc19YesA), IARG_END);

        INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)IfProc20, IARG_FAST_ANALYSIS_CALL, IARG_ADDRINT, (ADDRINT)2, IARG_ADDRINT,
                         (ADDRINT)1, IARG_END);
        INS_InsertThenCall(ins, IPOINT_BEFORE, AFUNPTR(ThenProc20No), IARG_END);
        INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)IfProc20, IARG_FAST_ANALYSIS_CALL, IARG_ADDRINT, (ADDRINT)1, IARG_ADDRINT,
                         (ADDRINT)2, IARG_END);
        INS_InsertThenCall(ins, IPOINT_BEFORE, AFUNPTR(ThenProc20Yes), IARG_END);

        INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)IfProc21, IARG_FAST_ANALYSIS_CALL, IARG_ADDRINT, (ADDRINT)2, IARG_ADDRINT,
                         (ADDRINT)1, IARG_END);
        INS_InsertThenCall(ins, IPOINT_BEFORE, AFUNPTR(ThenProc21No), IARG_END);
        INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)IfProc21, IARG_FAST_ANALYSIS_CALL, IARG_ADDRINT, (ADDRINT)1, IARG_ADDRINT,
                         (ADDRINT)2, IARG_END);
        INS_InsertThenCall(ins, IPOINT_BEFORE, AFUNPTR(ThenProc21Yes), IARG_END);

        INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)IfProc22, IARG_FAST_ANALYSIS_CALL, IARG_ADDRINT, (ADDRINT)2, IARG_ADDRINT,
                         (ADDRINT)1, IARG_END);
        INS_InsertThenCall(ins, IPOINT_BEFORE, AFUNPTR(ThenProc22No), IARG_END);
        INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)IfProc22, IARG_FAST_ANALYSIS_CALL, IARG_ADDRINT, (ADDRINT)2, IARG_ADDRINT,
                         (ADDRINT)2, IARG_END);
        INS_InsertThenCall(ins, IPOINT_BEFORE, AFUNPTR(ThenProc22NoA), IARG_END);
        INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)IfProc22, IARG_FAST_ANALYSIS_CALL, IARG_ADDRINT, (ADDRINT)1, IARG_ADDRINT,
                         (ADDRINT)2, IARG_END);
        INS_InsertThenCall(ins, IPOINT_BEFORE, AFUNPTR(ThenProc22Yes), IARG_END);

        INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)IfProc23, IARG_FAST_ANALYSIS_CALL, IARG_ADDRINT, (ADDRINT)2, IARG_ADDRINT,
                         (ADDRINT)1, IARG_END);
        INS_InsertThenCall(ins, IPOINT_BEFORE, AFUNPTR(ThenProc23No), IARG_END);
        INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)IfProc23, IARG_FAST_ANALYSIS_CALL, IARG_ADDRINT, (ADDRINT)2, IARG_ADDRINT,
                         (ADDRINT)2, IARG_END);
        INS_InsertThenCall(ins, IPOINT_BEFORE, AFUNPTR(ThenProc23NoA), IARG_END);
        INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)IfProc23, IARG_FAST_ANALYSIS_CALL, IARG_ADDRINT, (ADDRINT)1, IARG_ADDRINT,
                         (ADDRINT)2, IARG_END);
        INS_InsertThenCall(ins, IPOINT_BEFORE, AFUNPTR(ThenProc23Yes), IARG_END);

        INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)IfProc24, IARG_FAST_ANALYSIS_CALL, IARG_ADDRINT, (ADDRINT)2, IARG_ADDRINT,
                         (ADDRINT)1, IARG_END);
        INS_InsertThenCall(ins, IPOINT_BEFORE, AFUNPTR(ThenProc24No), IARG_END);
        INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)IfProc24, IARG_FAST_ANALYSIS_CALL, IARG_ADDRINT, (ADDRINT)2, IARG_ADDRINT,
                         (ADDRINT)2, IARG_END);
        INS_InsertThenCall(ins, IPOINT_BEFORE, AFUNPTR(ThenProc24NoA), IARG_END);
        INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)IfProc24, IARG_FAST_ANALYSIS_CALL, IARG_ADDRINT, (ADDRINT)1, IARG_ADDRINT,
                         (ADDRINT)2, IARG_END);
        INS_InsertThenCall(ins, IPOINT_BEFORE, AFUNPTR(ThenProc24Yes), IARG_END);

        INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)IfProc25, IARG_FAST_ANALYSIS_CALL, IARG_ADDRINT, (ADDRINT)2, IARG_ADDRINT,
                         (ADDRINT)1, IARG_END);
        INS_InsertThenCall(ins, IPOINT_BEFORE, AFUNPTR(ThenProc25No), IARG_END);
        INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)IfProc25, IARG_FAST_ANALYSIS_CALL, IARG_ADDRINT, (ADDRINT)2, IARG_ADDRINT,
                         (ADDRINT)2, IARG_END);
        INS_InsertThenCall(ins, IPOINT_BEFORE, AFUNPTR(ThenProc25NoA), IARG_END);
        INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)IfProc25, IARG_FAST_ANALYSIS_CALL, IARG_ADDRINT, (ADDRINT)1, IARG_ADDRINT,
                         (ADDRINT)2, IARG_END);
        INS_InsertThenCall(ins, IPOINT_BEFORE, AFUNPTR(ThenProc25Yes), IARG_END);

        INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)IfProc26, IARG_FAST_ANALYSIS_CALL, IARG_ADDRINT, (ADDRINT)1, IARG_ADDRINT,
                         (ADDRINT)2, IARG_END);
        INS_InsertThenCall(ins, IPOINT_BEFORE, AFUNPTR(ThenProc26No), IARG_END);
        INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)IfProc26, IARG_FAST_ANALYSIS_CALL, IARG_ADDRINT, (ADDRINT)127, IARG_ADDRINT,
                         (ADDRINT)127, IARG_END);
        INS_InsertThenCall(ins, IPOINT_BEFORE, AFUNPTR(ThenProc26Yes), IARG_END);

        INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)IfProc27, IARG_FAST_ANALYSIS_CALL, IARG_ADDRINT, (ADDRINT)1, IARG_ADDRINT,
                         (ADDRINT)2, IARG_END);
        INS_InsertThenCall(ins, IPOINT_BEFORE, AFUNPTR(ThenProc27No), IARG_END);
        INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)IfProc27, IARG_FAST_ANALYSIS_CALL, IARG_ADDRINT, (ADDRINT)127, IARG_ADDRINT,
                         (ADDRINT)127, IARG_END);
        INS_InsertThenCall(ins, IPOINT_BEFORE, AFUNPTR(ThenProc27Yes), IARG_END);

        INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)IfProc28, IARG_FAST_ANALYSIS_CALL, IARG_ADDRINT, (ADDRINT)127, IARG_ADDRINT,
                         (ADDRINT)127, IARG_END);
        INS_InsertThenCall(ins, IPOINT_BEFORE, AFUNPTR(ThenProc28No), IARG_END);
        INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)IfProc28, IARG_FAST_ANALYSIS_CALL, IARG_ADDRINT, (ADDRINT)1, IARG_ADDRINT,
                         (ADDRINT)2, IARG_END);
        INS_InsertThenCall(ins, IPOINT_BEFORE, AFUNPTR(ThenProc28Yes), IARG_END);

        INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)IfProc29, IARG_FAST_ANALYSIS_CALL, IARG_ADDRINT, (ADDRINT)127, IARG_ADDRINT,
                         (ADDRINT)127, IARG_END);
        INS_InsertThenCall(ins, IPOINT_BEFORE, AFUNPTR(ThenProc29No), IARG_END);
        INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)IfProc29, IARG_FAST_ANALYSIS_CALL, IARG_ADDRINT, (ADDRINT)1, IARG_ADDRINT,
                         (ADDRINT)2, IARG_END);
        INS_InsertThenCall(ins, IPOINT_BEFORE, AFUNPTR(ThenProc29Yes), IARG_END);

        INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)IfProc30, IARG_FAST_ANALYSIS_CALL, IARG_ADDRINT, (ADDRINT)3, IARG_ADDRINT,
                         (ADDRINT)4, IARG_END);
        INS_InsertThenCall(ins, IPOINT_BEFORE, AFUNPTR(ThenProc30No), IARG_END);
        INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)IfProc30, IARG_FAST_ANALYSIS_CALL, IARG_ADDRINT, (ADDRINT)1, IARG_ADDRINT,
                         (ADDRINT)2, IARG_END);
        INS_InsertThenCall(ins, IPOINT_BEFORE, AFUNPTR(ThenProc30Yes), IARG_END);

        INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)IfProc31, IARG_FAST_ANALYSIS_CALL, IARG_ADDRINT, (ADDRINT)3, IARG_ADDRINT,
                         (ADDRINT)4, IARG_END);
        INS_InsertThenCall(ins, IPOINT_BEFORE, AFUNPTR(ThenProc31No), IARG_END);
        INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)IfProc31, IARG_FAST_ANALYSIS_CALL, IARG_ADDRINT, (ADDRINT)1, IARG_ADDRINT,
                         (ADDRINT)2, IARG_END);
        INS_InsertThenCall(ins, IPOINT_BEFORE, AFUNPTR(ThenProc31Yes), IARG_END);

        INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)IfProc32, IARG_FAST_ANALYSIS_CALL, IARG_ADDRINT, (ADDRINT)1, IARG_ADDRINT,
                         (ADDRINT)2, IARG_END);
        INS_InsertThenCall(ins, IPOINT_BEFORE, AFUNPTR(ThenProc32No), IARG_END);
        INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)IfProc32, IARG_FAST_ANALYSIS_CALL, IARG_ADDRINT, (ADDRINT)3, IARG_ADDRINT,
                         (ADDRINT)4, IARG_END);
        INS_InsertThenCall(ins, IPOINT_BEFORE, AFUNPTR(ThenProc32Yes), IARG_END);

        INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)IfProc33, IARG_FAST_ANALYSIS_CALL, IARG_ADDRINT, (ADDRINT)1, IARG_ADDRINT,
                         (ADDRINT)2, IARG_END);
        INS_InsertThenCall(ins, IPOINT_BEFORE, AFUNPTR(ThenProc33No), IARG_END);
        INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)IfProc33, IARG_FAST_ANALYSIS_CALL, IARG_ADDRINT, (ADDRINT)3, IARG_ADDRINT,
                         (ADDRINT)4, IARG_END);
        INS_InsertThenCall(ins, IPOINT_BEFORE, AFUNPTR(ThenProc33Yes), IARG_END);

        INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)IfProc34, IARG_FAST_ANALYSIS_CALL, IARG_ADDRINT, (ADDRINT)0, IARG_ADDRINT,
                         (ADDRINT)1, IARG_END);
        INS_InsertThenCall(ins, IPOINT_BEFORE, AFUNPTR(ThenProc34No), IARG_END);
        INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)IfProc34, IARG_FAST_ANALYSIS_CALL, IARG_ADDRINT, (ADDRINT)0, IARG_ADDRINT,
                         (ADDRINT)-1, IARG_END);
        INS_InsertThenCall(ins, IPOINT_BEFORE, AFUNPTR(ThenProc34Yes), IARG_END);

        INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)IfProc35, IARG_FAST_ANALYSIS_CALL, IARG_ADDRINT, (ADDRINT)0, IARG_ADDRINT,
                         (ADDRINT)1, IARG_END);
        INS_InsertThenCall(ins, IPOINT_BEFORE, AFUNPTR(ThenProc35No), IARG_END);
        INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)IfProc35, IARG_FAST_ANALYSIS_CALL, IARG_ADDRINT, (ADDRINT)0, IARG_ADDRINT,
                         (ADDRINT)-1, IARG_END);
        INS_InsertThenCall(ins, IPOINT_BEFORE, AFUNPTR(ThenProc35Yes), IARG_END);

        INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)IfProc36, IARG_FAST_ANALYSIS_CALL, IARG_ADDRINT, (ADDRINT)0, IARG_ADDRINT,
                         (ADDRINT)-1, IARG_END);
        INS_InsertThenCall(ins, IPOINT_BEFORE, AFUNPTR(ThenProc36No), IARG_END);
        INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)IfProc36, IARG_FAST_ANALYSIS_CALL, IARG_ADDRINT, (ADDRINT)0, IARG_ADDRINT,
                         (ADDRINT)1, IARG_END);
        INS_InsertThenCall(ins, IPOINT_BEFORE, AFUNPTR(ThenProc36Yes), IARG_END);

        INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)IfProc37, IARG_FAST_ANALYSIS_CALL, IARG_ADDRINT, (ADDRINT)0, IARG_ADDRINT,
                         (ADDRINT)-1, IARG_END);
        INS_InsertThenCall(ins, IPOINT_BEFORE, AFUNPTR(ThenProc37No), IARG_END);
        INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)IfProc37, IARG_FAST_ANALYSIS_CALL, IARG_ADDRINT, (ADDRINT)0, IARG_ADDRINT,
                         (ADDRINT)1, IARG_END);
        INS_InsertThenCall(ins, IPOINT_BEFORE, AFUNPTR(ThenProc37Yes), IARG_END);

        INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)IfProc38, IARG_FAST_ANALYSIS_CALL, IARG_ADDRINT, (ADDRINT)0, IARG_ADDRINT,
                         (ADDRINT)0, IARG_END);
        INS_InsertThenCall(ins, IPOINT_BEFORE, AFUNPTR(ThenProc38No), IARG_END);
        INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)IfProc38, IARG_FAST_ANALYSIS_CALL, IARG_ADDRINT, (ADDRINT)0, IARG_ADDRINT,
                         (ADDRINT)1, IARG_END);
        INS_InsertThenCall(ins, IPOINT_BEFORE, AFUNPTR(ThenProc38Yes), IARG_END);

        INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)IfProc39, IARG_FAST_ANALYSIS_CALL, IARG_ADDRINT, (ADDRINT)0, IARG_ADDRINT,
                         (ADDRINT)0, IARG_END);
        INS_InsertThenCall(ins, IPOINT_BEFORE, AFUNPTR(ThenProc39No), IARG_END);
        INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)IfProc39, IARG_FAST_ANALYSIS_CALL, IARG_ADDRINT, (ADDRINT)0, IARG_ADDRINT,
                         (ADDRINT)1, IARG_END);
        INS_InsertThenCall(ins, IPOINT_BEFORE, AFUNPTR(ThenProc39Yes), IARG_END);

        INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)IfProc40, IARG_FAST_ANALYSIS_CALL, IARG_END);
        INS_InsertThenCall(ins, IPOINT_BEFORE, AFUNPTR(ThenProc40No), IARG_END);

        instrumented++;
    }
}

// This function is called when the application exits
VOID Fini(INT32 code, VOID* v)
{
    printf("numTimesThenProc6YesCalled %d numTimesThenProc6NoCalled %d\n", numTimesThenProc6YesCalled, numTimesThenProc6NoCalled);
    printf("numTimesThenProc7YesCalled %d numTimesThenProc7NoCalled %d\n", numTimesThenProc7YesCalled, numTimesThenProc7NoCalled);
    printf("numTimesThenProc10YesCalled %d numTimesThenProc10NoCalled %d\n", numTimesThenProc10YesCalled,
           numTimesThenProc10NoCalled);
    printf("numTimesThenProc11YesCalled %d numTimesThenProc11NoCalled %d\n", numTimesThenProc11YesCalled,
           numTimesThenProc11NoCalled);
    printf("numTimesThenProc12YesCalled %d numTimesThenProc12NoCalled %d\n", numTimesThenProc12YesCalled,
           numTimesThenProc12NoCalled);
    printf("numTimesThenProc13YesCalled %d numTimesThenProc13NoCalled %d\n", numTimesThenProc13YesCalled,
           numTimesThenProc13NoCalled);
    printf("numTimesThenProc12YesACalled %d numTimesThenProc13YesACalled %d\n", numTimesThenProc12YesACalled,
           numTimesThenProc13YesACalled);
    printf("numTimesThenProc14YesCalled %d numTimesThenProc14NoCalled %d\n", numTimesThenProc14YesCalled,
           numTimesThenProc14NoCalled);
    printf("numTimesThenProc15YesCalled %d numTimesThenProc15NoCalled %d\n", numTimesThenProc15YesCalled,
           numTimesThenProc15NoCalled);
    printf("numTimesThenProc16YesCalled %d numTimesThenProc16NoCalled %d\n", numTimesThenProc16YesCalled,
           numTimesThenProc16NoCalled);
    printf("numTimesThenProc17YesCalled %d numTimesThenProc17NoCalled %d\n", numTimesThenProc17YesCalled,
           numTimesThenProc17NoCalled);
    printf("numTimesThenProc18YesCalled %d numTimesThenProc18NoCalled %d\n", numTimesThenProc18YesCalled,
           numTimesThenProc18NoCalled);
    printf("numTimesThenProc19YesCalled %d numTimesThenProc19NoCalled %d\n", numTimesThenProc19YesCalled,
           numTimesThenProc19NoCalled);
    printf("numTimesThenProc18YesACalled %d numTimesThenProc19YesACalled %d\n", numTimesThenProc18YesACalled,
           numTimesThenProc19YesACalled);
    printf("numTimesThenProc20YesCalled %d numTimesThenProc20NoCalled %d\n", numTimesThenProc20YesCalled,
           numTimesThenProc20NoCalled);
    printf("numTimesThenProc21YesCalled %d numTimesThenProc21NoCalled %d\n", numTimesThenProc21YesCalled,
           numTimesThenProc21NoCalled);
    printf("numTimesThenProc22YesCalled %d numTimesThenProc22NoCalled %d\n", numTimesThenProc22YesCalled,
           numTimesThenProc22NoCalled);
    printf("numTimesThenProc23YesCalled %d numTimesThenProc23NoCalled %d\n", numTimesThenProc23YesCalled,
           numTimesThenProc23NoCalled);
    printf("numTimesThenProc22NoACalled %d numTimesThenProc23NoACalled %d\n", numTimesThenProc22NoACalled,
           numTimesThenProc23NoACalled);
    printf("numTimesThenProc24YesCalled %d numTimesThenProc24NoCalled %d\n", numTimesThenProc24YesCalled,
           numTimesThenProc24NoCalled);
    printf("numTimesThenProc25YesCalled %d numTimesThenProc25NoCalled %d\n", numTimesThenProc25YesCalled,
           numTimesThenProc25NoCalled);
    printf("numTimesThenProc24NoACalled %d numTimesThenProc25NoACalled %d\n", numTimesThenProc24NoACalled,
           numTimesThenProc25NoACalled);
    printf("numTimesThenProc26YesCalled %d numTimesThenProc26NoCalled %d\n", numTimesThenProc26YesCalled,
           numTimesThenProc26NoCalled);
    printf("numTimesThenProc27YesCalled %d numTimesThenProc27NoCalled %d\n", numTimesThenProc27YesCalled,
           numTimesThenProc27NoCalled);
    printf("numTimesThenProc28YesCalled %d numTimesThenProc28NoCalled %d\n", numTimesThenProc28YesCalled,
           numTimesThenProc28NoCalled);
    printf("numTimesThenProc29YesCalled %d numTimesThenProc29NoCalled %d\n", numTimesThenProc29YesCalled,
           numTimesThenProc29NoCalled);
    printf("numTimesThenProc30YesCalled %d numTimesThenProc30NoCalled %d\n", numTimesThenProc30YesCalled,
           numTimesThenProc30NoCalled);
    printf("numTimesThenProc31YesCalled %d numTimesThenProc31NoCalled %d\n", numTimesThenProc31YesCalled,
           numTimesThenProc31NoCalled);
    printf("numTimesThenProc32YesCalled %d numTimesThenProc32NoCalled %d\n", numTimesThenProc32YesCalled,
           numTimesThenProc32NoCalled);
    printf("numTimesThenProc33YesCalled %d numTimesThenProc33NoCalled %d\n", numTimesThenProc33YesCalled,
           numTimesThenProc33NoCalled);
    printf("numTimesThenProc34YesCalled %d numTimesThenProc34NoCalled %d\n", numTimesThenProc34YesCalled,
           numTimesThenProc34NoCalled);
    printf("numTimesThenProc35YesCalled %d numTimesThenProc35NoCalled %d\n", numTimesThenProc35YesCalled,
           numTimesThenProc35NoCalled);
    printf("numTimesThenProc36YesCalled %d numTimesThenProc36NoCalled %d\n", numTimesThenProc36YesCalled,
           numTimesThenProc36NoCalled);
    printf("numTimesThenProc37YesCalled %d numTimesThenProc37NoCalled %d\n", numTimesThenProc37YesCalled,
           numTimesThenProc37NoCalled);
    printf("numTimesThenProc38YesCalled %d numTimesThenProc38NoCalled %d\n", numTimesThenProc38YesCalled,
           numTimesThenProc38NoCalled);
    printf("numTimesThenProc39YesCalled %d numTimesThenProc39NoCalled %d\n", numTimesThenProc39YesCalled,
           numTimesThenProc39NoCalled);
    printf("numTimesThenProc40NoCalled  %d\n", numTimesThenProc40NoCalled);
    if (!(numTimesThenProc6YesCalled && numTimesThenProc7YesCalled && numTimesThenProc10YesCalled &&
          numTimesThenProc11YesCalled && numTimesThenProc12YesCalled && numTimesThenProc13YesCalled &&
          numTimesThenProc12YesACalled && numTimesThenProc14YesCalled && numTimesThenProc15YesCalled &&
          numTimesThenProc16YesCalled && numTimesThenProc17YesCalled && numTimesThenProc18YesCalled &&
          numTimesThenProc19YesCalled && numTimesThenProc18YesACalled && numTimesThenProc19YesACalled &&
          numTimesThenProc20YesCalled && numTimesThenProc20YesCalled && numTimesThenProc21YesCalled &&
          numTimesThenProc22YesCalled && numTimesThenProc23YesCalled && numTimesThenProc24YesCalled &&
          numTimesThenProc25YesCalled && numTimesThenProc26YesCalled && numTimesThenProc27YesCalled &&
          numTimesThenProc28YesCalled && numTimesThenProc29YesCalled && numTimesThenProc30YesCalled &&
          numTimesThenProc31YesCalled && numTimesThenProc32YesCalled && numTimesThenProc33YesCalled &&
          numTimesThenProc34YesCalled && numTimesThenProc35YesCalled && numTimesThenProc36YesCalled &&
          numTimesThenProc37YesCalled && numTimesThenProc38YesCalled && numTimesThenProc39YesCalled))
    {
        printf("ERROR a THEN routine that was expected to be called was not called\n");
        exit(1);
    }
    if (numTimesThenProc6NoCalled || numTimesThenProc7NoCalled || numTimesThenProc10NoCalled || numTimesThenProc11NoCalled ||
        numTimesThenProc12NoCalled || numTimesThenProc13NoCalled || numTimesThenProc14NoCalled || numTimesThenProc15NoCalled ||
        numTimesThenProc16NoCalled || numTimesThenProc17NoCalled || numTimesThenProc18NoCalled || numTimesThenProc19NoCalled ||
        numTimesThenProc20NoCalled || numTimesThenProc21NoCalled || numTimesThenProc22NoCalled || numTimesThenProc23NoCalled ||
        numTimesThenProc22NoACalled || numTimesThenProc23NoACalled || numTimesThenProc24NoCalled || numTimesThenProc25NoCalled ||
        numTimesThenProc25NoACalled || numTimesThenProc26NoCalled || numTimesThenProc27NoCalled || numTimesThenProc28NoCalled ||
        numTimesThenProc29NoCalled || numTimesThenProc30NoCalled || numTimesThenProc31NoCalled || numTimesThenProc32NoCalled ||
        numTimesThenProc33NoCalled || numTimesThenProc34NoCalled || numTimesThenProc35NoCalled || numTimesThenProc36NoCalled ||
        numTimesThenProc37NoCalled || numTimesThenProc38NoCalled || numTimesThenProc39NoCalled || numTimesThenProc40NoCalled)
    {
        printf("ERROR a THEN routine that was NOT expected to be called was called\n");
        exit(1);
    }
}

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 Usage() { return -1; }

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */
/*   argc, argv are the entire command line: pin -t <toolname> -- ...    */
/* ===================================================================== */

int main(int argc, char* argv[])
{
    // Initialize pin
    if (PIN_Init(argc, argv)) return Usage();

    // Register Instruction to be called to instrument instructions
    INS_AddInstrumentFunction(Instruction, 0);

    // Register Fini to be called when the application exits
    PIN_AddFiniFunction(Fini, 0);

    // Start the program, never returns
    PIN_StartProgram();

    return 0;
}
