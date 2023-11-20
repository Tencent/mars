#!/usr/bin/env python3
import os
import glob
import time
import shutil
import argparse
from typing import List
from mars_utils import *

SCRIPT_PATH = os.path.split(os.path.realpath(__file__))[0]


BUILD_OUT_PATH = 'cmake_build/Windows'
WIN_LIBS_INSTALL_PATH = BUILD_OUT_PATH + "/Windows.out/"
WIN_RESULT_DIR = WIN_LIBS_INSTALL_PATH + 'win/'
WIN_BUILD_CMD = 'cmake ../.. -G "Visual Studio 16 2019" -T v142 && cmake --build . --target install --config %s'
WIN_GEN_PROJECT_CMD = 'cmake ../.. -G "Visual Studio 16 2019" -T v142'
SSL_ARCH = 'x86'

def build_windows(incremental:bool, tag='', config:str='', lib_exe_path:str = ""):
    before_time:float = time.time()
    gen_mars_revision_file('comm', tag)

    clean_windows(BUILD_OUT_PATH, incremental)
    os.chdir(BUILD_OUT_PATH)
    
    print("build cmd:" + WIN_BUILD_CMD %config)
    ret:int = os.system(WIN_BUILD_CMD %config)
    os.chdir(SCRIPT_PATH)

    if 0 != ret:
        print('!!!!!!!!!!!!!!!!!!build fail!!!!!!!!!!!!!!!!!!!!')
        return False

    if os.path.exists(WIN_RESULT_DIR):
        shutil.rmtree(WIN_RESULT_DIR)
    os.makedirs(WIN_RESULT_DIR)
        
    src_libs:List[str] = glob.glob(WIN_LIBS_INSTALL_PATH + '*.lib')
    src_libs.append('openssl/openssl_lib_windows/%s/libcrypto.lib' %SSL_ARCH)
    src_libs.append('openssl/openssl_lib_windows/%s/libssl.lib' %SSL_ARCH)
    
    merge_win_static_libs(src_libs, WIN_RESULT_DIR + 'mars.lib', lib_exe_path)
    
    headers:dict[str, str] = dict()
    headers.update(COMM_COPY_HEADER_FILES)
    headers.update(WIN_COPY_EXT_FILES)
    copy_file_mapping(headers, '../', WIN_RESULT_DIR)
    sub_folders = ["app", "baseevent", "comm", "boost", "xlog", "sdt", "stn"]
    copy_windows_pdb(BUILD_OUT_PATH, sub_folders, config, WIN_LIBS_INSTALL_PATH)

    print('==================Output========================')
    print("libs: %s" %(WIN_RESULT_DIR))
    print("pdb files: %s" %(WIN_LIBS_INSTALL_PATH))

    after_time:float = time.time()
    print("use time:%d s" % (int(after_time - before_time)))
    return True
    
def build_windows_xlog(incremental:bool, tag:str='', config:str='Release', lib_exe_path:str = ""):
    before_time:float = time.time()
    gen_mars_revision_file('comm', tag)

    clean_windows(BUILD_OUT_PATH, incremental)
    os.chdir(BUILD_OUT_PATH)
    
    print("build cmd:" + WIN_BUILD_CMD %config)
    ret:int = os.system(WIN_BUILD_CMD %config)
    os.chdir(SCRIPT_PATH)

    if 0 != ret:
        print('!!!!!!!!!!!!!!!!!!build fail!!!!!!!!!!!!!!!!!!!!')
        return False

    if os.path.exists(WIN_RESULT_DIR):
        shutil.rmtree(WIN_RESULT_DIR)
    os.makedirs(WIN_RESULT_DIR)
    
    needed_libs:List[str] = [
        os.path.normpath(WIN_LIBS_INSTALL_PATH + 'comm.lib'), 
        os.path.normpath(WIN_LIBS_INSTALL_PATH + 'mars-boost.lib'),
        os.path.normpath(WIN_LIBS_INSTALL_PATH + 'xlog.lib')
    ]
    for lib in glob.glob(WIN_LIBS_INSTALL_PATH + '*.lib'):
        if os.path.normpath(lib) in needed_libs:
            pass
        else:
            os.remove(lib)

    merge_win_static_libs(needed_libs, WIN_RESULT_DIR + 'xlog.lib', lib_exe_path)
    
    headers:dict[str, str] = dict()
    headers.update(XLOG_COPY_HEADER_FILES)
    headers.update(WIN_COPY_EXT_FILES)
    copy_file_mapping(headers, '../', WIN_RESULT_DIR)
    
    sub_folders:List[str] = ["comm", "boost", "xlog"]
    copy_windows_pdb(BUILD_OUT_PATH, sub_folders, config, WIN_LIBS_INSTALL_PATH)

    print('==================Output========================')
    print("libs: %s" %(WIN_RESULT_DIR))
    print("pdb files: %s" %(WIN_LIBS_INSTALL_PATH))

    after_time:float = time.time()
    print("use time:%d s" % (int(after_time - before_time)))
    return True

def gen_win_project(tag=''):
    before_time:float = time.time()

    gen_mars_revision_file('comm', tag)
    
    clean_windows(BUILD_OUT_PATH, False)
    os.chdir(BUILD_OUT_PATH)
    ret:int = os.system(WIN_GEN_PROJECT_CMD)
    os.chdir(SCRIPT_PATH)
    
    after_time:float = time.time()

    print("use time:%d s" % (int(after_time - before_time)))

    if 0 != ret:
        print('!!!!!!!!!!!!!!!!!!gen project file fail!!!!!!!!!!!!!!!!!!!!')
        return False
    return True


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--config", type=str, help='''"Release" or "Debug"''')
    parser.add_argument("--incremental", type=bool)
    group = parser.add_mutually_exclusive_group()
    group.add_argument("--xlog", action="store_true")
    group.add_argument("--mars", action="store_true")
    group.add_argument("--gen_project", action="store_true")
    args = parser.parse_args()

    print(args)
    assert args.config in ["Release", "Debug", None] and args.incremental in [True, False, None]
    incremental:bool = False
    config:str = "Release"
    if args.config != None:
        config = args.config
    if args.incremental != None:
        incremental = args.incremental

    if "MSVC_BIN_HOST64_PATH" not in os.environ:
        DEFAULT_MSVC_BIN_HOST64_PATH = "C:/Program Files (x86)\Microsoft Visual Studio/2019/Professional/VC/Tools/MSVC/14.29.30133/bin/Hostx64/"
        print("please set MSVC_BIN_HOST64_PATH in environ, something like ", DEFAULT_MSVC_BIN_HOST64_PATH)
        exit(-1)
    LIB_EXE_PATH: str = os.path.join(os.environ["MSVC_BIN_HOST64_PATH"], "x86", "lib.exe")
    if not "MSVC_TOOLS_PATH" in os.environ:
        DEFAULT_MSVC_TOOLS_PATH = "C:/Program Files (x86)\Microsoft Visual Studio/2019/Professional/Common7/Tools"
        print("please set MSVC_TOOLS_PATH in environ, something like ", DEFAULT_MSVC_TOOLS_PATH)
        exit(-1)
    check_vs_env("\"" + os.path.join(os.environ["MSVC_TOOLS_PATH"], "VsDevCmd.bat") + "\"")

    if args.gen_project:
        gen_win_project()
    elif args.xlog:
        build_windows_xlog(incremental=incremental, config=config, lib_exe_path = LIB_EXE_PATH)
    elif args.mars:
        build_windows(incremental=incremental, config=config,  lib_exe_path = LIB_EXE_PATH)
    else:
        assert False


if __name__ == '__main__':
    main()
