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
#ifndef _UAPI_LINUX_ANDROID_ALARM_H
#define _UAPI_LINUX_ANDROID_ALARM_H
#include <linux/ioctl.h>
#include <linux/time.h>
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
enum android_alarm_type {
 ANDROID_ALARM_RTC_WAKEUP,
 ANDROID_ALARM_RTC,
 ANDROID_ALARM_ELAPSED_REALTIME_WAKEUP,
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 ANDROID_ALARM_ELAPSED_REALTIME,
 ANDROID_ALARM_SYSTEMTIME,
 ANDROID_ALARM_TYPE_COUNT,
};
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
enum android_alarm_return_flags {
 ANDROID_ALARM_RTC_WAKEUP_MASK = 1U << ANDROID_ALARM_RTC_WAKEUP,
 ANDROID_ALARM_RTC_MASK = 1U << ANDROID_ALARM_RTC,
 ANDROID_ALARM_ELAPSED_REALTIME_WAKEUP_MASK =
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 1U << ANDROID_ALARM_ELAPSED_REALTIME_WAKEUP,
 ANDROID_ALARM_ELAPSED_REALTIME_MASK =
 1U << ANDROID_ALARM_ELAPSED_REALTIME,
 ANDROID_ALARM_SYSTEMTIME_MASK = 1U << ANDROID_ALARM_SYSTEMTIME,
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 ANDROID_ALARM_TIME_CHANGE_MASK = 1U << 16
};
#define ANDROID_ALARM_CLEAR(type) _IO('a', 0 | ((type) << 4))
#define ANDROID_ALARM_WAIT _IO('a', 1)
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define ALARM_IOW(c, type, size) _IOW('a', (c) | ((type) << 4), size)
#define ANDROID_ALARM_SET(type) ALARM_IOW(2, type, struct timespec)
#define ANDROID_ALARM_SET_AND_WAIT(type) ALARM_IOW(3, type, struct timespec)
#define ANDROID_ALARM_GET_TIME(type) ALARM_IOW(4, type, struct timespec)
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define ANDROID_ALARM_SET_RTC _IOW('a', 5, struct timespec)
#define ANDROID_ALARM_BASE_CMD(cmd) (cmd & ~(_IOC(0, 0, 0xf0, 0)))
#define ANDROID_ALARM_IOCTL_TO_TYPE(cmd) (_IOC_NR(cmd) >> 4)
#endif
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */

