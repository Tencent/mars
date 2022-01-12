#!/usr/bin/env python
import os
import sys
import glob

from mars_utils import *


SCRIPT_PATH = os.path.split(os.path.realpath(__file__))[0]

BUILD_OUT_PATH = 'cmake_build/iOS'
INSTALL_PATH = BUILD_OUT_PATH + '/Darwin.out'

IOS_BUILD_SIMULATOR_CMD = 'cmake ../.. -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=../../ios.toolchain.cmake -DIOS_PLATFORM=SIMULATOR64 -DIOS_ARCH="x86_64" -DENABLE_ARC=0 -DENABLE_BITCODE=0 -DENABLE_VISIBILITY=1 && make -j8 && make install'
IOS_BUILD_SIMULATOR_ARM64_CMD = 'cmake ../.. -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=../../ios.toolchain.cmake -DIOS_PLATFORM=SIMULATORARM64 -DIOS_ARCH="arm64" -DENABLE_ARC=0 -DENABLE_BITCODE=0 -DENABLE_VISIBILITY=1 && make -j8 && make install'
IOS_BUILD_OS_CMD = 'cmake ../.. -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=../../ios.toolchain.cmake -DIOS_PLATFORM=OS -DIOS_ARCH="arm64" -DENABLE_ARC=0 -DENABLE_BITCODE=0 -DENABLE_VISIBILITY=1 && make -j8 && make install'

GEN_IOS_OS_PROJ = 'cmake ../.. -G Xcode -DCMAKE_TOOLCHAIN_FILE=../../ios.toolchain.cmake -DIOS_PLATFORM=OS -DIOS_ARCH="arm64" -DENABLE_ARC=0 -DENABLE_BITCODE=0 -DENABLE_VISIBILITY=1'
OPEN_SSL_ARCHS = ['x86_64', 'arm64']


def build_ios(tag=''):
    gen_mars_revision_file('comm', tag)
    
    clean(BUILD_OUT_PATH)
    os.chdir(BUILD_OUT_PATH)
    
    ret = os.system(IOS_BUILD_OS_CMD)
    os.chdir(SCRIPT_PATH)
    if ret != 0:
        print('!!!!!!!!!!!build os fail!!!!!!!!!!!!!!!')
        return False

    libtool_os_dst_lib = INSTALL_PATH + '/os'
    libtool_src_lib = glob.glob(INSTALL_PATH + '/*.a')
    libtool_src_lib.append(BUILD_OUT_PATH + '/zstd/libzstd.a')

    if not libtool_libs(libtool_src_lib, libtool_os_dst_lib):
        return False

    clean(BUILD_OUT_PATH)
    os.chdir(BUILD_OUT_PATH)
    ret = os.system(IOS_BUILD_SIMULATOR_CMD)
    os.chdir(SCRIPT_PATH)
    if ret != 0:
        print('!!!!!!!!!!!build simulator fail!!!!!!!!!!!!!!!')
        return False
    
    libtool_simulator_dst_lib = INSTALL_PATH + '/simulator'
    if not libtool_libs(libtool_src_lib, libtool_simulator_dst_lib):
        return False

    lipo_src_libs = []
    lipo_src_libs.append(libtool_os_dst_lib)
    lipo_src_libs.append(libtool_simulator_dst_lib)
    ssl_lib = INSTALL_PATH + '/ssl'
    if not lipo_thin_libs('openssl/openssl_lib_iOS/libssl.a', ssl_lib, OPEN_SSL_ARCHS):
        return False

    crypto_lib = INSTALL_PATH + '/crypto'
    if not lipo_thin_libs('openssl/openssl_lib_iOS/libcrypto.a', crypto_lib, OPEN_SSL_ARCHS):
        return False

    lipo_src_libs.append(ssl_lib)
    lipo_src_libs.append(crypto_lib)

    lipo_dst_lib = INSTALL_PATH + '/mars'

    if not libtool_libs(lipo_src_libs, lipo_dst_lib):
        return False

    dst_framework_path = INSTALL_PATH + '/mars.framework'
    make_static_framework(lipo_dst_lib, dst_framework_path, COMM_COPY_HEADER_FILES, '../')

    print('==================Output========================')
    print(dst_framework_path)
    return True

def build_ios_xlog(tag=''):
    gen_mars_revision_file('comm', tag)
    
    clean(BUILD_OUT_PATH)
    os.chdir(BUILD_OUT_PATH)
    
    ret = os.system(IOS_BUILD_OS_CMD)
    os.chdir(SCRIPT_PATH)
    if ret != 0:
        print('!!!!!!!!!!!build os fail!!!!!!!!!!!!!!!\n')
        return False

    libtool_os_dst_lib = INSTALL_PATH + '/os_arm64'
    libtool_src_libs = [INSTALL_PATH + '/libcomm.a',
                        INSTALL_PATH + '/libmars-boost.a',
                        INSTALL_PATH + '/libxlog.a',
                        BUILD_OUT_PATH + '/zstd/libzstd.a']
    if not libtool_libs(libtool_src_libs, libtool_os_dst_lib):
        return False

    clean(BUILD_OUT_PATH)
    os.chdir(BUILD_OUT_PATH)
    ret = os.system(IOS_BUILD_SIMULATOR_CMD)
    os.chdir(SCRIPT_PATH)
    if ret != 0:
        print('!!!!!!!!!!!build simulator x86_64 fail!!!!!!!!!!!!!!!\n')
        return False
    
    libtool_simulator_x86_64_dst_lib = INSTALL_PATH + '/simulator_x86_64'
    if not libtool_libs(libtool_src_libs, libtool_simulator_x86_64_dst_lib):
        return False

    lipo_src_libs = []
    lipo_src_libs.append(libtool_os_dst_lib)
    lipo_src_libs.append(libtool_simulator_x86_64_dst_lib)
    lipo_dst_lib = INSTALL_PATH + '/mars'

    if not lipo_libs(lipo_src_libs, lipo_dst_lib):
        return False

    dst_framework_path = INSTALL_PATH + '/mars.framework'
    make_static_framework(lipo_dst_lib, dst_framework_path, XLOG_COPY_HEADER_FILES, '../')

    print('==================Output========================\n')
    print(dst_framework_path)


