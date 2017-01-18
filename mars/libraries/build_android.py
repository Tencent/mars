#!/usr/bin/env python

import shutil
import os
import sys
import glob

from mars_utils import *

NDK_BUILD_CMD = "ndk-build _ARCH_=%s NDK_DEBUG=0 -j 4 -B SDK=0 LIBPREFIX=mars -C "
MARS_LIBS_PATH = "mars_android_sdk"
XLOG_LIBS_PATH = "mars_xlog_sdk"


BUILD_XLOG_PATHS = ("openssl", "comm", "log")
COPY_XLOG_FILES = {"../log/crypt/log_crypt.h": "jni/log_crypt.h",
                "../log/crypt/decode_mars_log_file.py": "jni/decode_mars_log_file.py.rewriteme",
                "../mk_template/JNI_OnLoad.cpp": "jni/JNI_OnLoad.cc"
                }

BUILD_MARS_PATHS = ("openssl", "comm", "baseevent", "log", "app", "sdt", "stn")

COPY_MARS_FILES = {"../stn/proto/longlink_packer.h": "jni/longlink_packer.h",
               "../stn/proto/shortlink_packer.h": "jni/shortlink_packer.h",
                "../stn/proto/longlink_packer.cc": "jni/longlink_packer.cc.rewriteme",
                "../stn/proto/shortlink_packer.cc": "jni/shortlink_packer.cc.rewriteme",
                "../log/crypt/log_crypt.h": "jni/log_crypt.h",
                "../log/crypt/log_crypt.cc": "jni/log_crypt.cc.rewriteme",
                "../log/crypt/decode_mars_log_file.py": "jni/decode_mars_log_file.py.rewriteme",
                "../mk_template/JNI_OnLoad.cpp": "jni/JNI_OnLoad.cc"
                }


def build_android_xlog_static_libs(_path="mars_xlog_sdk", _arch="armeabi"):
    libs_save_path = _path + "/mars_libs"
    src_save_path = _path + "/"


    shutil.rmtree(libs_save_path, True)
    for i in range(0, len(BUILD_XLOG_PATHS)):
        shutil.rmtree("../" + BUILD_XLOG_PATHS[i] + "/mars_libs", True)
        shutil.rmtree("../" + BUILD_XLOG_PATHS[i] + "/obj", True)

    gen_revision_file(libs_save_path, libs_save_path)

    for i in range(0, len(BUILD_XLOG_PATHS)):
        if not os.path.exists("../" + BUILD_XLOG_PATHS[i] + "/jni"):
            continue
        if 0 != os.system(NDK_BUILD_CMD %(_arch) + "../" + BUILD_XLOG_PATHS[i]):
            return -1


    for i in range(len(BUILD_XLOG_PATHS)-1, -1, -1):
        if not os.path.exists("../" + BUILD_XLOG_PATHS[i] + "/jni"):
            continue
        
        files = os.listdir("../" + BUILD_XLOG_PATHS[i] + "/libs")
        for f in files:
            if os.path.isfile(f):
                continue

            cpu_libs = os.path.join(libs_save_path, f)
            cpu_symbols = os.path.join(cpu_libs, "symbols")
            #print cpu_libs, cpu_symbols
            if not os.path.exists(cpu_libs):
                os.makedirs(cpu_libs)
            if not os.path.exists(cpu_symbols):
                os.makedirs(cpu_symbols)

            for lib in glob.glob("../" + BUILD_XLOG_PATHS[i] + "/libs/" + f + "/*.so"):
                shutil.copy(lib, cpu_libs)
            for lib in glob.glob("../" + BUILD_XLOG_PATHS[i] + "/obj/local/" + f + "/*.a"):
                shutil.copy(lib, cpu_libs)
            
            for lib in glob.glob("../" + BUILD_XLOG_PATHS[i] + "/obj/local/" + f + "/*.so"):
                shutil.copy(lib, cpu_symbols)



    for (src, dst) in COPY_XLOG_FILES.items():
        dst_path = src_save_path + dst[:dst.rfind("/")]
        if not os.path.exists(dst_path):
            os.makedirs(dst_path)
        if os.path.isfile(src):
            shutil.copy(src, src_save_path + dst)

    print("build succeed!")
    return 0

def build_android_xlog_shared_libs(_path="mars_xlog_sdk", _arch="armeabi"):

    if 0 != build_android_xlog_static_libs(_path, _arch):
        print("build static libs fail!!!")
        return -1


    shutil.rmtree(_path + "/libs", True)
    shutil.rmtree(_path + "/obj", True)


    if 0 != os.system(NDK_BUILD_CMD %(_arch) + _path):
        print("build fail!!!")
        return -1

    print("build succeed!")
    return 0


