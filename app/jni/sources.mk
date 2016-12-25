
TEMP_LOCAL_PATH :=$(call my-dir)

LOCAL_C_INCLUDES += $(TEMP_LOCAL_PATH)/../src   $(TEMP_LOCAL_PATH)/../../log/src/  $(TEMP_LOCAL_PATH)/../

include $(TEMP_LOCAL_PATH)/../../mk_template/flags.mk
LOCAL_CFLAGS += -fvisibility=hidden

SRC := $(wildcard $(TEMP_LOCAL_PATH)/../src/*.cc)
SRC := $(SRC:$(LOCAL_PATH)/%=%)
LOCAL_SRC_FILES += $(SRC)

SRC := $(wildcard $(TEMP_LOCAL_PATH)/../src/unix/*.cc)
SRC := $(SRC:$(LOCAL_PATH)/%=%)
LOCAL_SRC_FILES += $(SRC)

SRC := $(wildcard $(TEMP_LOCAL_PATH)/*.cc)
SRC := $(SRC:$(LOCAL_PATH)/%=%)
LOCAL_SRC_FILES += $(SRC)

LOCAL_C_INCLUDES += $(TEMP_LOCAL_PATH)/../ $(TEMP_LOCAL_PATH)/../src $(TEMP_LOCAL_PATH)/../../ $(TEMP_LOCAL_PATH)/../../../