def build_ios_xlog_xcframework(tag=''):
    gen_mars_revision_file('comm', tag)

    clean(BUILD_OUT_PATH)
    os.chdir(BUILD_OUT_PATH)

    ret = os.system(IOS_BUILD_OS_CMD)
    os.chdir(SCRIPT_PATH)
    if ret != 0:
        print('!!!!!!!!!!!build os fail!!!!!!!!!!!!!!!\n')
        return False

    libtool_os_dst_lib = INSTALL_PATH + '/mars'
    libtool_src_libs = [INSTALL_PATH + '/libcomm.a',
                        INSTALL_PATH + '/libmars-boost.a',
                        INSTALL_PATH + '/libxlog.a',
                        BUILD_OUT_PATH + '/zstd/libzstd.a']
    if not libtool_libs(libtool_src_libs, libtool_os_dst_lib):
        return False

    dst_iphoneos_arm64_framework_path = INSTALL_PATH + '/iphoneos/mars.framework'
    make_static_framework(libtool_os_dst_lib, dst_iphoneos_arm64_framework_path,
                          XLOG_COPY_HEADER_FILES, '../')

    print('==================Output iphoneos arm64 ========================\n')
    print(dst_iphoneos_arm64_framework_path)

    clean(BUILD_OUT_PATH)
    os.chdir(BUILD_OUT_PATH)
    ret = os.system(IOS_BUILD_SIMULATOR_CMD)
    os.chdir(SCRIPT_PATH)
    if ret != 0:
        print('!!!!!!!!!!!build simulator x86_64 fail!!!!!!!!!!!!!!!\n')
        return False

    libtool_simulator_x86_64_dst_lib = INSTALL_PATH + '/mars_simulator_x86_64'
    if not libtool_libs(libtool_src_libs, libtool_simulator_x86_64_dst_lib):
        return False

    clean(BUILD_OUT_PATH)
    os.chdir(BUILD_OUT_PATH)
    ret = os.system(IOS_BUILD_SIMULATOR_ARM64_CMD)
    os.chdir(SCRIPT_PATH)
    if ret != 0:
        print('!!!!!!!!!!!build simulator arm64 fail!!!!!!!!!!!!!!!\n')
        return False

    libtool_simulator_arm64_dst_lib = INSTALL_PATH + '/mars_simulator_arm64'
    if not libtool_libs(libtool_src_libs, libtool_simulator_arm64_dst_lib):
        return False

    lipo_src_libs = []
    lipo_src_libs.append(libtool_simulator_x86_64_dst_lib)
    lipo_src_libs.append(libtool_simulator_arm64_dst_lib)
    lipo_dst_lib = INSTALL_PATH + '/mars'
    
    if not lipo_libs(lipo_src_libs, lipo_dst_lib):
        return False

    dst_simulator_x86_64_arm64_framework_path = INSTALL_PATH + \
        '/simulator/mars.framework'
    make_static_framework(lipo_dst_lib, dst_simulator_x86_64_arm64_framework_path,
                          XLOG_COPY_HEADER_FILES, '../')

    print('==================Output simulator_arm64 ========================\n')
    print(dst_simulator_x86_64_arm64_framework_path)

    src_framwworks = [
        dst_iphoneos_arm64_framework_path,
        dst_simulator_x86_64_arm64_framework_path,
    ]

    dst_xcframework_path = INSTALL_PATH + '/mars.xcframework'
    make_xcframework(src_framwworks, dst_xcframework_path)
    print('==================Output xcframework ========================\n')
    print(dst_xcframework_path)

def gen_ios_project():
    gen_mars_revision_file('comm')
    clean(BUILD_OUT_PATH)
    os.chdir(BUILD_OUT_PATH)

    ret = os.system(GEN_IOS_OS_PROJ)
    os.chdir(SCRIPT_PATH)
    if ret != 0:
        print('!!!!!!!!!!!gen fail!!!!!!!!!!!!!!!')
        return False


    print('==================Output========================')
    print('project file: %s/%s' %(SCRIPT_PATH, BUILD_OUT_PATH))
    
    return True

def main():
    while True:
        if len(sys.argv) >= 2:
            build_ios(sys.argv[1])
            break
        else:
            num = raw_input(
                'Enter menu:\n1. Clean && build mars.\n2. Clean && build xlog.\n3. Clean && build xlog xcframework.\n4. Gen iOS mars Project.\n5. Exit\n')
            if num == '1':
                build_ios()
                break
            if num == '2':
                build_ios_xlog()
                break
            if num == '3':
                build_ios_xlog_xcframework()
                break
            elif num == '4':
                gen_ios_project()
                break
            elif num == '5':
                break
            else:
                build_ios()
                break

if __name__ == '__main__':
    main()
