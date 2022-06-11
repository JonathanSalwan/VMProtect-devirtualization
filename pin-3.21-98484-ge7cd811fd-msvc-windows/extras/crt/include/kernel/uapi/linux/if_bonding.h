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
#ifndef _LINUX_IF_BONDING_H
#define _LINUX_IF_BONDING_H
#include <linux/if.h>
#include <linux/types.h>
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#include <linux/if_ether.h>
#define BOND_ABI_VERSION 2
#define BOND_ENSLAVE_OLD (SIOCDEVPRIVATE)
#define BOND_RELEASE_OLD (SIOCDEVPRIVATE + 1)
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define BOND_SETHWADDR_OLD (SIOCDEVPRIVATE + 2)
#define BOND_SLAVE_INFO_QUERY_OLD (SIOCDEVPRIVATE + 11)
#define BOND_INFO_QUERY_OLD (SIOCDEVPRIVATE + 12)
#define BOND_CHANGE_ACTIVE_OLD (SIOCDEVPRIVATE + 13)
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define BOND_CHECK_MII_STATUS (SIOCGMIIPHY)
#define BOND_MODE_ROUNDROBIN 0
#define BOND_MODE_ACTIVEBACKUP 1
#define BOND_MODE_XOR 2
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define BOND_MODE_BROADCAST 3
#define BOND_MODE_8023AD 4
#define BOND_MODE_TLB 5
#define BOND_MODE_ALB 6
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define BOND_LINK_UP 0
#define BOND_LINK_FAIL 1
#define BOND_LINK_DOWN 2
#define BOND_LINK_BACK 3
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define BOND_STATE_ACTIVE 0
#define BOND_STATE_BACKUP 1
#define BOND_DEFAULT_MAX_BONDS 1
#define BOND_DEFAULT_TX_QUEUES 16
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define BOND_DEFAULT_RESEND_IGMP 1
#define BOND_XMIT_POLICY_LAYER2 0
#define BOND_XMIT_POLICY_LAYER34 1
#define BOND_XMIT_POLICY_LAYER23 2
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define BOND_XMIT_POLICY_ENCAP23 3
#define BOND_XMIT_POLICY_ENCAP34 4
typedef struct ifbond {
 __s32 bond_mode;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 __s32 num_slaves;
 __s32 miimon;
} ifbond;
typedef struct ifslave {
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 __s32 slave_id;
 char slave_name[IFNAMSIZ];
 __s8 link;
 __s8 state;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 __u32 link_failure_count;
} ifslave;
struct ad_info {
 __u16 aggregator_id;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 __u16 ports;
 __u16 actor_key;
 __u16 partner_key;
 __u8 partner_system[ETH_ALEN];
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
};
#endif
