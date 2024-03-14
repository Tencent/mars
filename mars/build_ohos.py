#!/usr/bin/env python3
import os
import sys
import glob
import time
import shutil
import platform

from mars_utils import *


SCRIPT_PATH = os.path.split(os.path.realpath(__file__))[0]

def system_is_windows():
    return platform.system() == 'Windows'

def system_architecture_is64():
    return platform.machine().endswith('64')


if system_is_windows():
    OHOS_GENERATOR = '-G "Unix Makefiles"'
else:
    OHOS_GENERATOR = ''

try:
    OHOS_SDK_ROOT = os.environ['OHOS_SDK_ROOT']
except KeyError as identifier:
    OHOS_SDK_ROOT = ''


BUILD_OUT_PATH = 'cmake_build/ohos'
OHOS_LIBS_INSTALL_PATH = BUILD_OUT_PATH + '/'
OHOS_BUILD_CMD = 'cmake %s -DOHOS_ARCH="%s" ' \
                    '-DOHOS_PLATFORM=OHOS -DCMAKE_TOOLCHAIN_FILE=%s/openharmony/11/native/build/cmake/ohos.toolchain.cmake ' \
                    '-DOHOS_STL="c++_shared" ' \
                    '&& cmake --build . '
OHOS_SYMBOL_PATH = 'ohoslibraries/mars_ohos_sdk/obj/local/'
OHOS_LIBS_PATH = 'ohoslibraries/mars_ohos_sdk/libs/'
OHOS_XLOG_SYMBOL_PATH = 'ohoslibraries/mars_xlog_sdk/obj/local/'
OHOS_XLOG_LIBS_PATH = 'ohoslibraries/mars_xlog_sdk/libs/'


OHOS_STL_FILE = {
        'armeabi-v7a': OHOS_SDK_ROOT + '/openharmony/11/native/llvm/lib/arm-linux-ohos/libc++_shared.so',
        'x86': OHOS_SDK_ROOT + '/sources/cxx-stl/llvm-libc++/libs/x86_64-linux-ohos/libc++_shared.so',
        'arm64-v8a': OHOS_SDK_ROOT + '/openharmony/11/native/llvm/lib/aarch64-linux-ohos/libc++_shared.so',
        'x86_64': OHOS_SDK_ROOT + '/sources/cxx-stl/llvm-libc++/libs/x86_64-linux-ohos/libc++_shared.so',
        }



def build_ohos(incremental, arch, target_option=''):

    before_time = time.time()
    
    clean(BUILD_OUT_PATH, incremental)
    os.chdir(BUILD_OUT_PATH)
    
    build_cmd = OHOS_BUILD_CMD %(SCRIPT_PATH, arch, OHOS_SDK_ROOT)
    print("build cmd:" + build_cmd)
    ret = os.system(build_cmd)
    os.chdir(SCRIPT_PATH)

    if 0 != ret:
        print('!!!!!!!!!!!!!!!!!!build fail!!!!!!!!!!!!!!!!!!!!')
        return False

    if len(target_option) > 0:
        symbol_path = OHOS_XLOG_SYMBOL_PATH
        lib_path = OHOS_XLOG_LIBS_PATH
    else:
        symbol_path = OHOS_SYMBOL_PATH
        lib_path = OHOS_LIBS_PATH

    if not os.path.exists(symbol_path):
        os.makedirs(symbol_path)

    symbol_path = symbol_path + arch
    if os.path.exists(symbol_path):
        shutil.rmtree(symbol_path)

    os.mkdir(symbol_path)

    
    if not os.path.exists(lib_path):
        os.makedirs(lib_path)

    lib_path = lib_path + arch
    if os.path.exists(lib_path):
        shutil.rmtree(lib_path)

    os.mkdir(lib_path)


    for f in glob.glob(OHOS_LIBS_INSTALL_PATH + "*.so"):
        shutil.copy(f, symbol_path)
        shutil.copy(f, lib_path)

    # copy stl
    shutil.copy(OHOS_STL_FILE[arch], symbol_path)
    shutil.copy(OHOS_STL_FILE[arch], lib_path)


    print('==================Output========================')
    print('libs(release): %s' %(lib_path))
    print('symbols(must store permanently): %s' %(symbol_path))


    after_time = time.time()

    print("use time:%d s" % (int(after_time - before_time)))
    return True

def main(incremental, archs, target_option='', tag=''):
    gen_mars_revision_file(SCRIPT_PATH + '/comm', tag)

    # if os.path.exists(OHOS_LIBS_PATH):
    #     shutil.rmtree(OHOS_LIBS_PATH)

    # if os.path.exists(OHOS_SYMBOL_PATH):
    #     shutil.rmtree(OHOS_SYMBOL_PATH)

    for arch in archs:
        if not build_ohos(incremental, arch, target_option):
            return

if __name__ == '__main__':

    while True:
        if len(sys.argv) >= 3:
            archs = sys.argv[2:]
            main(False, archs, tag=sys.argv[1])
            break
        else:
            archs = {'armeabi-v7a', 'arm64-v8a'}
            num = input('Enter menu:\n1. Clean && build mars.\n2. Build incrementally mars.\n3. Clean && build xlog.\n4. Exit\n')
            if num == '1':
                main(False, archs)
                break
            elif num == '2':
                main(True, archs)
                break
            elif num == '3':
                main(False, archs, '--target libzstd_static marsxlog')
                break
            elif num == '4':
                break
            else:
                main(False, archs)
                break


