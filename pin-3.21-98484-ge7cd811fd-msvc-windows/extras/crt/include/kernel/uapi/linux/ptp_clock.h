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
#ifndef _PTP_CLOCK_H_
#define _PTP_CLOCK_H_
#include <linux/ioctl.h>
#include <linux/types.h>
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define PTP_ENABLE_FEATURE (1<<0)
#define PTP_RISING_EDGE (1<<1)
#define PTP_FALLING_EDGE (1<<2)
struct ptp_clock_time {
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 __s64 sec;
 __u32 nsec;
 __u32 reserved;
};
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
struct ptp_clock_caps {
 int max_adj;
 int n_alarm;
 int n_ext_ts;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 int n_per_out;
 int pps;
 int rsv[15];
};
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
struct ptp_extts_request {
 unsigned int index;
 unsigned int flags;
 unsigned int rsv[2];
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
};
struct ptp_perout_request {
 struct ptp_clock_time start;
 struct ptp_clock_time period;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 unsigned int index;
 unsigned int flags;
 unsigned int rsv[4];
};
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define PTP_MAX_SAMPLES 25
struct ptp_sys_offset {
 unsigned int n_samples;
 unsigned int rsv[3];
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 struct ptp_clock_time ts[2 * PTP_MAX_SAMPLES + 1];
};
#define PTP_CLK_MAGIC '='
#define PTP_CLOCK_GETCAPS _IOR(PTP_CLK_MAGIC, 1, struct ptp_clock_caps)
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define PTP_EXTTS_REQUEST _IOW(PTP_CLK_MAGIC, 2, struct ptp_extts_request)
#define PTP_PEROUT_REQUEST _IOW(PTP_CLK_MAGIC, 3, struct ptp_perout_request)
#define PTP_ENABLE_PPS _IOW(PTP_CLK_MAGIC, 4, int)
#define PTP_SYS_OFFSET _IOW(PTP_CLK_MAGIC, 5, struct ptp_sys_offset)
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
struct ptp_extts_event {
 struct ptp_clock_time t;
 unsigned int index;
 unsigned int flags;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 unsigned int rsv[2];
};
#endif
