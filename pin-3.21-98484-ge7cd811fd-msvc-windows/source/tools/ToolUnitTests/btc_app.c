/*
 * Copyright (C) 2010-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * Expanded from just testing ret far to include tests for 
 * a number of other instructions with implicit registers
 */
#include <stdio.h>
#include <stdint.h>

typedef int bool;
#define true 1
#define false 0
typedef unsigned long long UINT64;
typedef signed long long INT64;
typedef unsigned int UINT32;
typedef unsigned short UINT16;

extern int btc(char*, INT64);

static bool isBitOne(char* buffer, UINT32 index)
{
    UINT32 byteOffs = index >> 3;
    UINT32 bitNo    = index & 7;

    return (buffer[byteOffs] & (1 << bitNo)) != 0;
}

int main()
{
    uint8_t buffer[128];
    int i;
    INT64 bitNo;
    int maxBitToOffset = 64 * 8 - 1;
    int errors         = 0;

    for (i = 0; i < 128; i++)
    {
        buffer[i] = 0xff;
    }

    // Flip the bits to zero using btc
    for (bitNo = maxBitToOffset; bitNo >= -maxBitToOffset; bitNo -= 63)
    {
        int res = btc(&buffer[64], bitNo);

        if (res & 2)
        {
            fprintf(stderr, "Bit index register corrupted by btc\n");
            errors++;
        }
        if (res & 1)
        {
            if (isBitOne(&buffer[0], 64 * 8 + bitNo) != false)
            {
                fprintf(stderr, "Bit %d not cleared by btc\n", bitNo);
                errors++;
            }
        }
        else
        {
            errors++;
            fprintf(stderr, "Bit %d not seen as set by btc\n", bitNo);
        }
    }

    // Flip the bits back to one using bts
    for (bitNo = maxBitToOffset; bitNo >= -maxBitToOffset; bitNo -= 63)
    {
        if (bts(&buffer[64], bitNo) == 0)
        {
            if (isBitOne(&buffer[0], 64 * 8 + bitNo) != true)
            {
                fprintf(stderr, "Bit %d not set by bts\n", bitNo);
                errors++;
            }
        }
        else
        {
            errors++;
            fprintf(stderr, "Bit %d not seen as clear by bts\n", bitNo);
        }
    }

    // Flip the bits back to zero using btr
    for (bitNo = maxBitToOffset; bitNo >= -maxBitToOffset; bitNo -= 63)
    {
        if (btr(&buffer[64], bitNo) == 1)
        {
            if (isBitOne(&buffer[0], 64 * 8 + bitNo) != false)
            {
                fprintf(stderr, "Bit %d not cleared by btr\n", bitNo);
                errors++;
            }
        }
        else
        {
            errors++;
            fprintf(stderr, "Bit %d not seen as set by btr\n", bitNo);
        }
    }

    // Check that they are zero using bt
    for (bitNo = maxBitToOffset; bitNo >= -maxBitToOffset; bitNo -= 63)
    {
        if (bt(&buffer[64], bitNo) != 0)
        {
            errors++;
            fprintf(stderr, "Bit %d not seen as clear by bt\n", bitNo);
        }
    }

    if (errors == 0)
    {
        fprintf(stderr, "All OK\n");
        return 0;
    }
    else
    {
        fprintf(stderr, "%d errors\n", errors);
        return errors;
    }
}
