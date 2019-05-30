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
WIN_BUILD_CMD = 'cmake ../.. && cmake --build . --target install --config %s'
WIN_GEN_PROJECT_CMD = 'cmake ../..'


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
        
    merge_win_static_libs(glob.glob(WIN_LIBS_INSTALL_PATH + '*.lib'), WIN_RESULT_DIR + 'mars.lib')
    
    headers = dict()
    headers.update(COMM_COPY_HEADER_FILES)
    headers.update(WIN_COPY_EXT_FILES)
    copy_file_mapping(headers, '../../', WIN_RESULT_DIR)
    
    sub_folders = ["app", "baseevent", "comm", "boost", "xlog", "sdt", "stn"]
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
    while True:
        if len(sys.argv) >= 3:
            build_windows(False, sys.argv[1], sys.argv[2])
            break
        else:
            num = raw_input('Enter menu(or usage: python build_windows.py <tag> <Debug/Release>):\n1. Clean && build Release.\n2. Build Release incrementally.\n3. Clean && build Debug.\n4. Build Debug incrementally.\n5. Gen project file.\n6. Exit\n')
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
                gen_win_project()
                break
            elif num == '6':
                break
            else:
                build_windows(False)
                break

if __name__ == '__main__':
    main()
