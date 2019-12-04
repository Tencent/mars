cmake_minimum_required (VERSION 3.6)

set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}" CACHE PATH "Installation directory" FORCE)
message(STATUS "CMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}")

project (comm)

include(CMakeUtils.txt)
include(CMakeExtraFlags.txt)

include_directories(.)
include_directories(..)
include_directories(../..)
include_directories(assert)
include_directories(corepattern)
include_directories(crypt)
include_directories(debugger)
include_directories(dns)
include_directories(messagequeue)
include_directories(network)
include_directories(socket)
include_directories(thread)
include_directories(xlogger)

file(GLOB SELF_TEMP_SRC_FILES RELATIVE ${PROJECT_SOURCE_DIR} *.cc *.c *.h)
list(APPEND SELF_SRC_FILES ${SELF_TEMP_SRC_FILES})


file(GLOB SELF_TEMP_SRC_FILES RELATIVE ${PROJECT_SOURCE_DIR} assert/*.c assert/*.h)
source_group(assert FILES ${SELF_TEMP_SRC_FILES})
list(APPEND SELF_SRC_FILES ${SELF_TEMP_SRC_FILES})

file(GLOB SELF_TEMP_SRC_FILES RELATIVE ${PROJECT_SOURCE_DIR} crypt/*.cc crypt/*.c crypt/*.h)
source_group(crypt FILES ${SELF_TEMP_SRC_FILES})
list(APPEND SELF_SRC_FILES ${SELF_TEMP_SRC_FILES})

file(GLOB SELF_TEMP_SRC_FILES RELATIVE ${PROJECT_SOURCE_DIR} network/*.cc network/*.c network/*.h)
source_group(network FILES ${SELF_TEMP_SRC_FILES})
list(APPEND SELF_SRC_FILES ${SELF_TEMP_SRC_FILES})

file(GLOB SELF_TEMP_SRC_FILES RELATIVE ${PROJECT_SOURCE_DIR} socket/*.cc socket/*.h)
source_group(socket FILES ${SELF_TEMP_SRC_FILES})
list(APPEND SELF_SRC_FILES ${SELF_TEMP_SRC_FILES})

file(GLOB SELF_TEMP_SRC_FILES RELATIVE ${PROJECT_SOURCE_DIR} xlogger/*.c xlogger/*.h)
source_group(xlogger FILES ${SELF_TEMP_SRC_FILES})
list(APPEND SELF_SRC_FILES ${SELF_TEMP_SRC_FILES})

file(GLOB SELF_TEMP_SRC_FILES RELATIVE ${PROJECT_SOURCE_DIR} corepattern/*.cc corepattern/*.h)
source_group(corepattern FILES ${SELF_TEMP_SRC_FILES})
list(APPEND SELF_SRC_FILES ${SELF_TEMP_SRC_FILES})

file(GLOB SELF_TEMP_SRC_FILES RELATIVE ${PROJECT_SOURCE_DIR} dns/*.cc dns/*.h)
source_group(dns FILES ${SELF_TEMP_SRC_FILES})
list(APPEND SELF_SRC_FILES ${SELF_TEMP_SRC_FILES})

file(GLOB SELF_TEMP_SRC_FILES RELATIVE ${PROJECT_SOURCE_DIR} messagequeue/*.cc messagequeue/*.h)
source_group(messagequeue FILES ${SELF_TEMP_SRC_FILES})
list(APPEND SELF_SRC_FILES ${SELF_TEMP_SRC_FILES})

 
 
if(MSVC)
    add_definitions(/FI"../projdef.h")
    
    include_directories(windows)
    include_directories(windows/SocketSelect)
    include_directories(windows/thread)
    include_directories(windows/win32)
    include_directories(windows/zlib)
    
    list(REMOVE_ITEM SELF_SRC_FILES
            network/local_routetable.h
            network/local_routetable.cc
            socket/getsocktcpinfo.h
            socket/getsocktcpinfo.cc)

    file(GLOB SELF_TEMP_SRC_FILES RELATIVE ${PROJECT_SOURCE_DIR} windows/*.c windows/*.cc windows/*.cpp windows/*.h)
    source_group(windows FILES ${SELF_TEMP_SRC_FILES})
    list(APPEND SELF_SRC_FILES ${SELF_TEMP_SRC_FILES})
            
    file(GLOB SELF_TEMP_SRC_FILES RELATIVE ${PROJECT_SOURCE_DIR} windows/zlib/*.c windows/zlib/*.h)
    source_group(windows\\zlib FILES ${SELF_TEMP_SRC_FILES})
    list(APPEND SELF_SRC_FILES ${SELF_TEMP_SRC_FILES})
    
    file(GLOB SELF_TEMP_SRC_FILES RELATIVE ${PROJECT_SOURCE_DIR} windows/win32/*.cpp windows/win32/*.h)
    source_group(windows\\win32 FILES ${SELF_TEMP_SRC_FILES})
    list(APPEND SELF_SRC_FILES ${SELF_TEMP_SRC_FILES})   

    file(GLOB SELF_TEMP_SRC_FILES RELATIVE ${PROJECT_SOURCE_DIR} windows/*.cpp windows/*.c windows/*.h)
    source_group(windows FILES ${SELF_TEMP_SRC_FILES})
    list(APPEND SELF_SRC_FILES ${SELF_TEMP_SRC_FILES})

    file(GLOB SELF_TEMP_SRC_FILES RELATIVE ${PROJECT_SOURCE_DIR} windows/sys/*.c windows/sys/*.h)
    source_group(windows\\sys FILES ${SELF_TEMP_SRC_FILES})
    list(APPEND SELF_SRC_FILES ${SELF_TEMP_SRC_FILES})   

    file(GLOB SELF_TEMP_SRC_FILES RELATIVE ${PROJECT_SOURCE_DIR} windows/SocketSelect/*.cc windows/SocketSelect/*.h)
    source_group(windows\\SocketSelect FILES ${SELF_TEMP_SRC_FILES})
    list(APPEND SELF_SRC_FILES ${SELF_TEMP_SRC_FILES})      
    
else()   
    
    file(GLOB SELF_TEMP_SRC_FILES RELATIVE ${PROJECT_SOURCE_DIR} unix/socket/*.cc unix/socket/*.h)
    source_group(unix\\socket FILES ${SELF_TEMP_SRC_FILES})
    list(APPEND SELF_SRC_FILES ${SELF_TEMP_SRC_FILES})

endif()

if(ANDROID)

    add_definitions(-DUSING_XLOG_WEAK_FUNC)

    file(GLOB SELF_ANDROID_SRC_FILES RELATIVE ${PROJECT_SOURCE_DIR}
            android/*.cc
            android/*.c
            jni/*.cc
            jni/*.c
            jni/util/*.cc)
        
    list(APPEND SELF_SRC_FILES ${SELF_ANDROID_SRC_FILES})
elseif(APPLE)

    file(GLOB SELF_TEMP_SRC_FILES RELATIVE ${PROJECT_SOURCE_DIR} objc/*.mm objc/*.h)
    source_group(objc FILES ${SELF_TEMP_SRC_FILES})
    list(APPEND SELF_SRC_FILES ${SELF_TEMP_SRC_FILES} debugger/debugger_utils.c)
endif()


BuildWithUnitTest("${PROJECT_NAME}" "${SELF_SRC_FILES}")
    
    
