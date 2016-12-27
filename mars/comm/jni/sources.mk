
TEMP_LOCAL_PATH :=$(call my-dir)/..

include $(TEMP_LOCAL_PATH)/../mk_template/util.mk
include $(TEMP_LOCAL_PATH)/../mk_template/flags.mk

LOCAL_CFLAGS += -fvisibility=hidden
LOCAL_CFLAGS += -Wno-error=strict-aliasing

LOCAL_CFLAGS += -DUSING_XLOG_WEAK_FUNC

SRC := $(wildcard $(TEMP_LOCAL_PATH)/*.c)
SRC := $(SRC:$(LOCAL_PATH)/%=%)
LOCAL_SRC_FILES += $(SRC)

SRC := $(wildcard $(TEMP_LOCAL_PATH)/*.cc)
SRC := $(SRC:$(LOCAL_PATH)/%=%)
LOCAL_SRC_FILES += $(SRC)

SRC := $(wildcard $(TEMP_LOCAL_PATH)/crypt/*.c)
SRC := $(SRC:$(LOCAL_PATH)/%=%)
LOCAL_SRC_FILES += $(SRC)

SRC := $(wildcard $(TEMP_LOCAL_PATH)/thread/*.cc)
SRC := $(SRC:$(LOCAL_PATH)/%=%)
LOCAL_SRC_FILES += $(SRC)

SRC := $(wildcard $(TEMP_LOCAL_PATH)/crypt/*.cc)
SRC := $(SRC:$(LOCAL_PATH)/%=%)
LOCAL_SRC_FILES += $(SRC)

SRC := $(call enum-files-subdir-all-src,$(TEMP_LOCAL_PATH)/../boost/libs)
SRC := $(foreach n,$(SRC), $(if $(filter-out %/win32/,$(dir $(n))),$(n)))
SRC := $(foreach n,$(SRC), $(if $(filter-out %/windows/,$(dir $(n))),$(n)))
EXCLUDE_SRC_FILES := $(TEMP_LOCAL_PATH)/../boost/libs/thread/src/pthread/thread.cpp
SRC := $(filter-out $(EXCLUDE_SRC_FILES),$(SRC))
ifeq ($(TARGET_ARCH), arm)
	arm_src_files := $(TEMP_LOCAL_PATH)/../boost/libs/context/src/asm/jump_arm_aapcs_elf_gas.S \
		$(TEMP_LOCAL_PATH)/../boost/libs/context/src/asm/make_arm_aapcs_elf_gas.S
	SRC += $(arm_src_files)
	LOCAL_CFLAGS += $(arm_cflags)
endif
ifeq ($(TARGET_ARCH), arm64)
	arm_src_files := $(TEMP_LOCAL_PATH)/../boost/libs/context/src/asm/jump_arm64_aapcs_elf_gas.S \
		$(TEMP_LOCAL_PATH)/../boost/libs/context/src/asm/make_arm64_aapcs_elf_gas.S
	SRC += $(arm_src_files)
	LOCAL_CFLAGS += $(arm_cflags)
endif
ifeq ($(TARGET_ARCH), x86)
	arm_src_files := $(TEMP_LOCAL_PATH)/../boost/libs/context/src/asm/jump_i386_sysv_elf_gas.S \
		$(TEMP_LOCAL_PATH)/../boost/libs/context/src/asm/make_i386_sysv_elf_gas.S
	SRC += $(arm_src_files)
	LOCAL_CFLAGS += $(arm_cflags)
endif
ifeq ($(TARGET_ARCH), x86_64)
	arm_src_files := $(TEMP_LOCAL_PATH)/../boost/libs/context/src/asm/jump_x86_64_sysv_elf_gas.S \
		$(TEMP_LOCAL_PATH)/../boost/libs/context/src/asm/make_x86_64_sysv_elf_gas.S
	SRC += $(arm_src_files)
	LOCAL_CFLAGS += $(arm_cflags)
endif

SRC := $(SRC:$(LOCAL_PATH)/%=%)
LOCAL_SRC_FILES += $(SRC)

SRC := $(wildcard $(TEMP_LOCAL_PATH)/android/*.c)
SRC := $(SRC:$(LOCAL_PATH)/%=%)
LOCAL_SRC_FILES += $(SRC)

SRC := $(wildcard $(TEMP_LOCAL_PATH)/network/*.c*)
SRC := $(SRC:$(LOCAL_PATH)/%=%)
LOCAL_SRC_FILES += $(SRC)

SRC := $(wildcard $(TEMP_LOCAL_PATH)/android/*.cc)
SRC := $(SRC:$(LOCAL_PATH)/%=%)
LOCAL_SRC_FILES += $(SRC)

SRC := $(wildcard $(TEMP_LOCAL_PATH)/DnsResolve/*.cc)
SRC := $(SRC:$(LOCAL_PATH)/%=%)
LOCAL_SRC_FILES += $(SRC)

SRC := $(wildcard $(TEMP_LOCAL_PATH)/messagequeue/*.cc)
SRC := $(SRC:$(LOCAL_PATH)/%=%)
LOCAL_SRC_FILES += $(SRC)

SRC := $(wildcard $(TEMP_LOCAL_PATH)/socket/*.c)
SRC := $(SRC:$(LOCAL_PATH)/%=%)
LOCAL_SRC_FILES += $(SRC)

SRC := $(wildcard $(TEMP_LOCAL_PATH)/socket/*.cc)
SRC := $(SRC:$(LOCAL_PATH)/%=%)
LOCAL_SRC_FILES += $(SRC)

SRC := $(wildcard $(TEMP_LOCAL_PATH)/jni/*.c*)
SRC := $(SRC:$(LOCAL_PATH)/%=%)
LOCAL_SRC_FILES += $(SRC)

SRC := $(wildcard $(TEMP_LOCAL_PATH)/jni/util/*.cc)
SRC := $(SRC:$(LOCAL_PATH)/%=%)
LOCAL_SRC_FILES += $(SRC)

SRC := $(wildcard $(TEMP_LOCAL_PATH)/corepattern/*.cc)
SRC := $(SRC:$(LOCAL_PATH)/%=%)
LOCAL_SRC_FILES += $(SRC)

SRC := $(wildcard $(TEMP_LOCAL_PATH)/coroutine/*.cc)
SRC := $(SRC:$(LOCAL_PATH)/%=%)
LOCAL_SRC_FILES += $(SRC)

SRC := $(wildcard $(TEMP_LOCAL_PATH)/unix/socketselect/*.cc)
SRC := $(SRC:$(LOCAL_PATH)/%=%)
LOCAL_SRC_FILES += $(SRC)

SRC := $(wildcard $(TEMP_LOCAL_PATH)/debugger/*.cc)
SRC := $(SRC:$(LOCAL_PATH)/%=%)
LOCAL_SRC_FILES += $(SRC)

SRC := $(wildcard $(TEMP_LOCAL_PATH)/http/*.cc)
SRC := $(SRC:$(LOCAL_PATH)/%=%)
LOCAL_SRC_FILES += $(SRC)

SRC := $(wildcard $(TEMP_LOCAL_PATH)/dns/*.cc)
SRC := $(SRC:$(LOCAL_PATH)/%=%)
LOCAL_SRC_FILES += $(SRC)

SRC := $(wildcard $(TEMP_LOCAL_PATH)/assert/*.c)
SRC := $(SRC:$(LOCAL_PATH)/%=%)
LOCAL_SRC_FILES += $(SRC)

SRC := $(wildcard $(TEMP_LOCAL_PATH)/xlogger/*.c)
SRC := $(SRC:$(LOCAL_PATH)/%=%)
LOCAL_SRC_FILES += $(SRC)


