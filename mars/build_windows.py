#!/usr/bin/env python
import os
import sys
import glob
import time
import shutil
import platform

from mars_utils import *

SCRIPT_PATH = os.path.split(os.path.realpath(__file__))[0]


BUILD_OUT_PATH = 'cmake_build/Windows'
WIN_LIBS_INSTALL_PATH = BUILD_OUT_PATH + "/Windows.out/"
WIN_RESULT_DIR = WIN_LIBS_INSTALL_PATH + 'win/'
WIN_BUILD_CMD = 'cmake ../.. -G "Visual Studio 14 2015" -T v140_xp && cmake --build . --target install --config %s'
WIN_GEN_PROJECT_CMD = 'cmake ../.. -G "Visual Studio 14 2015" -T v140_xp'
SSL_ARCH = 'x86'

def build_windows(incremental, tag='', config='Release'):
    before_time = time.time()
    gen_mars_revision_file('comm', tag)

    clean_windows(BUILD_OUT_PATH, incremental)
    os.chdir(BUILD_OUT_PATH)
    
    print("build cmd:" + WIN_BUILD_CMD %config)
    ret = os.system(WIN_BUILD_CMD %config)
    os.chdir(SCRIPT_PATH)

    if 0 != ret:
        print('!!!!!!!!!!!!!!!!!!build fail!!!!!!!!!!!!!!!!!!!!')
        return False

    if os.path.exists(WIN_RESULT_DIR):
        shutil.rmtree(WIN_RESULT_DIR)
    os.makedirs(WIN_RESULT_DIR)
        
    src_libs = glob.glob(WIN_LIBS_INSTALL_PATH + '*.lib')
    src_libs.append('openssl/openssl_lib_windows/%s/libcrypto.lib' %SSL_ARCH)
    src_libs.append('openssl/openssl_lib_windows/%s/libssl.lib' %SSL_ARCH)
    
    merge_win_static_libs(src_libs, WIN_RESULT_DIR + 'mars.lib')
    
    headers = dict()
    headers.update(COMM_COPY_HEADER_FILES)
    headers.update(WIN_COPY_EXT_FILES)
    copy_file_mapping(headers, '../', WIN_RESULT_DIR)
    sub_folders = ["app", "baseevent", "comm", "boost", "xlog", "sdt", "stn"]
    copy_windows_pdb(BUILD_OUT_PATH, sub_folders, config, WIN_LIBS_INSTALL_PATH)

    print('==================Output========================')
    print("libs: %s" %(WIN_RESULT_DIR))
    print("pdb files: %s" %(WIN_LIBS_INSTALL_PATH))

    after_time = time.time()
    print("use time:%d s" % (int(after_time - before_time)))
    return True
    
def build_windows_xlog(incremental, tag='', config='Release'):
    before_time = time.time()
    gen_mars_revision_file('comm', tag)

    clean_windows(BUILD_OUT_PATH, incremental)
    os.chdir(BUILD_OUT_PATH)
    
    print("build cmd:" + WIN_BUILD_CMD %config)
    ret = os.system(WIN_BUILD_CMD %config)
    os.chdir(SCRIPT_PATH)

    if 0 != ret:
        print('!!!!!!!!!!!!!!!!!!build fail!!!!!!!!!!!!!!!!!!!!')
        return False

    if os.path.exists(WIN_RESULT_DIR):
        shutil.rmtree(WIN_RESULT_DIR)
    os.makedirs(WIN_RESULT_DIR)
    
    needed_libs = [os.path.normpath(WIN_LIBS_INSTALL_PATH + 'comm.lib'), os.path.normpath(WIN_LIBS_INSTALL_PATH + 'mars-boost.lib'), os.path.normpath(WIN_LIBS_INSTALL_PATH + 'xlog.lib')]
    for lib in glob.glob(WIN_LIBS_INSTALL_PATH + '*.lib'):
        if os.path.normpath(lib) in needed_libs:
            pass
        else:
            os.remove(lib)

    merge_win_static_libs(needed_libs, WIN_RESULT_DIR + 'xlog.lib')
    
    headers = dict()
    headers.update(XLOG_COPY_HEADER_FILES)
    headers.update(WIN_COPY_EXT_FILES)
    copy_file_mapping(headers, '../', WIN_RESULT_DIR)
    
    sub_folders = ["comm", "boost", "xlog"]
    copy_windows_pdb(BUILD_OUT_PATH, sub_folders, config, WIN_LIBS_INSTALL_PATH)

    print('==================Output========================')
    print("libs: %s" %(WIN_RESULT_DIR))
    print("pdb files: %s" %(WIN_LIBS_INSTALL_PATH))

    after_time = time.time()
    print("use time:%d s" % (int(after_time - before_time)))
    return True

def gen_win_project(tag=''):
    before_time = time.time()

    gen_mars_revision_file('comm', tag)
    
    clean_windows(BUILD_OUT_PATH, False)
    os.chdir(BUILD_OUT_PATH)
    ret = os.system(WIN_GEN_PROJECT_CMD)
    os.chdir(SCRIPT_PATH)
    
    
    after_time = time.time()

    print("use time:%d s" % (int(after_time - before_time)))

    if 0 != ret:
        print('!!!!!!!!!!!!!!!!!!gen project file fail!!!!!!!!!!!!!!!!!!!!')
        return False
    return True


def main():

    if not check_vs_env():
        return

    while True:
        if len(sys.argv) >= 3:
            build_windows(False, sys.argv[1], sys.argv[2])
            break
        else:
            num = raw_input('Enter menu(or usage: python build_windows.py <tag> <Debug/Release>):\n'\
            '1. Clean && build mars Release.\n2. Build mars Release incrementally.\n3. Clean && build mars Debug.\n'\
            '4. Build mars Debug incrementally.\n5. Clean && build xlog Release.\n6. Clean && build xlog Debug.\n7. Gen mars project file.\n8. Exit\n')
            if num == '1':
                build_windows(False, config='Release')
                break
            elif num == '2':
                build_windows(True, config='Release')
                break
            elif num == '3':
                build_windows(False, config='Debug')
                break
            elif num == '4':
                build_windows(True, config='Debug')
                break
            elif num == '5':
                build_windows_xlog(False, config='Release')
                break
            elif num == '6':
                build_windows_xlog(False, config='Debug')
                break
            elif num == '7':
                gen_win_project()
                break
            elif num == '8':
                break
            else:
                build_windows(False)
                break

if __name__ == '__main__':
    main()
