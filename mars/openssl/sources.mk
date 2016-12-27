TEMP_LOCAL_PATH :=$(call my-dir)
include $(TEMP_LOCAL_PATH)/../mk_template/flags.mk
include $(TEMP_LOCAL_PATH)/../mk_template/util.mk

LOCAL_C_INCLUDES +=  $(TEMP_LOCAL_PATH)/../comm $(TEMP_LOCAL_PATH)/../ $(TEMP_LOCAL_PATH)/../../ $(TEMP_LOCAL_PATH)/../../../

SRC := $(call enum-files-subdir0,$(TEMP_LOCAL_PATH)/crypto,*.c)
SRC += $(call enum-files-subdir1,$(TEMP_LOCAL_PATH)/crypto,*.c)
SRC += $(call enum-files-subdir0,$(TEMP_LOCAL_PATH)/export,*.c)
SRC += $(call enum-files-subdir0,$(TEMP_LOCAL_PATH)/export/crypto,*.cpp)
#SRC += $(call enum-files-subdir0,$(TEMP_LOCAL_PATH)/fips,*.c)
#SRC += $(call enum-files-subdir1,$(TEMP_LOCAL_PATH)/fips,*.c)

#arm_src_files := \
    bn/asm/armv4-mont.S \
    sha/asm/sha1-armv4-large.S \
    sha/asm/sha256-armv4.S \
    sha/asm/sha512-armv4.S \
    aes/asm/aes-armv4.S 
arm_src_files := ../crypto/sha/asm/sha1-armv4-large.S
arm_cflags := -DSHA1_ASM
ifeq ($(TARGET_ARCH), arm)
	SRC += $(arm_src_files)
	LOCAL_CFLAGS += $(arm_cflags)
endif
#arm_src_files := $(arm_src_files:%=$(TEMP_LOCAL_PATH)/crypto/%) 
#non_arm_src_files := $(TEMP_LOCAL_PATH)/crypto/aes/aes_core.c

#arm_cflags := -DOPENSSL_BN_ASM_MONT -DAES_ASM -DSHA1_ASM -DSHA256_ASM -DSHA512_ASM

#ifeq ($(TARGET_ARCH), arm)
#	SRC += $(arm_src_files)
#	LOCAL_CFLAGS += $(arm_cflags)
#else
#	SRC += $(non_arm_src_files)
#endif

SRC := $(SRC:$(LOCAL_PATH)/%=%) 
LOCAL_SRC_FILES += $(SRC)

LOCAL_C_INCLUDES +=	$(TEMP_LOCAL_PATH)/ $(TEMP_LOCAL_PATH)/export_include $(TEMP_LOCAL_PATH)/ $(TEMP_LOCAL_PATH)/crypto $(TEMP_LOCAL_PATH)/crypto/asn1  $(TEMP_LOCAL_PATH)/crypto/evp $(TEMP_LOCAL_PATH)/include $(TEMP_LOCAL_PATH)/include/openssl


