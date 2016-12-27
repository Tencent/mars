
TEMP_LOCAL_PATH :=$(call my-dir)

LOCAL_C_INCLUDES += $(TEMP_LOCAL_PATH)/../src
LOCAL_C_INCLUDES += $(TEMP_LOCAL_PATH)/../
LOCAL_C_INCLUDES += $(TEMP_LOCAL_PATH)/../../ $(TEMP_LOCAL_PATH)/../../../ $(TEMP_LOCAL_PATH)/../src/activecheck 
LOCAL_C_INCLUDES += $(TEMP_LOCAL_PATH)/../src/ $(TEMP_LOCAL_PATH)/../src/checkimpl $(TEMP_LOCAL_PATH)/../src/remotecheck $(TEMP_LOCAL_PATH)/../src/tools

include $(TEMP_LOCAL_PATH)/../../mk_template/flags.mk
include $(LOCAL_PATH)/../../mk_template/util.mk

LOCAL_CFLAGS += -fvisibility=hidden

SRC := $(wildcard $(TEMP_LOCAL_PATH)/../*.cc)
SRC := $(SRC:$(LOCAL_PATH)/%=%)
LOCAL_SRC_FILES += $(SRC)

SRC := $(wildcard $(TEMP_LOCAL_PATH)/../src/activecheck/*.cc)
SRC := $(SRC:$(LOCAL_PATH)/%=%)
LOCAL_SRC_FILES += $(SRC)

SRC := $(wildcard $(TEMP_LOCAL_PATH)/../src/checkimpl/*.cc)
SRC := $(SRC:$(LOCAL_PATH)/%=%)
LOCAL_SRC_FILES += $(SRC)

SRC := $(wildcard $(TEMP_LOCAL_PATH)/../src/tools/*.cc)
SRC := $(SRC:$(LOCAL_PATH)/%=%)
LOCAL_SRC_FILES += $(SRC)

SRC := $(wildcard $(TEMP_LOCAL_PATH)/../src/*.cc)
SRC := $(SRC:$(LOCAL_PATH)/%=%)
LOCAL_SRC_FILES += $(SRC)
SRC := $(wildcard $(TEMP_LOCAL_PATH)/../jni/*.cc)
SRC := $(SRC:$(LOCAL_PATH)/%=%)
LOCAL_SRC_FILES += $(SRC)