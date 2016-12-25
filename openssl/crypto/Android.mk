LOCAL_PATH:= $(call my-dir)

arm_cflags := -DOPENSSL_BN_ASM_MONT -DAES_ASM -DSHA1_ASM -DSHA256_ASM -DSHA512_ASM
arm_src_files := \
    bn/asm/armv4-mont.S
non_arm_src_files := 

local_src_files := \
	cryptlib.c \
	mem.c \
	mem_clr.c \
	mem_dbg.c \
	cversion.c \
	ex_data.c \
	cpt_err.c \
	ebcdic.c \
	uid.c \
	o_time.c \
	o_str.c \
	o_dir.c \
	bn/bn_add.c \
	bn/bn_asm.c \
	bn/bn_blind.c \
	bn/bn_ctx.c \
	bn/bn_div.c \
	bn/bn_err.c \
	bn/bn_exp.c \
	bn/bn_exp2.c \
	bn/bn_gcd.c \
	bn/bn_gf2m.c \
	bn/bn_kron.c \
	bn/bn_lib.c \
	bn/bn_mod.c \
	bn/bn_mont.c \
	bn/bn_mpi.c \
	bn/bn_mul.c \
	bn/bn_nist.c \
	bn/bn_prime.c \
	bn/bn_rand.c \
	bn/bn_recp.c \
	bn/bn_shift.c \
	bn/bn_sqr.c \
	bn/bn_sqrt.c \
	bn/bn_word.c \
	buffer/buffer.c \
	ec/ec2_mult.c \
	ec/ec2_smpl.c \
	ec/ec_check.c \
	ec/ec_curve.c \
	ec/ec_cvt.c \
	ec/ec_err.c \
	ec/ec_key.c \
	ec/ec_lib.c \
	ec/ec_mult.c \
	ec/ec_print.c \
	ec/ecp_mont.c \
	ec/ecp_nist.c \
	ec/ecp_smpl.c \
	ecdh/ech_err.c \
	ecdh/ech_key.c \
	ecdh/ech_lib.c \
	ecdh/ech_ossl.c \
	ecdsa/ecs_err.c \
	ecdsa/ecs_lib.c \
	ecdsa/ecs_ossl.c \
	rand/rand_egd.c \
	rand/rand_lib.c \
	rand/randfile.c \

local_c_includes := \
	$(NDK_PROJECT_PATH) \
	$(NDK_PROJECT_PATH)/crypto/asn1 \
	$(NDK_PROJECT_PATH)/crypto/evp \
	$(NDK_PROJECT_PATH)/include \
	$(NDK_PROJECT_PATH)/include/openssl

local_c_flags := -DNO_WINDOWS_BRAINDEATH

#######################################

# target
include $(CLEAR_VARS)
include $(LOCAL_PATH)/../android-config.mk
LOCAL_SRC_FILES += $(local_src_files)
LOCAL_CFLAGS += $(local_c_flags)
LOCAL_C_INCLUDES += $(local_c_includes)
LOCAL_LDLIBS += -lz
ifeq ($(TARGET_ARCH),arm)
	LOCAL_SRC_FILES += $(arm_src_files)
	LOCAL_CFLAGS += $(arm_cflags)
else
	LOCAL_SRC_FILES += $(non_arm_src_files)
endif
ifeq ($(TARGET_SIMULATOR),true)
	# Make valgrind happy.
	LOCAL_CFLAGS += -DPURIFY
    LOCAL_LDLIBS += -ldl
endif
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE:= libcrypto
include $(BUILD_SHARED_LIBRARY)

#######################################
# host shared library
ifeq ($(WITH_HOST_DALVIK),true)
    include $(CLEAR_VARS)
    include $(LOCAL_PATH)/../android-config.mk
    LOCAL_SRC_FILES += $(local_src_files)
    LOCAL_CFLAGS += $(local_c_flags) -DPURIFY
    LOCAL_C_INCLUDES += $(local_c_includes)
    LOCAL_SRC_FILES += $(non_arm_src_files)
    LOCAL_LDLIBS += -ldl
    LOCAL_MODULE_TAGS := optional
    LOCAL_MODULE:= libcrypto
    include $(BUILD_SHARED_LIBRARY)
endif

########################################
# host static library, which is used by some SDK tools.

include $(CLEAR_VARS)
include $(LOCAL_PATH)/../android-config.mk
LOCAL_SRC_FILES += $(local_src_files)
LOCAL_CFLAGS += $(local_c_flags) -DPURIFY
LOCAL_C_INCLUDES += $(local_c_includes)
LOCAL_SRC_FILES += $(non_arm_src_files)
LOCAL_LDLIBS += -ldl
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE:= libcrypto_static
include $(BUILD_STATIC_LIBRARY)
