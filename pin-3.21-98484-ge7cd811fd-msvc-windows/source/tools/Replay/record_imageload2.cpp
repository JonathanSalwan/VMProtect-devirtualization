/*
 * Copyright (C) 2010-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

//
// This tool records or replays image load events and associated image data.
// The test requires using it twice.
//
// Note that on replay it doesn't execute the loaded code, just re-generates the
// image load trace.
//

#include <cstdio>
#include <cstdlib>
#include <vector>
#include <cstring>
#include "pin.H"
using std::string;
using std::vector;

KNOB< BOOL > KnobReplay(KNOB_MODE_WRITEONCE, "pintool", "r", "0", "replay if 1, default is to log");
KNOB< BOOL > KnobVerbose(KNOB_MODE_WRITEONCE, "pintool", "v", "0", "print more verbose messages");
KNOB< string > KnobFilesPrefix(KNOB_MODE_WRITEONCE, "pintool", "p", "", "prefix to add to all generated filenames");

// This is questionable, but should be fine. Doing something smarter does not seem warranted
// for this simple test.
// The problems with MAX_PATH (or Windows PATH_MAX) are described here
// http://stackoverflow.com/questions/833291/is-there-an-equivalent-to-winapis-max-path-under-linux-unix
// for instance.
//
#define MAX_FILENAME_LENGTH 4096

static FILE* trace;
static FILE* imgLog;
static int imageCount                = 0;
static BOOL logging                  = FALSE;
static const char* END_RTN_LIST      = "END_RTN_LIST";
static const char* FAKE_IMAGE_SUFFIX = ".fake";

// This struct describe an RTN which was recorded
typedef struct
{
    string name;
    ADDRINT startAddr;
} RTN_INFO;

// Count the number of RTN objects inside an IMG
static int CountImageRtns(IMG img)
{
    int numRtns = 0;

    for (SEC sec = IMG_SecHead(img); SEC_Valid(sec); sec = SEC_Next(sec))
    {
        if (SEC_Type(sec) != SEC_TYPE_EXEC)
        {
            continue;
        }
        for (RTN rtn = SEC_RtnHead(sec); RTN_Valid(rtn); rtn = RTN_Next(rtn))
        {
            if (RTN_IsArtificial(rtn))
            {
                continue;
            }
            numRtns++;
        }
    }

    return numRtns;
}

// Find and return the memory region of an image's executable section
static ADDRESS_RANGE FindImageTextMargin(IMG img)
{
    ADDRESS_RANGE res;
#ifndef TARGET_MAC
    ADDRINT low  = ~0L;
    ADDRINT high = 0L;
    for (SEC sec = IMG_SecHead(img); SEC_Valid(sec); sec = SEC_Next(sec))
    {
        if (SEC_Type(sec) != SEC_TYPE_EXEC)
        {
            continue;
        }

        low  = MIN(low, SEC_Address(sec));
        high = MAX(high, SEC_Address(sec) + SEC_Size(sec) - 1);
    }
    res._low  = low;
    res._high = high;
#else
    ADDRESS_RANGE range;
    res._low  = IMG_LowAddress(img);
    res._high = IMG_HighAddress(img);
#endif
    return res;
}

/*
 * In replay test, we create the images in PIN using different name
 * (with the .fake suffix).
 * This is done in order to ensure than PIN doesn't find the binary
 * file for the replayed image.
 * The different image name only exists inside PIN as this test takes
 * care of translating the image name back to the original, log time,
 * name when writing image names to the test's log files.
 * This routine translates image name from log time to image name in
 * replay time.
 */
static string LogImageNameToReplayImageName(const string& logImage) { return logImage + FAKE_IMAGE_SUFFIX; }

/*
 * In replay test, we create the images in PIN using different name
 * (See LogImageNameToReplayImageName).
 * This routine translates image name from log time to image name in
 * replay time.
 */
static string ReplayImageNameToLogImageName(const string& replayImage)
{
    size_t suf_len = strlen(FAKE_IMAGE_SUFFIX);
    ASSERT(replayImage.length() >= suf_len, "Replay image name is too short: " + replayImage);
    ASSERT(0 == replayImage.compare(replayImage.length() - suf_len, suf_len, FAKE_IMAGE_SUFFIX),
           "Replay image name doesn't end with " + FAKE_IMAGE_SUFFIX + ": " + replayImage);
    return replayImage.substr(0, replayImage.length() - suf_len);
}

