/*
 * Copyright (C) 2015-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

// <COMPONENT>: os-apis
// <FILE-TYPE>: component public header
/// @file memory.h

/*!
 * @defgroup OS_APIS_IPC IPC
 * @brief Contains IPC related os apis
 */

#ifndef OS_APIS_IPC_H
#define OS_APIS_IPC_H

#include "os-apis.h"
#include <sys/types.h>
#include <sys/socket.h>

/*! @ingroup OS_APIS_IPC
 * Socket domain
 */
typedef enum
{
    //! Internet socket
    OS_APIS_IPC_SOCKET_DOMAIN_INET = 0,
    OS_APIS_IPC_SOCKET_DOMAIN_UNIX = 1
} OS_APIS_IPC_SOCKET_DOMAIN;

/*! @ingroup OS_APIS_IPC
 * Socket domain
 */
typedef enum
{
    //! Full-duplex byte stream
    OS_APIS_IPC_SOCKET_TYPE_STREAM = 0
} OS_APIS_IPC_SOCKET_TYPE;

/*! @ingroup OS_APIS_IPC
 * Socket domain
 */
typedef enum
{
    //! Full-duplex byte stream
    OS_APIS_IPC_SOCKET_PROTOCOL_DEFAULT = 0
} OS_APIS_IPC_SOCKET_PROTOCOL;

/*! @ingroup OS_APIS_IPC
 * Value for argument @b how to OS_Shutdown()
 */
typedef enum
{
    //! Invalid value for socket shutdown
    OS_APIS_IPC_SHUTDOWN_INVALID = 0,
    //! Shutdown socket's read side
    OS_APIS_IPC_SHUTDOWN_READ = (1 << 0),
    //! Shutdown socket's write side
    OS_APIS_IPC_SHUTDOWN_WRITE = (1 << 1)
} OS_APIS_IPC_SHUTDOWN;

/*! @ingroup OS_APIS_IPC
 * Value for argument @b how to OS_Shutdown()
 */
typedef enum
{
    //! No flags specified
    OS_APIS_IPC_SENDTO_FLAGS_NONE = 0,
    //! Send out of band message
    OS_APIS_IPC_SENDTO_FLAGS_OOB = (1 << 0),
    //! bypass routing, use direct interface
    OS_APIS_IPC_SENDTO_FLAGS_DONT_ROUTE = (1 << 1),
    //! Don't send SIGPIPE when the other side of the connection is closed
    OS_APIS_IPC_SENDTO_FLAGS_NO_SIGNAL = (1 << 2)
} OS_APIS_IPC_SENDTO_FLAGS;

/*! @ingroup OS_APIS_IPC
 * Creates a socket. See man page of socket(2) for more details.
 *
 * @param[in]  domain   Socket domain
 * @param[in]  type     Socket type
 * @param[in]  protocol Socket Protocol
 * @param[out] fd       Created file descriptor for the socket
 *
 * @retval     OS_RETURN_CODE_NO_ERROR           If the operation succeeded
 * @retval     OS_RETURN_CODE_INVALID_ARGS       One of the input arguments is invalid.
 * @retval     OS_RETURN_CODE_FILE_OPEN_FAILED   If the operation failed
 *
 * @par Availability:
 *   @b O/S:   Linux & macOS* \n
 *   @b CPU:   All \n
 */
OS_RETURN_CODE OS_Socket(OS_APIS_IPC_SOCKET_DOMAIN domain, OS_APIS_IPC_SOCKET_TYPE type, OS_APIS_IPC_SOCKET_PROTOCOL protocol,
                         NATIVE_FD* fd);

/*! @ingroup OS_APIS_IPC
 * Accepts a new connection on a listening socket. See man page of accept(2) for more details.
 *
 * @param[in]     sockfd   The listening socket
 * @param[out]    addr     The address of the remote connection that was accepted.
 * @param[in,out] addrlen  The size in bytes of @b addr, returns the number of bytes writen to addr.
 * @param[out]    newfd    The file descriptor of the socket created for this session.
 *
 * @retval     OS_RETURN_CODE_NO_ERROR           If the operation succeeded
 * @retval     OS_RETURN_CODE_FILE_OPEN_FAILED   If the operation failed
 *
 * @par Availability:
 *   @b O/S:   Linux & macOS* \n
 *   @b CPU:   All \n
 */
OS_RETURN_CODE OS_Accept(NATIVE_FD sockfd, struct sockaddr* addr, USIZE* addrlen, NATIVE_FD* newfd);

/*! @ingroup OS_APIS_IPC
 * Listen for incomming connection in a socket. See man page of listen(2) for more details.
 *
 * @param[in]     sockfd   The socket to listen on.
 * @param[in]     backlog  Number of connections to backlog (the meanning of it depends on the OS and kernel version).
 *
 * @retval     OS_RETURN_CODE_NO_ERROR           If the operation succeeded
 * @retval     OS_RETURN_CODE_FILE_OPEN_FAILED   If the operation failed
 *
 * @par Availability:
 *   @b O/S:   Linux & macOS* \n
 *   @b CPU:   All \n
 */
