#!/usr/bin/env python3
import os
import sys
import glob

from mars_utils import *


SCRIPT_PATH = os.path.split(os.path.realpath(__file__))[0]

BUILD_OUT_PATH = 'cmake_build/iOS'
INSTALL_PATH = BUILD_OUT_PATH + '/iOS.out'

OPEN_SSL_ARCHS = ['x86_64', 'arm64']
#由于Mars官方项目自带的Openssl不带模拟器Arm64 和 x86架构
#只能手动自己编译Openssl 关于模拟器架构
#使用项目 https://github.com/SerenitySpace/openssl-spm 手动魔改编译模拟器架构
#生成后放到了 /mars/mars/openssl-simulator-arch 下


def build_ios_xlog_xcframework(tag=''):
    print('Building ios xcframework')
    gen_mars_revision_file('comm', tag)
    clean(BUILD_OUT_PATH)
    if os.path.exists(BUILD_OUT_PATH):
        shutil.rmtree(BUILD_OUT_PATH)
    os.makedirs(BUILD_OUT_PATH)
    if os.path.exists(INSTALL_PATH):
        shutil.rmtree(INSTALL_PATH)
    os.makedirs(INSTALL_PATH)
    #生成不同架构的SSL和Crypto
    ssl_lib = INSTALL_PATH + '/ssl'
    if not lipo_thin_libs('openssl/openssl_lib_iOS/libssl.a', ssl_lib, OPEN_SSL_ARCHS):
        return False

    crypto_lib = INSTALL_PATH + '/crypto'
    if not lipo_thin_libs('openssl/openssl_lib_iOS/libcrypto.a', crypto_lib, OPEN_SSL_ARCHS):
        return False
    
    archs = ['OS','SIMULATOR64','SIMULATORARM64']
    for arch in archs:
        genLib(arch)
  
    #创建XCFramework
    #合并Simulator 新建framework
    dst_simulator_path = INSTALL_PATH + '/ios-arm64_x86_64-simulator'
    if os.path.exists(dst_simulator_path):
        shutil.rmtree(dst_simulator_path)
    os.makedirs(dst_simulator_path)

    dst_simulator_lib = dst_simulator_path + '/mars'
    dst_simulator_framework_path = dst_simulator_path + '/mars.framework'

    cmd = 'lipo -create %s %s -output %s' % ( INSTALL_PATH + '/SIMULATOR64-lib' , INSTALL_PATH + '/SIMULATORARM64-lib'  , dst_simulator_lib)
    ret = os.system(cmd)
    if ret != 0:
        print('!!!!!!!!!!!Create Simulatro Framework: %s fail!!!!!!!!!!!!!!!' % dst_simulator_lib)
        return False
    make_static_framework(dst_simulator_lib, dst_simulator_framework_path, XLOG_COPY_HEADER_FILES, '../')

    #创建OS Framework
    dst_os_path = INSTALL_PATH + '/ios-arm64'
    if os.path.exists(dst_os_path):
        shutil.rmtree(dst_os_path)

    os.makedirs(dst_os_path)
    dst_os_framework_path = dst_os_path + '/mars.framework'
    copy_file(INSTALL_PATH + '/OS' , dst_os_path + '/mars' )
    dst_os_lib = dst_os_path + '/mars'
    make_static_framework(dst_os_lib, dst_os_framework_path, XLOG_COPY_HEADER_FILES, '../')

    print('Creating XCFramework')
    xcframeworkdst = INSTALL_PATH + '/mars.xcframework'
    xcmd = 'xcodebuild -create-xcframework -framework %s -framework %s -output %s' % (dst_os_framework_path , dst_simulator_framework_path , xcframeworkdst)
    ret = os.system(xcmd)
    if ret != 0:
        print('!!!!!!!!!!!xcframework  fail!!!!!!!!!!!!!!!')
        return False

    print('==================Output========================')
    print(xcframeworkdst)

#Arch: OS , SIMULATOR64 , SIMULATORARM64
def genLib(arch = ''):
    print('Building Arch:%s Lib'%(arch)) 
    clean(BUILD_OUT_PATH)
    os.chdir(BUILD_OUT_PATH)
    #生成不同架构的产物
    build_cmd = 'cmake ../.. -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=../../ios.toolchain.cmake -DPLATFORM=%s -DENABLE_ARC=0 -DENABLE_BITCODE=0 -DENABLE_VISIBILITY=1 && make -j8 && make install' % (arch)
    ret = os.system(build_cmd)
    os.chdir(SCRIPT_PATH)
    if ret != 0:
        print('!!!!!!!!!!!build os fail!!!!!!!!!!!!!!!')
        assert('Build Fail')
        return False
    lib_name = '%s-lib' %(arch)
    ssl_crypto_lib = INSTALL_PATH + '/crypto.arm64'
    if arch == 'OS':
        lib_name = 'OS'
        ssl_crypto_lib = INSTALL_PATH + '/crypto.arm64'
    elif arch == 'SIMULATOR64':
        lib_name == 'ios-simulatro64'
        ssl_crypto_lib = SCRIPT_PATH + '/openssl-simulator-arch/x86/libcrypto.a'

    elif arch == 'SIMULATORARM64':
        lib_name == 'ios-simulatrox86'
        ssl_crypto_lib = SCRIPT_PATH + '/openssl-simulator-arch/arm64/libcrypto.a'

    else:
        assert('不支持架构,需要手动扩展')


    libtool_os_dst_lib = INSTALL_PATH + '/' + lib_name
    libtool_src_lib = [INSTALL_PATH + '/libcomm.a',
                        INSTALL_PATH + '/libmars-boost.a',
                        INSTALL_PATH + '/libxlog.a',
                        BUILD_OUT_PATH + '/zstd/libzstd.a']
    
    libtool_src_lib.append(ssl_crypto_lib)


    if not libtool_libs(libtool_src_lib, libtool_os_dst_lib):
        return False




def main():
    build_ios_xlog_xcframework()

if __name__ == '__main__':
    print('\n\n')
    print('============注意:============')
    print('\n\n')
    print('xlog使用了SSL中的符号,官方脚本XLog编译出来无法使用,MD5符号在 libcrypto 产物中')
    print('使用自编译的Openssl模拟器产物,可以到 https://github.com/SerenitySpace/openssl-spm 手动编译')
    print('\n\n\n\n\n')
    main()
