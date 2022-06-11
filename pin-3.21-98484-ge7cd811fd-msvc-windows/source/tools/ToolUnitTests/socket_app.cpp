/*
 * Copyright (C) 2007-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <winsock2.h>
#include <windows.h>
#include <stdio.h>

//=======================================================================
// This is the application for testing the funreplace_alert tool.
// It uses alertable system call and receives callbacks in a function
// replaced by the tool.
//=======================================================================

static void init_server(void)
{
    struct WSAData wsaData;
    SOCKET socketFd;
    char yes;
    struct sockaddr_in localAddr;

    // Initialize winsock2:
    WSAStartup(MAKEWORD(2, 0), &wsaData);

    // socket:
    socketFd = socket(AF_INET, SOCK_STREAM, 0);
    yes      = 1;
    setsockopt(socketFd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

    // bind:
    localAddr.sin_family      = AF_INET;
    localAddr.sin_port        = 0;
    localAddr.sin_addr.s_addr = htonl(INADDR_ANY); // use my local IP address
    memset(&(localAddr.sin_zero), '\0', 8);
    bind(socketFd, (struct sockaddr*)&localAddr, sizeof(struct sockaddr));

    // listen:
    printf("--> mpi::init_server, before listen() ...\n");
    fflush(stdout);

    //listen() internally throws exception on ia32. Exceptions are not supported
    //in replacement functions, so we comment it this call to listen().
    //listen( socketFd, 3 );

    printf("--> mpi::init_server, after listen() ...\n");
    fflush(stdout);
}

//=======================================================================

extern "C" __declspec(dllexport) void my_mpi_init(void)
{
    printf("--> mpi::mpi_init\n");
    fflush(stdout);
    init_server();
}

int main()
{
    my_mpi_init();
    return 0;
}
