cmake_minimum_required (VERSION 3.6)
project (sample)

set(SELF_LIBS_OUT ${CMAKE_SYSTEM_NAME}.out)

include_directories(.)
include_directories(WTL)
include_directories(../..)
include_directories(../../mars)
include_directories(../../mars/comm/windows)
include_directories(proto/protobuf)


include_directories(../../mars/comm)
include_directories(../../mars/openssl/include)

add_subdirectory(../../mars/log xlog.out)
add_subdirectory(../../mars/app app.out)
add_subdirectory(../../mars/baseevent baseevent.out)
add_subdirectory(../../mars/boost boost.out)
add_subdirectory(../../mars/comm comm.out)
add_subdirectory(../../mars/sdt sdt.out)
add_subdirectory(../../mars/stn stn.out)

add_definitions(-D_WIN32 -DWIN32 -DUNICODE -D_UNICODE)
add_definitions(-D_CRT_NO_VA_START_VALIDATION -D_CRT_SECURE_NO_WARNINGS)

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

set(SELF_SRC_FILES
		Business/ChatCGITask.cpp
		Business/GetConvListCGITask.cpp
		Business/HelloCGITask.cpp
		Business/MarsWrapper.cpp
		ChatDlg.cpp
		MainDlg.cpp
		PingServerDlg.cpp
		proto/generate/chat.pb.cc
		proto/generate/main.pb.cc
		proto/generate/messagepush.pb.cc
		proto/protobuf/google/protobuf/any.cc
		proto/protobuf/google/protobuf/any.pb.cc
		proto/protobuf/google/protobuf/api.pb.cc
		proto/protobuf/google/protobuf/arena.cc
		proto/protobuf/google/protobuf/arenastring.cc
		proto/protobuf/google/protobuf/descriptor.cc
		proto/protobuf/google/protobuf/descriptor.pb.cc
		proto/protobuf/google/protobuf/descriptor_database.cc
		proto/protobuf/google/protobuf/duration.pb.cc
		proto/protobuf/google/protobuf/dynamic_message.cc
		proto/protobuf/google/protobuf/empty.pb.cc
		proto/protobuf/google/protobuf/extension_set.cc
		proto/protobuf/google/protobuf/extension_set_heavy.cc
		proto/protobuf/google/protobuf/field_mask.pb.cc
		proto/protobuf/google/protobuf/generated_message_reflection.cc
		proto/protobuf/google/protobuf/generated_message_table_driven.cc
		proto/protobuf/google/protobuf/generated_message_util.cc
		proto/protobuf/google/protobuf/io/coded_stream.cc
		proto/protobuf/google/protobuf/io/gzip_stream.cc
		proto/protobuf/google/protobuf/io/printer.cc
		proto/protobuf/google/protobuf/io/strtod.cc
		proto/protobuf/google/protobuf/io/tokenizer.cc
		proto/protobuf/google/protobuf/io/zero_copy_stream.cc
		proto/protobuf/google/protobuf/io/zero_copy_stream_impl.cc
		proto/protobuf/google/protobuf/io/zero_copy_stream_impl_lite.cc
		proto/protobuf/google/protobuf/map_field.cc
		proto/protobuf/google/protobuf/message.cc
		proto/protobuf/google/protobuf/message_lite.cc
		proto/protobuf/google/protobuf/reflection_ops.cc
		proto/protobuf/google/protobuf/repeated_field.cc
		proto/protobuf/google/protobuf/service.cc
		proto/protobuf/google/protobuf/source_context.pb.cc
		proto/protobuf/google/protobuf/struct.pb.cc
		proto/protobuf/google/protobuf/stubs/atomicops_internals_x86_gcc.cc
		proto/protobuf/google/protobuf/stubs/atomicops_internals_x86_msvc.cc
		proto/protobuf/google/protobuf/stubs/bytestream.cc
		proto/protobuf/google/protobuf/stubs/common.cc
		proto/protobuf/google/protobuf/stubs/int128.cc
		proto/protobuf/google/protobuf/stubs/mathlimits.cc
		proto/protobuf/google/protobuf/stubs/once.cc
		proto/protobuf/google/protobuf/stubs/status.cc
		proto/protobuf/google/protobuf/stubs/statusor.cc
		proto/protobuf/google/protobuf/stubs/stringpiece.cc
		proto/protobuf/google/protobuf/stubs/stringprintf.cc
		proto/protobuf/google/protobuf/stubs/structurally_valid.cc
		proto/protobuf/google/protobuf/stubs/strutil.cc
		proto/protobuf/google/protobuf/stubs/substitute.cc
		proto/protobuf/google/protobuf/stubs/time.cc
		proto/protobuf/google/protobuf/text_format.cc
		proto/protobuf/google/protobuf/timestamp.pb.cc
		proto/protobuf/google/protobuf/type.pb.cc
		proto/protobuf/google/protobuf/unknown_field_set.cc
		proto/protobuf/google/protobuf/util/delimited_message_util.cc
		proto/protobuf/google/protobuf/util/field_comparator.cc
		proto/protobuf/google/protobuf/util/field_mask_util.cc
		proto/protobuf/google/protobuf/util/internal/datapiece.cc
		proto/protobuf/google/protobuf/util/internal/default_value_objectwriter.cc
		proto/protobuf/google/protobuf/util/internal/error_listener.cc
		proto/protobuf/google/protobuf/util/internal/field_mask_utility.cc
		proto/protobuf/google/protobuf/util/internal/json_escaping.cc
		proto/protobuf/google/protobuf/util/internal/json_objectwriter.cc
		proto/protobuf/google/protobuf/util/internal/json_stream_parser.cc
		proto/protobuf/google/protobuf/util/internal/object_writer.cc
		proto/protobuf/google/protobuf/util/internal/protostream_objectsource.cc
		proto/protobuf/google/protobuf/util/internal/protostream_objectwriter.cc
		proto/protobuf/google/protobuf/util/internal/proto_writer.cc
		proto/protobuf/google/protobuf/util/internal/type_info.cc
		proto/protobuf/google/protobuf/util/internal/utility.cc
		proto/protobuf/google/protobuf/util/json_util.cc
		proto/protobuf/google/protobuf/util/message_differencer.cc
		proto/protobuf/google/protobuf/util/time_util.cc
		proto/protobuf/google/protobuf/util/type_resolver_util.cc
		proto/protobuf/google/protobuf/wire_format.cc
		proto/protobuf/google/protobuf/wire_format_lite.cc
		proto/protobuf/google/protobuf/wrappers.pb.cc
		PublicComponentV2/app_callback.cpp
		PublicComponentV2/longlink_packer.cc
		PublicComponentV2/shortlink_packer.cc
		PublicComponentV2/stn_callback.cpp
		Sample.cpp
		stdafx.cpp
		Wrapper/NetworkService.cpp)
    

