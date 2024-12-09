#!/usr/bin/env python3
import os
import sys
import glob
import re
import shutil

from mars_utils import *


SCRIPT_PATH = os.path.split(os.path.realpath(__file__))[0]

BUILD_OUT_PATH = 'cmake_build/iOS'
INSTALL_PATH = BUILD_OUT_PATH + '/iOS.out'

GEN_IOS_OS_PROJ = 'cmake ../.. -G Xcode -DCMAKE_TOOLCHAIN_FILE=../../ios.toolchain.cmake -DPLATFORM=OS -DIOS_ARCH="arm64" -DENABLE_ARC=0 -DENABLE_BITCODE=0 -DENABLE_VISIBILITY=1'

def generate_lib(platform, deps = None, filter = None):
    clean(BUILD_OUT_PATH)
    os.chdir(BUILD_OUT_PATH)

    command = 'cmake ../.. -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=../../ios.toolchain.cmake -DPLATFORM=%s -DENABLE_ARC=0 -DENABLE_BITCODE=0 -DENABLE_VISIBILITY=1 && make -j8 && make install' % platform
    ret = os.system(command)
    os.chdir(SCRIPT_PATH)
    if ret != 0:
        print('!!!!!!!!!!!build %s fail!!!!!!!!!!!!!!!' % platform)
        return None

    libs = glob.glob(INSTALL_PATH + '/*.a')
    libs.append(BUILD_OUT_PATH + '/zstd/libzstd.a')
    if filter:
        pattern = re.compile(filter)
        libs = [lib for lib in libs if pattern.search(lib)]
    if deps:
        libs.extend(deps)
    dest_lib = '%s/mars-%s' % (INSTALL_PATH, platform)
    if not libtool_libs(libs, dest_lib):
        return None
    return dest_lib

def make_mars_framework(src_libs, platform, header_files):
    lib = INSTALL_PATH + '/mars'
    if not libtool_libs(src_libs, lib):
        return None

    path = '%s/%s/mars.framework' % (INSTALL_PATH, platform)
    if os.path.exists(path):
        shutil.rmtree(path)
    os.makedirs(path)
    make_static_framework(lib, path, header_files, '../')
    return path

def make_mars_xcframework(frameworks):
    path = INSTALL_PATH + '/mars.xcframework'
    option = ' '.join(['-framework %s' % framework for framework in frameworks])
    command = 'xcodebuild -create-xcframework %s -output %s' % (option, path)
    ret = os.system(command)
    if ret != 0:
        print('!!!!!!!!!!!make xcframework fail!!!!!!!!!!!!!!!')
        return None
    return path

def extract_arch(lib, arch):
    dst_lib = lib.replace('.a', '-%s.a' % arch)
    command = 'lipo %s -extract %s -o %s' % (lib, arch, dst_lib)
    ret = os.system(command)
    if ret != 0:
        print('!!!!!!!!!!!extract %s fail!!!!!!!!!!!!!!!' % arch)
        return None
    return dst_lib

def build_ios(tag=''):
    gen_mars_revision_file('comm', tag)

    deps = ['openssl/openssl_lib_iOS/libssl.a', 'openssl/openssl_lib_iOS/libcrypto.a']

    os_deps = [extract_arch(dep, 'arm64') for dep in deps]
    os_lib = generate_lib('OS64', deps=os_deps)
    for dep in os_deps:
        os.remove(dep)
    if not os_lib:
        return False
    os_framework = make_mars_framework([os_lib], 'OS', COMM_COPY_HEADER_FILES)
    if not os_framework:
        return False

    simulator_deps = [extract_arch(dep, 'x86_64') for dep in deps]
    simulator_lib = generate_lib('SIMULATOR64', deps=simulator_deps)
    for dep in simulator_deps:
        os.remove(dep)
    if not simulator_lib:
        return False
    simulator_framework = make_mars_framework([simulator_lib], 'SIMULATOR', COMM_COPY_HEADER_FILES)
    if not simulator_framework:
        return False

    xcframework_path = make_mars_xcframework([os_framework, simulator_framework])
    if not xcframework_path:
        return False

    print('==================Output========================')
    print(xcframework_path)
    return True

def build_ios_xlog(tag=''):
    gen_mars_revision_file('comm', tag)

    filter = '(libcomm.a|libmars-boost.a|libxlog.a|libzstd.a)'

    os_lib = generate_lib('OS64', filter=filter)
    if not os_lib:
        return False
    os_framework = make_mars_framework([os_lib], 'OS', XLOG_COPY_HEADER_FILES)
    if not os_framework:
        return False

    simulator64_lib = generate_lib('SIMULATOR64', filter=filter)
    if not simulator64_lib:
        return False
    simulatorarm64_lib = generate_lib('SIMULATORARM64', filter=filter)
    if not simulatorarm64_lib:
        return False
    simulator_framework = make_mars_framework([simulator64_lib, simulatorarm64_lib], 'SIMULATOR', XLOG_COPY_HEADER_FILES)
    if not simulator_framework:
        return False

    xcframework_path = make_mars_xcframework([os_framework, simulator_framework])
    if not xcframework_path:
        return False

    print('==================Output========================')
    print(xcframework_path)
    return True



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
            num = input('Enter menu:\n1. Clean && build mars.\n2. Clean && build xlog.\n3. Gen iOS mars Project.\n4. Exit\n')
            if num == '1':
                build_ios()
                break
            if num == '2':
                build_ios_xlog()
                break
            elif num == '3':
                gen_ios_project()
                break
            elif num == '4':
                break
            else:
                build_ios()
                break

if __name__ == '__main__':
    main()
