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
#ifndef _UAPI_LINUX_BTRFS_H
#define _UAPI_LINUX_BTRFS_H
#include <linux/types.h>
#include <linux/ioctl.h>
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define BTRFS_IOCTL_MAGIC 0x94
#define BTRFS_VOL_NAME_MAX 255
#define BTRFS_PATH_NAME_MAX 4087
struct btrfs_ioctl_vol_args {
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 __s64 fd;
 char name[BTRFS_PATH_NAME_MAX + 1];
};
#define BTRFS_DEVICE_PATH_NAME_MAX 1024
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define BTRFS_SUBVOL_CREATE_ASYNC (1ULL << 0)
#define BTRFS_SUBVOL_RDONLY (1ULL << 1)
#define BTRFS_SUBVOL_QGROUP_INHERIT (1ULL << 2)
#define BTRFS_FSID_SIZE 16
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define BTRFS_UUID_SIZE 16
#define BTRFS_QGROUP_INHERIT_SET_LIMITS (1ULL << 0)
struct btrfs_qgroup_limit {
 __u64 flags;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 __u64 max_rfer;
 __u64 max_excl;
 __u64 rsv_rfer;
 __u64 rsv_excl;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
};
struct btrfs_qgroup_inherit {
 __u64 flags;
 __u64 num_qgroups;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 __u64 num_ref_copies;
 __u64 num_excl_copies;
 struct btrfs_qgroup_limit lim;
 __u64 qgroups[0];
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
};
struct btrfs_ioctl_qgroup_limit_args {
 __u64 qgroupid;
 struct btrfs_qgroup_limit lim;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
};
#define BTRFS_SUBVOL_NAME_MAX 4039
struct btrfs_ioctl_vol_args_v2 {
 __s64 fd;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 __u64 transid;
 __u64 flags;
 union {
 struct {
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 __u64 size;
 struct btrfs_qgroup_inherit __user *qgroup_inherit;
 };
 __u64 unused[4];
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 };
 char name[BTRFS_SUBVOL_NAME_MAX + 1];
};
struct btrfs_scrub_progress {
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 __u64 data_extents_scrubbed;
 __u64 tree_extents_scrubbed;
 __u64 data_bytes_scrubbed;
 __u64 tree_bytes_scrubbed;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 __u64 read_errors;
 __u64 csum_errors;
 __u64 verify_errors;
 __u64 no_csum;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 __u64 csum_discards;
 __u64 super_errors;
 __u64 malloc_errors;
 __u64 uncorrectable_errors;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 __u64 corrected_errors;
 __u64 last_physical;
 __u64 unverified_errors;
};
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define BTRFS_SCRUB_READONLY 1
struct btrfs_ioctl_scrub_args {
 __u64 devid;
 __u64 start;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 __u64 end;
 __u64 flags;
 struct btrfs_scrub_progress progress;
 __u64 unused[(1024-32-sizeof(struct btrfs_scrub_progress))/8];
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
};
#define BTRFS_IOCTL_DEV_REPLACE_CONT_READING_FROM_SRCDEV_MODE_ALWAYS 0
#define BTRFS_IOCTL_DEV_REPLACE_CONT_READING_FROM_SRCDEV_MODE_AVOID 1
struct btrfs_ioctl_dev_replace_start_params {
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 __u64 srcdevid;
 __u64 cont_reading_from_srcdev_mode;
 __u8 srcdev_name[BTRFS_DEVICE_PATH_NAME_MAX + 1];
 __u8 tgtdev_name[BTRFS_DEVICE_PATH_NAME_MAX + 1];
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
};
#define BTRFS_IOCTL_DEV_REPLACE_STATE_NEVER_STARTED 0
#define BTRFS_IOCTL_DEV_REPLACE_STATE_STARTED 1
#define BTRFS_IOCTL_DEV_REPLACE_STATE_FINISHED 2
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define BTRFS_IOCTL_DEV_REPLACE_STATE_CANCELED 3
#define BTRFS_IOCTL_DEV_REPLACE_STATE_SUSPENDED 4
struct btrfs_ioctl_dev_replace_status_params {
 __u64 replace_state;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 __u64 progress_1000;
 __u64 time_started;
 __u64 time_stopped;
 __u64 num_write_errors;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 __u64 num_uncorrectable_read_errors;
};
#define BTRFS_IOCTL_DEV_REPLACE_CMD_START 0
#define BTRFS_IOCTL_DEV_REPLACE_CMD_STATUS 1
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define BTRFS_IOCTL_DEV_REPLACE_CMD_CANCEL 2
#define BTRFS_IOCTL_DEV_REPLACE_RESULT_NO_ERROR 0
#define BTRFS_IOCTL_DEV_REPLACE_RESULT_NOT_STARTED 1
#define BTRFS_IOCTL_DEV_REPLACE_RESULT_ALREADY_STARTED 2
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
struct btrfs_ioctl_dev_replace_args {
 __u64 cmd;
 __u64 result;
 union {
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 struct btrfs_ioctl_dev_replace_start_params start;
 struct btrfs_ioctl_dev_replace_status_params status;
 };
 __u64 spare[64];
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
};
struct btrfs_ioctl_dev_info_args {
 __u64 devid;
 __u8 uuid[BTRFS_UUID_SIZE];
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 __u64 bytes_used;
 __u64 total_bytes;
 __u64 unused[379];
 __u8 path[BTRFS_DEVICE_PATH_NAME_MAX];
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
};
struct btrfs_ioctl_fs_info_args {
 __u64 max_id;
 __u64 num_devices;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 __u8 fsid[BTRFS_FSID_SIZE];
 __u64 reserved[124];
};
struct btrfs_ioctl_feature_flags {
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 __u64 compat_flags;
 __u64 compat_ro_flags;
 __u64 incompat_flags;
};
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define BTRFS_BALANCE_CTL_PAUSE 1
#define BTRFS_BALANCE_CTL_CANCEL 2
struct btrfs_balance_args {
 __u64 profiles;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 __u64 usage;
 __u64 devid;
 __u64 pstart;
 __u64 pend;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 __u64 vstart;
 __u64 vend;
 __u64 target;
 __u64 flags;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 __u64 unused[8];
} __attribute__ ((__packed__));
struct btrfs_balance_progress {
 __u64 expected;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 __u64 considered;
 __u64 completed;
};
#define BTRFS_BALANCE_STATE_RUNNING (1ULL << 0)
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define BTRFS_BALANCE_STATE_PAUSE_REQ (1ULL << 1)
#define BTRFS_BALANCE_STATE_CANCEL_REQ (1ULL << 2)
struct btrfs_ioctl_balance_args {
 __u64 flags;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 __u64 state;
 struct btrfs_balance_args data;
 struct btrfs_balance_args meta;
 struct btrfs_balance_args sys;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 struct btrfs_balance_progress stat;
 __u64 unused[72];
};
#define BTRFS_INO_LOOKUP_PATH_MAX 4080
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
struct btrfs_ioctl_ino_lookup_args {
 __u64 treeid;
 __u64 objectid;
 char name[BTRFS_INO_LOOKUP_PATH_MAX];
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
};
struct btrfs_ioctl_search_key {
 __u64 tree_id;
 __u64 min_objectid;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 __u64 max_objectid;
 __u64 min_offset;
 __u64 max_offset;
 __u64 min_transid;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 __u64 max_transid;
 __u32 min_type;
 __u32 max_type;
 __u32 nr_items;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 __u32 unused;
 __u64 unused1;
 __u64 unused2;
 __u64 unused3;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 __u64 unused4;
};
struct btrfs_ioctl_search_header {
 __u64 transid;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 __u64 objectid;
 __u64 offset;
 __u32 type;
 __u32 len;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
};
#define BTRFS_SEARCH_ARGS_BUFSIZE (4096 - sizeof(struct btrfs_ioctl_search_key))
struct btrfs_ioctl_search_args {
 struct btrfs_ioctl_search_key key;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 char buf[BTRFS_SEARCH_ARGS_BUFSIZE];
};
struct btrfs_ioctl_clone_range_args {
 __s64 src_fd;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 __u64 src_offset, src_length;
 __u64 dest_offset;
};
#define BTRFS_DEFRAG_RANGE_COMPRESS 1
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define BTRFS_DEFRAG_RANGE_START_IO 2
#define BTRFS_SAME_DATA_DIFFERS 1
struct btrfs_ioctl_same_extent_info {
 __s64 fd;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 __u64 logical_offset;
 __u64 bytes_deduped;
 __s32 status;
 __u32 reserved;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
};
struct btrfs_ioctl_same_args {
 __u64 logical_offset;
 __u64 length;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 __u16 dest_count;
 __u16 reserved1;
 __u32 reserved2;
 struct btrfs_ioctl_same_extent_info info[0];
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
};
struct btrfs_ioctl_space_info {
 __u64 flags;
 __u64 total_bytes;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 __u64 used_bytes;
};
struct btrfs_ioctl_space_args {
 __u64 space_slots;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 __u64 total_spaces;
 struct btrfs_ioctl_space_info spaces[0];
};
struct btrfs_data_container {
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 __u32 bytes_left;
 __u32 bytes_missing;
 __u32 elem_cnt;
 __u32 elem_missed;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 __u64 val[0];
};
struct btrfs_ioctl_ino_path_args {
 __u64 inum;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 __u64 size;
 __u64 reserved[4];
 __u64 fspath;
};
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
struct btrfs_ioctl_logical_ino_args {
 __u64 logical;
 __u64 size;
 __u64 reserved[4];
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 __u64 inodes;
};
enum btrfs_dev_stat_values {
 BTRFS_DEV_STAT_WRITE_ERRS,
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 BTRFS_DEV_STAT_READ_ERRS,
 BTRFS_DEV_STAT_FLUSH_ERRS,
 BTRFS_DEV_STAT_CORRUPTION_ERRS,
 BTRFS_DEV_STAT_GENERATION_ERRS,
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 BTRFS_DEV_STAT_VALUES_MAX
};
#define BTRFS_DEV_STATS_RESET (1ULL << 0)
struct btrfs_ioctl_get_dev_stats {
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 __u64 devid;
 __u64 nr_items;
 __u64 flags;
 __u64 values[BTRFS_DEV_STAT_VALUES_MAX];
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 __u64 unused[128 - 2 - BTRFS_DEV_STAT_VALUES_MAX];
};
#define BTRFS_QUOTA_CTL_ENABLE 1
#define BTRFS_QUOTA_CTL_DISABLE 2
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define BTRFS_QUOTA_CTL_RESCAN__NOTUSED 3
struct btrfs_ioctl_quota_ctl_args {
 __u64 cmd;
 __u64 status;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
};
struct btrfs_ioctl_quota_rescan_args {
 __u64 flags;
 __u64 progress;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 __u64 reserved[6];
};
struct btrfs_ioctl_qgroup_assign_args {
 __u64 assign;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 __u64 src;
 __u64 dst;
};
struct btrfs_ioctl_qgroup_create_args {
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 __u64 create;
 __u64 qgroupid;
};
struct btrfs_ioctl_timespec {
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 __u64 sec;
 __u32 nsec;
};
struct btrfs_ioctl_received_subvol_args {
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 char uuid[BTRFS_UUID_SIZE];
 __u64 stransid;
 __u64 rtransid;
 struct btrfs_ioctl_timespec stime;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 struct btrfs_ioctl_timespec rtime;
 __u64 flags;
 __u64 reserved[16];
};
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define BTRFS_SEND_FLAG_NO_FILE_DATA 0x1
#define BTRFS_SEND_FLAG_OMIT_STREAM_HEADER 0x2
#define BTRFS_SEND_FLAG_OMIT_END_CMD 0x4
#define BTRFS_SEND_FLAG_MASK   (BTRFS_SEND_FLAG_NO_FILE_DATA |   BTRFS_SEND_FLAG_OMIT_STREAM_HEADER |   BTRFS_SEND_FLAG_OMIT_END_CMD)
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
struct btrfs_ioctl_send_args {
 __s64 send_fd;
 __u64 clone_sources_count;
 __u64 __user *clone_sources;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 __u64 parent_root;
 __u64 flags;
 __u64 reserved[4];
};
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
enum btrfs_err_code {
 notused,
 BTRFS_ERROR_DEV_RAID1_MIN_NOT_MET,
 BTRFS_ERROR_DEV_RAID10_MIN_NOT_MET,
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 BTRFS_ERROR_DEV_RAID5_MIN_NOT_MET,
 BTRFS_ERROR_DEV_RAID6_MIN_NOT_MET,
 BTRFS_ERROR_DEV_TGT_REPLACE,
 BTRFS_ERROR_DEV_MISSING_NOT_FOUND,
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 BTRFS_ERROR_DEV_ONLY_WRITABLE,
 BTRFS_ERROR_DEV_EXCL_RUN_IN_PROGRESS
};
#define BTRFS_IOC_SNAP_CREATE _IOW(BTRFS_IOCTL_MAGIC, 1,   struct btrfs_ioctl_vol_args)
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define BTRFS_IOC_DEFRAG _IOW(BTRFS_IOCTL_MAGIC, 2,   struct btrfs_ioctl_vol_args)
#define BTRFS_IOC_RESIZE _IOW(BTRFS_IOCTL_MAGIC, 3,   struct btrfs_ioctl_vol_args)
#define BTRFS_IOC_SCAN_DEV _IOW(BTRFS_IOCTL_MAGIC, 4,   struct btrfs_ioctl_vol_args)
#define BTRFS_IOC_TRANS_START _IO(BTRFS_IOCTL_MAGIC, 6)
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define BTRFS_IOC_TRANS_END _IO(BTRFS_IOCTL_MAGIC, 7)
#define BTRFS_IOC_SYNC _IO(BTRFS_IOCTL_MAGIC, 8)
#define BTRFS_IOC_CLONE _IOW(BTRFS_IOCTL_MAGIC, 9, int)
#define BTRFS_IOC_ADD_DEV _IOW(BTRFS_IOCTL_MAGIC, 10,   struct btrfs_ioctl_vol_args)
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define BTRFS_IOC_RM_DEV _IOW(BTRFS_IOCTL_MAGIC, 11,   struct btrfs_ioctl_vol_args)
#define BTRFS_IOC_BALANCE _IOW(BTRFS_IOCTL_MAGIC, 12,   struct btrfs_ioctl_vol_args)
#define BTRFS_IOC_CLONE_RANGE _IOW(BTRFS_IOCTL_MAGIC, 13,   struct btrfs_ioctl_clone_range_args)
#define BTRFS_IOC_SUBVOL_CREATE _IOW(BTRFS_IOCTL_MAGIC, 14,   struct btrfs_ioctl_vol_args)
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define BTRFS_IOC_SNAP_DESTROY _IOW(BTRFS_IOCTL_MAGIC, 15,   struct btrfs_ioctl_vol_args)
#define BTRFS_IOC_DEFRAG_RANGE _IOW(BTRFS_IOCTL_MAGIC, 16,   struct btrfs_ioctl_defrag_range_args)
#define BTRFS_IOC_TREE_SEARCH _IOWR(BTRFS_IOCTL_MAGIC, 17,   struct btrfs_ioctl_search_args)
#define BTRFS_IOC_INO_LOOKUP _IOWR(BTRFS_IOCTL_MAGIC, 18,   struct btrfs_ioctl_ino_lookup_args)
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define BTRFS_IOC_DEFAULT_SUBVOL _IOW(BTRFS_IOCTL_MAGIC, 19, __u64)
#define BTRFS_IOC_SPACE_INFO _IOWR(BTRFS_IOCTL_MAGIC, 20,   struct btrfs_ioctl_space_args)
#define BTRFS_IOC_START_SYNC _IOR(BTRFS_IOCTL_MAGIC, 24, __u64)
#define BTRFS_IOC_WAIT_SYNC _IOW(BTRFS_IOCTL_MAGIC, 22, __u64)
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define BTRFS_IOC_SNAP_CREATE_V2 _IOW(BTRFS_IOCTL_MAGIC, 23,   struct btrfs_ioctl_vol_args_v2)
#define BTRFS_IOC_SUBVOL_CREATE_V2 _IOW(BTRFS_IOCTL_MAGIC, 24,   struct btrfs_ioctl_vol_args_v2)
#define BTRFS_IOC_SUBVOL_GETFLAGS _IOR(BTRFS_IOCTL_MAGIC, 25, __u64)
#define BTRFS_IOC_SUBVOL_SETFLAGS _IOW(BTRFS_IOCTL_MAGIC, 26, __u64)
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define BTRFS_IOC_SCRUB _IOWR(BTRFS_IOCTL_MAGIC, 27,   struct btrfs_ioctl_scrub_args)
#define BTRFS_IOC_SCRUB_CANCEL _IO(BTRFS_IOCTL_MAGIC, 28)
#define BTRFS_IOC_SCRUB_PROGRESS _IOWR(BTRFS_IOCTL_MAGIC, 29,   struct btrfs_ioctl_scrub_args)
#define BTRFS_IOC_DEV_INFO _IOWR(BTRFS_IOCTL_MAGIC, 30,   struct btrfs_ioctl_dev_info_args)
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define BTRFS_IOC_FS_INFO _IOR(BTRFS_IOCTL_MAGIC, 31,   struct btrfs_ioctl_fs_info_args)
#define BTRFS_IOC_BALANCE_V2 _IOWR(BTRFS_IOCTL_MAGIC, 32,   struct btrfs_ioctl_balance_args)
#define BTRFS_IOC_BALANCE_CTL _IOW(BTRFS_IOCTL_MAGIC, 33, int)
#define BTRFS_IOC_BALANCE_PROGRESS _IOR(BTRFS_IOCTL_MAGIC, 34,   struct btrfs_ioctl_balance_args)
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define BTRFS_IOC_INO_PATHS _IOWR(BTRFS_IOCTL_MAGIC, 35,   struct btrfs_ioctl_ino_path_args)
#define BTRFS_IOC_LOGICAL_INO _IOWR(BTRFS_IOCTL_MAGIC, 36,   struct btrfs_ioctl_ino_path_args)
#define BTRFS_IOC_SET_RECEIVED_SUBVOL _IOWR(BTRFS_IOCTL_MAGIC, 37,   struct btrfs_ioctl_received_subvol_args)
#define BTRFS_IOC_SEND _IOW(BTRFS_IOCTL_MAGIC, 38, struct btrfs_ioctl_send_args)
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define BTRFS_IOC_DEVICES_READY _IOR(BTRFS_IOCTL_MAGIC, 39,   struct btrfs_ioctl_vol_args)
#define BTRFS_IOC_QUOTA_CTL _IOWR(BTRFS_IOCTL_MAGIC, 40,   struct btrfs_ioctl_quota_ctl_args)
#define BTRFS_IOC_QGROUP_ASSIGN _IOW(BTRFS_IOCTL_MAGIC, 41,   struct btrfs_ioctl_qgroup_assign_args)
#define BTRFS_IOC_QGROUP_CREATE _IOW(BTRFS_IOCTL_MAGIC, 42,   struct btrfs_ioctl_qgroup_create_args)
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define BTRFS_IOC_QGROUP_LIMIT _IOR(BTRFS_IOCTL_MAGIC, 43,   struct btrfs_ioctl_qgroup_limit_args)
#define BTRFS_IOC_QUOTA_RESCAN _IOW(BTRFS_IOCTL_MAGIC, 44,   struct btrfs_ioctl_quota_rescan_args)
#define BTRFS_IOC_QUOTA_RESCAN_STATUS _IOR(BTRFS_IOCTL_MAGIC, 45,   struct btrfs_ioctl_quota_rescan_args)
#define BTRFS_IOC_QUOTA_RESCAN_WAIT _IO(BTRFS_IOCTL_MAGIC, 46)
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define BTRFS_IOC_GET_FSLABEL _IOR(BTRFS_IOCTL_MAGIC, 49,   char[BTRFS_LABEL_SIZE])
#define BTRFS_IOC_SET_FSLABEL _IOW(BTRFS_IOCTL_MAGIC, 50,   char[BTRFS_LABEL_SIZE])
#define BTRFS_IOC_GET_DEV_STATS _IOWR(BTRFS_IOCTL_MAGIC, 52,   struct btrfs_ioctl_get_dev_stats)
#define BTRFS_IOC_DEV_REPLACE _IOWR(BTRFS_IOCTL_MAGIC, 53,   struct btrfs_ioctl_dev_replace_args)
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define BTRFS_IOC_FILE_EXTENT_SAME _IOWR(BTRFS_IOCTL_MAGIC, 54,   struct btrfs_ioctl_same_args)
#define BTRFS_IOC_GET_FEATURES _IOR(BTRFS_IOCTL_MAGIC, 57,   struct btrfs_ioctl_feature_flags)
#define BTRFS_IOC_SET_FEATURES _IOW(BTRFS_IOCTL_MAGIC, 57,   struct btrfs_ioctl_feature_flags[2])
#define BTRFS_IOC_GET_SUPPORTED_FEATURES _IOR(BTRFS_IOCTL_MAGIC, 57,   struct btrfs_ioctl_feature_flags[3])
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#endif