add_executable(${PROJECT_NAME} WIN32 ${SELF_SRC_FILES} Sample.rc)
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${SELF_LIBS_OUT})
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
                
set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "/GL /Zc:wchar_t")
set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "/LTCG")

  
if(CMAKE_CL_64)
	add_definitions(-D_WIN64 -DWIN64)
endif()

set(CMAKE_FIND_LIBRARY_PREFIXES "")
set(CMAKE_FIND_LIBRARY_SUFFIXES ".lib")

link_directories(${SELF_LIBS_OUT})

#find_library(CRYPT_LIB crypto PATHS ../../mars/openssl/openssl_lib_windows/x86 NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
#find_library(SSL_LIB ssl PATHS ../../mars/openssl/openssl_lib_windows/x86 NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)

get_filename_component(CRYPT_LIB ../../mars/openssl/openssl_lib_windows/x86/libcrypto.lib ABSOLUTE)
get_filename_component(SSL_LIB ../../mars/openssl/openssl_lib_windows/x86/libssl.lib ABSOLUTE)

target_link_libraries(${PROJECT_NAME} ws2_32.lib comsuppw.lib
						xlog
                        app
                        baseevent
                        mars-boost
                        comm
                        sdt
                        stn
                        ${CRYPT_LIB}
                        ${SSL_LIB})
  