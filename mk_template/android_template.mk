
#LOCAL_EXPORT_STATIC_LIBRARIES := $(LOCAL_EXPORT_STATIC_LIBRARIES)
#LOCAL_EXPORT_SHARED_LIBRARIES := $(LOCAL_EXPORT_SHARED_LIBRARIES)
#LOCAL_EXPORT_SRC_FILES := $(LOCAL_EXPORT_SRC_FILES)

include  $(LOCAL_PATH)/import.mk
include  $(LOCAL_PATH)/build.conf

include $(CLEAR_VARS)
LOCAL_MODULE := $(SELF_LOCAL_MODULE)
LOCAL_MODULE_FILENAME := lib$(LIBPREFIX)$(LOCAL_MODULE)

LOCAL_C_INCLUDES += $(SELF_LOCAL_EXPORT_C_INCLUDES)

include $(LOCAL_PATH)/sources.mk
ifeq ($(SELF_BUILD_CMD), BUILD_SHARED_LIBRARY)
SRC := $(LOCAL_EXPORT_SRC_FILES:$(LOCAL_PATH)/%=%)
LOCAL_SRC_FILES += $(SRC)

ifeq ($(SELF_JNI_ONLOAD_FILE),)
LOCAL_SRC_FILES +=  ../../mk_template/JNI_OnLoad.cpp
else
LOCAL_SRC_FILES += $(SELF_JNI_ONLOAD_FILE)
endif
endif

SRC := $(SELF_LOCAL_EXPORT_SRC_FILES:$(LOCAL_PATH)/%=%)
LOCAL_SRC_FILES += $(SRC)

LOCAL_SRC_FILES := $(sort $(LOCAL_SRC_FILES))

sort-libraries-impl = $(if $(filter $(words $2),0),$1,$(if $(filter $1,$2),,$1) $(call sort-libraries-impl,$(firstword $2),$(wordlist 2,$(words $2),$2)))
sort-libraries = $(strip $(call sort-libraries-impl,$(firstword $1),$(wordlist 2,$(words $1),$1)))

filter-static-libraries-impl = $(if $(filter $(words $3),0),\
									,\
									$(if $(filter %_shared{,$(firstword $3)),\
										$(call filter-static-libraries-impl,$1 1,$(if $(filter $(words $2),0),$1 1,$2),$(wordlist 2,$(words $3),$3)),\
										$(if $(filter %_static{,$(firstword $3)),\
											$(call filter-static-libraries-impl,$1 1,$2,$(wordlist 2,$(words $3),$3)),\
											$(if $(filter },$(firstword $3)),\
												$(call filter-static-libraries-impl,$(wordlist 2,$(words $1),$1),$(if $(filter $(words $1),$(words $2)),,$2),$(wordlist 2,$(words $3),$3)),\
												$(if $(filter $(words $2),0), $(firstword $3),) $(call filter-static-libraries-impl,$1,$2,$(wordlist 2,$(words $3),$3))\
											)\
										)\
									)\
								)
									
filter-static-libraries = $(strip $(call filter-static-libraries-impl,,,$1))

LOCAL_STATIC_LIBRARIES += $(call sort-libraries,$(call filter-static-libraries,$(LOCAL_EXPORT_STATIC_LIBRARIES)))
LOCAL_SHARED_LIBRARIES += $(call sort-libraries,$(LOCAL_EXPORT_SHARED_LIBRARIES))
#LOCAL_STATIC_LIBRARIES += $(LOCAL_EXPORT_STATIC_LIBRARIES)
#LOCAL_SHARED_LIBRARIES += $ $(LOCAL_EXPORT_SHARED_LIBRARIES)  
LOCAL_LDLIBS    += $(SELF_LOCAL_EXPORT_LDLIBS)

unique = $(if $1,$(firstword $1) $(call uniq,$(filter-out $(firstword $1),$1)))
LOCAL_C_INCLUDES := $(call unique,$(abspath $(LOCAL_C_INCLUDES)))
LOCAL_C_INCLUDES := $(LOCAL_C_INCLUDES:$(abspath .)%=.%)
LOCAL_C_INCLUDES := $(LOCAL_C_INCLUDES:$(abspath ..)%=..%)
LOCAL_C_INCLUDES := $(LOCAL_C_INCLUDES:$(abspath ../..)%=../..%)
LOCAL_C_INCLUDES := $(LOCAL_C_INCLUDES:$(abspath ../../..)%=../../..%)
$(info 'LOCAL_C_INCLUDES = $(LOCAL_C_INCLUDES)')

LOCAL_EXPORT_STATIC_LIBRARIES := $(call unique,$(LOCAL_EXPORT_STATIC_LIBRARIES))
LOCAL_EXPORT_SHARED_LIBRARIES := $(call unique,$(LOCAL_EXPORT_SHARED_LIBRARIES))
LOCAL_STATIC_LIBRARIES := $(call unique,$(LOCAL_STATIC_LIBRARIES))
LOCAL_SHARED_LIBRARIES := $(call unique,$(LOCAL_SHARED_LIBRARIES))
LOCAL_LDLIBS := $(call unique,$(LOCAL_LDLIBS))
LOCAL_EXPORT_SRC_FILES := $(call unique,$(LOCAL_EXPORT_SRC_FILES))
LOCAL_SRC_FILES := $(call unique,$(LOCAL_SRC_FILES))

$(info 'SELF_BUILD_CMD = $(SELF_BUILD_CMD)')
$(info 'LOCAL_EXPORT_STATIC_LIBRARIES = $(LOCAL_EXPORT_STATIC_LIBRARIES)')
$(info 'LOCAL_EXPORT_SHARED_LIBRARIES = $(LOCAL_EXPORT_SHARED_LIBRARIES)')
$(info 'LOCAL_STATIC_LIBRARIES = $(LOCAL_STATIC_LIBRARIES)')
$(info 'LOCAL_SHARED_LIBRARIES = $(LOCAL_SHARED_LIBRARIES)')
$(info 'LOCAL_LDLIBS = $(LOCAL_LDLIBS)')
$(info 'LOCAL_EXPORT_SRC_FILES = $(LOCAL_EXPORT_SRC_FILES) $(SELF_LOCAL_EXPORT_SRC_FILES)')
$(info 'LOCAL_SRC_FILES=$(abspath $(LOCAL_SRC_FILES:%=jni/%))')
LOCAL_CPP_EXTENSION := .cc .cpp
include $($(SELF_BUILD_CMD))

ifeq ($(SELF_BUILD_CMD), BUILD_SHARED_LIBRARY)
$(call import-module,android/cpufeatures)
endif

