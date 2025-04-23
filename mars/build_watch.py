#!/usr/bin/env python3
import os
import sys
import glob

from mars_utils import *


SCRIPT_PATH = os.path.split(os.path.realpath(__file__))[0]

BUILD_OUT_PATH = 'cmake_build/watchos'
INSTALL_PATH = os.path.join(BUILD_OUT_PATH, 'watchOS.out')

WATCH_BUILD_SIMULATOR_CMD = 'cmake ../.. -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=../../ios.toolchain.cmake -DPLATFORM=SIMULATOR_WATCHOS -DIOS_DEPLOYMENT_TARGET=2.0 -DENABLE_ARC=0 -DENABLE_BITCODE=0 -DENABLE_VISIBILITY=1 && make -j8 && make install'
WATCH_BUILD_OS_CMD = 'cmake ../.. -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=../../ios.toolchain.cmake -DPLATFORM=WATCHOS -DIOS_DEPLOYMENT_TARGET=2.0 -DENABLE_ARC=0 -DENABLE_BITCODE=0 -DENABLE_VISIBILITY=1 && make -j8 && make install'

GEN_WATCH_OS_PROJ = 'cmake ../.. -G Xcode -DCMAKE_TOOLCHAIN_FILE=../../ios.toolchain.cmake -DPLATFORM=OS -DIOS_DEPLOYMENT_TARGET=2.0 -DIOS_ARCH="armv7;arm64" -DCMAKE_XCODE_ATTRIBUTE_VALID_ARCHS="armv7k;arm64_32;arm64" -DENABLE_ARC=0 -DENABLE_BITCODE=0 -DENABLE_VISIBILITY=1'


def build_watch(tag=''):
    gen_mars_revision_file('comm', tag)

    clean(BUILD_OUT_PATH)
    os.chdir(BUILD_OUT_PATH)

    ret = os.system(WATCH_BUILD_OS_CMD)
    os.chdir(SCRIPT_PATH)
    if ret != 0:
        print('!!!!!!!!!!!build os fail!!!!!!!!!!!!!!!')
        return False

    libtool_os_dst_lib = INSTALL_PATH + '/os'
    libtool_src_libs = [INSTALL_PATH + '/libcomm.a',
                        INSTALL_PATH + '/libmars-boost.a',
                        INSTALL_PATH + '/libxlog.a',
                        BUILD_OUT_PATH + '/zstd/libzstd.a']
    if not libtool_libs(libtool_src_libs, libtool_os_dst_lib):
        return False

    clean(BUILD_OUT_PATH)
    os.chdir(BUILD_OUT_PATH)
    ret = os.system(WATCH_BUILD_SIMULATOR_CMD)
    os.chdir(SCRIPT_PATH)
    if ret != 0:
        print('!!!!!!!!!!!build simulator fail!!!!!!!!!!!!!!!')
        return False

    libtool_simulator_dst_lib = INSTALL_PATH + '/simulator'
    if not libtool_libs(libtool_src_libs, libtool_simulator_dst_lib):
        return False

    dst_framework_path = INSTALL_PATH + '/device/mars.framework'
    make_static_framework(libtool_os_dst_lib, dst_framework_path, XLOG_COPY_HEADER_FILES, '../')
    os.system('mv ' + dst_framework_path + '/os ' + dst_framework_path + '/mars')

    dst_framework_simulator_path = INSTALL_PATH + '/simu/mars.framework'
    make_static_framework(libtool_simulator_dst_lib, dst_framework_simulator_path, XLOG_COPY_HEADER_FILES, '../')
    os.system('mv ' + dst_framework_simulator_path + '/simulator ' + dst_framework_simulator_path + '/mars')
    os.system('xcodebuild -create-xcframework -framework "' + dst_framework_path + '" -framework "' + dst_framework_simulator_path + '" -output "' + INSTALL_PATH + '/mars.xcframework"')

    print('==================Output========================')
    print(INSTALL_PATH + "/mars.xcframework")
    return True

def gen_watch_project():
    gen_mars_revision_file('comm')
    clean(BUILD_OUT_PATH)
    os.chdir(BUILD_OUT_PATH)

    ret = os.system(GEN_WATCH_OS_PROJ)
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
            build_watch(sys.argv[1])
            break
        else:
            num = input('Enter menu:\n1. Clean && build.\n2. Gen Watch Project.\n3. Exit\n')
            if num == '1':
                build_watch()
                break
            elif num == '2':
                gen_watch_project()
                break
            elif num == '3':
                break
            else:
                build_watch()
                break

if __name__ == '__main__':
    main()