// Writes the image load event to the file "imgLog"
static void LogImageLoad(IMG img, void* v)
{
    // Ensure that we can't overflow when we read it back.
    ASSERTX(IMG_Name(img).length() < MAX_FILENAME_LENGTH);

    ADDRESS_RANGE range = FindImageTextMargin(img);

    // Log the data needed to restore it
    fprintf(imgLog, "L '%s' %llx %lx %llx %d \n", IMG_Name(img).c_str(), (unsigned long long)range._low,
            (long)(range._high - range._low + 1), (unsigned long long)IMG_LoadOffset(img), (int)IMG_IsMainExecutable(img));

    for (SEC sec = IMG_SecHead(img); SEC_Valid(sec); sec = SEC_Next(sec))
    {
        if (SEC_Type(sec) != SEC_TYPE_EXEC)
        {
            continue;
        }
        for (RTN rtn = SEC_RtnHead(sec); RTN_Valid(rtn); rtn = RTN_Next(rtn))
        {
            if (RTN_IsArtificial(rtn))
            {
                continue;
            }
            fprintf(imgLog, "\t'%s' %llx\n", RTN_Name(rtn).c_str(), (unsigned long long)RTN_Address(rtn));
        }
    }
    fprintf(imgLog, "%s\n", END_RTN_LIST);
}

// Writes the image unload event to the file "imgLog"
static void LogImageUnload(IMG img, void*)
{
    ASSERTX(IMG_Name(img).length() < MAX_FILENAME_LENGTH);

    // Log the unload event.
    fprintf(imgLog, "U '%s'\n", IMG_Name(img).c_str());
}

// Parse the image load operation from "imgLog"
static void ParseImageLoadLine(string& imageName, ADDRINT* startAddr, USIZE* size, ADDRINT* offset, BOOL* mainExe,
                               vector< RTN_INFO >* rtns)
{
    char imgNameBuffer[MAX_FILENAME_LENGTH];

    long sz;
    int exe;
    unsigned long long addrBuf;
    unsigned long long offsetBuf;
    int itemsRead = fscanf(imgLog, " '%[^']' %llx %lx %llx %d \n", &imgNameBuffer[0], &addrBuf, &sz, &offsetBuf, &exe);
    if (itemsRead != 5)
    {
        fprintf(trace, "ParseImageLoadLine: Failed to parse; parsed %d expected to parse 5\n", itemsRead);
        exit(1);
    }
    *startAddr = (ADDRINT)addrBuf;
    *offset    = (ADDRINT)offsetBuf;
    imageName  = imgNameBuffer;
    imageName  = LogImageNameToReplayImageName(imageName);
    *size      = (USIZE)sz;
    *mainExe   = (BOOL)exe;
    char line[4096];
    bool endOfList = false;

    while (!endOfList && (NULL != fgets(line, sizeof(line), imgLog)))
    {
        RTN_INFO rtnInfo;
        // On PIN CRT (and Unix CRTs) on Windows, fgets returns the '\r' character. Remove it.
        char* eol = strchr(line, '\r');
        if (NULL != eol) *eol = 0;
        // Remove the new line character.
        eol = strchr(line, '\n');
        if (NULL != eol) *eol = 0;
        endOfList = (0 == strcmp(line, END_RTN_LIST));
        if (!endOfList)
        {
            char* name    = (char*)malloc(strlen(line) + 1);
            bool parse_ok = false;
            do
            {
                const char* delim_1 = strchr(line, '\'');
                const char* delim_2 = strrchr(line, '\'');
                if ((delim_1 == NULL) || (delim_1 == NULL)) break;
                if (sscanf(delim_2, "' %llx\n", &addrBuf) != 1) break;
                name[0] = '\0';
                strncat(name, delim_1 + 1, delim_2 - delim_1 - 1);
                parse_ok = true;
            }
            while (0);
            if (!parse_ok)
            {
                fprintf(trace, "'%s'\n", line);
                fprintf(trace, "ParseImageLoadLine (rtn names): Failed to parse line;\n");
                exit(1);
            }

            rtnInfo.startAddr = (ADDRINT)addrBuf;
            rtnInfo.name      = name;
            free(name);
            rtns->push_back(rtnInfo);
        }
    }
}

// Parse the image unload operation from "imgLog"
static void ParseImageUnloadLine(string& imageName)
{
    char imgNameBuffer[MAX_FILENAME_LENGTH];

    int itemsRead = fscanf(imgLog, " '%[^']'\n", &imgNameBuffer[0]);
    if (itemsRead != 1)
    {
        fprintf(trace, "ParseImageUnloadLine: Failed to parse\n");
        exit(1);
    }
    imageName = imgNameBuffer;
    imageName = LogImageNameToReplayImageName(imageName);
}

