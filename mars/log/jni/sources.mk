TEMP_LOCAL_PATH :=$(call my-dir)

include $(TEMP_LOCAL_PATH)/../../mk_template/flags.mk

LOCAL_CFLAGS += -Wno-error=maybe-uninitialized  # x86 compile: ini.h

SRC := $(wildcard $(TEMP_LOCAL_PATH)/../src/loglogic/*.cc)
SRC := $(SRC:$(LOCAL_PATH)/%=%)
LOCAL_SRC_FILES += $(SRC)

SRC := $(wildcard $(TEMP_LOCAL_PATH)/../src/*.cc)
SRC := $(SRC:$(LOCAL_PATH)/%=%)
LOCAL_SRC_FILES += $(SRC)

SRC := $(wildcard $(TEMP_LOCAL_PATH)/../interface/*.cc)
SRC := $(SRC:$(LOCAL_PATH)/%=%)
LOCAL_SRC_FILES += $(SRC)

SRC := $(wildcard $(TEMP_LOCAL_PATH)/../jni/*.cc)
SRC := $(SRC:$(LOCAL_PATH)/%=%)
LOCAL_SRC_FILES += $(SRC)

SRC := $(LOCAL_PATH)/../../comm/xlogger/xloggerbase.c
SRC := $(SRC:$(LOCAL_PATH)/%=%)
LOCAL_SRC_FILES += $(SRC)


LOCAL_C_INCLUDES += $(TEMP_LOCAL_PATH)/../ $(TEMP_LOCAL_PATH)/../src $(TEMP_LOCAL_PATH)/../../ $(TEMP_LOCAL_PATH)/../../../
LOCAL_LDFLAGS += -Wl,--version-script=$(TEMP_LOCAL_PATH)/export.exp
