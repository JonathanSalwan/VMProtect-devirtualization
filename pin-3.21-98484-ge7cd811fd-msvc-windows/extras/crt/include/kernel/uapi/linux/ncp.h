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
#ifndef _LINUX_NCP_H
#define _LINUX_NCP_H
#include <linux/types.h>
#define NCP_PTYPE (0x11)
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define NCP_PORT (0x0451)
#define NCP_ALLOC_SLOT_REQUEST (0x1111)
#define NCP_REQUEST (0x2222)
#define NCP_DEALLOC_SLOT_REQUEST (0x5555)
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
struct ncp_request_header {
 __u16 type;
 __u8 sequence;
 __u8 conn_low;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 __u8 task;
 __u8 conn_high;
 __u8 function;
 __u8 data[0];
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
} __attribute__((packed));
#define NCP_REPLY (0x3333)
#define NCP_WATCHDOG (0x3E3E)
#define NCP_POSITIVE_ACK (0x9999)
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
struct ncp_reply_header {
 __u16 type;
 __u8 sequence;
 __u8 conn_low;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 __u8 task;
 __u8 conn_high;
 __u8 completion_code;
 __u8 connection_state;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 __u8 data[0];
} __attribute__((packed));
#define NCP_VOLNAME_LEN (16)
#define NCP_NUMBER_OF_VOLUMES (256)
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
struct ncp_volume_info {
 __u32 total_blocks;
 __u32 free_blocks;
 __u32 purgeable_blocks;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 __u32 not_yet_purgeable_blocks;
 __u32 total_dir_entries;
 __u32 available_dir_entries;
 __u8 sectors_per_block;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 char volume_name[NCP_VOLNAME_LEN + 1];
};
#define AR_READ (cpu_to_le16(1))
#define AR_WRITE (cpu_to_le16(2))
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define AR_EXCLUSIVE (cpu_to_le16(0x20))
#define NCP_FILE_ID_LEN 6
#define NW_NS_DOS 0
#define NW_NS_MAC 1
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define NW_NS_NFS 2
#define NW_NS_FTAM 3
#define NW_NS_OS2 4
#define RIM_NAME (cpu_to_le32(1))
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define RIM_SPACE_ALLOCATED (cpu_to_le32(2))
#define RIM_ATTRIBUTES (cpu_to_le32(4))
#define RIM_DATA_SIZE (cpu_to_le32(8))
#define RIM_TOTAL_SIZE (cpu_to_le32(0x10))
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define RIM_EXT_ATTR_INFO (cpu_to_le32(0x20))
#define RIM_ARCHIVE (cpu_to_le32(0x40))
#define RIM_MODIFY (cpu_to_le32(0x80))
#define RIM_CREATION (cpu_to_le32(0x100))
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define RIM_OWNING_NAMESPACE (cpu_to_le32(0x200))
#define RIM_DIRECTORY (cpu_to_le32(0x400))
#define RIM_RIGHTS (cpu_to_le32(0x800))
#define RIM_ALL (cpu_to_le32(0xFFF))
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define RIM_COMPRESSED_INFO (cpu_to_le32(0x80000000))
#define NSIBM_NFS_NAME 0x0001
#define NSIBM_NFS_MODE 0x0002
#define NSIBM_NFS_GID 0x0004
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define NSIBM_NFS_NLINKS 0x0008
#define NSIBM_NFS_RDEV 0x0010
#define NSIBM_NFS_LINK 0x0020
#define NSIBM_NFS_CREATED 0x0040
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define NSIBM_NFS_UID 0x0080
#define NSIBM_NFS_ACSFLAG 0x0100
#define NSIBM_NFS_MYFLAG 0x0200
#define OC_MODE_OPEN 0x01
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define OC_MODE_TRUNCATE 0x02
#define OC_MODE_REPLACE 0x02
#define OC_MODE_CREATE 0x08
#define OC_ACTION_NONE 0x00
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define OC_ACTION_OPEN 0x01
#define OC_ACTION_CREATE 0x02
#define OC_ACTION_TRUNCATE 0x04
#define OC_ACTION_REPLACE 0x04
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#ifndef AR_READ_ONLY
#define AR_READ_ONLY 0x0001
#define AR_WRITE_ONLY 0x0002
#define AR_DENY_READ 0x0004
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define AR_DENY_WRITE 0x0008
#define AR_COMPATIBILITY 0x0010
#define AR_WRITE_THROUGH 0x0040
#define AR_OPEN_COMPRESSED 0x0100
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#endif
struct nw_nfs_info {
 __u32 mode;
 __u32 rdev;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
};
struct nw_info_struct {
 __u32 spaceAlloc;
 __le32 attributes;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 __u16 flags;
 __le32 dataStreamSize;
 __le32 totalStreamSize;
 __u16 numberOfStreams;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 __le16 creationTime;
 __le16 creationDate;
 __u32 creatorID;
 __le16 modifyTime;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 __le16 modifyDate;
 __u32 modifierID;
 __le16 lastAccessDate;
 __u16 archiveTime;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 __u16 archiveDate;
 __u32 archiverID;
 __u16 inheritedRightsMask;
 __le32 dirEntNum;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 __le32 DosDirNum;
 __u32 volNumber;
 __u32 EADataSize;
 __u32 EAKeyCount;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 __u32 EAKeySize;
 __u32 NSCreator;
 __u8 nameLen;
 __u8 entryName[256];
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
} __attribute__((packed));
#define DM_ATTRIBUTES (cpu_to_le32(0x02))
#define DM_CREATE_DATE (cpu_to_le32(0x04))
#define DM_CREATE_TIME (cpu_to_le32(0x08))
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define DM_CREATOR_ID (cpu_to_le32(0x10))
#define DM_ARCHIVE_DATE (cpu_to_le32(0x20))
#define DM_ARCHIVE_TIME (cpu_to_le32(0x40))
#define DM_ARCHIVER_ID (cpu_to_le32(0x80))
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define DM_MODIFY_DATE (cpu_to_le32(0x0100))
#define DM_MODIFY_TIME (cpu_to_le32(0x0200))
#define DM_MODIFIER_ID (cpu_to_le32(0x0400))
#define DM_LAST_ACCESS_DATE (cpu_to_le32(0x0800))
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define DM_INHERITED_RIGHTS_MASK (cpu_to_le32(0x1000))
#define DM_MAXIMUM_SPACE (cpu_to_le32(0x2000))
struct nw_modify_dos_info {
 __le32 attributes;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 __le16 creationDate;
 __le16 creationTime;
 __u32 creatorID;
 __le16 modifyDate;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 __le16 modifyTime;
 __u32 modifierID;
 __u16 archiveDate;
 __u16 archiveTime;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 __u32 archiverID;
 __le16 lastAccessDate;
 __u16 inheritanceGrantMask;
 __u16 inheritanceRevokeMask;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 __u32 maximumSpace;
} __attribute__((packed));
struct nw_search_sequence {
 __u8 volNumber;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 __u32 dirBase;
 __u32 sequence;
} __attribute__((packed));
#endif
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
