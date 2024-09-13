
message(STATUS "==============config ${PROJECT_NAME}====================")

# just debug;release
set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)

add_definitions(-DXLOGGER_TAG="mars::${PROJECT_NAME}")

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if(UNITTEST)
    message("defined UNITTEST option")
    add_definitions(-DUNITTEST)
endif()
if(DISABLE_QUIC_PROTOCOL)
    message("QUIC disabled by option DISABLE_QUIC_PROTOCOL")
    add_definitions(-DDISABLE_QUIC_PROTOCOL)
endif()
if(DISABLE_PCDN_PROTOCOL)
    message("PCDN disabled by option DISABLE_PCDN_PROTOCOL")
    add_definitions(-DDISABLE_PCDN_PROTOCOL)
endif()

if(MSVC)
    # add DEBUG macro .. release has NDEBUG defaultly
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /DDEBUG")

    if(CMAKE_CL_64)
        add_definitions(-D_WIN64 -DWIN64)
    endif()

    add_definitions(-D_WIN32 -DWIN32 -DUNICODE -D_UNICODE -DNOMINMAX -D_LIB)
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP /Zc:threadSafeInit-")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
    
    # generate pdb file
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /Zi")
    set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /DEBUG /OPT:REF /OPT:ICF")
 
    set(CompilerFlags
        CMAKE_CXX_FLAGS
        CMAKE_CXX_FLAGS_DEBUG
        CMAKE_CXX_FLAGS_RELEASE
        CMAKE_C_FLAGS
        CMAKE_C_FLAGS_DEBUG
        CMAKE_C_FLAGS_RELEASE)
        
    foreach(CompilerFlag ${CompilerFlags})
        string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
    endforeach()

else()
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -DDEBUG")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions")
    
endif()
    

if(ANDROID)
    option(CPP_CALL_BACK "use cpp callback" OFF)

    if(CPP_CALL_BACK)
        add_definitions(-DCPP_CALL_BACK)
    endif()
    if(NATIVE_CALLBACK)
        add_definitions(-DNATIVE_CALLBACK)
    endif()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpic -std=gnu++14 -Wno-deprecated-register")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fdata-sections")
    if(DEF_USE_CPP_CALLBACK)
        add_definitions(-DUSE_CPP_CALLBACK=1)
    endif()

elseif(APPLE)

    # for gen xcode project file
    set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "gnu++14")
    set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++")

    # for build
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++14 -stdlib=libc++")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -g -gline-tables-only -Os")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -g -gline-tables-only -Os")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g -O0")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0")
    
    if (NOT DEFINED ENABLE_BITCODE)
        set(ENABLE_BITCODE TRUE CACHE BOOL "Wheter or not to enable bitcode")
    endif()

    if (ENABLE_BITCODE)
        set(CMAKE_XCODE_ATTRIBUTE_ENABLE_BITCODE "YES")
    else()
        set(CMAKE_XCODE_ATTRIBUTE_ENABLE_BITCODE "NO")
    endif()

    set(CMAKE_XCODE_ATTRIBUTE_STRIP_STYLE "Debuging")
    set(CMAKE_XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET "10.0")
    set(CMAKE_XCODE_ATTRIBUTE_OTHER_LDFLAGS "-ObjC")
    set(CMAKE_XCODE_ATTRIBUTE_DEBUG_INFORMATION_FORMAT "dwarf-with-dsym")
    set(CMAKE_XCODE_ATTRIBUTE_CLANG_DEBUG_INFORMATION_LEVEL[variant=Debug] "default")
    set(CMAKE_XCODE_ATTRIBUTE_CLANG_DEBUG_INFORMATION_LEVEL[variant=Release] "line-tables-only")

    if(DEFINED IOS_DEPLOYMENT_TARGET)
        message(STATUS "setting IOS_DEPLOYMENT_TARGET=${IOS_DEPLOYMENT_TARGET}")
        set(CMAKE_XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET "${IOS_DEPLOYMENT_TARGET}")
    endif()

elseif(UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++14 -fPIC -ffunction-sections -fdata-sections -Os")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC -ffunction-sections -fdata-sections -Os")
endif()
 
macro(BuildWithUnitTest projname sourcefiles)
    set(SRCFILES "${sourcefiles}")
    if (UNITTEST)  
        add_library(${projname}.test STATIC ${SRCFILES})
        install(TARGETS ${projname}.test ARCHIVE DESTINATION ${CMAKE_SYSTEM_NAME}.out)
        if (APPLE AND NOT DISABLE_QUIC_PROTOCOL)
            target_link_libraries(${projname}.test PRIVATE "${ANDROMEDA_FRAMEWORK}")
        endif()
    else()
        list(FILTER SRCFILES EXCLUDE REGEX ".*_unittest.cc$")
        list(FILTER SRCFILES EXCLUDE REGEX ".*_mock.cc$")
        add_library(${projname} STATIC ${SRCFILES})
        install(TARGETS ${projname} ARCHIVE DESTINATION ${CMAKE_SYSTEM_NAME}.out)
        if (APPLE AND NOT DISABLE_QUIC_PROTOCOL)
            target_link_libraries(${projname} PRIVATE "${ANDROMEDA_FRAMEWORK}")
        endif()
    endif()
endmacro()

macro(BuildSharedLib name src exp_file_path libs 3rd_libs link_flags)
    message("\n")
    message("name:              ${name}")
    message("src:               ${src}")
    message("exp_file_path:     ${exp_file_path}")
    message("libs:              ${libs}")
    message("3rd_libs:          ${3rd_libs}")
    message("link_flags:        ${link_flags}")
    add_library(${name} SHARED ${src})
    get_filename_component(EXPORT_FILE ${exp_file_path} ABSOLUTE)
    target_link_libraries(${name} PRIVATE
            ${link_flags}
            -Wl,--version-script=${EXPORT_FILE}
            -Wl,--start-group
            ${libs}
            ${3rd_libs}
            -Wl,--end-group
            )
endmacro()

function(read_version_file version_file_path output_version)
  # 读取version文件
  file(STRINGS "${version_file_path}" version_file_content)

  # 提取各个字段的值
  foreach (line IN LISTS version_file_content)
    if (line MATCHES "MAJOR=(.*)")
      set(MAJOR_VERSION "${CMAKE_MATCH_1}")
    elseif (line MATCHES "MINOR=(.*)")
      set(MINOR_VERSION "${CMAKE_MATCH_1}")
    elseif (line MATCHES "BUILD=(.*)")
      set(BUILD_VERSION "${CMAKE_MATCH_1}")
    elseif (line MATCHES "PATCH=(.*)")
      set(PATCH_VERSION "${CMAKE_MATCH_1}")
    endif()
  endforeach()

  # 拼接字符串
  set(version_string "${MAJOR_VERSION}.${MINOR_VERSION}.${BUILD_VERSION}.${PATCH_VERSION}")

  # 设置输出变量
  set(${output_version} ${version_string} PARENT_SCOPE)
endfunction()
