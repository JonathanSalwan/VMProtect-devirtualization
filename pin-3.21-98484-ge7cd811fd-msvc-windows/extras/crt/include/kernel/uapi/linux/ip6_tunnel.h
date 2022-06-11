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
#ifndef _IP6_TUNNEL_H
#define _IP6_TUNNEL_H
#include <linux/types.h>
#define IPV6_TLV_TNL_ENCAP_LIMIT 4
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define IPV6_DEFAULT_TNL_ENCAP_LIMIT 4
#define IP6_TNL_F_IGN_ENCAP_LIMIT 0x1
#define IP6_TNL_F_USE_ORIG_TCLASS 0x2
#define IP6_TNL_F_USE_ORIG_FLOWLABEL 0x4
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define IP6_TNL_F_MIP6_DEV 0x8
#define IP6_TNL_F_RCV_DSCP_COPY 0x10
#define IP6_TNL_F_USE_ORIG_FWMARK 0x20
struct ip6_tnl_parm {
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 char name[IFNAMSIZ];
 int link;
 __u8 proto;
 __u8 encap_limit;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 __u8 hop_limit;
 __be32 flowinfo;
 __u32 flags;
 struct in6_addr laddr;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 struct in6_addr raddr;
};
struct ip6_tnl_parm2 {
 char name[IFNAMSIZ];
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 int link;
 __u8 proto;
 __u8 encap_limit;
 __u8 hop_limit;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 __be32 flowinfo;
 __u32 flags;
 struct in6_addr laddr;
 struct in6_addr raddr;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 __be16 i_flags;
 __be16 o_flags;
 __be32 i_key;
 __be32 o_key;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
};
#endif