static IMG FindNamedImg(const string& imgName)
{
    // Visit every loaded image
    for (IMG img = APP_ImgTail(); IMG_Valid(img); img = IMG_Prev(img))
    {
        if (IMG_Name(img) == imgName) return img;
    }
    ASSERT(false, "No valid image found for " + imgName);
    return IMG_Invalid();
}

void checkSourceLocationOnFakeImg(const IMG& img)
{
    PIN_LockClient();
    for (SEC sec = IMG_SecHead(img); SEC_Valid(sec); sec = SEC_Next(sec))
    {
        if (SEC_Type(sec) != SEC_TYPE_EXEC)
        {
            continue;
        }
        for (RTN rtn = SEC_RtnHead(sec); RTN_Valid(rtn); rtn = RTN_Next(rtn))
        {
            string filename;
            INT32 line, col;
            PIN_GetSourceLocation(RTN_Address(rtn), &col, &line, &filename);
        }
    }
    PIN_UnlockClient();
}

// Replay the image log.
// We run this before the each instruction of the code as an analysis routine.
// So we eat up the image loads one instruction at a time!
// We can also call it before PIN_StartProgram, to check that queuing
// the replay calls up works.
//
static void ReplayImageEntry()
{
    if (feof(imgLog)) return;

    char tag = fgetc(imgLog);
    switch (tag)
    {
        case 'L':
        {
            string imageName;
            ADDRINT startAddr;
            ADDRINT offset;
            USIZE size;
            BOOL mainExe;
            vector< RTN_INFO > rtns;

            ParseImageLoadLine(imageName, &startAddr, &size, &offset, &mainExe, &rtns);
            if (KnobVerbose)
                fprintf(stderr, "Replaying load for %s, address: %llx offset: %llx, size: %lx\n", imageName.c_str(),
                        (unsigned long long)startAddr, (unsigned long long)offset, (long)size);

            // Create a temporary IMG object
            IMG img = IMG_CreateAt(imageName.c_str(), startAddr, size, offset, mainExe);
            ASSERT(IMG_Valid(img), "IMG_CreateAt for " + imageName + " is invalid");

            // Populate the IMG object with recorded RTNs
            PIN_LockClient();

            for (vector< RTN_INFO >::iterator it = rtns.begin(); it < rtns.end(); it++)
            {
                RTN rtn = RTN_CreateAt(it->startAddr, it->name);
                ASSERT(RTN_Valid(rtn), "Failed to create RTN " + it->name + " at address " + hexstr(it->startAddr));
            }

            // And, finally, inform Pin that it is all there, which will invoke
            // image load callbacks.
            IMG_ReplayImageLoad(img);

            PIN_UnlockClient();
            break;
        }
        case 'U':
        {
            string imageName;
            ParseImageUnloadLine(imageName);

            IMG img = FindNamedImg(imageName);
            if (KnobVerbose) fprintf(stderr, "Replaying unload for %s\n", imageName.c_str());
            // And, finally, inform Pin that it has gone, which will invoke
            // image unload callbacks.
            PIN_LockClient();
            IMG_ReplayImageUnload(img);
            PIN_UnlockClient();
            break;
        }
        case EOF:
            return;
        default:
            fprintf(trace, "Unexpected line in log file starting with '%c'\n", tag);
            exit(1);
    }
}

static VOID InstrumentInstruction(INS ins, void*) { INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)ReplayImageEntry, IARG_END); }

// Logging to demonstrate that the rest of the tool does replay the image load
// and unload operations, and that the Pin data structures are valid.

// Print the list of images currently loaded, with some information about each.
static VOID PrintImageList()
{
    for (IMG img = APP_ImgHead(); IMG_Valid(img); img = IMG_Next(img))
    {
        string imageName = IMG_Name(img);
        if (KnobReplay.Value())
        {
            imageName = ReplayImageNameToLogImageName(imageName);
        }
        ADDRESS_RANGE range = FindImageTextMargin(img);

        fprintf(trace, "   L  %-40s [0x%llx:0x%llx] offset 0x%llx %4d RTNs\n", imageName.c_str(), (unsigned long long)range._low,
                (unsigned long long)range._high, (unsigned long long)IMG_LoadOffset(img), CountImageRtns(img));
    }
}

