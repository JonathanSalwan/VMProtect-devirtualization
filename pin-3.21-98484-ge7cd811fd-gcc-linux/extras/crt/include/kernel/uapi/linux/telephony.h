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
#ifndef TELEPHONY_H
#define TELEPHONY_H
#define TELEPHONY_VERSION 3013
#define PHONE_VENDOR_IXJ 1
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define PHONE_VENDOR_QUICKNET PHONE_VENDOR_IXJ
#define PHONE_VENDOR_VOICETRONIX 2
#define PHONE_VENDOR_ACULAB 3
#define PHONE_VENDOR_DIGI 4
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define PHONE_VENDOR_FRANKLIN 5
#define QTI_PHONEJACK 100
#define QTI_LINEJACK 300
#define QTI_PHONEJACK_LITE 400
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define QTI_PHONEJACK_PCI 500
#define QTI_PHONECARD 600
typedef enum {
 vendor = 0,
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 device,
 port,
 codec,
 dsp
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
} phone_cap;
struct phone_capability {
 char desc[80];
 phone_cap captype;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 int cap;
 int handle;
};
typedef enum {
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 pots = 0,
 pstn,
 handset,
 speaker
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
} phone_ports;
#define PHONE_CAPABILITIES _IO ('q', 0x80)
#define PHONE_CAPABILITIES_LIST _IOR ('q', 0x81, struct phone_capability *)
#define PHONE_CAPABILITIES_CHECK _IOW ('q', 0x82, struct phone_capability *)
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
typedef struct {
 char month[3];
 char day[3];
 char hour[3];
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 char min[3];
 int numlen;
 char number[11];
 int namelen;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 char name[80];
} PHONE_CID;
#define PHONE_RING _IO ('q', 0x83)
#define PHONE_HOOKSTATE _IO ('q', 0x84)
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define PHONE_MAXRINGS _IOW ('q', 0x85, char)
#define PHONE_RING_CADENCE _IOW ('q', 0x86, short)
#define OLD_PHONE_RING_START _IO ('q', 0x87)
#define PHONE_RING_START _IOW ('q', 0x87, PHONE_CID *)
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define PHONE_RING_STOP _IO ('q', 0x88)
#define USA_RING_CADENCE 0xC0C0
#define PHONE_REC_CODEC _IOW ('q', 0x89, int)
#define PHONE_REC_START _IO ('q', 0x8A)
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define PHONE_REC_STOP _IO ('q', 0x8B)
#define PHONE_REC_DEPTH _IOW ('q', 0x8C, int)
#define PHONE_FRAME _IOW ('q', 0x8D, int)
#define PHONE_REC_VOLUME _IOW ('q', 0x8E, int)
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define PHONE_REC_VOLUME_LINEAR _IOW ('q', 0xDB, int)
#define PHONE_REC_LEVEL _IO ('q', 0x8F)
#define PHONE_PLAY_CODEC _IOW ('q', 0x90, int)
#define PHONE_PLAY_START _IO ('q', 0x91)
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define PHONE_PLAY_STOP _IO ('q', 0x92)
#define PHONE_PLAY_DEPTH _IOW ('q', 0x93, int)
#define PHONE_PLAY_VOLUME _IOW ('q', 0x94, int)
#define PHONE_PLAY_VOLUME_LINEAR _IOW ('q', 0xDC, int)
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define PHONE_PLAY_LEVEL _IO ('q', 0x95)
#define PHONE_DTMF_READY _IOR ('q', 0x96, int)
#define PHONE_GET_DTMF _IOR ('q', 0x97, int)
#define PHONE_GET_DTMF_ASCII _IOR ('q', 0x98, int)
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define PHONE_DTMF_OOB _IOW ('q', 0x99, int)
#define PHONE_EXCEPTION _IOR ('q', 0x9A, int)
#define PHONE_PLAY_TONE _IOW ('q', 0x9B, char)
#define PHONE_SET_TONE_ON_TIME _IOW ('q', 0x9C, int)
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define PHONE_SET_TONE_OFF_TIME _IOW ('q', 0x9D, int)
#define PHONE_GET_TONE_ON_TIME _IO ('q', 0x9E)
#define PHONE_GET_TONE_OFF_TIME _IO ('q', 0x9F)
#define PHONE_GET_TONE_STATE _IO ('q', 0xA0)
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define PHONE_BUSY _IO ('q', 0xA1)
#define PHONE_RINGBACK _IO ('q', 0xA2)
#define PHONE_DIALTONE _IO ('q', 0xA3)
#define PHONE_CPT_STOP _IO ('q', 0xA4)
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define PHONE_PSTN_SET_STATE _IOW ('q', 0xA4, int)
#define PHONE_PSTN_GET_STATE _IO ('q', 0xA5)
#define PSTN_ON_HOOK 0
#define PSTN_RINGING 1
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define PSTN_OFF_HOOK 2
#define PSTN_PULSE_DIAL 3
#define PHONE_WINK_DURATION _IOW ('q', 0xA6, int)
#define PHONE_WINK _IOW ('q', 0xAA, int)
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
typedef enum {
 G723_63 = 1,
 G723_53 = 2,
 TS85 = 3,
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 TS48 = 4,
 TS41 = 5,
 G728 = 6,
 G729 = 7,
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 ULAW = 8,
 ALAW = 9,
 LINEAR16 = 10,
 LINEAR8 = 11,
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 WSS = 12,
 G729B = 13
} phone_codec;
struct phone_codec_data
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
{
 phone_codec type;
 unsigned short buf_min, buf_opt, buf_max;
};
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define PHONE_QUERY_CODEC _IOWR ('q', 0xA7, struct phone_codec_data *)
#define PHONE_PSTN_LINETEST _IO ('q', 0xA8)
#define PHONE_VAD _IOW ('q', 0xA9, int)
struct phone_except
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
{
 unsigned int dtmf_ready:1;
 unsigned int hookstate:1;
 unsigned int pstn_ring:1;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 unsigned int caller_id:1;
 unsigned int pstn_wink:1;
 unsigned int f0:1;
 unsigned int f1:1;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 unsigned int f2:1;
 unsigned int f3:1;
 unsigned int flash:1;
 unsigned int fc0:1;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 unsigned int fc1:1;
 unsigned int fc2:1;
 unsigned int fc3:1;
 unsigned int reserved:18;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
};
union telephony_exception {
 struct phone_except bits;
 unsigned int bytes;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
};
#endif
