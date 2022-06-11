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
#ifndef _UAPI_ADFS_FS_H
#define _UAPI_ADFS_FS_H
#include <linux/types.h>
#include <linux/magic.h>
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
struct adfs_discrecord {
 __u8 log2secsize;
 __u8 secspertrack;
 __u8 heads;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 __u8 density;
 __u8 idlen;
 __u8 log2bpmb;
 __u8 skew;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 __u8 bootoption;
 __u8 lowsector;
 __u8 nzones;
 __le16 zone_spare;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 __le32 root;
 __le32 disc_size;
 __le16 disc_id;
 __u8 disc_name[10];
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 __le32 disc_type;
 __le32 disc_size_high;
 __u8 log2sharesize:4;
 __u8 unused40:4;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 __u8 big_flag:1;
 __u8 unused41:1;
 __u8 nzones_high;
 __le32 format_version;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 __le32 root_size;
 __u8 unused52[60 - 52];
};
#define ADFS_DISCRECORD (0xc00)
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define ADFS_DR_OFFSET (0x1c0)
#define ADFS_DR_SIZE 60
#define ADFS_DR_SIZE_BITS (ADFS_DR_SIZE << 3)
#endif
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
