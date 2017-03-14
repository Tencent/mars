LOCAL_PATH :=$(call my-dir)

include $(CLEAR_VARS)  
LOCAL_MODULE := openssl_crypto
LOCAL_SRC_FILES := ../openssl_lib_android/libcrypto.a
include $(PREBUILT_STATIC_LIBRARY)


include $(CLEAR_VARS)

LOCAL_MODULE := cryptowrapper
#include  $(LOCAL_PATH)/../../mk_template/flags.mk
include $(LOCAL_PATH)/android-config.mk
include  $(LOCAL_PATH)/../sources.mk

$(info 'info LOCAL_SRC_FILES=$(LOCAL_SRC_FILES)')
$(info 'info SCR end----------------------------------')

LOCAL_CFLAGS := $(LOCAL_CFLAGS:-Werror=)
LOCAL_CFLAGS += -fvisibility=hidden
LOCAL_STATIC_LIBRARIES += openssl_crypto

include $(BUILD_STATIC_LIBRARY)
