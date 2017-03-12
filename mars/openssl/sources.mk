TEMP_LOCAL_PATH :=$(call my-dir)
include $(TEMP_LOCAL_PATH)/../mk_template/flags.mk
include $(TEMP_LOCAL_PATH)/../mk_template/util.mk

LOCAL_C_INCLUDES +=  $(TEMP_LOCAL_PATH)/../comm $(TEMP_LOCAL_PATH)/../ $(TEMP_LOCAL_PATH)/../../ $(TEMP_LOCAL_PATH)/../../../

SRC += $(call enum-files-subdir0,$(TEMP_LOCAL_PATH)/export,*.c)
SRC += $(call enum-files-subdir0,$(TEMP_LOCAL_PATH)/export/crypto,*.cpp)

SRC := $(SRC:$(LOCAL_PATH)/%=%) 
LOCAL_SRC_FILES += $(SRC)

LOCAL_C_INCLUDES +=	$(TEMP_LOCAL_PATH)/ $(TEMP_LOCAL_PATH)/export_include $(TEMP_LOCAL_PATH)/ $(TEMP_LOCAL_PATH)/include $(TEMP_LOCAL_PATH)/include/openssl


