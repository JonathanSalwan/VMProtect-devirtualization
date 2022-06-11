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
#ifndef _UAPI_LINUX_IN_H
#define _UAPI_LINUX_IN_H
#include <linux/types.h>
#include <linux/socket.h>
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
enum {
 IPPROTO_IP = 0,
#define IPPROTO_IP IPPROTO_IP
 IPPROTO_ICMP = 1,
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define IPPROTO_ICMP IPPROTO_ICMP
 IPPROTO_IGMP = 2,
#define IPPROTO_IGMP IPPROTO_IGMP
 IPPROTO_IPIP = 4,
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define IPPROTO_IPIP IPPROTO_IPIP
 IPPROTO_TCP = 6,
#define IPPROTO_TCP IPPROTO_TCP
 IPPROTO_EGP = 8,
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define IPPROTO_EGP IPPROTO_EGP
 IPPROTO_PUP = 12,
#define IPPROTO_PUP IPPROTO_PUP
 IPPROTO_UDP = 17,
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define IPPROTO_UDP IPPROTO_UDP
 IPPROTO_IDP = 22,
#define IPPROTO_IDP IPPROTO_IDP
 IPPROTO_TP = 29,
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define IPPROTO_TP IPPROTO_TP
 IPPROTO_DCCP = 33,
#define IPPROTO_DCCP IPPROTO_DCCP
 IPPROTO_IPV6 = 41,
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define IPPROTO_IPV6 IPPROTO_IPV6
 IPPROTO_RSVP = 46,
#define IPPROTO_RSVP IPPROTO_RSVP
 IPPROTO_GRE = 47,
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define IPPROTO_GRE IPPROTO_GRE
 IPPROTO_ESP = 50,
#define IPPROTO_ESP IPPROTO_ESP
 IPPROTO_AH = 51,
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define IPPROTO_AH IPPROTO_AH
 IPPROTO_MTP = 92,
#define IPPROTO_MTP IPPROTO_MTP
 IPPROTO_BEETPH = 94,
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define IPPROTO_BEETPH IPPROTO_BEETPH
 IPPROTO_ENCAP = 98,
#define IPPROTO_ENCAP IPPROTO_ENCAP
 IPPROTO_PIM = 103,
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define IPPROTO_PIM IPPROTO_PIM
 IPPROTO_COMP = 108,
#define IPPROTO_COMP IPPROTO_COMP
 IPPROTO_SCTP = 132,
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define IPPROTO_SCTP IPPROTO_SCTP
 IPPROTO_UDPLITE = 136,
#define IPPROTO_UDPLITE IPPROTO_UDPLITE
 IPPROTO_RAW = 255,
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define IPPROTO_RAW IPPROTO_RAW
 IPPROTO_MAX
};
struct in_addr {
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 __be32 s_addr;
};
#define IP_TOS 1
#define IP_TTL 2
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define IP_HDRINCL 3
#define IP_OPTIONS 4
#define IP_ROUTER_ALERT 5
#define IP_RECVOPTS 6
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define IP_RETOPTS 7
#define IP_PKTINFO 8
#define IP_PKTOPTIONS 9
#define IP_MTU_DISCOVER 10
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define IP_RECVERR 11
#define IP_RECVTTL 12
#define IP_RECVTOS 13
#define IP_MTU 14
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define IP_FREEBIND 15
#define IP_IPSEC_POLICY 16
#define IP_XFRM_POLICY 17
#define IP_PASSSEC 18
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define IP_TRANSPARENT 19
#define IP_RECVRETOPTS IP_RETOPTS
#define IP_ORIGDSTADDR 20
#define IP_RECVORIGDSTADDR IP_ORIGDSTADDR
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define IP_MINTTL 21
#define IP_NODEFRAG 22
#define IP_PMTUDISC_DONT 0
#define IP_PMTUDISC_WANT 1
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define IP_PMTUDISC_DO 2
#define IP_PMTUDISC_PROBE 3
#define IP_PMTUDISC_INTERFACE 4
#define IP_MULTICAST_IF 32
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define IP_MULTICAST_TTL 33
#define IP_MULTICAST_LOOP 34
#define IP_ADD_MEMBERSHIP 35
#define IP_DROP_MEMBERSHIP 36
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define IP_UNBLOCK_SOURCE 37
#define IP_BLOCK_SOURCE 38
#define IP_ADD_SOURCE_MEMBERSHIP 39
#define IP_DROP_SOURCE_MEMBERSHIP 40
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define IP_MSFILTER 41
#define MCAST_JOIN_GROUP 42
#define MCAST_BLOCK_SOURCE 43
#define MCAST_UNBLOCK_SOURCE 44
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define MCAST_LEAVE_GROUP 45
#define MCAST_JOIN_SOURCE_GROUP 46
#define MCAST_LEAVE_SOURCE_GROUP 47
#define MCAST_MSFILTER 48
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define IP_MULTICAST_ALL 49
#define IP_UNICAST_IF 50
#define MCAST_EXCLUDE 0
#define MCAST_INCLUDE 1
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define IP_DEFAULT_MULTICAST_TTL 1
#define IP_DEFAULT_MULTICAST_LOOP 1
struct ip_mreq {
 struct in_addr imr_multiaddr;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 struct in_addr imr_interface;
};
struct ip_mreqn {
 struct in_addr imr_multiaddr;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 struct in_addr imr_address;
 int imr_ifindex;
};
struct ip_mreq_source {
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 __be32 imr_multiaddr;
 __be32 imr_interface;
 __be32 imr_sourceaddr;
};
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
struct ip_msfilter {
 __be32 imsf_multiaddr;
 __be32 imsf_interface;
 __u32 imsf_fmode;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 __u32 imsf_numsrc;
 __be32 imsf_slist[1];
};
#define IP_MSFILTER_SIZE(numsrc)   (sizeof(struct ip_msfilter) - sizeof(__u32)   + (numsrc) * sizeof(__u32))
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
struct group_req {
 __u32 gr_interface;
 struct __kernel_sockaddr_storage gr_group;
};
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
struct group_source_req {
 __u32 gsr_interface;
 struct __kernel_sockaddr_storage gsr_group;
 struct __kernel_sockaddr_storage gsr_source;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
};
struct group_filter {
 __u32 gf_interface;
 struct __kernel_sockaddr_storage gf_group;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 __u32 gf_fmode;
 __u32 gf_numsrc;
 struct __kernel_sockaddr_storage gf_slist[1];
};
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define GROUP_FILTER_SIZE(numsrc)   (sizeof(struct group_filter) - sizeof(struct __kernel_sockaddr_storage)   + (numsrc) * sizeof(struct __kernel_sockaddr_storage))
struct in_pktinfo {
 int ipi_ifindex;
 struct in_addr ipi_spec_dst;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 struct in_addr ipi_addr;
};
#define __SOCK_SIZE__ 16
struct sockaddr_in {
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 __kernel_sa_family_t sin_family;
 __be16 sin_port;
 struct in_addr sin_addr;
 unsigned char __pad[__SOCK_SIZE__ - sizeof(short int) -
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 sizeof(unsigned short int) - sizeof(struct in_addr)];
};
#define sin_zero __pad
#define IN_CLASSA(a) ((((long int) (a)) & 0x80000000) == 0)
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define IN_CLASSA_NET 0xff000000
#define IN_CLASSA_NSHIFT 24
#define IN_CLASSA_HOST (0xffffffff & ~IN_CLASSA_NET)
#define IN_CLASSA_MAX 128
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define IN_CLASSB(a) ((((long int) (a)) & 0xc0000000) == 0x80000000)
#define IN_CLASSB_NET 0xffff0000
#define IN_CLASSB_NSHIFT 16
#define IN_CLASSB_HOST (0xffffffff & ~IN_CLASSB_NET)
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define IN_CLASSB_MAX 65536
#define IN_CLASSC(a) ((((long int) (a)) & 0xe0000000) == 0xc0000000)
#define IN_CLASSC_NET 0xffffff00
#define IN_CLASSC_NSHIFT 8
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define IN_CLASSC_HOST (0xffffffff & ~IN_CLASSC_NET)
#define IN_CLASSD(a) ((((long int) (a)) & 0xf0000000) == 0xe0000000)
#define IN_MULTICAST(a) IN_CLASSD(a)
#define IN_MULTICAST_NET 0xF0000000
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define IN_EXPERIMENTAL(a) ((((long int) (a)) & 0xf0000000) == 0xf0000000)
#define IN_BADCLASS(a) IN_EXPERIMENTAL((a))
#define INADDR_ANY ((unsigned long int) 0x00000000)
#define INADDR_BROADCAST ((unsigned long int) 0xffffffff)
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define INADDR_NONE ((unsigned long int) 0xffffffff)
#define IN_LOOPBACKNET 127
#define INADDR_LOOPBACK 0x7f000001
#define IN_LOOPBACK(a) ((((long int) (a)) & 0xff000000) == 0x7f000000)
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define INADDR_UNSPEC_GROUP 0xe0000000U
#define INADDR_ALLHOSTS_GROUP 0xe0000001U
#define INADDR_ALLRTRS_GROUP 0xe0000002U
#define INADDR_MAX_LOCAL_GROUP 0xe00000ffU
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#include <asm/byteorder.h>
#endif
