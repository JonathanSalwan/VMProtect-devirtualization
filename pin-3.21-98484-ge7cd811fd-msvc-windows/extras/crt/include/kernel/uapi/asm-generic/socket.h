/*
 * Copyright (C) 2015-2019 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/****************************************************************************
 ****************************************************************************
 ***
 ***   This header was automatically generated from a Linux kernel header
 ***   of the same name, to make information necessary for userspace to
 ***   call into the kernel available to libc.  It contains only constants,
 ***   structures, and macros generated from the original header, and thus,
 ***   contains no copyrightable information.
 ***
 ***   To edit the content of this header, modify the corresponding
 ***   source file (e.g. under external/kernel-headers/original/) then
 ***   run bionic/libc/kernel/tools/update_all.py
 ***
 ***   Any manual change here will be lost the next time this script will
 ***   be run. You've been warned!
 ***
 ****************************************************************************
 ****************************************************************************/
#ifndef __ASM_GENERIC_SOCKET_H
#define __ASM_GENERIC_SOCKET_H
#include <asm/sockios.h>

#ifdef TARGET_MAC

#define SOL_SOCKET      0xffff          /* options for socket level */

#define SO_DEBUG        0x0001          /* turn on debugging info recording */
#define SO_ACCEPTCONN   0x0002          /* socket has had listen() */
#define SO_REUSEADDR    0x0004          /* allow local address reuse */
#define SO_KEEPALIVE    0x0008          /* keep connections alive */
#define SO_DONTROUTE    0x0010          /* just use interface addresses */
#define SO_BROADCAST    0x0020          /* permit sending of broadcast msgs */
#define SO_USELOOPBACK  0x0040          /* bypass hardware when possible */
#define SO_LINGER       0x0080          /* linger on close if data present (in ticks) */
#define SO_OOBINLINE    0x0100          /* leave received OOB data in line */
#define SO_REUSEPORT    0x0200          /* allow local address & port reuse */
#define SO_TIMESTAMP    0x0400          /* timestamp received dgram traffic */
#define SO_TIMESTAMP_MONOTONIC  0x0800  /* Monotonically increasing timestamp on rcvd dgram */
#define SO_DONTTRUNC    0x2000          /* APPLE: Retain unread data */
                                        /*  (ATOMIC proto) */
#define SO_WANTMORE             0x4000          /* APPLE: Give hint when more data ready */
#define SO_WANTOOBFLAG  0x8000          /* APPLE: Want OOB in MSG_FLAG on receive */

/*
 * Additional options, not kept in so_options.
 */
#define SO_SNDBUF       0x1001          /* send buffer size */
#define SO_RCVBUF       0x1002          /* receive buffer size */
#define SO_SNDLOWAT     0x1003          /* send low-water mark */
#define SO_RCVLOWAT     0x1004          /* receive low-water mark */
#define SO_SNDTIMEO     0x1005          /* send timeout */
#define SO_RCVTIMEO     0x1006          /* receive timeout */
#define SO_ERROR        0x1007          /* get error status and clear */
#define SO_TYPE         0x1008          /* get socket type */
#define SO_LABEL        0x1010          /* socket's MAC label */
#define SO_PEERLABEL    0x1011          /* socket's peer MAC label */
#define SO_NREAD        0x1020          /* APPLE: get 1st-packet byte count */
#define SO_NKE          0x1021          /* APPLE: Install socket-level NKE */
#define SO_NOSIGPIPE    0x1022          /* APPLE: No SIGPIPE on EPIPE */
#define SO_NOADDRERR    0x1023          /* APPLE: Returns EADDRNOTAVAIL when src is not available anymore */
#define SO_NWRITE       0x1024          /* APPLE: Get number of bytes currently in send socket buffer */
#define SO_REUSESHAREUID        0x1025          /* APPLE: Allow reuse of port/socket by different userids */
#define SO_LINGER_SEC   0x1080          /* linger on close if data present (in seconds) */
#define SO_RESTRICTIONS 0x1081  /* APPLE: deny inbound/outbound/both/flag set */
#define SO_RESTRICT_DENYIN              0x00000001      /* flag for SO_RESTRICTIONS - deny inbound */
#define SO_RESTRICT_DENYOUT             0x00000002      /* flag for SO_RESTRICTIONS - deny outbound */
#define SO_RESTRICT_DENYSET             0x80000000      /* flag for SO_RESTRICTIONS - deny has been set */
#define SO_RANDOMPORT   0x1082  /* APPLE: request local port randomization */
#define SO_NP_EXTENSIONS        0x1083  /* To turn off some POSIX behavior */

#else

#define SOL_SOCKET 1
#define SO_DEBUG 1
#define SO_REUSEADDR 2
#define SO_TYPE 3
#define SO_ERROR 4
#define SO_DONTROUTE 5
#define SO_BROADCAST 6
#define SO_SNDBUF 7
#define SO_RCVBUF 8
#define SO_SNDBUFFORCE 32
#define SO_RCVBUFFORCE 33
#define SO_KEEPALIVE 9
#define SO_OOBINLINE 10
#define SO_NO_CHECK 11
#define SO_PRIORITY 12
#define SO_LINGER 13
#define SO_BSDCOMPAT 14
#define SO_REUSEPORT 15
#ifndef SO_PASSCRED
#define SO_PASSCRED 16
#define SO_PEERCRED 17
#define SO_RCVLOWAT 18
#define SO_SNDLOWAT 19
#define SO_RCVTIMEO 20
#define SO_SNDTIMEO 21
#endif
#define SO_SECURITY_AUTHENTICATION 22
#define SO_SECURITY_ENCRYPTION_TRANSPORT 23
#define SO_SECURITY_ENCRYPTION_NETWORK 24
#define SO_BINDTODEVICE 25
#define SO_ATTACH_FILTER 26
#define SO_DETACH_FILTER 27
#define SO_GET_FILTER SO_ATTACH_FILTER
#define SO_PEERNAME 28
#define SO_TIMESTAMP 29
#define SCM_TIMESTAMP SO_TIMESTAMP
#define SO_ACCEPTCONN 30
#define SO_PEERSEC 31
#define SO_PASSSEC 34
#define SO_TIMESTAMPNS 35
#define SCM_TIMESTAMPNS SO_TIMESTAMPNS
#define SO_MARK 36
#define SO_TIMESTAMPING 37
#define SCM_TIMESTAMPING SO_TIMESTAMPING
#define SO_PROTOCOL 38
#define SO_DOMAIN 39
#define SO_RXQ_OVFL 40
#define SO_WIFI_STATUS 41
#define SCM_WIFI_STATUS SO_WIFI_STATUS
#define SO_PEEK_OFF 42
#define SO_NOFCS 43
#define SO_LOCK_FILTER 44
#define SO_SELECT_ERR_QUEUE 45
#define SO_BUSY_POLL 46
#define SO_MAX_PACING_RATE 47
#define SO_BPF_EXTENSIONS 48

#endif // TARGET_MAC

#endif
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
