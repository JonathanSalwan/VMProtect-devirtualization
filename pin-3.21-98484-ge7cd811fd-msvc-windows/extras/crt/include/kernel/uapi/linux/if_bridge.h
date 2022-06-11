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
#ifndef _UAPI_LINUX_IF_BRIDGE_H
#define _UAPI_LINUX_IF_BRIDGE_H
#include <linux/types.h>
#include <linux/if_ether.h>
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define SYSFS_BRIDGE_ATTR "bridge"
#define SYSFS_BRIDGE_FDB "brforward"
#define SYSFS_BRIDGE_PORT_SUBDIR "brif"
#define SYSFS_BRIDGE_PORT_ATTR "brport"
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define SYSFS_BRIDGE_PORT_LINK "bridge"
#define BRCTL_VERSION 1
#define BRCTL_GET_VERSION 0
#define BRCTL_GET_BRIDGES 1
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define BRCTL_ADD_BRIDGE 2
#define BRCTL_DEL_BRIDGE 3
#define BRCTL_ADD_IF 4
#define BRCTL_DEL_IF 5
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define BRCTL_GET_BRIDGE_INFO 6
#define BRCTL_GET_PORT_LIST 7
#define BRCTL_SET_BRIDGE_FORWARD_DELAY 8
#define BRCTL_SET_BRIDGE_HELLO_TIME 9
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define BRCTL_SET_BRIDGE_MAX_AGE 10
#define BRCTL_SET_AGEING_TIME 11
#define BRCTL_SET_GC_INTERVAL 12
#define BRCTL_GET_PORT_INFO 13
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define BRCTL_SET_BRIDGE_STP_STATE 14
#define BRCTL_SET_BRIDGE_PRIORITY 15
#define BRCTL_SET_PORT_PRIORITY 16
#define BRCTL_SET_PATH_COST 17
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define BRCTL_GET_FDB_ENTRIES 18
#define BR_STATE_DISABLED 0
#define BR_STATE_LISTENING 1
#define BR_STATE_LEARNING 2
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define BR_STATE_FORWARDING 3
#define BR_STATE_BLOCKING 4
struct __bridge_info {
 __u64 designated_root;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 __u64 bridge_id;
 __u32 root_path_cost;
 __u32 max_age;
 __u32 hello_time;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 __u32 forward_delay;
 __u32 bridge_max_age;
 __u32 bridge_hello_time;
 __u32 bridge_forward_delay;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 __u8 topology_change;
 __u8 topology_change_detected;
 __u8 root_port;
 __u8 stp_enabled;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 __u32 ageing_time;
 __u32 gc_interval;
 __u32 hello_timer_value;
 __u32 tcn_timer_value;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 __u32 topology_change_timer_value;
 __u32 gc_timer_value;
};
struct __port_info {
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 __u64 designated_root;
 __u64 designated_bridge;
 __u16 port_id;
 __u16 designated_port;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 __u32 path_cost;
 __u32 designated_cost;
 __u8 state;
 __u8 top_change_ack;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 __u8 config_pending;
 __u8 unused0;
 __u32 message_age_timer_value;
 __u32 forward_delay_timer_value;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 __u32 hold_timer_value;
};
struct __fdb_entry {
 __u8 mac_addr[ETH_ALEN];
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 __u8 port_no;
 __u8 is_local;
 __u32 ageing_timer_value;
 __u8 port_hi;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 __u8 pad0;
 __u16 unused;
};
#define BRIDGE_FLAGS_MASTER 1
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define BRIDGE_FLAGS_SELF 2
#define BRIDGE_MODE_VEB 0
#define BRIDGE_MODE_VEPA 1
enum {
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 IFLA_BRIDGE_FLAGS,
 IFLA_BRIDGE_MODE,
 IFLA_BRIDGE_VLAN_INFO,
 __IFLA_BRIDGE_MAX,
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
};
#define IFLA_BRIDGE_MAX (__IFLA_BRIDGE_MAX - 1)
#define BRIDGE_VLAN_INFO_MASTER (1<<0)
#define BRIDGE_VLAN_INFO_PVID (1<<1)
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define BRIDGE_VLAN_INFO_UNTAGGED (1<<2)
struct bridge_vlan_info {
 __u16 flags;
 __u16 vid;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
};
enum {
 MDBA_UNSPEC,
 MDBA_MDB,
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 MDBA_ROUTER,
 __MDBA_MAX,
};
#define MDBA_MAX (__MDBA_MAX - 1)
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
enum {
 MDBA_MDB_UNSPEC,
 MDBA_MDB_ENTRY,
 __MDBA_MDB_MAX,
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
};
#define MDBA_MDB_MAX (__MDBA_MDB_MAX - 1)
enum {
 MDBA_MDB_ENTRY_UNSPEC,
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 MDBA_MDB_ENTRY_INFO,
 __MDBA_MDB_ENTRY_MAX,
};
#define MDBA_MDB_ENTRY_MAX (__MDBA_MDB_ENTRY_MAX - 1)
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
enum {
 MDBA_ROUTER_UNSPEC,
 MDBA_ROUTER_PORT,
 __MDBA_ROUTER_MAX,
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
};
#define MDBA_ROUTER_MAX (__MDBA_ROUTER_MAX - 1)
struct br_port_msg {
 __u8 family;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 __u32 ifindex;
};
struct br_mdb_entry {
 __u32 ifindex;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define MDB_TEMPORARY 0
#define MDB_PERMANENT 1
 __u8 state;
 struct {
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 union {
 __be32 ip4;
 struct in6_addr ip6;
 } u;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 __be16 proto;
 } addr;
};
enum {
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 MDBA_SET_ENTRY_UNSPEC,
 MDBA_SET_ENTRY,
 __MDBA_SET_ENTRY_MAX,
};
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define MDBA_SET_ENTRY_MAX (__MDBA_SET_ENTRY_MAX - 1)
#endif
