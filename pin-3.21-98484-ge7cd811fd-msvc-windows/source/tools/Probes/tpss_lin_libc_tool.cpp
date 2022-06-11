/*
 * Copyright (C) 2012-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * This tool mimics the behavior of TPSS on Linux by adding probes to various libc functions.
 * However, in this tool these probes are merely empty wrappers that call the original functions.
 * The objective of the test is to verify that probe generation and insertion don't cause Pin
 * to crash.
 *
 * This file is part of the tpss_lin_libc tool and compiles against the tool's libc (e.g. PIN CRT).
 * This file implements all the intrumenting logic thus requires PIN headers.
 */
#include "pin.H"
#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include <unistd.h>
using std::cerr;
using std::endl;
using std::hex;
using std::ios;
using std::ofstream;
using std::string;

/* ===================================================================== */
/* Commandline Switches                                                  */
/* ===================================================================== */

typedef int* INT_PTR;

typedef void* VOID_PTR;

typedef char* CHAR_PTR;

ofstream OutFile;

KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "tpss_lin_libc.txt", "specify tool log file name");

/* ===================================================================== */
/* Pointers to the original functions that we probe                      */
/* ===================================================================== */
extern AFUNPTR fptrnanosleep;
extern AFUNPTR fptrdl_iterate_phdr;
extern AFUNPTR fptrsystem;
extern AFUNPTR fptralarm;
extern AFUNPTR fptrrecvmsg;
extern AFUNPTR fptrsendmsg;
extern AFUNPTR fptrpause;
extern AFUNPTR fptrsigtimedwait;
extern AFUNPTR fptrsigwaitinfo;
extern AFUNPTR fptrepoll_wait;
extern AFUNPTR fptrppoll;
extern AFUNPTR fptrmsgsnd;
extern AFUNPTR fptrmsgrcv;
extern AFUNPTR fptrsemop;
extern AFUNPTR fptrsemtimedop;
extern AFUNPTR fptrusleep;
extern AFUNPTR fptrualarm;
extern AFUNPTR fptrgetitimer;
extern AFUNPTR fptrsigwait;
extern AFUNPTR fptrmsgget;
extern AFUNPTR fptrsemget;
extern AFUNPTR fptrwait;
extern AFUNPTR fptrwaitpid;
extern AFUNPTR fptrwaitid;
extern AFUNPTR fptrwait3;
extern AFUNPTR fptrwait4;
extern AFUNPTR fptrreadv;
extern AFUNPTR fptrwritev;
extern AFUNPTR fptrflock;
extern AFUNPTR fptrflockfile;
extern AFUNPTR fptrfunlockfile;
extern AFUNPTR fptrlockf;
extern AFUNPTR fptrsetenv;
extern AFUNPTR fptrunsetenv;
extern AFUNPTR fptrgetenv;
extern AFUNPTR fptrperror;
extern AFUNPTR fptrmmap;
extern AFUNPTR fptrmunmap;
extern AFUNPTR fptrfileno;
extern AFUNPTR fptrgetpid;
extern AFUNPTR fptrgetppid;
extern AFUNPTR fptrmemset;
extern AFUNPTR fptrmemcpy;
extern AFUNPTR fptraccess;
extern AFUNPTR fptrlseek;
extern AFUNPTR fptrlseek64;
extern AFUNPTR fptrfdatasync;
extern AFUNPTR fptrunlink;
extern AFUNPTR fptrstrlen;
extern AFUNPTR fptrwcslen;
extern AFUNPTR fptrstrcpy;
extern AFUNPTR fptrstrncpy;
extern AFUNPTR fptrstrcat;
extern AFUNPTR fptrstrstr;
extern AFUNPTR fptrstrrchr;
extern AFUNPTR fptrstrcmp;
extern AFUNPTR fptrstrncmp;
extern AFUNPTR fptrsigaddset;
extern AFUNPTR fptrsigdelset;
extern AFUNPTR fptrstrerror;
extern AFUNPTR fptrbind;
extern AFUNPTR fptrlisten;
extern AFUNPTR fptruname;
extern AFUNPTR fptrgethostname;
extern AFUNPTR fptrkill;
extern AFUNPTR fptrsched_yield;
extern AFUNPTR fptrtimer_settime;
extern AFUNPTR fptrsigaltstack;
extern AFUNPTR fptrshutdown;
extern AFUNPTR fptrsleep;
extern AFUNPTR fptrsocket;
extern AFUNPTR fptrselect;
extern AFUNPTR fptrpoll;
extern AFUNPTR fptraccept;
extern AFUNPTR fptrconnect;
extern AFUNPTR fptrrecv;
extern AFUNPTR fptrrecvfrom;
extern AFUNPTR fptrsend;
extern AFUNPTR fptrgetwc;
extern AFUNPTR fptrsetitimer;
extern AFUNPTR fptrsigpending;
extern AFUNPTR fptrsigaction;
extern AFUNPTR fptrsignal;
extern AFUNPTR fptrabort;
extern AFUNPTR fptrsendto;
extern AFUNPTR fptr_IO_getc;
extern AFUNPTR fptrgetchar;
extern AFUNPTR fptrgetwchar;
extern AFUNPTR fptrgets;
extern AFUNPTR fptrfgets;
extern AFUNPTR fptrfgetwc;
extern AFUNPTR fptrfread;
extern AFUNPTR fptrfwrite;
extern AFUNPTR fptropen;
extern AFUNPTR fptrgetw;
extern AFUNPTR fptrfgetc;
extern AFUNPTR fptrfgetws;
extern AFUNPTR fptrpipe;
extern AFUNPTR fptrread;
extern AFUNPTR fptrwrite;
extern AFUNPTR fptrfopen;
extern AFUNPTR fptrfdopen;
extern AFUNPTR fptrclose;
extern AFUNPTR fptrfclose;
extern AFUNPTR fptrcallrpc;
extern AFUNPTR fptrclnt_broadcast;
extern AFUNPTR fptrclntudp_create;
extern AFUNPTR fptrclntudp_bufcreate;
extern AFUNPTR fptrpmap_getmaps;
extern AFUNPTR fptrpmap_getport;
extern AFUNPTR fptrpmap_rmtcall;
extern AFUNPTR fptrpmap_set;
extern AFUNPTR fptrclntraw_create;
extern AFUNPTR fptrsvc_run;
extern AFUNPTR fptrsvc_sendreply;
extern AFUNPTR fptrsvcraw_create;
extern AFUNPTR fptrsvctcp_create;
extern AFUNPTR fptrsvcudp_bufcreate;
extern AFUNPTR fptrsvcudp_create;
extern AFUNPTR fptr_exit;
extern AFUNPTR fptrsigprocmask;
extern AFUNPTR fptrexit;
extern AFUNPTR fptrpselect;
extern AFUNPTR fptrioctl;
extern AFUNPTR fptr__libc_dlopen_mode;
extern AFUNPTR fptr__errno_location;
extern AFUNPTR fptrsyscall;

/* ===================================================================== */
/* Replacement functions implemented in the other part of the tool       */
/* ===================================================================== */
extern "C"
{
    void mysleep();
    void mysocket();
    void myshutdown();
    void myselect();
    void mypoll();
    void mypselect();
    void myaccept();
    void myconnect();
    void myrecv();
    void myrecvfrom();
    void mysend();
    void mysendto();
    void mygetwc();
    void mygetw();
    void my_IO_getc();
    void mygetchar();
    void mygetwchar();
    void mygets();
    void myfgetc();
    void myfgetwc();
    void myfgets();
    void myfgetws();
    void myfread();
    void myfwrite();
    void mypipe();
    void myread();
    void mywrite();
    void myopen();
    void myfopen();
    void myfdopen();
    void myclose();
    void myfclose();
    void mycallrpc();
    void myclnt_broadcast();
    void myclntudp_create();
    void myclntudp_bufcreate();
    void mypmap_getmaps();
    void mypmap_getport();
    void mypmap_rmtcall();
    void mypmap_set();
    void myclntraw_create();
    void mysvc_run();
    void mysvc_sendreply();
    void mysvcraw_create();
    void mysvctcp_create();
    void mysvcudp_bufcreate();
    void mysvcudp_create();
    void myabort();
    void my_exit();
    void my_nanosleep();
    void mysignal();
    void mysigprocmask();
    void mysigpending();
    void mysigaction();
    void mysetitimer();
    void myexit();
    void mydl_iterate_phdr();
    void mysystem();
    void myalarm();
    void myrecvmsg();
    void mysendmsg();
    void mypause();
    void mysigtimedwait();
    void mysigwaitinfo();
    void myepoll_wait();
    void myppoll();
    void mymsgsnd();
    void mymsgrcv();
    void mymsgrcv();
    void mysemtimedop();
    void myusleep();
    void myualarm();
    void mygetitimer();
    void mysigwait();
    void mymsgget();
    void mysemget();
    void mywait();
    void mywaitpid();
    void mywaitid();
    void mywait3();
    void mywait4();
    void myreadv();
    void mywritev();
    void myflockfile();
    void mylockf();
    void mysetenv();
    void myunsetenv();
    void mygetenv();
    void myperror();
    void mymmap();
    void mymunmap();
    void myfileno();
    void mygetpid();
    void mygetppid();
    void mymemset();
    void mymemcpy();
    void myaccess();
    void mylseek();
    void mylseek64();
    void myfdatasync();
    void myunlink();
    void mystrlen();
    void mywcslen();
    void mystrcpy();
    void mystrcpy();
    void mystrcat();
    void mystrstr();
    void mystrchr0();
    void mystrrchr();
    void mystrcmp();
    void mystrncmp();
    void mysigaddset();
    void mysigdelset();
    void mysigismember();
    void mystrerror();
    void mybind();
    void mylisten();
    void myuname();
    void mygethostname();
    void mykill();
    void mysched_yield();
    void mytimer_settime();
    void mysigaltstack();
    void myioctl();
    void myflock();
    void my__libc_dlopen_mode();
    void my__errno_location();
    void mysyscall();
    void myfunlockfile();
}

