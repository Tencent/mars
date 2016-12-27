LOCAL_PATH :=$(call my-dir)

$(shell python $(LOCAL_PATH)/../copy_log_export.py)

include $(LOCAL_PATH)/../../mk_template/android_template.mk

