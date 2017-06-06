LOCAL_PATH :=$(call my-dir)

include $(CLEAR_VARS)

MARS_LIBS_PATH := ../mars_libs

LOCAL_MODULE := comm
LOCAL_SRC_FILES := $(MARS_LIBS_PATH)/$(TARGET_ARCH_ABI)/libmarscomm.a

include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)

ifneq ($(XLOG_NO_CRYPT),1)

LOCAL_MODULE := openssl_crypto
LOCAL_SRC_FILES := ../../../../../mars/mars/openssl/openssl_lib_android/libcrypto.a

include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)

endif

LOCAL_MODULE := static_xlog
LOCAL_SRC_FILES := $(MARS_LIBS_PATH)/$(TARGET_ARCH_ABI)/libmarsxlog.a

include $(PREBUILT_STATIC_LIBRARY)
include $(CLEAR_VARS)

LOCAL_MODULE := marsxlog

LOCAL_SRC_FILES := JNI_OnLoad.cc import.cc
LOCAL_STATIC_LIBRARIES += static_xlog comm

ifneq ($(XLOG_NO_CRYPT),1)
LOCAL_STATIC_LIBRARIES += openssl_crypto
endif

LOCAL_LDLIBS += -llog -lz
include $(LOCAL_PATH)/../../../mk_template/flags.mk
#LOCAL_CPPFLAGS += -frtti
#LOCAL_CFLAGS += -Werror -Wextra -Wall -Wno-error=conversion -Wno-error=sign-conversion -Werror=sign-compare 
#LOCAL_CFLAGS += -Wno-unused-parameter -Wno-missing-field-initializers 
#LOCAL_CFLAGS +=  -ffunction-sections
LOCAL_LDFLAGS += -Wl,--gc-sections,--version-script=../../log/jni/export.exp 
LOCAL_LDFLAGS += -Wl,--no-whole-archive
LOCAL_C_INCLUDES := $(LOCAL_PATH)/.. $(LOCAL_PATH)/../../.. $(LOCAL_PATH)/../../../..

include $(LOCAL_PATH)/define_macros.mk

include $(BUILD_SHARED_LIBRARY)



