
TEMP_LOCAL_PATH :=$(call my-dir)/..

include $(TEMP_LOCAL_PATH)/../mk_template/flags.mk
include $(LOCAL_PATH)/../../mk_template/util.mk

LOCAL_CFLAGS += -fvisibility=hidden
LOCAL_CFLAGS += -Wno-error=maybe-uninitialized  # x86 compile: ini.h
LOCAL_CFLAGS := $(LOCAL_CFLAGS:-Wconversion=)

SRC := $(wildcard $(TEMP_LOCAL_PATH)/jni/*.cc)
SRC := $(SRC:$(LOCAL_PATH)/%=%)
LOCAL_SRC_FILES += $(SRC)

SRC := $(wildcard $(TEMP_LOCAL_PATH)/src/*.cc)
SRC := $(SRC:$(LOCAL_PATH)/%=%)
LOCAL_SRC_FILES += $(SRC)

SRC := $(wildcard $(TEMP_LOCAL_PATH)/*.cc)
SRC := $(SRC:$(LOCAL_PATH)/%=%)
LOCAL_SRC_FILES += $(SRC)

$(info 'garry=$(LOCAL_SRC_FILES)')

LOCAL_C_INCLUDES += $(TEMP_LOCAL_PATH)/ $(TEMP_LOCAL_PATH)/src $(TEMP_LOCAL_PATH)/../../
