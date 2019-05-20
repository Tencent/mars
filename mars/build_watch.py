#!/usr/bin/env python
import os
import sys
import glob

from mars_utils import *


SCRIPT_PATH = os.path.split(os.path.realpath(__file__))[0]

BUILD_OUT_PATH = 'cmake_build/watchos'
INSTALL_PATH = BUILD_OUT_PATH + '/Darwin.out'

WATCH_BUILD_SIMULATOR_CMD = 'cmake ../.. -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=../../ios.toolchain.cmake -DIOS_PLATFORM=SIMULATOR_WATCHOS -DIOS_DEPLOYMENT_TARGET=2.0 -DIOS_ARCH="i386" -DENABLE_ARC=0 -DENABLE_BITCODE=1 -DENABLE_VISIBILITY=1 && make -j8 && make install'
WATCH_BUILD_OS_CMD = 'cmake ../.. -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=../../ios.toolchain.cmake -DIOS_PLATFORM=WATCHOS -DIOS_DEPLOYMENT_TARGET=2.0 -DIOS_ARCH="armv7k;arm64_32" -DENABLE_ARC=0 -DENABLE_BITCODE=1 -DENABLE_VISIBILITY=1 && make -j8 && make install'

GEN_WATCH_OS_PROJ = 'cmake ../.. -G Xcode -DCMAKE_TOOLCHAIN_FILE=../../ios.toolchain.cmake -DIOS_PLATFORM=OS -DIOS_DEPLOYMENT_TARGET=2.0 -DIOS_ARCH="armv7;arm64" -DCMAKE_XCODE_ATTRIBUTE_VALID_ARCHS="armv7k;arm64_32" -DENABLE_ARC=0 -DENABLE_BITCODE=1 -DENABLE_VISIBILITY=1'


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
    if not libtool_libs(glob.glob(INSTALL_PATH + '/*.a'), libtool_os_dst_lib):
        return False

    clean(BUILD_OUT_PATH)
    os.chdir(BUILD_OUT_PATH)
    ret = os.system(WATCH_BUILD_SIMULATOR_CMD)
    os.chdir(SCRIPT_PATH)
    if ret != 0:
        print('!!!!!!!!!!!build simulator fail!!!!!!!!!!!!!!!')
        return False
    
    libtool_simulator_dst_lib = INSTALL_PATH + '/simulator'
    if not libtool_libs(glob.glob(INSTALL_PATH + '/*.a'), libtool_simulator_dst_lib):
        return False

    lipo_src_libs = []
    lipo_src_libs.append(libtool_os_dst_lib)
    lipo_src_libs.append(libtool_simulator_dst_lib)
    lipo_dst_lib = INSTALL_PATH + '/mars'

    if not lipo_libs(lipo_src_libs, lipo_dst_lib):
        return False

    dst_framework_path = INSTALL_PATH + '/mars.framework'
    make_static_framework(lipo_dst_lib, dst_framework_path, COMM_COPY_HEADER_FILES, '../')

    print('==================Output========================')
    print(dst_framework_path)
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
            num = raw_input('Enter menu:\n1. Clean && build.\n2. Gen Watch Project.\n3. Exit\n')
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
