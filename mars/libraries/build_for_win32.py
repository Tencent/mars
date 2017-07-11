#!/usr/bin/env python

import sys
import os
import glob
import time
import getpass
import subprocess
import shutil

from mars_utils import *

PROJECT_PATH = RELATIVE_PATH + "win32proj/"
PROJECT_FILE = "mars.sln"
LIB_NAME = "mars.lib"
BUILD_CONFIGURE = {"Release":'"Release|Win32"', "Debug":'"Debug|Win32"'}

def build_win32_impl(save_path, cfg_folder, cfg):

    gen_revision_file(save_path, save_path)
    vs_tool_dir = os.getenv("VS140COMNTOOLS")
    vs_cmd_path = vs_tool_dir + "../IDE/devenv"
    cmd = '"%s" "%s" /Clean %s' %(vs_cmd_path, PROJECT_PATH + PROJECT_FILE, cfg)
    ret = os.system('"%s"' %cmd)

    if 0 != ret:
        print("Clean Failed")
        return
        
    cmd = '"%s" "%s" /Build %s' %(vs_cmd_path, PROJECT_PATH + PROJECT_FILE , cfg)
    ret = os.system('"%s"' %cmd)
    if 0 != ret:
        print("Build Failed")
        return

    lib_save_path = save_path + "/" + cfg_folder
    if not os.path.exists(lib_save_path):
        os.makedirs(lib_save_path)
        
    shutil.copy(PROJECT_PATH + cfg_folder + "/lib/" + LIB_NAME, lib_save_path)
        
    copy_files(RELATIVE_PATH, save_path + "/include/mars", save_path, WIN_COPY_EXT_FILES)

def build_win32(save_path):
    vs_tool_dir = os.getenv("VS140COMNTOOLS")
    
    if not vs_tool_dir:
        print("You must install visual studio 2015 for build.")
        return
        
    if not os.path.isfile(PROJECT_PATH + PROJECT_FILE):
        print("Project file %s not exist" %PROJECT_PATH)
        return
        
    while True:
        if 3 <= len(sys.argv):
            menu_select = sys.argv[2]
        else:
            menu_select = raw_input("input build config.\n1. Release.\n2. Debug.\n3. all.\n").strip()
        
        if "3" == menu_select:
            for (folder, cfg) in BUILD_CONFIGURE.items():
                build_win32_impl(save_path, folder, cfg)
            return
        elif "1" == menu_select or "2" == menu_select:
            build_win32_impl(save_path, BUILD_CONFIGURE.items()[int(menu_select)-1][0], BUILD_CONFIGURE.items()[int(menu_select)-1][0])
            return
        else:
            pass
    

def main():
    if 2 <= len(sys.argv):
        prefix = sys.argv[1]
    else:
        prefix = raw_input("input prefix for save directory. like `trunk`,`br`,`tag`: ").strip()

    save_path = prefix + "_[%s]@%s@%s" % (time.strftime('%Y-%m-%d_%H.%M', time.localtime()), get_revision(RELATIVE_PATH), getpass.getuser())

    build_win32(save_path)

if __name__ == "__main__":
    main()
