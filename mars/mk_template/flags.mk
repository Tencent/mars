
LOCAL_CFLAGS += -DXLOGGER_TAG='"mars::$(LOCAL_MODULE)"'
LOCAL_CPPFLAGS += -frtti #-fexceptions 
#LOCAL_CPPFLAGS += -Wno-error=extern-c-compat
LOCAL_CFLAGS += -Werror -Wall
LOCAL_CFLAGS += -Wsign-compare -Wtype-limits -Wuninitialized -Wclobbered -Wunused-but-set-parameter -Wempty-body #-Wextra 64bit
LOCAL_CFLAGS += -Wno-error=conversion -Wno-error=sign-conversion -Werror=sign-compare -Wno-error=format -Wno-error=pointer-to-int-cast
LOCAL_CFLAGS += -Wno-unused-parameter -Wno-missing-field-initializers 

NDK_VERSION := $(strip $(patsubst android-ndk-%,%,$(filter android-ndk-%, $(subst /, ,$(dir $(TARGET_CC))))))
ifneq ($(filter r13 r13b, $(NDK_VERSION)),)
LOCAL_CFLAGS += -Wno-unknown-warning-option -Wno-deprecated-register -Wno-mismatched-tags -Wno-char-subscripts	#ndk r13 compile
LOCAL_CFLAGS +=	-Wno-infinite-recursion -Wno-gnu-designator -Wno-unused-const-variable	-Wno-unused-local-typedef -Wno-unused-private-field -Wno-error=unused-variable
LOCAL_CFLAGS += -Wno-overloaded-virtual#ndk r13 compile

LOCAL_LDLIBS += -latomic
endif

LOCAL_CFLAGS +=  -fdata-sections
LOCAL_LDFLAGS += -Wl,--gc-sections 

MY_USED_GCC_VERSION_CODE = $(shell `$(NDK_ROOT)/ndk-which gcc` -dumpversion)
$(info 'info MY_USED_GCC_VERSION_CODE = $(MY_USED_GCC_VERSION_CODE)')

ifeq ($(MY_USED_GCC_VERSION_CODE),4.6)
   	$(info '[W] complier with gcc lower version(<=4.6), some LOCAL_CFLAG was disabled.')
else
	LOCAL_CFLAGS += -Wno-unused-local-typedefs
endif
