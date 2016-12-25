TEMP_LOCAL_PATH :=$(call my-dir)
APP_ABI := ${_ARCH_}
APP_STL := stlport_shared
APP_CPPFLAGS +=-std=gnu++11
APP_MODULES := crypto
