
LOCAL_CFLAGS += -DXLOGGER_TAG='"mars::$(LOCAL_MODULE)"'
LOCAL_CPPFLAGS += -frtti #-fexceptions 
#LOCAL_CPPFLAGS += -Wno-error=extern-c-compat
LOCAL_CFLAGS += -Werror -Wall
LOCAL_CFLAGS += -Wsign-compare -Wtype-limits -Wuninitialized -Wclobbered -Wunused-parameter -Wempty-body #-Wextra 64bit
LOCAL_CFLAGS += -Wno-error=conversion -Wno-error=sign-conversion -Werror=sign-compare -Wno-error=format -Wno-error=pointer-to-int-cast -Wno-char-subscripts 
LOCAL_CFLAGS += -Wno-unused-parameter -Wno-missing-field-initializers -Wno-error=unused-variable 

# clang
#LOCAL_CFLAGS += -Wno-unknown-warning-option -Wno-deprecated-register -Wno-mismatched-tags -Wno-char-subscripts
#LOCAL_CFLAGS +=	-Wno-infinite-recursion -Wno-gnu-designator -Wno-unused-const-variable	-Wno-unused-local-typedef -Wno-unused-private-field -Wno-error=unused-variable
#LOCAL_CFLAGS += -Wno-overloaded-virtual
#LOCAL_LDLIBS += -latomic

LOCAL_CFLAGS +=  -fdata-sections
LOCAL_LDFLAGS += -Wl,--gc-sections 

LOCAL_CFLAGS += -Wno-unused-local-typedefs
