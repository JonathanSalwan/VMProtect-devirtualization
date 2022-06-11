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
#ifndef __LINUX_DECNET_NETFILTER_H
#define __LINUX_DECNET_NETFILTER_H
#include <linux/netfilter.h>
#include <limits.h>
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define NFC_DN_SRC 0x0001
#define NFC_DN_DST 0x0002
#define NFC_DN_IF_IN 0x0004
#define NFC_DN_IF_OUT 0x0008
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define NF_DN_PRE_ROUTING 0
#define NF_DN_LOCAL_IN 1
#define NF_DN_FORWARD 2
#define NF_DN_LOCAL_OUT 3
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define NF_DN_POST_ROUTING 4
#define NF_DN_HELLO 5
#define NF_DN_ROUTE 6
#define NF_DN_NUMHOOKS 7
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
enum nf_dn_hook_priorities {
 NF_DN_PRI_FIRST = INT_MIN,
 NF_DN_PRI_CONNTRACK = -200,
 NF_DN_PRI_MANGLE = -150,
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 NF_DN_PRI_NAT_DST = -100,
 NF_DN_PRI_FILTER = 0,
 NF_DN_PRI_NAT_SRC = 100,
 NF_DN_PRI_DNRTMSG = 200,
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 NF_DN_PRI_LAST = INT_MAX,
};
struct nf_dn_rtmsg {
 int nfdn_ifindex;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
};
#define NFDN_RTMSG(r) ((unsigned char *)(r) + NLMSG_ALIGN(sizeof(struct nf_dn_rtmsg)))
#define DNRMG_L1_GROUP 0x01
#define DNRMG_L2_GROUP 0x02
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
enum {
 DNRNG_NLGRP_NONE,
#define DNRNG_NLGRP_NONE DNRNG_NLGRP_NONE
 DNRNG_NLGRP_L1,
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define DNRNG_NLGRP_L1 DNRNG_NLGRP_L1
 DNRNG_NLGRP_L2,
#define DNRNG_NLGRP_L2 DNRNG_NLGRP_L2
 __DNRNG_NLGRP_MAX
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
};
#define DNRNG_NLGRP_MAX (__DNRNG_NLGRP_MAX - 1)
#endif
