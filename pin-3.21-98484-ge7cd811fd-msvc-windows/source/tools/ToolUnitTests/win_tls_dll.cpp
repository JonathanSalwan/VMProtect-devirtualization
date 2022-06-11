/*
 * Copyright (C) 2008-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <Windows.h>
#include <stdio.h>

//TlS usage in application
static const int RESEREVED_TLS = 100;
DWORD tlsIndexes[RESEREVED_TLS];

BOOL DllMain(HANDLE hDll, DWORD dwReason, LPVOID lpReserved)
{
    switch (dwReason)
    {
        case DLL_PROCESS_ATTACH:
        {
            for (int i = 0; i < RESEREVED_TLS; i++)
            {
                tlsIndexes[i] = TlsAlloc();
                if (tlsIndexes[i] == TLS_OUT_OF_INDEXES)
                {
                    fprintf(stderr, "Failed to allocate tls index number %d\n", i);
                    return FALSE;
                }
            }
            break;
        }
        case DLL_THREAD_ATTACH:
        {
            for (int i = 0; i < RESEREVED_TLS; i++)
            {
                bool res = TlsSetValue(tlsIndexes[i], reinterpret_cast< LPVOID >(i));
                if (res == FALSE)
                {
                    fprintf(stderr, "Failed to set tls index number %d\n", i);
                    return FALSE;
                }
            }
            break;
        }
        case DLL_THREAD_DETACH:
        {
            for (int i = 0; i < RESEREVED_TLS; i++)
            {
                int val = reinterpret_cast< int >(TlsGetValue(tlsIndexes[i]));
                if (val != i)
                {
                    fprintf(stderr, "Failed to get tls index number %d\n", i);
                    return FALSE;
                }
            }
            break;
        }
        case DLL_PROCESS_DETACH:
        {
            for (int i = 0; i < RESEREVED_TLS; i++)
            {
                BOOL res = TlsFree(tlsIndexes[i]);
                if (res == FALSE)
                {
                    fprintf(stderr, "Failed to free tls index number %d\n", i);
                    return FALSE;
                }
            }
            break;
        }
        default:
        {
            break;
        }
    }
    return TRUE;
}

extern "C" __declspec(dllexport) void Nothing() {}
