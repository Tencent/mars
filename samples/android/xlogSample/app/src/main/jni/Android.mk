LOCAL_PATH := $(call my-dir)

include  $(LOCAL_PATH)/export_include/export.mk


include $(CLEAR_VARS)

LOCAL_MODULE    := native-lib
LOCAL_SRC_FILES += native-lib.cpp

LOCAL_SHARED_LIBRARIES += xlog stl

LOCAL_LDLIBS    += -llog

include $(BUILD_SHARED_LIBRARY)



