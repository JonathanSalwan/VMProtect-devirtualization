/*
 * Copyright (C) 2012-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <windows.h>
#include <stdio.h>
int readln(FILE* fp, char* target)
{
    int i = 0;
    while (1)
    {
        target[i] = fgetc(fp);
        if (EOF == target[i])
        {
            target[i] = ' ';
            return (EOF);
        }
        if ('\n' == target[i])
        {
            target[i] = ' ';
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

int main(int argc, char* argv[])
{
    char correctCmdLine[1024];
    int i, j, k, correctCmdLineChar;
    int argvNum;

    memset(correctCmdLine, 0, 1024);
    FILE* fp = fopen("pin_invoke.txt", "r");
    readln(fp, correctCmdLine);
    correctCmdLineChar                 = strlen(correctCmdLine);
    correctCmdLine[correctCmdLineChar] = ' ';

    for (i = 1; i < argc; i++)
    {
        strcat(correctCmdLine, " ");
        strcat(correctCmdLine, (argv[i]));
    }
    correctCmdLineChar = 0;

    while (correctCmdLine[correctCmdLineChar] != 0)
    {
        if (correctCmdLine[correctCmdLineChar] == '/')
        {
            correctCmdLine[correctCmdLineChar] = '\\';
        }
        else if (correctCmdLine[correctCmdLineChar] == '-' && correctCmdLine[correctCmdLineChar + 1] == '-')
        {
            correctCmdLine[correctCmdLineChar - 1] = ' ';
        }
        correctCmdLineChar++;
    }

    printf("%s\n", correctCmdLine);
}
