LOCAL_PATH :=$(call my-dir)

include $(CLEAR_VARS)

MARS_LIBS_PATH := ../mars_libs

LOCAL_MODULE := comm
LOCAL_SRC_FILES := $(MARS_LIBS_PATH)/$(TARGET_ARCH_ABI)/libmarscomm.a

include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE := baseevent
LOCAL_SRC_FILES := $(MARS_LIBS_PATH)/$(TARGET_ARCH_ABI)/libmarsbaseevent.a

include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE := appcomm
LOCAL_SRC_FILES := $(MARS_LIBS_PATH)/$(TARGET_ARCH_ABI)/libmarsappcomm.a

include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)


LOCAL_MODULE := crypto
LOCAL_SRC_FILES := $(MARS_LIBS_PATH)/$(TARGET_ARCH_ABI)/libcrypto.a

include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE := stn
LOCAL_SRC_FILES := $(MARS_LIBS_PATH)/$(TARGET_ARCH_ABI)/libmarsstn.a

include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE := sdt
LOCAL_SRC_FILES := $(MARS_LIBS_PATH)/$(TARGET_ARCH_ABI)/libmarssdt.a

include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE := static_xlog
LOCAL_SRC_FILES := $(MARS_LIBS_PATH)/$(TARGET_ARCH_ABI)/libmarsxlog.a

include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)

include $(LOCAL_PATH)/../../../mk_template/flags.mk

LOCAL_MODULE := marsxlog

LOCAL_SRC_FILES := JNI_OnLoad.cc log_crypt.cc import.cc
LOCAL_STATIC_LIBRARIES += static_xlog comm

LOCAL_LDLIBS += -llog -lz
#LOCAL_CPPFLAGS += -frtti
#LOCAL_CFLAGS += -Werror -Wextra -Wall -Wno-error=conversion -Wno-error=sign-conversion -Werror=sign-compare 
#LOCAL_CFLAGS += -Wno-unused-parameter -Wno-missing-field-initializers
#LOCAL_CFLAGS +=  -fdata-sections
LOCAL_LDFLAGS += -Wl,--gc-sections,--version-script=../../log/jni/export.exp 
LOCAL_C_INCLUDES := $(LOCAL_PATH)/.. $(LOCAL_PATH)/../../..

include $(LOCAL_PATH)/define_macros.mk

include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)

include $(LOCAL_PATH)/../../../mk_template/flags.mk

LOCAL_MODULE := marsstn


LOCAL_SRC_FILES := shortlink_packer.cc longlink_packer.cc JNI_OnLoad.cc import.cc

LOCAL_STATIC_LIBRARIES += stn sdt appcomm baseevent comm crypto mmjpeg jpeg_static
LOCAL_SHARED_LIBRARIES += marsxlog

LOCAL_LDLIBS += -llog -lz -ljnigraphics 
#LOCAL_CPPFLAGS += -frtti
#LOCAL_CFLAGS += -Wextra -Wall -Wno-error=conversion -Wno-error=sign-conversion -Werror=sign-compare 
#LOCAL_CFLAGS += -Wno-unused-parameter -Wno-missing-field-initializers 
#LOCAL_CFLAGS +=  -fdata-sections
#LOCAL_CFLAGS +=  -fvisibility=hidden
LOCAL_LDFLAGS += -Wl,--gc-sections,--version-script=jni/export.exp
LOCAL_C_INCLUDES := $(LOCAL_PATH)/.. $(LOCAL_PATH)/../../..

include $(LOCAL_PATH)/define_macros.mk

include $(BUILD_SHARED_LIBRARY)



