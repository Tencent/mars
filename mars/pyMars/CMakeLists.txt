# author:dj

cmake_minimum_required(VERSION 3.1)
project(mars)

SET(CMAKE_C_COMPILER
    /Library/Developer/CommandLineTools/usr/bin/cc
)
SET(CMAKE_CXX_COMPILER
    /Library/Developer/CommandLineTools/usr/bin/c++
)

include_directories(/Users/lapsule/anaconda2/include/python2.7)
include_directories(/Users/lapsule/.local/include/python2.7)
include_directories(..)
include_directories(../..)
include_directories(../../mars/comm)
include_directories(../../mars/comm/xlogger)
include_directories(../stn)
include_directories(../sdt)
include_directories(../log)
include_directories(../baseevent)
include_directories(../boost)
include_directories(./src)
include_directories(../app)

include_directories(./PublicComponentV2)
AUX_SOURCE_DIRECTORY(./PublicComponentV2    PUB_SRC)
include_directories(./Bussiness/)
AUX_SOURCE_DIRECTORY(./Bussiness/   BUSS_SRC)
include_directories(./Component)
AUX_SOURCE_DIRECTORY(./Component    COMP_SRC)
include_directories(../../samples/Mac/Proto)
AUX_SOURCE_DIRECTORY(../../samples/Mac/Proto    PROTO_SRC)
include_directories(./protobuf-objc/src/runtime/Classes)
AUX_SOURCE_DIRECTORY(./protobuf-objc/src/runtime/Classes    PROTOBUF_SRC)

AUX_SOURCE_DIRECTORY(../app APP_SRCS)
AUX_SOURCE_DIRECTORY(../sdt SDT_MAIN_SRCS)
AUX_SOURCE_DIRECTORY(../sdt/src SDT_SRCS)
AUX_SOURCE_DIRECTORY(../sdt/src/activecheck SDT_ACT_CHECK_SRCS)
AUX_SOURCE_DIRECTORY(../sdt/src/checkimpl SDT_CHECK_IMPL_SRCS)
AUX_SOURCE_DIRECTORY(../sdt/src/tools SDT_TOOLS_SRCS)
AUX_SOURCE_DIRECTORY(../sdt SDT_SRCS)
AUX_SOURCE_DIRECTORY(../stn STN_MAIN_SRCS)
AUX_SOURCE_DIRECTORY(../stn/src STN_SRCS)
AUX_SOURCE_DIRECTORY(../stn/proto STN_PROTO_SRCS)
AUX_SOURCE_DIRECTORY(../baseevent/src BASEEVENT_SRCS)

AUX_SOURCE_DIRECTORY(../comm COMM_SRCS)
AUX_SOURCE_DIRECTORY(../comm/network COMM_NW_SRCS)
AUX_SOURCE_DIRECTORY(../comm/socket COMM_SOCKET_SRCS)
AUX_SOURCE_DIRECTORY(../comm/coroutine COMM_COROUTINE_SRCS)
AUX_SOURCE_DIRECTORY(../comm/corepattern COMM_COREPATTERN_SRCS)
AUX_SOURCE_DIRECTORY(../comm/crypt COMM_CRYPT_SRCS)
AUX_SOURCE_DIRECTORY(../comm/debugger COMM_DEBUGGER_SRCS)
AUX_SOURCE_DIRECTORY(../comm/dns COMM_DNS_SRCS)
AUX_SOURCE_DIRECTORY(../comm/assert COMM_ASSERT_SRCS)
AUX_SOURCE_DIRECTORY(../comm/thread COMM_THREAD_SRCS)
AUX_SOURCE_DIRECTORY(../comm/unix/socket COMM_UNIX_SOCKET_SRCS)
AUX_SOURCE_DIRECTORY(../comm/unix/thread COMM_UNIX_THREAD_SRCS)
AUX_SOURCE_DIRECTORY(../comm/messagequeue MSGQUEUE_SRCS)
AUX_SOURCE_DIRECTORY(../comm/xlogger XLOGGER_SRCS)

AUX_SOURCE_DIRECTORY(../log LOG_MAIN_SRCS)
AUX_SOURCE_DIRECTORY(../log/crypt LOG_CRYPT_SRCS)
AUX_SOURCE_DIRECTORY(../log/crypt/micro-ecc-master LOG_CRYPT_ECC_SRCS)
AUX_SOURCE_DIRECTORY(../log/src LOG_SRCS)
AUX_SOURCE_DIRECTORY(../comm/objc LOG_OBJC_SRCS)


AUX_SOURCE_DIRECTORY(../boost/atomic/src    ATOMIC_SRC)
AUX_SOURCE_DIRECTORY(../boost/exception/src    EXCEPTION_SRC)

AUX_SOURCE_DIRECTORY(../boost/libs/coroutine/src    COROUTINE_MAIN_SRC)
AUX_SOURCE_DIRECTORY(../boost/libs/coroutine/src/detail    COROUTINE_DETAIL_SRC)
AUX_SOURCE_DIRECTORY(../boost/libs/coroutine/src/posix    COROUTINE_POSIX_SRC)


# AUX_SOURCE_DIRECTORY(../boost/libs/coroutine/src/windows    COROUTINE_WINDOWS_SRC)
# LIST (APPEND COROUTINE_SRC
#     ${COROUTINE_MAIN_SRC}
#     ${COROUTINE_DETAIL_SRC}
#     ${COROUTINE_POSIX_SRC}
#     # ${COROUTINE_WINDOWS_SRC}
# )