// Prints all RTNs in a given IMG to the trace file.
// We use that file later on to see if the record and replay went the same
static VOID PrintRTNs(IMG img)
{
    for (SEC sec = IMG_SecHead(img); SEC_Valid(sec); sec = SEC_Next(sec))
    {
        if (SEC_Type(sec) != SEC_TYPE_EXEC)
        {
            continue;
        }
        for (RTN rtn = SEC_RtnHead(sec); RTN_Valid(rtn); rtn = RTN_Next(rtn))
        {
            if (RTN_IsArtificial(rtn))
            {
                continue;
            }

            fprintf(trace, "Function '%s' loaded at %llx\n", RTN_Name(rtn).c_str(), (unsigned long long)RTN_Address(rtn));

            // check that both RTN_InsHeadOnly and RTN_InsHead correctly look at the
            // fetched region and either both fetch valid instruction or both fetch
            // invalid instructions

            if (!IMG_IsMainExecutable(img)) continue;

            RTN_Open(rtn);
            INS headonly        = RTN_InsHeadOnly(rtn);
            BOOL valid_headonly = INS_Valid(headonly);
            RTN_Close(rtn);

            RTN_Open(rtn);
            INS head        = RTN_InsHead(rtn);
            BOOL valid_head = INS_Valid(head);
            RTN_Close(rtn);

            if (valid_headonly != valid_head)
            {
                fprintf(trace, "Failed to fetch RTN instruction for %s\n", RTN_Name(rtn).c_str());
            }
        }
    }
}

// Trace an image load event
static VOID TraceImageLoad(IMG img, VOID* v)
{
    string imageName = IMG_Name(img);
    if (KnobReplay.Value())
    {
        imageName = ReplayImageNameToLogImageName(imageName);
    }
    fprintf(trace, "[%2d]+ %-40s\n", imageCount++, imageName.c_str());
    PrintRTNs(img);
    PrintImageList();
}

// Trace an image unload event
static VOID TraceImageUnload(IMG img, VOID* v)
{
    string imageName = IMG_Name(img);
    if (KnobReplay.Value())
    {
        imageName = ReplayImageNameToLogImageName(imageName);
    }
    fprintf(trace, "[%2d]- %-40s\n", imageCount--, imageName.c_str());
    PrintImageList();
    if (KnobReplay.Value())
    {
        checkSourceLocationOnFakeImg(img);
    }
}

// This function is called when the application exits
static VOID Fini(INT32 code, VOID* v)
{
    fclose(trace);
    fclose(imgLog);
}

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

static INT32 Usage()
{
    PIN_ERROR("This tool prints a log of image load and unload events, logs them and can replay them\n" +
              KNOB_BASE::StringKnobSummary() + "\n");
    return -1;
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */

int main(int argc, char* argv[])
{
    // Initialize symbol processing
    PIN_InitSymbols();

    // Initialize pin
    if (PIN_Init(argc, argv)) return Usage();

    logging        = !KnobReplay.Value();
    string logFile = KnobFilesPrefix.Value() + ".record";

    if (logging)
    {
        string traceFile = KnobFilesPrefix.Value() + "_rec.out";
        trace            = fopen(traceFile.c_str(), "w");
        ASSERTX(NULL != trace);
        imgLog = fopen(logFile.c_str(), "w");
        ASSERTX(NULL != imgLog);

        IMG_AddInstrumentFunction(LogImageLoad, 0);
        IMG_AddUnloadFunction(LogImageUnload, 0);
    }
    else
    { // Replaying
        string traceFile = KnobFilesPrefix.Value() + "_play.out";
        trace            = fopen(traceFile.c_str(), "w");
        ASSERTX(NULL != trace);
        imgLog = fopen(logFile.c_str(), "r");
        ASSERTX(NULL != imgLog);

        // We will handle image load operations.
        PIN_SetReplayMode(REPLAY_MODE_IMAGEOPS);
        // And then we replay the first two image load ops before we start the program.
        // We do this even before adding the image instrumentation callback, that should still work,
        // Pin should defer these and replay them inside PIN_StartProgram.
        ReplayImageEntry();
        ReplayImageEntry();

        INS_AddInstrumentFunction(InstrumentInstruction, 0);
    }

    // These Trace functions demonstrate that the events are happening, they are the client...
    IMG_AddInstrumentFunction(TraceImageLoad, 0);
    IMG_AddUnloadFunction(TraceImageUnload, 0);

    // Register Fini to be called when the application exits
    PIN_AddFiniFunction(Fini, 0);

    // Start the program, never returns
    PIN_StartProgram();

    return 0;
}