def build_android_mars_static_libs(_path="mars_android_sdk", _arch="armeabi"):
    libs_save_path = _path + "/mars_libs"
    src_save_path = _path + "/"
    

    shutil.rmtree(libs_save_path, True)
    for i in range(0, len(BUILD_MARS_PATHS)):
        shutil.rmtree("../" + BUILD_MARS_PATHS[i] + "/mars_libs", True)
        shutil.rmtree("../" + BUILD_MARS_PATHS[i] + "/obj", True)

    gen_revision_file(libs_save_path, sys.argv[2] if 3 <= len(sys.argv) else "default")

    for i in range(0, len(BUILD_MARS_PATHS)):
        if not os.path.exists("../" + BUILD_MARS_PATHS[i] + "/jni"):
            continue
        if 0 != os.system(NDK_BUILD_CMD %(_arch) + "../" + BUILD_MARS_PATHS[i]):
            return -1


    for i in range(len(BUILD_MARS_PATHS)-1, -1, -1):
        if not os.path.exists("../" + BUILD_MARS_PATHS[i] + "/jni"):
            continue
        
        files = os.listdir("../" + BUILD_MARS_PATHS[i] + "/libs")
        for f in files:
            if os.path.isfile(f):
                continue

            cpu_libs = os.path.join(libs_save_path, f)
            cpu_symbols = os.path.join(cpu_libs, "symbols")
            if not os.path.exists(cpu_libs):
                os.makedirs(cpu_libs)
            if not os.path.exists(cpu_symbols):
                os.makedirs(cpu_symbols)

            for lib in glob.glob("../" + BUILD_MARS_PATHS[i] + "/libs/" + f + "/*.so"):
                if os.path.isfile(lib):
                    shutil.copy(lib, cpu_libs)
            for lib in glob.glob("../" + BUILD_MARS_PATHS[i] + "/obj/local/" + f + "/*.a"):
                if os.path.isfile(lib):
                    shutil.copy(lib, cpu_libs)
            
            for lib in glob.glob("../" + BUILD_MARS_PATHS[i] + "/obj/local/" + f + "/*.so"):
                if os.path.isfile(lib):
                    shutil.copy(lib, cpu_symbols)



    for (src, dst) in COPY_MARS_FILES.items():
        dst_path = src_save_path + dst[:dst.rfind("/")]
        if not os.path.exists(dst_path):
            os.makedirs(dst_path)
        if os.path.isfile(src):
            shutil.copy(src, src_save_path + dst)

    print("build succeed!")
    return 0

def build_android_mars_shared_libs(_path="mars_android_sdk", _arch="armeabi"):

    if 0 != build_android_mars_static_libs(_path, _arch):
        print("build static libs fail!!!")
        return -1


    shutil.rmtree(_path + "/libs", True)
    shutil.rmtree(_path + "/obj", True)


    if 0 != os.system(NDK_BUILD_CMD %(_arch) + _path):
        print("build fail!!!")
        return -1

    print("build succeed!")
    return 0

def choose_android_mars_jni_arch():
    platforms = ['armeabi', 'x86', 'mips', 'armeabi-v7a', 'arm64-v8a', 'x86_64', 'mips64']
    archnum = raw_input("Enter the architecture which would like to build:\n1. armeabi.\n2. x86.\n3. mips.\n4. armeabi-v7a.\n5. arm64-v8a.\n6. x86_64.\n7. mips64.\n8. exit.\n")

    arr = []
    
    archs = archnum.split(',')
    for i in range(0, len(archs)):
        if archs[i] >= "1" and archs[i] <= str(len(platforms)):
            arr.append(platforms[int(archs[i])-1])

    return arr


    
