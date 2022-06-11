/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/* This verifies that the interceptsegv_for_oaddr_cache.test ran as expected by analyzing
   the oaddr cache info activity in the pin.log file produced by the test
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <assert.h>
#include <setjmp.h>
#include <sys/types.h>

#define MAX_STR 4096
char line[MAX_STR];
int numInvalidations          = 0;
int numFinds                  = 0;
int numFindsAfterInvalidation = 0;

void VerifyAndExit()
{
    if (numInvalidations > 0 && numFinds > 0 && numFindsAfterInvalidation > 0)
    {
        exit(0);
    }
    else
    {
        if (numInvalidations <= 0)
        {
            printf("** error expected to have some invalidations\n");
        }
        if (numFinds <= 0)
        {
            printf("** error expected to have some finds\n");
        }
        if (numFindsAfterInvalidation <= 0)
        {
            printf("** error expected to have some finds after invalidation\n");
        }
        exit(-1);
    }
}

char* AdvanceToBlank(char* ptr)
{
    while (*ptr != ' ')
    {
        ptr++;
    }
    return (ptr);
}

char* AdvanceToNonBlank(char* ptr)
{
    while (*ptr == ' ')
    {
        ptr++;
    }
    return (ptr);
}

int readln(FILE* fp, char* target)
{
    int i = 0;
    memset(target, 0, MAX_STR);
    while (1)
    {
        target[i] = fgetc(fp);
        if (EOF == target[i])
        {
            target[i] = '\n';
            return (EOF);
        }
        if ('\n' == target[i])
        {
            target[i]     = ' ';
            target[i + 1] = 0;
            break;
        }
        /*
	  if (0xa == target[i]) {
		  break;
	  }
	  if (0xd == target[i]) {
		  break;
	  }
	  */
        i++;
    }
    return (EOF + 1);
}

#define MAX_CRBS 8
char curOaddr[32];
struct crb
{
    char name[32];
    char ccEntryName[32];
    char oaddrName[32];
};
struct crb crbs[MAX_CRBS];
int numCrbs = 0;

/*
              ptr points to ->         <- endPtr points to
ccd:	  New cachedOaddrInfo 0xfeb660 for oaddr 0x604f6c ccEntry is 0x66c054 ccEntry iaddr 0x804853f ccEntry oaddr 0x604f58
*/

void RecordNewCachedOaddrInfo(char* ptr, char* endPtr)
{
    int i;

    *endPtr = 0;

    crbs[numCrbs].name[0]        = 0;
    crbs[numCrbs].ccEntryName[0] = 0;
    crbs[numCrbs].oaddrName[0]   = 0;

    strcpy(crbs[numCrbs].name, ptr);
    *endPtr = ' ';
    ptr     = strstr(ptr, "is");
    if (ptr == NULL)
    {
        printf("** error could not find ccEntry is\n");
        exit(-1);
    }
    ptr     = AdvanceToBlank(ptr);
    ptr     = AdvanceToNonBlank(ptr);
    endPtr  = AdvanceToBlank(ptr);
    *endPtr = 0;
    strcpy(crbs[numCrbs].ccEntryName, ptr);
    *endPtr = ' ';
    strcpy(crbs[numCrbs].oaddrName, curOaddr);
    printf("** new cachedRegBindings# %d name %s ccEntryName %s oaddrName %s\n", numCrbs, crbs[numCrbs].name,
           crbs[numCrbs].ccEntryName, crbs[numCrbs].oaddrName);

    /* verify we do not have this recorded - if it is then it should have been found */
    for (i = 0; i < numCrbs; i++)
    {
        if (!strcmp(crbs[numCrbs].oaddrName, crbs[i].oaddrName))
        {
            printf("** error found entry for same oaddrName at cachedOaddrInfo# %d\n", i);
            exit(-1);
        }
    }
    numCrbs++;
}

void HandleLookup(char* line)
{
    char* endPtr;
    printf("%s\n", line);
    char* ptr = strstr(line, "0x");
    if (ptr == NULL)
    {
        exit(-1);
    }
    endPtr  = AdvanceToBlank(ptr);
    *endPtr = '\0';
    strcpy(curOaddr, ptr);
}

