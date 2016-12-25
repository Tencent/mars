LOCAL_PATH :=$(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := crypto
#include  $(LOCAL_PATH)/../../mk_template/flags.mk
include $(LOCAL_PATH)/android-config.mk
include  $(LOCAL_PATH)/../sources.mk

$(info 'info LOCAL_SRC_FILES=$(LOCAL_SRC_FILES)')
$(info 'info SCR end----------------------------------')

LOCAL_CFLAGS := $(LOCAL_CFLAGS:-Werror=)
LOCAL_CFLAGS += -fvisibility=hidden

include $(BUILD_STATIC_LIBRARY)