def main():
    if not check_env():
        return

    while True:
        flag = 0
        archs = []
        if len(sys.argv) >=2 and len(sys.argv[1])==1 and sys.argv[1] >="1" and sys.argv[1] <="5":
            num = sys.argv[1]
            platforms = ['x86', 'x86_64', 'armeabi', 'arm64-v8a', 'armeabi-v7a', 'mips', 'mips64']
            if len(sys.argv) >=3 and sys.argv[2] in platforms:
                global NDK_BUILD_CMD
                NDK_BUILD_CMD = "ndk-build _ARCH_=" + sys.argv[2] + " NDK_DEBUG=0 -j 4 -B SDK=0 LIBPREFIX=mars -C "
                flag = 1
        else:
            num = raw_input("Enter menu:\n1. build mars shared libs.\n2. build mars static libs.\n3. build xlog static libs.\n4. build xlog shared libs.\n5. exit.\n")
            archs = choose_android_mars_jni_arch()
            if len(archs) == 0:
                return
                
        if flag == 1:
            if "1" == num:
                return build_android_mars_shared_libs()
            elif "2" == num:
                return build_android_mars_static_libs()
            elif "3" == num:
                return build_android_xlog_static_libs()
            elif "4" == num:
                return build_android_xlog_shared_libs()
            elif "5" ==num:
                return 0
            else:
                pass
        else:

            
            if "1" == num or "2" == num:
                sdk_path = MARS_LIBS_PATH
            elif "3" == num or "4" == num:
                sdk_path = XLOG_LIBS_PATH
            else:
                continue

            SO_CACHE_DIR = sdk_path + "/so_cache/"
            SO_SYMBOL_CACHE_DIR = sdk_path + "/so_cache/symbol/"
            SO_DES_DIR = sdk_path + "/libs/"
            SO_SYMBOL_DES_IR = sdk_path + "/obj/local/"
            if os.path.exists(SO_CACHE_DIR):
                shutil.rmtree(SO_CACHE_DIR)

            STATIC_CACHE_DIR = sdk_path + "/static_cache/"
            STATIC_DES_DIR = sdk_path + "/mars_libs/"
            if os.path.exists(STATIC_CACHE_DIR):
                shutil.rmtree(STATIC_CACHE_DIR)

            for i in range(0, len(archs)):
                print ("build %s" %(archs[i]))

                arch = archs[i]

                if "1" == num:
                    build_android_mars_shared_libs(MARS_LIBS_PATH, arch)
                elif "2" == num:
                    build_android_mars_static_libs(MARS_LIBS_PATH, arch)
                elif "3" == num:
                    build_android_xlog_static_libs(XLOG_LIBS_PATH, arch)
                elif "4" == num:
                    build_android_xlog_shared_libs(XLOG_LIBS_PATH, arch)

                elif "5" ==num:
                    return 0
                else:
                    return 0

                if "1" == num or "4" == num:
                    libs_cache_dir = SO_CACHE_DIR + arch
                    symbols_cache_dir = SO_SYMBOL_CACHE_DIR + arch
                    libs_des_dir = SO_DES_DIR + arch
                    symbols_des_dir = SO_SYMBOL_DES_IR + arch

                    if not os.path.exists(libs_cache_dir):
                        os.makedirs(libs_cache_dir)
                    if not os.path.exists(symbols_cache_dir):
                        os.makedirs(symbols_cache_dir)

                    for lib in glob.glob(libs_des_dir + "/*.so"):
                        shutil.copy(lib, libs_cache_dir)
                    for lib in glob.glob(symbols_des_dir + "/*.so"):
                        shutil.copy(lib, symbols_cache_dir)

                if "2" == num or "3" == num:
                    sta_cache_dir = STATIC_CACHE_DIR + arch
                    sta_des_dir = STATIC_DES_DIR + arch
                        
                    if not os.path.exists(sta_cache_dir):
                        os.makedirs(sta_cache_dir)
                    
                    for lib in glob.glob(sta_des_dir + "/*"):
                        if  os.path.isfile(lib):
                            shutil.copy(lib, sta_cache_dir)
                        else:
                            for symbollib in glob.glob(sta_des_dir + "/symbols/*.so"):
                                shutil.copytree(sta_des_dir + "/symbols", sta_cache_dir + "/symbols")

            if "1" == num or "4" == num:
                if os.path.exists(SO_DES_DIR):
                    shutil.rmtree(SO_DES_DIR)
                if os.path.exists(SO_SYMBOL_DES_IR):
                    shutil.rmtree(SO_SYMBOL_DES_IR)
                for i in range(0, len(archs)):
                    shutil.copytree(SO_CACHE_DIR + archs[i], SO_DES_DIR + archs[i])
                    shutil.copytree(SO_SYMBOL_CACHE_DIR + archs[i], SO_SYMBOL_DES_IR + archs[i])

            if "2" == num or "3" == num:
                if os.path.exists(STATIC_DES_DIR):
                    shutil.rmtree(STATIC_DES_DIR)
                for i in range(0, len(archs)):
                    shutil.copytree(STATIC_CACHE_DIR + archs[i], STATIC_DES_DIR + archs[i])

            return



if __name__ == "__main__":
    main()