/*
ccd:	  New cachedRegBindings 0xfeb660 for oaddr 0x604f6c ccEntry is 0x66c054 ccEntry iaddr 0x804853f ccEntry oaddr 0x604f58
*/
void HandleNew(char* line)
{
    char* ptr = strstr(line, "cachedRegBindings");
    char* endPtr;
    printf("%s\n", line);
    printf("** New cachedRegBindings:\n");
    if (ptr == NULL)
    {
        printf("** error could not find cachedRegBindings\n");
        exit(-1);
    }
    ptr    = AdvanceToBlank(ptr);
    ptr    = AdvanceToNonBlank(ptr);
    endPtr = AdvanceToBlank(ptr);
    RecordNewCachedOaddrInfo(ptr, endPtr);
}

/*
 ccd:	  Found cachedRegBindings 0xfeb660
*/
void HandleFound(char* line)
{
    char* ptr = strstr(line, "cachedRegBindings");
    char* endPtr;
    int found = 0;
    int i, theCrbs, x;
    printf("%s\n", line);
    if (ptr == NULL)
    {
        printf("** error could not find cachedRegBindings\n");
        exit(-1);
    }
    ptr     = AdvanceToBlank(ptr);
    ptr     = AdvanceToNonBlank(ptr);
    endPtr  = AdvanceToBlank(ptr);
    *endPtr = 0;
    for (i = 0; i < numCrbs; i++)
    {
        if (!strcmp(crbs[i].name, ptr))
        {
            found   = 1;
            theCrbs = i;
            break;
        }
    }
    if (!found)
    {
        printf("** error could not find a valid cachedRegBindings with name %s\n", ptr);
        exit(-1);
    }
    if (strcmp(crbs[theCrbs].oaddrName, curOaddr))
    {
        printf("** error cachedRegBindings# %d has different oaddr %s %s\n", theCrbs, crbs[theCrbs].oaddrName, curOaddr);
        exit(-1);
    }

    numFinds++;
    if (numInvalidations > 0)
    {
        numFindsAfterInvalidation++;
    }
}

/*
 ccd:	Invalidate ccEntry 0x2cd054
 ccd:	Remove cachedRegBindings 0x124700 ccEntry is 0x2cd054 ccEntry iaddr 0x804853f
*/
void HandleInvalidate(char* line, FILE* fp)
{
    char* ptr;
    char* endPtr;
    int i, x;
    int numFound;

    printf("\n%s\n", line);
    x = readln(fp, line);
    if (x == EOF)
    {
        printf("** error unexpected EOF100\n");
        exit(-1);
    }
    printf("%s\n", line);
    ptr = strstr(line, "RemoveEntriesInRange oaddr");
    if (ptr == NULL)
    {
        return;
    }

    while (1)
    {
        x = readln(fp, line);
        if (x == EOF)
        {
            printf("** error unexpected EOF10\n");
            exit(-1);
        }
        printf("%s\n", line);
        ptr = strstr(line, "None found");
        if (ptr != NULL)
        {
            return;
        }
        ptr = strstr(line, "Remove cachedRegBindings");
        if (ptr == NULL)
        {
            return;
        }
        ptr = strstr(line, "0x");
        if (ptr == NULL)
        {
            printf("** error could not find the address of the cachedRegBinding\n");
            exit(-1);
        }
        endPtr   = AdvanceToBlank(ptr);
        *endPtr  = '\0';
        numFound = 0;
        for (i = 0; i < numCrbs; i++)
        {
            if (!strcmp(crbs[i].name, ptr))
            {
                crbs[i].name[0] = 0;
                numFound++;
            }
        }
        if (numFound != 1)
        {
            printf("** error did not expect to find %d etries to invalidate\n", numFound);
            exit(-1);
        }
        numInvalidations++;
    }
}

int main(int argc, char* argv[])
{
    int x;

    FILE* fp = fopen(argv[1], "r");
    if (fp == NULL)
    {
        printf("** error could not open pin logfile: %s\n", argv[1]);
        exit(-1);
    }
    x = readln(fp, line);
    if (x == EOF)
    {
        printf("** error unexpected EOF100\n");
        exit(-1);
    }
    while (1)
    {
        if (strstr(line, "Lookup"))
        {
            HandleLookup(line);
        }
        else if (strstr(line, "New"))
        {
            HandleNew(line);
        }
        else if (strstr(line, "Invalidate ccEntry"))
        {
            HandleInvalidate(line, fp);
        }
        else if (strstr(line, "Found"))
        {
            HandleFound(line);
        }

        x = readln(fp, line);
        if (x == EOF)
        {
            VerifyAndExit();
        }
    }
}
