MY_LOCAL_PATH :=$(call my-dir)

MARS_SRC_ROOT_DIR = $(MY_LOCAL_PATH)/../../..
include $(MARS_SRC_ROOT_DIR)/comm/jni/Android.mk
include $(MARS_SRC_ROOT_DIR)/baseevent/jni/Android.mk
include $(MARS_SRC_ROOT_DIR)/app/jni/Android.mk
include $(MARS_SRC_ROOT_DIR)/openssl/jni/Android.mk
include $(MARS_SRC_ROOT_DIR)/sdt/jni/Android.mk
include $(MARS_SRC_ROOT_DIR)/stn/jni/Android.mk
include $(MARS_SRC_ROOT_DIR)/log/jni/Android.mk

LOCAL_PATH :=$(MY_LOCAL_PATH)

include $(CLEAR_VARS)

include $(LOCAL_PATH)/../../../mk_template/flags.mk

LOCAL_MODULE := marsxlog

LOCAL_SRC_FILES := JNI_OnLoad.cc log_crypt.cc import.cc
LOCAL_STATIC_LIBRARIES += comm
LOCAL_WHOLE_STATIC_LIBRARIES += xlog

LOCAL_LDLIBS += -llog -lz
#LOCAL_CPPFLAGS += -frtti
#LOCAL_CFLAGS += -Werror -Wextra -Wall -Wno-error=conversion -Wno-error=sign-conversion -Werror=sign-compare 
#LOCAL_CFLAGS += -Wno-unused-parameter -Wno-missing-field-initializers
#LOCAL_CFLAGS +=  -fdata-sections
LOCAL_LDFLAGS += -Wl,--gc-sections,--version-script=$(MARS_SRC_ROOT_DIR)/log/jni/export.exp
LOCAL_C_INCLUDES := $(LOCAL_PATH)/.. $(LOCAL_PATH)/../../..

include $(LOCAL_PATH)/define_macros.mk

include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)

include $(LOCAL_PATH)/../../../mk_template/flags.mk

LOCAL_MODULE := marsstn


LOCAL_SRC_FILES := shortlink_packer.cc longlink_packer.cc JNI_OnLoad.cc import.cc

LOCAL_STATIC_LIBRARIES += stn sdt appcomm baseevent comm crypto
LOCAL_SHARED_LIBRARIES += marsxlog

LOCAL_LDLIBS += -llog -lz -ljnigraphics 
#LOCAL_CPPFLAGS += -frtti
#LOCAL_CFLAGS += -Wextra -Wall -Wno-error=conversion -Wno-error=sign-conversion -Werror=sign-compare 
#LOCAL_CFLAGS += -Wno-unused-parameter -Wno-missing-field-initializers 
#LOCAL_CFLAGS +=  -fdata-sections
#LOCAL_CFLAGS +=  -fvisibility=hidden
LOCAL_LDFLAGS += -Wl,--gc-sections,--version-script=$(LOCAL_PATH)/export.exp
LOCAL_C_INCLUDES := $(LOCAL_PATH)/.. $(LOCAL_PATH)/../../..

include $(LOCAL_PATH)/define_macros.mk

include $(BUILD_SHARED_LIBRARY)