OS_RETURN_CODE OS_Listen(NATIVE_FD sockfd, INT backlog);

/*! @ingroup OS_APIS_IPC
 * Connect this socket to a remote socket. See man page of connet(2) for more details.
 *
 * @param[in]     sockfd   The socket to initiate the connect from.
 * @param[in]     addr     The address of the remote socket to connect to.
 * @param[in]     addrlen  The size in bytes of @b addr.
 *
 * @retval     OS_RETURN_CODE_NO_ERROR           If the operation succeeded
 * @retval     OS_RETURN_CODE_FILE_OPEN_FAILED   If the operation failed
 *
 * @par Availability:
 *   @b O/S:   Linux & macOS* \n
 *   @b CPU:   All \n
 */
OS_RETURN_CODE OS_Connect(NATIVE_FD sockfd, const struct sockaddr* addr, USIZE addrlen);

/*! @ingroup OS_APIS_IPC
 * Bind this socket to a local address. See man page of bind(2) for more details.
 *
 * @param[in]     sockfd   The socket to bind.
 * @param[in]     addr     The address to bind the socket to.
 * @param[in]     addrlen  The size in bytes of @b addr.
 *
 * @retval     OS_RETURN_CODE_NO_ERROR           If the operation succeeded
 * @retval     OS_RETURN_CODE_FILE_OPEN_FAILED   If the operation failed
 *
 * @par Availability:
 *   @b O/S:   Linux & macOS* \n
 *   @b CPU:   All \n
 */
OS_RETURN_CODE OS_Bind(NATIVE_FD sockfd, const struct sockaddr* addr, USIZE addrlen);

/*! @ingroup OS_APIS_IPC
 * Return the local address where the socket was bound. See man page of getsockname(2) for more details.
 *
 * @param[in]     sockfd   The socket to initiate the connect from.
 * @param[out]    addr     The address where the socket was bound.
 * @param[in,out] addrlen  The size in bytes of @b addr, returns the number of bytes writen to addr.
 *
 * @retval     OS_RETURN_CODE_NO_ERROR           If the operation succeeded
 * @retval     OS_RETURN_CODE_FILE_OPEN_FAILED   If the operation failed
 *
 * @par Availability:
 *   @b O/S:   Linux & macOS* \n
 *   @b CPU:   All \n
 */
OS_RETURN_CODE OS_GetSockName(NATIVE_FD sockfd, const struct sockaddr* addr, USIZE* addrlen);

/*! @ingroup OS_APIS_IPC
 * Causes all or part of a full-duplex connection on the socket associated with sockfd to be shut down.
 *
 * @param[in]     sockfd   The file descriptor of the socket to shut down.
 * @param[in]     how      Bitwise OR of the value in OS_APIS_IPC_SHUTDOWN.
 *                         If OS_APIS_IPC_SHUTDOWN_READ is specified, further receptions will be disallowed.
 *                         If OS_APIS_IPC_SHUTDOWN_WRITE is specified,further transmissions will be disallowed.
 *
 * @retval     OS_RETURN_CODE_NO_ERROR           If the operation succeeded
 * @retval     OS_RETURN_CODE_INVALID_ARGS       If the value of @b how is invalid.
 * @retval     OS_RETURN_CODE_FILE_CLOSE_FAILED  If the operation failed
 *
 * @par Availability:
 *   @b O/S:   Linux & macOS* \n
 *   @b CPU:   All \n
 */
OS_RETURN_CODE OS_Shutdown(NATIVE_FD sockfd, OS_APIS_IPC_SHUTDOWN how);

/*! @ingroup OS_APIS_IPC
 * Transmit a message to another socket.
 *
 * @param[in]     sockfd     The socket to send data to.
 * @param[in]     buffer     Data buffer
 * @param[in]     flags      Message flags
 * @param[in]     dest_addr  Socket destination address
 * @param[in]     dest_len   Size of dest_addr
 * @param[in,out] buf_length Length of the bytes in @b buffer / bytes sent
 *
 * @retval     OS_RETURN_CODE_NO_ERROR           If the operation succeeded
 * @retval     OS_RETURN_CODE_FILE_WRITE_FAILED  If the operation failed
 *
 * @par Availability:
 *   @b O/S:   Linux & macOS* \n
 *   @b CPU:   All \n
 */
OS_RETURN_CODE OS_SendTo(NATIVE_FD sockfd, const void* buffer, OS_APIS_IPC_SENDTO_FLAGS flags, const struct sockaddr* dest_addr,
                         USIZE dest_len, USIZE* buf_length);

#endif // OS_APIS_IPC_H
