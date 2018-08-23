#!/usr/bin/env python
import os
import sys
import glob
import time
import shutil
import platform

from mars_utils import *

SCRIPT_PATH = os.path.split(os.path.realpath(__file__))[0]


BUILD_OUT_PATH = 'cmake_build'
WIN_LIBS_INSTALL_PATH = BUILD_OUT_PATH + "/Windows.out/"
WIN_RESULT_DIR = WIN_LIBS_INSTALL_PATH + 'win/'
WIN_BUILD_CMD = 'cmake .. && cmake --build . --target install --config Release'
WIN_GEN_PROJECT_CMD = 'cmake ..'


def build_windows(incremental, tag=''):
    before_time = time.time()
    gen_mars_revision_file('comm', tag)

    clean(BUILD_OUT_PATH, incremental)
    os.chdir(BUILD_OUT_PATH)
    
    print("build cmd:" + WIN_BUILD_CMD)
    ret = os.system(WIN_BUILD_CMD)
    os.chdir(SCRIPT_PATH)

    if 0 != ret:
        print('!!!!!!!!!!!!!!!!!!build fail!!!!!!!!!!!!!!!!!!!!')
        return False

    if os.path.exists(WIN_RESULT_DIR):
        shutil.rmtree(WIN_RESULT_DIR)
    os.makedirs(WIN_RESULT_DIR)
        
    merge_win_static_libs(glob.glob(WIN_LIBS_INSTALL_PATH + '*.lib'), WIN_RESULT_DIR + 'mars.lib')
    copy_file_mapping(COMM_COPY_HEADER_FILES, '../../', WIN_RESULT_DIR)

    print('==================Output========================')
    print("libs: %s" %(WIN_RESULT_DIR))

    after_time = time.time()
    print("use time:%d s" % (int(after_time - before_time)))
    return True

def gen_win_project(tag=''):
    before_time = time.time()

    gen_mars_revision_file('comm', tag)
    
    clean(BUILD_OUT_PATH)
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
        if len(sys.argv) >= 2:
            build_windows(False, sys.argv[1])
            break
        else:
            num = raw_input('Enter menu:\n1. Clean && build.\n2. Build incrementally.\n3. Gen project file.\n4. Exit\n')
            if num == '1':
                build_windows(False)
                break
            elif num == '2':
                build_windows(True)
                break
            elif num == '3':
                gen_win_project()
                break
            elif num == '4':
                break
            else:
                build_windows(False)
                break

if __name__ == '__main__':
    main()