/* ===================================================================== */
/* Utility functions                                                     */
/* ===================================================================== */

// Print help information
INT32 Usage()
{
    cerr << "This tool mimics the behavior of TPSS on Linux by adding probes to various libc functions." << endl;
    cerr << KNOB_BASE::StringKnobSummary();
    cerr << endl;
    return 1;
}

// Utility function to return the time
string CurrentTime()
{
    char tmpbuf[128];
    time_t thetime = time(NULL);
    ctime_r(&thetime, tmpbuf);
    return tmpbuf;
}

extern "C" void printFunctionCalled(const char* funcName)
{
    OutFile << CurrentTime() << funcName << " called " << endl;
    OutFile.flush();
}

bool is_GLIBC2_24_or_newer(IMG img)
{
    char path[PATH_MAX] = {0};
    if (0 <= readlink(IMG_Name(img).c_str(), path, sizeof(path) - 1))
    {
        return (string("libc-2.24.so") == basename(path));
    }
    return false;
}

/* ===================================================================== */
/* Instrumnetation functions                                             */
/* ===================================================================== */

// Image load callback - inserts the probes.
void ImgLoad(IMG img, VOID_PTR v)
{
    // Called every time a new image is loaded
    if ((IMG_Name(img).find("libc.so") != string::npos) || (IMG_Name(img).find("LIBC.SO") != string::npos) ||
        (IMG_Name(img).find("LIBC.so") != string::npos))
    {
        RTN rtnsleep = RTN_FindByName(img, "sleep");
        if (RTN_Valid(rtnsleep) && RTN_IsSafeForProbedReplacement(rtnsleep))
        {
            OutFile << CurrentTime() << "Inserting probe for sleep at " << RTN_Address(rtnsleep) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnsleep, AFUNPTR(mysleep)));
            fptrsleep    = fptr;
        }

        RTN rtnsocket = RTN_FindByName(img, "socket");
        if (RTN_Valid(rtnsocket) && RTN_IsSafeForProbedReplacement(rtnsocket))
        {
            OutFile << CurrentTime() << "Inserting probe for socket at " << RTN_Address(rtnsocket) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnsocket, AFUNPTR(mysocket)));
            fptrsocket   = fptr;
        }

        RTN rtnshutdown = RTN_FindByName(img, "shutdown");
        if (RTN_Valid(rtnshutdown) && RTN_IsSafeForProbedReplacement(rtnshutdown))
        {
            OutFile << CurrentTime() << "Inserting probe for shutdown at " << RTN_Address(rtnshutdown) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnshutdown, AFUNPTR(myshutdown)));
            fptrshutdown = fptr;
        }

        RTN rtnselect = RTN_FindByName(img, "select");
        if (RTN_Valid(rtnselect) && RTN_IsSafeForProbedReplacement(rtnselect))
        {
            OutFile << CurrentTime() << "Inserting probe for select at " << RTN_Address(rtnselect) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnselect, AFUNPTR(myselect)));
            fptrselect   = fptr;
        }

        RTN rtnpoll = RTN_FindByName(img, "poll");
        if (RTN_Valid(rtnpoll) && RTN_IsSafeForProbedReplacement(rtnpoll))
        {
            OutFile << CurrentTime() << "Inserting probe for poll at " << RTN_Address(rtnpoll) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnpoll, AFUNPTR(mypoll)));
            fptrpoll     = fptr;
        }

        RTN rtnpselect = RTN_FindByName(img, "pselect");
        if (RTN_Valid(rtnpselect) && RTN_IsSafeForProbedReplacement(rtnpselect))
        {
            OutFile << CurrentTime() << "Inserting probe for pselect at " << RTN_Address(rtnpselect) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnpselect, AFUNPTR(mypselect)));
            fptrpselect  = fptr;
        }

        RTN rtnaccept = RTN_FindByName(img, "accept");
        if (RTN_Valid(rtnaccept) && RTN_IsSafeForProbedReplacement(rtnaccept))
        {
            OutFile << CurrentTime() << "Inserting probe for accept at " << RTN_Address(rtnaccept) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnaccept, AFUNPTR(myaccept)));
            fptraccept   = fptr;
        }

        RTN rtnconnect = RTN_FindByName(img, "connect");
        if (RTN_Valid(rtnconnect) && RTN_IsSafeForProbedReplacement(rtnconnect))
        {
            OutFile << CurrentTime() << "Inserting probe for connect at " << RTN_Address(rtnconnect) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnconnect, AFUNPTR(myconnect)));
            fptrconnect  = fptr;
        }

        RTN rtnrecv = RTN_FindByName(img, "recv");
        if (RTN_Valid(rtnrecv) && RTN_IsSafeForProbedReplacement(rtnrecv))
        {
            OutFile << CurrentTime() << "Inserting probe for recv at " << RTN_Address(rtnrecv) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnrecv, AFUNPTR(myrecv)));
            fptrrecv     = fptr;
        }

        RTN rtnrecvfrom = RTN_FindByName(img, "recvfrom");
        if (RTN_Valid(rtnrecvfrom) && RTN_IsSafeForProbedReplacement(rtnrecvfrom))
        {
            OutFile << CurrentTime() << "Inserting probe for recvfrom at " << RTN_Address(rtnrecvfrom) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnrecvfrom, AFUNPTR(myrecvfrom)));
            fptrrecvfrom = fptr;
        }

        RTN rtnsend = RTN_FindByName(img, "send");
        if (RTN_Valid(rtnsend) && RTN_IsSafeForProbedReplacement(rtnsend))
        {
            OutFile << CurrentTime() << "Inserting probe for send at " << RTN_Address(rtnsend) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnsend, AFUNPTR(mysend)));
            fptrsend     = fptr;
        }

        RTN rtnsendto = RTN_FindByName(img, "sendto");
        if (RTN_Valid(rtnsendto) && RTN_IsSafeForProbedReplacement(rtnsendto))
        {
            OutFile << CurrentTime() << "Inserting probe for sendto at " << RTN_Address(rtnsendto) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnsendto, AFUNPTR(mysendto)));
            fptrsendto   = fptr;
        }

        RTN rtngetwc = RTN_FindByName(img, "getwc");
        if (RTN_Valid(rtngetwc) && RTN_IsSafeForProbedReplacement(rtngetwc))
        {
            OutFile << CurrentTime() << "Inserting probe for getwc at " << RTN_Address(rtngetwc) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtngetwc, AFUNPTR(mygetwc)));
            fptrgetwc    = fptr;
        }

        RTN rtngetw = RTN_FindByName(img, "getw");
        if (RTN_Valid(rtngetw) && RTN_IsSafeForProbedReplacement(rtngetw))
        {
            OutFile << CurrentTime() << "Inserting probe for getw at " << RTN_Address(rtngetw) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtngetw, AFUNPTR(mygetw)));
            fptrgetw     = fptr;
        }

        RTN rtn_IO_getc = RTN_FindByName(img, "_IO_getc");
        if (RTN_Valid(rtn_IO_getc) && RTN_IsSafeForProbedReplacement(rtn_IO_getc))
        {
            OutFile << CurrentTime() << "Inserting probe for _IO_getc at " << RTN_Address(rtn_IO_getc) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtn_IO_getc, AFUNPTR(my_IO_getc)));
            fptr_IO_getc = fptr;
        }

        RTN rtngetchar = RTN_FindByName(img, "getchar");
        if (RTN_Valid(rtngetchar) && RTN_IsSafeForProbedReplacement(rtngetchar))
        {
            OutFile << CurrentTime() << "Inserting probe for getchar at " << RTN_Address(rtngetchar) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtngetchar, AFUNPTR(mygetchar)));
            fptrgetchar  = fptr;
        }

        RTN rtngetwchar = RTN_FindByName(img, "getwchar");
        if (RTN_Valid(rtngetwchar) && RTN_IsSafeForProbedReplacement(rtngetwchar))
        {
            OutFile << CurrentTime() << "Inserting probe for getwchar at " << RTN_Address(rtngetwchar) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtngetwchar, AFUNPTR(mygetwchar)));
            fptrgetwchar = fptr;
        }

        RTN rtngets = RTN_FindByName(img, "gets");
        if (RTN_Valid(rtngets) && RTN_IsSafeForProbedReplacement(rtngets))
        {
            OutFile << CurrentTime() << "Inserting probe for gets at " << RTN_Address(rtngets) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtngets, AFUNPTR(mygets)));
            fptrgets     = fptr;
        }

        RTN rtnfgetc = RTN_FindByName(img, "fgetc");
        if (RTN_Valid(rtnfgetc) && RTN_IsSafeForProbedReplacement(rtnfgetc))
        {
            OutFile << CurrentTime() << "Inserting probe for fgetc at " << RTN_Address(rtnfgetc) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnfgetc, AFUNPTR(myfgetc)));
            fptrfgetc    = fptr;
        }

        RTN rtnfgetwc = RTN_FindByName(img, "fgetwc");
        if (RTN_Valid(rtnfgetwc) && RTN_IsSafeForProbedReplacement(rtnfgetwc))
        {
            OutFile << CurrentTime() << "Inserting probe for fgetwc at " << RTN_Address(rtnfgetwc) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnfgetwc, AFUNPTR(myfgetwc)));
            fptrfgetwc   = fptr;
        }

        RTN rtnfgets = RTN_FindByName(img, "fgets");
        if (RTN_Valid(rtnfgets) && RTN_IsSafeForProbedReplacement(rtnfgets))
        {
            OutFile << CurrentTime() << "Inserting probe for fgets at " << RTN_Address(rtnfgets) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnfgets, AFUNPTR(myfgets)));
            fptrfgets    = fptr;
        }

        RTN rtnfgetws = RTN_FindByName(img, "fgetws");
        if (RTN_Valid(rtnfgetws) && RTN_IsSafeForProbedReplacement(rtnfgetws))
        {
            OutFile << CurrentTime() << "Inserting probe for fgetws at " << RTN_Address(rtnfgetws) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnfgetws, AFUNPTR(myfgetws)));
            fptrfgetws   = fptr;
        }

        RTN rtnfread = RTN_FindByName(img, "fread");
        if (RTN_Valid(rtnfread) && RTN_IsSafeForProbedReplacement(rtnfread))
        {
            OutFile << CurrentTime() << "Inserting probe for fread at " << RTN_Address(rtnfread) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnfread, AFUNPTR(myfread)));
            fptrfread    = fptr;
        }

        RTN rtnfwrite = RTN_FindByName(img, "fwrite");
        if (RTN_Valid(rtnfwrite) && RTN_IsSafeForProbedReplacement(rtnfwrite))
        {
            OutFile << CurrentTime() << "Inserting probe for fwrite at " << RTN_Address(rtnfwrite) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnfwrite, AFUNPTR(myfwrite)));
            fptrfwrite   = fptr;
        }

        RTN rtnpipe = RTN_FindByName(img, "pipe");
        if (RTN_Valid(rtnpipe) && RTN_IsSafeForProbedReplacement(rtnpipe))
        {
            OutFile << CurrentTime() << "Inserting probe for pipe at " << RTN_Address(rtnpipe) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnpipe, AFUNPTR(mypipe)));
            fptrpipe     = fptr;
        }

        RTN rtnread = RTN_FindByName(img, "read");
        if (RTN_Valid(rtnread) && RTN_IsSafeForProbedReplacement(rtnread))
        {
            OutFile << CurrentTime() << "Inserting probe for read at " << RTN_Address(rtnread) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnread, AFUNPTR(myread)));
            fptrread     = fptr;
        }

        RTN rtnwrite = RTN_FindByName(img, "write");
        if (RTN_Valid(rtnwrite) && RTN_IsSafeForProbedReplacement(rtnwrite))
        {
            OutFile << CurrentTime() << "Inserting probe for write at " << RTN_Address(rtnwrite) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnwrite, AFUNPTR(mywrite)));
            fptrwrite    = fptr;
        }

        RTN rtnopen = RTN_FindByName(img, "open");
        if (RTN_Valid(rtnopen) && RTN_IsSafeForProbedReplacement(rtnopen))
        {
            OutFile << CurrentTime() << "Inserting probe for open at " << RTN_Address(rtnopen) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnopen, AFUNPTR(myopen)));
            fptropen     = fptr;
        }

        RTN rtnfopen = RTN_FindByName(img, "fopen");
        if (RTN_Valid(rtnfopen) && RTN_IsSafeForProbedReplacement(rtnfopen))
        {
            OutFile << CurrentTime() << "Inserting probe for fopen at " << RTN_Address(rtnfopen) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnfopen, AFUNPTR(myfopen)));
            fptrfopen    = fptr;
        }

        RTN rtnfdopen = RTN_FindByName(img, "fdopen");
        if (RTN_Valid(rtnfdopen) && RTN_IsSafeForProbedReplacement(rtnfdopen))
        {
            OutFile << CurrentTime() << "Inserting probe for fdopen at " << RTN_Address(rtnfdopen) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnfdopen, AFUNPTR(myfdopen)));
            fptrfdopen   = fptr;
        }

        RTN rtnclose = RTN_FindByName(img, "close");
        if (RTN_Valid(rtnclose) && RTN_IsSafeForProbedReplacement(rtnclose))
        {
            OutFile << CurrentTime() << "Inserting probe for close at " << RTN_Address(rtnclose) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnclose, AFUNPTR(myclose)));
            fptrclose    = fptr;
        }

        RTN rtnfclose = RTN_FindByName(img, "fclose");
        if (RTN_Valid(rtnfclose) && RTN_IsSafeForProbedReplacement(rtnfclose))
        {
            OutFile << CurrentTime() << "Inserting probe for fclose at " << RTN_Address(rtnfclose) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnfclose, AFUNPTR(myfclose)));
            fptrfclose   = fptr;
        }

        RTN rtncallrpc = RTN_FindByName(img, "callrpc");
        if (RTN_Valid(rtncallrpc) && RTN_IsSafeForProbedReplacement(rtncallrpc))
        {
            OutFile << CurrentTime() << "Inserting probe for callrpc at " << RTN_Address(rtncallrpc) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtncallrpc, AFUNPTR(mycallrpc)));
            fptrcallrpc  = fptr;
        }

        RTN rtnclnt_broadcast = RTN_FindByName(img, "clnt_broadcast");
        if (RTN_Valid(rtnclnt_broadcast) && RTN_IsSafeForProbedReplacement(rtnclnt_broadcast))
        {
            OutFile << CurrentTime() << "Inserting probe for clnt_broadcast at " << RTN_Address(rtnclnt_broadcast) << endl;
            OutFile.flush();
            AFUNPTR fptr       = (RTN_ReplaceProbed(rtnclnt_broadcast, AFUNPTR(myclnt_broadcast)));
            fptrclnt_broadcast = fptr;
        }

        RTN rtnclntudp_create = RTN_FindByName(img, "clntudp_create");
        if (RTN_Valid(rtnclntudp_create) && RTN_IsSafeForProbedReplacement(rtnclntudp_create))
        {
            OutFile << CurrentTime() << "Inserting probe for clntudp_create at " << RTN_Address(rtnclntudp_create) << endl;
            OutFile.flush();
            AFUNPTR fptr       = (RTN_ReplaceProbed(rtnclntudp_create, AFUNPTR(myclntudp_create)));
            fptrclntudp_create = fptr;
        }

        RTN rtnclntudp_bufcreate = RTN_FindByName(img, "clntudp_bufcreate");
        if (RTN_Valid(rtnclntudp_bufcreate) && RTN_IsSafeForProbedReplacement(rtnclntudp_bufcreate))
        {
            OutFile << CurrentTime() << "Inserting probe for clntudp_bufcreate at " << RTN_Address(rtnclntudp_bufcreate) << endl;
            OutFile.flush();
            AFUNPTR fptr          = (RTN_ReplaceProbed(rtnclntudp_bufcreate, AFUNPTR(myclntudp_bufcreate)));
            fptrclntudp_bufcreate = fptr;
        }

        RTN rtnpmap_getmaps = RTN_FindByName(img, "pmap_getmaps");
        if (RTN_Valid(rtnpmap_getmaps) && RTN_IsSafeForProbedReplacement(rtnpmap_getmaps))
        {
            OutFile << CurrentTime() << "Inserting probe for pmap_getmaps at " << RTN_Address(rtnpmap_getmaps) << endl;
            OutFile.flush();
            AFUNPTR fptr     = (RTN_ReplaceProbed(rtnpmap_getmaps, AFUNPTR(mypmap_getmaps)));
            fptrpmap_getmaps = fptr;
        }

        RTN rtnpmap_getport = RTN_FindByName(img, "pmap_getport");
        if (RTN_Valid(rtnpmap_getport) && RTN_IsSafeForProbedReplacement(rtnpmap_getport))
        {
            OutFile << CurrentTime() << "Inserting probe for pmap_getport at " << RTN_Address(rtnpmap_getport) << endl;
            OutFile.flush();
            AFUNPTR fptr     = (RTN_ReplaceProbed(rtnpmap_getport, AFUNPTR(mypmap_getport)));
            fptrpmap_getport = fptr;
        }

        RTN rtnpmap_rmtcall = RTN_FindByName(img, "pmap_rmtcall");
        if (RTN_Valid(rtnpmap_rmtcall) && RTN_IsSafeForProbedReplacement(rtnpmap_rmtcall))
        {
            OutFile << CurrentTime() << "Inserting probe for pmap_rmtcall at " << RTN_Address(rtnpmap_rmtcall) << endl;
            OutFile.flush();
            AFUNPTR fptr     = (RTN_ReplaceProbed(rtnpmap_rmtcall, AFUNPTR(mypmap_rmtcall)));
            fptrpmap_rmtcall = fptr;
        }

        RTN rtnpmap_set = RTN_FindByName(img, "pmap_set");
        if (RTN_Valid(rtnpmap_set) && RTN_IsSafeForProbedReplacement(rtnpmap_set))
        {
            OutFile << CurrentTime() << "Inserting probe for pmap_set at " << RTN_Address(rtnpmap_set) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnpmap_set, AFUNPTR(mypmap_set)));
            fptrpmap_set = fptr;
        }

        RTN rtnclntraw_create = RTN_FindByName(img, "clntraw_create");
        if (RTN_Valid(rtnclntraw_create) && RTN_IsSafeForProbedReplacement(rtnclntraw_create))
        {
            OutFile << CurrentTime() << "Inserting probe for clntraw_create at " << RTN_Address(rtnclntraw_create) << endl;
            OutFile.flush();
            AFUNPTR fptr       = (RTN_ReplaceProbed(rtnclntraw_create, AFUNPTR(myclntraw_create)));
            fptrclntraw_create = fptr;
        }

        RTN rtnsvc_run = RTN_FindByName(img, "svc_run");
        if (RTN_Valid(rtnsvc_run) && RTN_IsSafeForProbedReplacement(rtnsvc_run))
        {
            OutFile << CurrentTime() << "Inserting probe for svc_run at " << RTN_Address(rtnsvc_run) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnsvc_run, AFUNPTR(mysvc_run)));
            fptrsvc_run  = fptr;
        }

        RTN rtnsvc_sendreply = RTN_FindByName(img, "svc_sendreply");
        if (RTN_Valid(rtnsvc_sendreply) && RTN_IsSafeForProbedReplacement(rtnsvc_sendreply))
        {
            OutFile << CurrentTime() << "Inserting probe for svc_sendreply at " << RTN_Address(rtnsvc_sendreply) << endl;
            OutFile.flush();
            AFUNPTR fptr      = (RTN_ReplaceProbed(rtnsvc_sendreply, AFUNPTR(mysvc_sendreply)));
            fptrsvc_sendreply = fptr;
        }

        RTN rtnsvcraw_create = RTN_FindByName(img, "svcraw_create");
        if (RTN_Valid(rtnsvcraw_create) && RTN_IsSafeForProbedReplacement(rtnsvcraw_create))
        {
            OutFile << CurrentTime() << "Inserting probe for svcraw_create at " << RTN_Address(rtnsvcraw_create) << endl;
            OutFile.flush();
            AFUNPTR fptr      = (RTN_ReplaceProbed(rtnsvcraw_create, AFUNPTR(mysvcraw_create)));
            fptrsvcraw_create = fptr;
        }

        RTN rtnsvctcp_create = RTN_FindByName(img, "svctcp_create");
        if (RTN_Valid(rtnsvctcp_create) && RTN_IsSafeForProbedReplacement(rtnsvctcp_create))
        {
            OutFile << CurrentTime() << "Inserting probe for svctcp_create at " << RTN_Address(rtnsvctcp_create) << endl;
            OutFile.flush();
            AFUNPTR fptr      = (RTN_ReplaceProbed(rtnsvctcp_create, AFUNPTR(mysvctcp_create)));
            fptrsvctcp_create = fptr;
        }

        RTN rtnsvcudp_bufcreate = RTN_FindByName(img, "svcudp_bufcreate");
        if (RTN_Valid(rtnsvcudp_bufcreate) && RTN_IsSafeForProbedReplacement(rtnsvcudp_bufcreate))
        {
            OutFile << CurrentTime() << "Inserting probe for svcudp_bufcreate at " << RTN_Address(rtnsvcudp_bufcreate) << endl;
            OutFile.flush();
            AFUNPTR fptr         = (RTN_ReplaceProbed(rtnsvcudp_bufcreate, AFUNPTR(mysvcudp_bufcreate)));
            fptrsvcudp_bufcreate = fptr;
        }

        RTN rtnsvcudp_create = RTN_FindByName(img, "svcudp_create");
        if (RTN_Valid(rtnsvcudp_create) && RTN_IsSafeForProbedReplacement(rtnsvcudp_create))
        {
            OutFile << CurrentTime() << "Inserting probe for svcudp_create at " << RTN_Address(rtnsvcudp_create) << endl;
            OutFile.flush();
            AFUNPTR fptr      = (RTN_ReplaceProbed(rtnsvcudp_create, AFUNPTR(mysvcudp_create)));
            fptrsvcudp_create = fptr;
        }

        RTN rtnabort = RTN_FindByName(img, "abort");
        if (RTN_Valid(rtnabort) && RTN_IsSafeForProbedReplacement(rtnabort))
        {
            OutFile << CurrentTime() << "Inserting probe for abort at " << RTN_Address(rtnabort) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnabort, AFUNPTR(myabort)));
            fptrabort    = fptr;
        }

        RTN rtn_exit = RTN_FindByName(img, "_exit");
        if (RTN_Valid(rtn_exit) && RTN_IsSafeForProbedReplacement(rtn_exit))
        {
            OutFile << CurrentTime() << "Inserting probe for _exit at " << RTN_Address(rtn_exit) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtn_exit, AFUNPTR(my_exit)));
            fptr_exit    = fptr;
        }

        RTN rtnnanosleep = RTN_FindByName(img, "nanosleep");
        if (RTN_Valid(rtnnanosleep) && RTN_IsSafeForProbedReplacement(rtnnanosleep))
        {
            OutFile << CurrentTime() << "Inserting probe for nanosleep at " << RTN_Address(rtnnanosleep) << endl;
            OutFile.flush();
            AFUNPTR fptr  = (RTN_ReplaceProbed(rtnnanosleep, AFUNPTR(my_nanosleep)));
            fptrnanosleep = fptr;
        }

        RTN rtnsignal = RTN_FindByName(img, "signal");
        if (RTN_Valid(rtnsignal) && RTN_IsSafeForProbedReplacement(rtnsignal))
        {
            OutFile << CurrentTime() << "Inserting probe for signal at " << RTN_Address(rtnsignal) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnsignal, AFUNPTR(mysignal)));
            fptrsignal   = fptr;
        }

        RTN rtnsigprocmask = RTN_FindByName(img, "sigprocmask");
        if (RTN_Valid(rtnsigprocmask) && RTN_IsSafeForProbedReplacement(rtnsigprocmask))
        {
            OutFile << CurrentTime() << "Inserting probe for sigprocmask at " << RTN_Address(rtnsigprocmask) << endl;
            OutFile.flush();
            AFUNPTR fptr    = (RTN_ReplaceProbed(rtnsigprocmask, AFUNPTR(mysigprocmask)));
            fptrsigprocmask = fptr;
        }

        RTN rtnsigpending = RTN_FindByName(img, "sigpending");
        if (RTN_Valid(rtnsigpending) && RTN_IsSafeForProbedReplacement(rtnsigpending))
        {
            OutFile << CurrentTime() << "Inserting probe for sigpending at " << RTN_Address(rtnsigpending) << endl;
            OutFile.flush();
            AFUNPTR fptr   = (RTN_ReplaceProbed(rtnsigpending, AFUNPTR(mysigpending)));
            fptrsigpending = fptr;
        }

        RTN rtnsigaction = RTN_FindByName(img, "sigaction");
        if (RTN_Valid(rtnsigaction) && RTN_IsSafeForProbedReplacement(rtnsigaction))
        {
            OutFile << CurrentTime() << "Inserting probe for sigaction at " << RTN_Address(rtnsigaction) << endl;
            OutFile.flush();
            AFUNPTR fptr  = (RTN_ReplaceProbed(rtnsigaction, AFUNPTR(mysigaction)));
            fptrsigaction = fptr;
        }

        RTN rtnsetitimer = RTN_FindByName(img, "setitimer");
        if (RTN_Valid(rtnsetitimer) && RTN_IsSafeForProbedReplacement(rtnsetitimer))
        {
            OutFile << CurrentTime() << "Inserting probe for setitimer at " << RTN_Address(rtnsetitimer) << endl;
            OutFile.flush();
            AFUNPTR fptr  = (RTN_ReplaceProbed(rtnsetitimer, AFUNPTR(mysetitimer)));
            fptrsetitimer = fptr;
        }

        RTN rtnexit = RTN_FindByName(img, "exit");
        if (RTN_Valid(rtnexit) && RTN_IsSafeForProbedReplacement(rtnexit))
        {
            OutFile << CurrentTime() << "Inserting probe for exit at " << RTN_Address(rtnexit) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnexit, AFUNPTR(myexit)));
            fptrexit     = fptr;
        }

        RTN rtndl_iterate_phdr = RTN_FindByName(img, "dl_iterate_phdr");
        if (RTN_Valid(rtnexit) && RTN_IsSafeForProbedReplacement(rtndl_iterate_phdr))
        {
            OutFile << CurrentTime() << "Inserting probe for dl_iterate_phdr at " << RTN_Address(rtndl_iterate_phdr) << endl;
            OutFile.flush();
            AFUNPTR fptr        = (RTN_ReplaceProbed(rtndl_iterate_phdr, AFUNPTR(mydl_iterate_phdr)));
            fptrdl_iterate_phdr = fptr;
        }

        RTN rtnsystem = RTN_FindByName(img, "system");
        if (RTN_Valid(rtnsystem) && RTN_IsSafeForProbedReplacement(rtnsystem))
        {
            OutFile << CurrentTime() << "Inserting probe for system at " << RTN_Address(rtnsystem) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnsystem, AFUNPTR(mysystem)));
            fptrsystem   = fptr;
        }

        RTN rtnalarm = RTN_FindByName(img, "alarm");
        if (RTN_Valid(rtnalarm) && RTN_IsSafeForProbedReplacement(rtnalarm))
        {
            OutFile << CurrentTime() << "Inserting probe for alarm at " << RTN_Address(rtnalarm) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnalarm, AFUNPTR(myalarm)));
            fptralarm    = fptr;
        }

        RTN rtnrecvmsg = RTN_FindByName(img, "recvmsg");
        if (RTN_Valid(rtnrecvmsg) && RTN_IsSafeForProbedReplacement(rtnrecvmsg))
        {
            OutFile << CurrentTime() << "Inserting probe for recvmsg at " << RTN_Address(rtnrecvmsg) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnrecvmsg, AFUNPTR(myrecvmsg)));
            fptrrecvmsg  = fptr;
        }

        RTN rtnsendmsg = RTN_FindByName(img, "sendmsg");
        if (RTN_Valid(rtnsendmsg) && RTN_IsSafeForProbedReplacement(rtnsendmsg))
        {
            OutFile << CurrentTime() << "Inserting probe for sendmsg at " << RTN_Address(rtnsendmsg) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnsendmsg, AFUNPTR(mysendmsg)));
            fptrsendmsg  = fptr;
        }

        RTN rtnpause = RTN_FindByName(img, "pause");
        if (RTN_Valid(rtnpause) && RTN_IsSafeForProbedReplacement(rtnpause))
        {
            OutFile << CurrentTime() << "Inserting probe for pause at " << RTN_Address(rtnpause) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnpause, AFUNPTR(mypause)));
            fptrpause    = fptr;
        }

        RTN rtnsigtimedwait = RTN_FindByName(img, "sigtimedwait");
        if (RTN_Valid(rtnsigtimedwait) && RTN_IsSafeForProbedReplacement(rtnsigtimedwait))
        {
            OutFile << CurrentTime() << "Inserting probe for sigtimedwait at " << RTN_Address(rtnsigtimedwait) << endl;
            OutFile.flush();
            AFUNPTR fptr     = (RTN_ReplaceProbed(rtnsigtimedwait, AFUNPTR(mysigtimedwait)));
            fptrsigtimedwait = fptr;
        }

        RTN rtnsigwaitinfo = RTN_FindByName(img, "sigwaitinfo");
        if (RTN_Valid(rtnsigwaitinfo) && RTN_IsSafeForProbedReplacement(rtnsigwaitinfo))
        {
            OutFile << CurrentTime() << "Inserting probe for sigwaitinfo at " << RTN_Address(rtnsigwaitinfo) << endl;
            OutFile.flush();
            AFUNPTR fptr    = (RTN_ReplaceProbed(rtnsigwaitinfo, AFUNPTR(mysigwaitinfo)));
            fptrsigwaitinfo = fptr;
        }

        RTN rtnepoll_wait = RTN_FindByName(img, "epoll_wait");
        if (RTN_Valid(rtnepoll_wait) && RTN_IsSafeForProbedReplacement(rtnepoll_wait))
        {
            OutFile << CurrentTime() << "Inserting probe for epoll_wait at " << RTN_Address(rtnepoll_wait) << endl;
            OutFile.flush();
            AFUNPTR fptr   = (RTN_ReplaceProbed(rtnepoll_wait, AFUNPTR(myepoll_wait)));
            fptrepoll_wait = fptr;
        }

        RTN rtnppoll = RTN_FindByName(img, "ppoll");
        if (RTN_Valid(rtnppoll) && RTN_IsSafeForProbedReplacement(rtnppoll))
        {
            OutFile << CurrentTime() << "Inserting probe for ppoll at " << RTN_Address(rtnppoll) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnppoll, AFUNPTR(myppoll)));
            fptrppoll    = fptr;
        }

        RTN rtnmsgsnd = RTN_FindByName(img, "msgsnd");
        if (RTN_Valid(rtnmsgsnd) && RTN_IsSafeForProbedReplacement(rtnmsgsnd))
        {
            OutFile << CurrentTime() << "Inserting probe for msgsnd at " << RTN_Address(rtnmsgsnd) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnmsgsnd, AFUNPTR(mymsgsnd)));
            fptrmsgsnd   = fptr;
        }

        RTN rtnmsgrcv = RTN_FindByName(img, "msgrcv");
        if (RTN_Valid(rtnmsgrcv) && RTN_IsSafeForProbedReplacement(rtnmsgrcv))
        {
            OutFile << CurrentTime() << "Inserting probe for msgrcv at " << RTN_Address(rtnmsgrcv) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnmsgrcv, AFUNPTR(mymsgrcv)));
            fptrmsgrcv   = fptr;
        }

        RTN rtnsemop = RTN_FindByName(img, "semop");
        if (RTN_Valid(rtnsemop) && RTN_IsSafeForProbedReplacement(rtnsemop))
        {
            OutFile << CurrentTime() << "Inserting probe for semop at " << RTN_Address(rtnsemop) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnsemop, AFUNPTR(mymsgrcv)));
            fptrsemop    = fptr;
        }

        RTN rtnsemtimedop = RTN_FindByName(img, "semtimedop");
        if (RTN_Valid(rtnsemtimedop) && RTN_IsSafeForProbedReplacement(rtnsemtimedop))
        {
            OutFile << CurrentTime() << "Inserting probe for semtimedop at " << RTN_Address(rtnsemtimedop) << endl;
            OutFile.flush();
            AFUNPTR fptr   = RTN_ReplaceProbed(rtnsemtimedop, AFUNPTR(mysemtimedop));
            fptrsemtimedop = fptr;
        }

        RTN rtnusleep = RTN_FindByName(img, "usleep");
        if (RTN_Valid(rtnusleep) && RTN_IsSafeForProbedReplacement(rtnusleep))
        {
            OutFile << CurrentTime() << "Inserting probe for usleep at " << RTN_Address(rtnusleep) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnusleep, AFUNPTR(myusleep)));
            fptrusleep   = fptr;
        }

        RTN rtnualarm = RTN_FindByName(img, "ualarm");
        if (RTN_Valid(rtnualarm) && RTN_IsSafeForProbedReplacement(rtnualarm))
        {
            OutFile << CurrentTime() << "Inserting probe for ualarm at " << RTN_Address(rtnualarm) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnualarm, AFUNPTR(myualarm)));
            fptrualarm   = fptr;
        }

        RTN rtngetitimer = RTN_FindByName(img, "getitimer");
        if (RTN_Valid(rtngetitimer) && RTN_IsSafeForProbedReplacement(rtngetitimer))
        {
            OutFile << CurrentTime() << "Inserting probe for getitimer at " << RTN_Address(rtngetitimer) << endl;
            OutFile.flush();
            AFUNPTR fptr  = (RTN_ReplaceProbed(rtngetitimer, AFUNPTR(mygetitimer)));
            fptrgetitimer = fptr;
        }

        RTN rtnsigwait = RTN_FindByName(img, "sigwait");
        if (RTN_Valid(rtnsigwait) && RTN_IsSafeForProbedReplacement(rtnsigwait))
        {
            OutFile << CurrentTime() << "Inserting probe for sigwait at " << RTN_Address(rtnsigwait) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnsigwait, AFUNPTR(mysigwait)));
            fptrsigwait  = fptr;
        }

        RTN rtnmsgget = RTN_FindByName(img, "msgget");
        if (RTN_Valid(rtnmsgget) && RTN_IsSafeForProbedReplacement(rtnmsgget))
        {
            OutFile << CurrentTime() << "Inserting probe for msgget at " << RTN_Address(rtnmsgget) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnmsgget, AFUNPTR(mymsgget)));
            fptrmsgget   = fptr;
        }

        RTN rtnsemget = RTN_FindByName(img, "semget");
        if (RTN_Valid(rtnsemget) && RTN_IsSafeForProbedReplacement(rtnsemget))
        {
            OutFile << CurrentTime() << "Inserting probe for semget at " << RTN_Address(rtnsemget) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnsemget, AFUNPTR(mysemget)));
            fptrsemget   = fptr;
        }

        RTN rtnwait = RTN_FindByName(img, "wait");
        if (RTN_Valid(rtnwait) && RTN_IsSafeForProbedReplacement(rtnwait))
        {
            OutFile << CurrentTime() << "Inserting probe for wait at " << RTN_Address(rtnwait) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnwait, AFUNPTR(mywait)));
            fptrwait     = fptr;
        }

        RTN rtnwaitpid = RTN_FindByName(img, "waitpid");
        if (RTN_Valid(rtnwaitpid) && RTN_IsSafeForProbedReplacement(rtnwaitpid))
        {
            OutFile << CurrentTime() << "Inserting probe for waitpid at " << RTN_Address(rtnwaitpid) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnwaitpid, AFUNPTR(mywaitpid)));
            fptrwaitpid  = fptr;
        }

        RTN rtnwaitid = RTN_FindByName(img, "waitid");
        if (RTN_Valid(rtnwaitid) && RTN_IsSafeForProbedReplacement(rtnwaitid))
        {
            OutFile << CurrentTime() << "Inserting probe for waitid at " << RTN_Address(rtnwaitid) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnwaitid, AFUNPTR(mywaitid)));
            fptrwaitid   = fptr;
        }

        RTN rtnwait3 = RTN_FindByName(img, "wait3");
        if (RTN_Valid(rtnwait3) && RTN_IsSafeForProbedReplacement(rtnwait3))
        {
            OutFile << CurrentTime() << "Inserting probe for wait3 at " << RTN_Address(rtnwait3) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnwait3, AFUNPTR(mywait3)));
            fptrwait3    = fptr;
        }

        RTN rtnwait4 = RTN_FindByName(img, "wait4");
        if (RTN_Valid(rtnwait3) && RTN_IsSafeForProbedReplacement(rtnwait4))
        {
            OutFile << CurrentTime() << "Inserting probe for wait4 at " << RTN_Address(rtnwait4) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnwait3, AFUNPTR(mywait4)));
            fptrwait4    = fptr;
        }

        RTN rtnreadv = RTN_FindByName(img, "readv");
        if (RTN_Valid(rtnreadv) && RTN_IsSafeForProbedReplacement(rtnreadv))
        {
            OutFile << CurrentTime() << "Inserting probe for readv at " << RTN_Address(rtnreadv) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnreadv, AFUNPTR(myreadv)));
            fptrreadv    = fptr;
        }

        RTN rtnwritev = RTN_FindByName(img, "writev");
        if (RTN_Valid(rtnwritev) && RTN_IsSafeForProbedReplacement(rtnwritev))
        {
            OutFile << CurrentTime() << "Inserting probe for writev at " << RTN_Address(rtnwritev) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnwritev, AFUNPTR(mywritev)));
            fptrwritev   = fptr;
        }

        RTN rtnflockfile = RTN_FindByName(img, "flockfile");
        if (RTN_Valid(rtnflockfile) && RTN_IsSafeForProbedReplacement(rtnflockfile))
        {
            OutFile << CurrentTime() << "Inserting probe for flockfile at " << RTN_Address(rtnflockfile) << endl;
            OutFile.flush();
            AFUNPTR fptr  = (RTN_ReplaceProbed(rtnflockfile, AFUNPTR(myflockfile)));
            fptrflockfile = fptr;
        }
        RTN rtnfunlockfile = RTN_FindByName(img, "funlockfile");
        if (RTN_Valid(rtnfunlockfile) && RTN_IsSafeForProbedReplacement(rtnfunlockfile))
        {
            OutFile << CurrentTime() << "Inserting probe for funlockfile at " << RTN_Address(rtnfunlockfile) << endl;
            OutFile.flush();
            AFUNPTR fptr    = (RTN_ReplaceProbed(rtnfunlockfile, AFUNPTR(myfunlockfile)));
            fptrfunlockfile = fptr;
        }

        RTN rtnlockf = RTN_FindByName(img, "lockf");
        if (RTN_Valid(rtnlockf) && RTN_IsSafeForProbedReplacement(rtnlockf))
        {
            OutFile << CurrentTime() << "Inserting probe for lockf at " << RTN_Address(rtnlockf) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnlockf, AFUNPTR(mylockf)));
            fptrlockf    = fptr;
        }

        RTN rtnsetenv = RTN_FindByName(img, "setenv");
        if (RTN_Valid(rtnsetenv) && RTN_IsSafeForProbedReplacement(rtnsetenv))
        {
            OutFile << CurrentTime() << "Inserting probe for setenv at " << RTN_Address(rtnsetenv) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnsetenv, AFUNPTR(mysetenv)));
            fptrsetenv   = fptr;
        }

        RTN rtnunsetenv = RTN_FindByName(img, "unsetenv");
        if (RTN_Valid(rtnunsetenv) && RTN_IsSafeForProbedReplacement(rtnunsetenv))
        {
            OutFile << CurrentTime() << "Inserting probe for unsetenv at " << RTN_Address(rtnunsetenv) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnunsetenv, AFUNPTR(myunsetenv)));
            fptrunsetenv = fptr;
        }

        RTN rtngetenv = RTN_FindByName(img, "getenv");
        if (RTN_Valid(rtngetenv) && RTN_IsSafeForProbedReplacement(rtngetenv))
        {
            OutFile << CurrentTime() << "Inserting probe for getenv at " << RTN_Address(rtngetenv) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtngetenv, AFUNPTR(mygetenv)));
            fptrgetenv   = fptr;
        }

        RTN rtnperror = RTN_FindByName(img, "perror");
        if (RTN_Valid(rtnperror) && RTN_IsSafeForProbedReplacement(rtnperror))
        {
            OutFile << CurrentTime() << "Inserting probe for perror at " << RTN_Address(rtnperror) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnperror, AFUNPTR(myperror)));
            fptrperror   = fptr;
        }

        RTN rtnmmap = RTN_FindByName(img, "mmap");
        if (RTN_Valid(rtnmmap) && RTN_IsSafeForProbedReplacement(rtnmmap))
        {
            OutFile << CurrentTime() << "Inserting probe for mmap at " << RTN_Address(rtnmmap) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnmmap, AFUNPTR(mymmap)));
            fptrmmap     = fptr;
        }

        RTN rtnmunmap = RTN_FindByName(img, "munmap");
        if (RTN_Valid(rtnmunmap) && RTN_IsSafeForProbedReplacement(rtnmunmap))
        {
            OutFile << CurrentTime() << "Inserting probe for munmap at " << RTN_Address(rtnmunmap) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnmunmap, AFUNPTR(mymunmap)));
            fptrmunmap   = fptr;
        }

        RTN rtnfileno = RTN_FindByName(img, "fileno");
        if (RTN_Valid(rtnfileno) && RTN_IsSafeForProbedReplacement(rtnfileno))
        {
            OutFile << CurrentTime() << "Inserting probe for fileno at " << RTN_Address(rtnfileno) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnfileno, AFUNPTR(myfileno)));
            fptrfileno   = fptr;
        }

        RTN rtngetpid = RTN_FindByName(img, "getpid");
        if (RTN_Valid(rtngetpid) && RTN_IsSafeForProbedReplacement(rtngetpid))
        {
            OutFile << CurrentTime() << "Inserting probe for getpid at " << RTN_Address(rtngetpid) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtngetpid, AFUNPTR(mygetpid)));
            fptrgetpid   = fptr;
        }

        RTN rtngetppid = RTN_FindByName(img, "getppid");
        if (RTN_Valid(rtngetppid) && RTN_IsSafeForProbedReplacement(rtngetppid))
        {
            OutFile << CurrentTime() << "Inserting probe for getppid at " << RTN_Address(rtngetppid) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtngetppid, AFUNPTR(mygetppid)));
            fptrgetppid  = fptr;
        }

        RTN rtnmemset = RTN_FindByName(img, "memset");
        if (RTN_Valid(rtnmemset) && RTN_IsSafeForProbedReplacement(rtnmemset))
        {
            OutFile << CurrentTime() << "Inserting probe for memset at " << RTN_Address(rtnmemset) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnmemset, AFUNPTR(mymemset)));
            fptrmemset   = fptr;
        }

        RTN rtnmemcpy = RTN_FindByName(img, "memcpy");
        if (RTN_Valid(rtnmemcpy) && RTN_IsSafeForProbedReplacement(rtnmemcpy))
        {
            if (!is_GLIBC2_24_or_newer(img))
            {
                OutFile << CurrentTime() << "Inserting probe for memcpy at " << RTN_Address(rtnmemcpy) << endl;
                OutFile.flush();
                AFUNPTR fptr = (RTN_ReplaceProbed(rtnmemcpy, AFUNPTR(mymemcpy)));
                fptrmemcpy   = fptr;
            }
        }

        RTN rtnaccess = RTN_FindByName(img, "access");
        if (RTN_Valid(rtnaccess) && RTN_IsSafeForProbedReplacement(rtnaccess))
        {
            OutFile << CurrentTime() << "Inserting probe for access at " << RTN_Address(rtnaccess) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnaccess, AFUNPTR(myaccess)));
            fptraccess   = fptr;
        }

        RTN rtnlseek = RTN_FindByName(img, "lseek");
        if (RTN_Valid(rtnlseek) && RTN_IsSafeForProbedReplacement(rtnlseek))
        {
            OutFile << CurrentTime() << "Inserting probe for lseek at " << RTN_Address(rtnlseek) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnlseek, AFUNPTR(mylseek)));
            fptrlseek    = fptr;
        }

        RTN rtnlseek64 = RTN_FindByName(img, "lseek64");
        if (RTN_Valid(rtnlseek64) && RTN_IsSafeForProbedReplacement(rtnlseek64))
        {
            OutFile << CurrentTime() << "Inserting probe for lseek64 at " << RTN_Address(rtnlseek64) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnlseek64, AFUNPTR(mylseek64)));
            fptrlseek64  = fptr;
        }

        RTN rtnfdatasync = RTN_FindByName(img, "fdatasync");
        if (RTN_Valid(rtnfdatasync) && RTN_IsSafeForProbedReplacement(rtnfdatasync))
        {
            OutFile << CurrentTime() << "Inserting probe for fdatasync at " << RTN_Address(rtnfdatasync) << endl;
            OutFile.flush();
            AFUNPTR fptr  = (RTN_ReplaceProbed(rtnfdatasync, AFUNPTR(myfdatasync)));
            fptrfdatasync = fptr;
        }

        RTN rtnunlink = RTN_FindByName(img, "unlink");
        if (RTN_Valid(rtnunlink) && RTN_IsSafeForProbedReplacement(rtnunlink))
        {
            OutFile << CurrentTime() << "Inserting probe for unlink at " << RTN_Address(rtnunlink) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnunlink, AFUNPTR(myunlink)));
            fptrunlink   = fptr;
        }

        RTN rtnstrlen = RTN_FindByName(img, "strlen");
        if (RTN_Valid(rtnstrlen) && RTN_IsSafeForProbedReplacement(rtnstrlen))
        {
            OutFile << CurrentTime() << "Inserting probe for strlen at " << RTN_Address(rtnstrlen) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnstrlen, AFUNPTR(mystrlen)));
            fptrstrlen   = fptr;
        }

        RTN rtnwcslen = RTN_FindByName(img, "wcslen");
        if (RTN_Valid(rtnwcslen) && RTN_IsSafeForProbedReplacement(rtnwcslen))
        {
            OutFile << CurrentTime() << "Inserting probe for wcslen at " << RTN_Address(rtnwcslen) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnwcslen, AFUNPTR(mywcslen)));
            fptrwcslen   = fptr;
        }

        RTN rtnstrcpy = RTN_FindByName(img, "strcpy");
        if (RTN_Valid(rtnstrcpy) && RTN_IsSafeForProbedReplacement(rtnstrcpy))
        {
            OutFile << CurrentTime() << "Inserting probe for strcpy at " << RTN_Address(rtnstrcpy) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnstrcpy, AFUNPTR(mystrcpy)));
            fptrstrcpy   = fptr;
        }

        RTN rtnstrncpy = RTN_FindByName(img, "strncpy");
        if (RTN_Valid(rtnstrncpy) && RTN_IsSafeForProbedReplacement(rtnstrncpy))
        {
            OutFile << CurrentTime() << "Inserting probe for strncpy at " << RTN_Address(rtnstrncpy) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnstrncpy, AFUNPTR(mystrcpy)));
            fptrstrncpy  = fptr;
        }

        RTN rtnstrcat = RTN_FindByName(img, "strcat");
        if (RTN_Valid(rtnstrcat) && RTN_IsSafeForProbedReplacement(rtnstrcat))
        {
            OutFile << CurrentTime() << "Inserting probe for strcat at " << RTN_Address(rtnstrcat) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnstrcat, AFUNPTR(mystrcat)));
            fptrstrcat   = fptr;
        }

        RTN rtnstrstr = RTN_FindByName(img, "strstr");
        if (RTN_Valid(rtnstrstr) && RTN_IsSafeForProbedReplacement(rtnstrstr))
        {
            OutFile << CurrentTime() << "Inserting probe for strstr at " << RTN_Address(rtnstrstr) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnstrstr, AFUNPTR(mystrstr)));
            fptrstrstr   = fptr;
        }

        RTN rtnstrchr0 = RTN_FindByName(img, "strchr0");
        if (RTN_Valid(rtnstrchr0) && RTN_IsSafeForProbedReplacement(rtnstrchr0))
        {
            OutFile << CurrentTime() << "Inserting probe for strchr0 at " << RTN_Address(rtnstrchr0) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnstrchr0, AFUNPTR(mystrchr0)));
            fptrstrrchr  = fptr;
        }

        RTN rtnstrrchr = RTN_FindByName(img, "strrchr");
        if (RTN_Valid(rtnstrrchr) && RTN_IsSafeForProbedReplacement(rtnstrrchr))
        {
            OutFile << CurrentTime() << "Inserting probe for strrchr at " << RTN_Address(rtnstrrchr) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnstrrchr, AFUNPTR(mystrrchr)));
            fptrstrrchr  = fptr;
        }

        RTN rtnstrcmp = RTN_FindByName(img, "strcmp");
        if (RTN_Valid(rtnstrcmp) && RTN_IsSafeForProbedReplacement(rtnstrcmp))
        {
            OutFile << CurrentTime() << "Inserting probe for strcmp at " << RTN_Address(rtnstrcmp) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnstrcmp, AFUNPTR(mystrcmp)));
            fptrstrcmp   = fptr;
        }

        RTN rtnstrncmp = RTN_FindByName(img, "strncmp");
        if (RTN_Valid(rtnstrncmp) && RTN_IsSafeForProbedReplacement(rtnstrncmp))
        {
            OutFile << CurrentTime() << "Inserting probe for strncmp at " << RTN_Address(rtnstrncmp) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnstrncmp, AFUNPTR(mystrncmp)));
            fptrstrncmp  = fptr;
        }

        RTN rtnsigaddset = RTN_FindByName(img, "sigaddset");
        if (RTN_Valid(rtnsigaddset) && RTN_IsSafeForProbedReplacement(rtnsigaddset))
        {
            OutFile << CurrentTime() << "Inserting probe for sigaddset at " << RTN_Address(rtnsigaddset) << endl;
            OutFile.flush();
            AFUNPTR fptr  = (RTN_ReplaceProbed(rtnsigaddset, AFUNPTR(mysigaddset)));
            fptrsigaddset = fptr;
        }

        RTN rtnsigdelset = RTN_FindByName(img, "sigdelset");
        if (RTN_Valid(rtnsigdelset) && RTN_IsSafeForProbedReplacement(rtnsigdelset))
        {
            OutFile << CurrentTime() << "Inserting probe for sigdelset at " << RTN_Address(rtnsigdelset) << endl;
            OutFile.flush();
            AFUNPTR fptr  = (RTN_ReplaceProbed(rtnsigdelset, AFUNPTR(mysigdelset)));
            fptrsigdelset = fptr;
        }

        RTN rtnsigismember = RTN_FindByName(img, "sigismember");
        if (RTN_Valid(rtnsigismember) && RTN_IsSafeForProbedReplacement(rtnsigismember))
        {
            OutFile << CurrentTime() << "Inserting probe for sigismember at " << RTN_Address(rtnsigismember) << endl;
            OutFile.flush();
            AFUNPTR fptr  = (RTN_ReplaceProbed(rtnsigismember, AFUNPTR(mysigismember)));
            fptrsigdelset = fptr;
        }

        RTN rtnstrerror = RTN_FindByName(img, "strerror");
        if (RTN_Valid(rtnstrerror) && RTN_IsSafeForProbedReplacement(rtnstrerror))
        {
            OutFile << CurrentTime() << "Inserting probe for strerror at " << RTN_Address(rtnstrerror) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnstrerror, AFUNPTR(mystrerror)));
            fptrstrerror = fptr;
        }

        RTN rtnbind = RTN_FindByName(img, "bind");
        if (RTN_Valid(rtnbind) && RTN_IsSafeForProbedReplacement(rtnbind))
        {
            OutFile << CurrentTime() << "Inserting probe for bind at " << RTN_Address(rtnbind) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnbind, AFUNPTR(mybind)));
            fptrbind     = fptr;
        }

        RTN rtnlisten = RTN_FindByName(img, "listen");
        if (RTN_Valid(rtnlisten) && RTN_IsSafeForProbedReplacement(rtnlisten))
        {
            OutFile << CurrentTime() << "Inserting probe for listen at " << RTN_Address(rtnlisten) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnlisten, AFUNPTR(mylisten)));
            fptrlisten   = fptr;
        }

        RTN rtnuname = RTN_FindByName(img, "uname");
        if (RTN_Valid(rtnuname) && RTN_IsSafeForProbedReplacement(rtnuname))
        {
            OutFile << CurrentTime() << "Inserting probe for uname at " << RTN_Address(rtnuname) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnuname, AFUNPTR(myuname)));
            fptruname    = fptr;
        }

        RTN rtngethostname = RTN_FindByName(img, "gethostname");
        if (RTN_Valid(rtngethostname) && RTN_IsSafeForProbedReplacement(rtngethostname))
        {
            OutFile << CurrentTime() << "Inserting probe for gethostname at " << RTN_Address(rtngethostname) << endl;
            OutFile.flush();
            AFUNPTR fptr    = (RTN_ReplaceProbed(rtngethostname, AFUNPTR(mygethostname)));
            fptrgethostname = fptr;
        }

        RTN rtnkill = RTN_FindByName(img, "kill");
        if (RTN_Valid(rtnkill) && RTN_IsSafeForProbedReplacement(rtnkill))
        {
            OutFile << CurrentTime() << "Inserting probe for kill at " << RTN_Address(rtnkill) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnkill, AFUNPTR(mykill)));
            fptrkill     = fptr;
        }

        RTN rtnsched_yield = RTN_FindByName(img, "sched_yield");
        if (RTN_Valid(rtnsched_yield) && RTN_IsSafeForProbedReplacement(rtnsched_yield))
        {
            OutFile << CurrentTime() << "Inserting probe for sched_yield at " << RTN_Address(rtnsched_yield) << endl;
            OutFile.flush();
            AFUNPTR fptr    = (RTN_ReplaceProbed(rtnsched_yield, AFUNPTR(mysched_yield)));
            fptrsched_yield = fptr;
        }

        RTN rtntimer_settime = RTN_FindByName(img, "timer_settime");
        if (RTN_Valid(rtntimer_settime) && RTN_IsSafeForProbedReplacement(rtntimer_settime))
        {
            OutFile << CurrentTime() << "Inserting probe for timer_settime at " << RTN_Address(rtntimer_settime) << endl;
            OutFile.flush();
            AFUNPTR fptr      = (RTN_ReplaceProbed(rtntimer_settime, AFUNPTR(mytimer_settime)));
            fptrtimer_settime = fptr;
        }

        RTN rtnsigaltstack = RTN_FindByName(img, "sigaltstack");
        if (RTN_Valid(rtnsigaltstack) && RTN_IsSafeForProbedReplacement(rtnsigaltstack))
        {
            OutFile << CurrentTime() << "Inserting probe for sigaltstack at " << RTN_Address(rtnsigaltstack) << endl;
            OutFile.flush();
            AFUNPTR fptr    = (RTN_ReplaceProbed(rtnsigaltstack, AFUNPTR(mysigaltstack)));
            fptrsigaltstack = fptr;
        }

        RTN rtnioctl = RTN_FindByName(img, "ioctl");
        if (RTN_Valid(rtnioctl) && RTN_IsSafeForProbedReplacement(rtnioctl))
        {
            OutFile << CurrentTime() << "Inserting probe for ioctl at " << RTN_Address(rtnioctl) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnioctl, AFUNPTR(myioctl)));
            fptrioctl    = fptr;
        }

        RTN rtnflock = RTN_FindByName(img, "flock");
        if (RTN_Valid(rtnflock) && RTN_IsSafeForProbedReplacement(rtnflock))
        {
            OutFile << CurrentTime() << "Inserting probe for flock at " << RTN_Address(rtnflock) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnflock, AFUNPTR(myflock)));
            fptrflock    = fptr;
        }

        RTN rtn__libc_dlopen_mode = RTN_FindByName(img, "__libc_dlopen_mode");
        if (RTN_Valid(rtn__libc_dlopen_mode) && RTN_IsSafeForProbedReplacement(rtn__libc_dlopen_mode))
        {
            OutFile << CurrentTime() << "Inserting probe for __libc_dlopen_mode at " << RTN_Address(rtn__libc_dlopen_mode)
                    << endl;
            OutFile.flush();
            AFUNPTR fptr           = (RTN_ReplaceProbed(rtn__libc_dlopen_mode, AFUNPTR(my__libc_dlopen_mode)));
            fptr__libc_dlopen_mode = fptr;
        }

        RTN rtn__errno_location = RTN_FindByName(img, "__errno_location");
        if (RTN_Valid(rtn__errno_location) && RTN_IsSafeForProbedReplacement(rtn__errno_location))
        {
            OutFile << CurrentTime() << "Inserting probe for __errno_location at " << RTN_Address(rtn__errno_location) << endl;
            OutFile.flush();
            AFUNPTR fptr         = (RTN_ReplaceProbed(rtn__errno_location, AFUNPTR(my__errno_location)));
            fptr__errno_location = fptr;
        }

        RTN rtnsyscall = RTN_FindByName(img, "syscall");
        if (RTN_Valid(rtnsyscall) && RTN_IsSafeForProbedReplacement(rtnsyscall))
        {
            OutFile << CurrentTime() << "Inserting probe for syscall at " << RTN_Address(rtnsyscall) << endl;
            OutFile.flush();
            AFUNPTR fptr = (RTN_ReplaceProbed(rtnsyscall, AFUNPTR(mysyscall)));
            fptrsyscall  = fptr;
        }
    }
    // finished instrumentation
}

/* ===================================================================== */
/* Main function                                                         */
/* ===================================================================== */

int main(int argc, char* argv[])
{
    // Initialize Pin
    PIN_InitSymbols();
    if (PIN_Init(argc, argv))
    {
        return Usage();
    }

    OutFile.open(KnobOutputFile.Value().c_str());
    OutFile << hex;
    OutFile.setf(ios::showbase);
    OutFile << CurrentTime() << "started!" << endl;
    OutFile.flush();

    // Register the instrumentation callback
    IMG_AddInstrumentFunction(ImgLoad, 0);

    // Start the application
    PIN_StartProgramProbed(); // never returns

    return 0;
}
