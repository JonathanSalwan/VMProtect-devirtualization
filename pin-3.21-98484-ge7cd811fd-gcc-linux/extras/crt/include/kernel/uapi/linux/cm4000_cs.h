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
#ifndef _UAPI_CM4000_H_
#define _UAPI_CM4000_H_
#include <linux/types.h>
#include <linux/ioctl.h>
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define MAX_ATR 33
#define CM4000_MAX_DEV 4
typedef struct atreq {
 __s32 atr_len;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 unsigned char atr[64];
 __s32 power_act;
 unsigned char bIFSD;
 unsigned char bIFSC;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
} atreq_t;
typedef struct ptsreq {
 __u32 protocol;
 unsigned char flags;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 unsigned char pts1;
 unsigned char pts2;
 unsigned char pts3;
} ptsreq_t;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define CM_IOC_MAGIC 'c'
#define CM_IOC_MAXNR 255
#define CM_IOCGSTATUS _IOR (CM_IOC_MAGIC, 0, unsigned char *)
#define CM_IOCGATR _IOWR(CM_IOC_MAGIC, 1, atreq_t *)
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define CM_IOCSPTS _IOW (CM_IOC_MAGIC, 2, ptsreq_t *)
#define CM_IOCSRDR _IO (CM_IOC_MAGIC, 3)
#define CM_IOCARDOFF _IO (CM_IOC_MAGIC, 4)
#define CM_IOSDBGLVL _IOW(CM_IOC_MAGIC, 250, int*)
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define CM_CARD_INSERTED 0x01
#define CM_CARD_POWERED 0x02
#define CM_ATR_PRESENT 0x04
#define CM_ATR_VALID 0x08
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define CM_STATE_VALID 0x0f
#define CM_NO_READER 0x10
#define CM_BAD_CARD 0x20
#endif
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
