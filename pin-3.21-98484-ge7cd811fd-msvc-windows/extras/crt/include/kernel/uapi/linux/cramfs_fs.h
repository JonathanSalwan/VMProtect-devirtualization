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
#ifndef _UAPI__CRAMFS_H
#define _UAPI__CRAMFS_H
#include <linux/types.h>
#include <linux/magic.h>
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define CRAMFS_SIGNATURE "Compressed ROMFS"
#define CRAMFS_MODE_WIDTH 16
#define CRAMFS_UID_WIDTH 16
#define CRAMFS_SIZE_WIDTH 24
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define CRAMFS_GID_WIDTH 8
#define CRAMFS_NAMELEN_WIDTH 6
#define CRAMFS_OFFSET_WIDTH 26
#define CRAMFS_MAXPATHLEN (((1 << CRAMFS_NAMELEN_WIDTH) - 1) << 2)
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
struct cramfs_inode {
 __u32 mode:CRAMFS_MODE_WIDTH, uid:CRAMFS_UID_WIDTH;
 __u32 size:CRAMFS_SIZE_WIDTH, gid:CRAMFS_GID_WIDTH;
 __u32 namelen:CRAMFS_NAMELEN_WIDTH, offset:CRAMFS_OFFSET_WIDTH;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
};
struct cramfs_info {
 __u32 crc;
 __u32 edition;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 __u32 blocks;
 __u32 files;
};
struct cramfs_super {
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 __u32 magic;
 __u32 size;
 __u32 flags;
 __u32 future;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 __u8 signature[16];
 struct cramfs_info fsid;
 __u8 name[16];
 struct cramfs_inode root;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
};
#define CRAMFS_FLAG_FSID_VERSION_2 0x00000001
#define CRAMFS_FLAG_SORTED_DIRS 0x00000002
#define CRAMFS_FLAG_HOLES 0x00000100
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define CRAMFS_FLAG_WRONG_SIGNATURE 0x00000200
#define CRAMFS_FLAG_SHIFTED_ROOT_OFFSET 0x00000400
#define CRAMFS_SUPPORTED_FLAGS ( 0x000000ff   | CRAMFS_FLAG_HOLES   | CRAMFS_FLAG_WRONG_SIGNATURE   | CRAMFS_FLAG_SHIFTED_ROOT_OFFSET )
#endif
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
