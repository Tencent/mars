TEMP_LOCAL_PATH :=$(call my-dir)

include $(TEMP_LOCAL_PATH)/../../flags.mk
#LOCAL_C_INCLUDES += $(TEMP_LOCAL_PATH)/src

SRC := $(wildcard $(TEMP_LOCAL_PATH)/../src/*.cpp)
SRC := $(SRC:$(LOCAL_PATH)/%=%)
LOCAL_SRC_FILES += $(SRC)