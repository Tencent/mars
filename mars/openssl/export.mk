
TEMP_LOCAL_PATH :=$(call my-dir)

#include $(CLEAR_VARS)  
#LOCAL_MODULE := cryptowrapper
#LOCAL_SRC_FILES := $(TEMP_LOCAL_PATH)/libs/$(TARGET_ARCH_ABI)/libcryptowrapper.so 
#LOCAL_SRC_FILES := $(LOCAL_SRC_FILES:$(LOCAL_PATH)/%=%)
#LOCAL_EXPORT_C_INCLUDES := $(TEMP_LOCAL_PATH)/src/openssl/include
#include $(PREBUILT_SHARED_LIBRARY)

ifeq ($(filter cryptowrapper,$(LOCAL_EXPORT_STATIC_LIBRARIES)),)
include $(CLEAR_VARS)  
LOCAL_MODULE := cryptowrapper
LOCAL_SRC_FILES := $(TEMP_LOCAL_PATH)/obj/local/$(TARGET_ARCH_ABI)/libcryptowrapper.a
LOCAL_SRC_FILES := $(LOCAL_SRC_FILES:$(LOCAL_PATH)/%=%)
LOCAL_EXPORT_C_INCLUDES := $(TEMP_LOCAL_PATH)/export_include
include $(PREBUILT_STATIC_LIBRARY)
endif

LOCAL_EXPORT_STATIC_LIBRARIES += cryptowrapper
