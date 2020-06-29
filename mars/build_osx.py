#!/usr/bin/env python
import os
import sys
import glob

from mars_utils import *


SCRIPT_PATH = os.path.split(os.path.realpath(__file__))[0]

BUILD_OUT_PATH = 'cmake_build/OSX'
INSTALL_PATH = BUILD_OUT_PATH + '/Darwin.out'

OSX_BUILD_OS_CMD = 'cmake ../.. -DCMAKE_BUILD_TYPE=Release -DENABLE_ARC=0 -DENABLE_BITCODE=0 && make -j8 && make install'

GEN_OSX_PROJ = 'cmake ../.. -G Xcode -DCMAKE_OSX_DEPLOYMENT_TARGET:STRING=10.9 -DENABLE_BITCODE=0'

def build_osx(tag=''):
    gen_mars_revision_file('comm', tag)


    clean(BUILD_OUT_PATH)
    os.chdir(BUILD_OUT_PATH)
    
    ret = os.system(OSX_BUILD_OS_CMD)
    os.chdir(SCRIPT_PATH)
    if ret != 0:
        print('!!!!!!!!!!!build fail!!!!!!!!!!!!!!!')
        return False

    ssl_lib = 'openssl/openssl_lib_osx/libssl.a'
    crypto_lib = 'openssl/openssl_lib_osx/libcrypto.a'
    libtool_os_dst_lib = INSTALL_PATH + '/mars'
    libtool_src_libs = glob.glob(INSTALL_PATH + '/*.a')
    libtool_src_libs.append(ssl_lib)
    libtool_src_libs.append(crypto_lib)
    if not libtool_libs(libtool_src_libs, libtool_os_dst_lib):
        return False

    dst_framework_path = INSTALL_PATH + '/mars.framework'
    make_static_framework(libtool_os_dst_lib, dst_framework_path, COMM_COPY_HEADER_FILES, '../')

    print('==================Output========================')
    print(dst_framework_path)
    return True

def gen_ios_project():
    gen_mars_revision_file('comm')

    clean(BUILD_OUT_PATH)
    os.chdir(BUILD_OUT_PATH)

    ret = os.system(GEN_OSX_PROJ)
    os.chdir(SCRIPT_PATH)
    if ret != 0:
        print('!!!!!!!!!!!gen fail!!!!!!!!!!!!!!!')
        return False

    return True

def main():
    while True:
        if len(sys.argv) >= 2:
            build_osx(sys.argv[1])
            break
        else:
            num = raw_input('Enter menu:\n1. Clean && build.\n2. Gen OSX Project.\n3. Exit\n')
            if num == '1':
                build_osx()
                break
            elif num == '2':
                gen_ios_project()
                break
            elif num == '3':
                break
            else:
                build_osx()
                break

if __name__ == '__main__':
    main()