AUX_SOURCE_DIRECTORY(../boost/libs/iostreams/src    IOSTREAMS_SRC)
AUX_SOURCE_DIRECTORY(../boost/libs/filesystem/src    FILESYSTEM_SRC)
AUX_SOURCE_DIRECTORY(../boost/libs/system/src    SYSTEM_SRC)
AUX_SOURCE_DIRECTORY(../boost/libs/smart_ptr/src    SMART_PTR_SRC)
AUX_SOURCE_DIRECTORY(../boost/libs/date_time/src/gregorian    DATE_TIME_GREGORIAN_SRC)
AUX_SOURCE_DIRECTORY(../boost/libs/date_time/src/posix_time    DATE_TIME_POSIX_SRC)
AUX_SOURCE_DIRECTORY(../boost/libs/thread/src    THREAD_MAIN_SRC)
AUX_SOURCE_DIRECTORY(../boost/libs/thread/src/pthread    THREAD_PTHREAD_SRC)
# AUX_SOURCE_DIRECTORY(../boost/libs/thread/src/win32    THREAD_WIN32_SRC)

AUX_SOURCE_DIRECTORY(../boost/libs/context/src/asm    CONTEXT_ASM_SRC)
AUX_SOURCE_DIRECTORY(../boost/libs/context/src    CONTEXT_MAIN_SRC)
AUX_SOURCE_DIRECTORY(../boost/libs/context/src/posix    CONTEXT_POSIX_SRC)
AUX_SOURCE_DIRECTORY(../boost/libs/context/src/windows    CONTEXT_WINDOWS_SRC)

set(CMAKE_CXX_STANDARD 17)
enable_language(ASM)
# enable_language(S)
SET(CMAKE_ASM_FLAGS "${CFLAGS} -x assembler-with-cpp")

set(CMAKE_CXX_FLAGS   "-objc") 
# not work...
# set(CMAKE_CXX_FLAGS   "-undefined dynamic_lookup")

# target_link_libraries(kkk "-framework CoreFoundation")

IF(APPLE)
   FIND_LIBRARY(CoreWLAN CoreWLAN)
   FIND_LIBRARY(Foundation Foundation)
   MARK_AS_ADVANCED (CoreWLAN)
   MARK_AS_ADVANCED (Foundation)
   SET(EXTRA_LIBS ${CoreWLAN} ${Foundation})
   link_libraries(${EXTRA_LIBS})
ENDIF (APPLE)

# link_libraries(/usr/local/lib/libboost_python27.dylib)
# link_libraries(/usr/local/lib/libboost_system.dylib)
# link_libraries(/usr/local/lib/libboost_coroutine.dylib)

#set environment variable
SET(MARS_SRC
    ${APP_SRCS}

    ${SDT_SRCS}
    ${SDT_MAIN_SRCS}
    ${SDT_ACT_CHECK_SRCS}
    ${SDT_CHECK_IMPL_SRCS}
    ${SDT_TOOLS_SRCS}

    ${STN_SRCS}
    ${STN_MAIN_SRCS}
    # ${STN_PROTO_SRCS}
    ${COMM_SRCS}
    ${LOG_MAIN_SRCS}
    ${LOG_CRYPT_SRCS}
    ${LOG_CRYPT_ECC_SRCS}
    ${LOG_SRCS}
    ${LOG_OBJC_SRCS}
    ${XLOGGER_SRCS}
    ${BASEEVENT_SRCS}

    ${ATOMIC_SRC}
    ${COROUTINE_MAIN_SRC}
    ${COROUTINE_DETAIL_SRC}
    ${COROUTINE_POSIX_SRC}
    ${EXCEPTION_SRC}
    ${IOSTREAMS_SRC}
    ${SYSTEM_SRC}
    ${FILESYSTEM_SRC}
    ${SMART_PTR_SRC}
    ${DATE_TIME_GREGORIAN_SRC}
    ${DATE_TIME_POSIX_SRC}
    ${COMM_NW_SRCS}
    ${COMM_SOCKET_SRCS}
    ${COMM_COROUTINE_SRCS}
    ${COMM_COREPATTERN_SRCS}
    ${COMM_CRYPT_SRCS}
    ${COMM_DEBUGGER_SRCS}
    ${COMM_DNS_SRCS}
    ${COMM_ASSERT_SRCS}
    ${COMM_THREAD_SRCS}
    ${COMM_UNIX_SOCKET_SRCS}
    ${COMM_UNIX_THREAD_SRCS}

    ${THREAD_MAIN_SRC}
    # ${THREAD_PTHREAD_SRC}

    # ${THREAD_WIN32_SRC}
    # ${CONTEXT_ASM_SRC}
    ${CONTEXT_MAIN_SRC}
    ${CONTEXT_POSIX_SRC}
    # ${CONTEXT_WINDOWS_SRC}

    ${MSGQUEUE_SRCS}


    ${PUB_SRC}
    ${BUSS_SRC}
    ${COMP_SRC}

    # 需要依赖 PBExtensionRegistry， 所以暂时不搞
    # ${PROTO_SRC}
    # ${PROTOBUF_SRC}

    /Users/lapsule/lapsule/mars/mars/boost/libs/context/src/asm/jump_x86_64_sysv_macho_gas.S
    src/mars_pybind.mm
    # src/mars_cc.mm
    # src/mars_callbacks.mm
    # src/pybind11_tests.cpp
)


add_subdirectory(pybind11)
pybind11_add_module(mars ${MARS_SRC})
# pybind11_add_module(mars ${MARS_SRC})
