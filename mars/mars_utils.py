#!/usr/bin/env python3
import os
import shutil
import time
import subprocess
import hashlib
from typing import List
from typing import Dict

COMM_COPY_HEADER_FILES = {
    "mars/comm/verinfo.h": "comm",
    "mars/comm/autobuffer.h": "comm",
    "mars/comm/http.h": "comm",
    "mars/comm/time_utils.h": "comm",
    "mars/comm/strutil.h": "comm",
    "mars/comm/string_cast.h": "comm",
    "mars/comm/comm_data.h": "comm",
    "mars/comm/platform_comm.h": "comm",
    "mars/comm/socket/local_ipstack.h": "comm",
    "mars/comm/socket/nat64_prefix_util.h": "comm",
    "mars/comm/has_member.h": "comm",
    "mars/comm/objc/scope_autoreleasepool.h": "comm",
    "mars/comm/objc/ThreadOperationQueue.h": "comm",

    "mars/comm/jni/util/JNI_OnLoad.h": "comm/jni/util",
    "mars/comm/jni/util/scope_jenv.h": "comm/jni/util",
    "mars/comm/jni/util/var_cache.h": "comm/jni/util",

    "mars/comm/socket/unix_socket.h": "comm/socket",
    "mars/comm/network/netinfo_util.h": "comm/network",

    "mars/comm/xlogger/preprocessor.h": "xlog",
    "mars/comm/xlogger/xloggerbase.h": "xlog",
    "mars/comm/xlogger/xlogger.h": "xlog",
    "mars/comm/xlogger/loginfo_extract.h": "xlog",

    "mars/boot/context.h": "boot",
    "mars/boot/base_manager.h": "boot",

    "mars/stn/stn.h": "stn",
    "mars/stn/config.h": "stn",
    "mars/stn/stn_logic.h": "stn",
    "mars/stn/stn_manager.h": "stn",
    "mars/stn/task_profile.h": "stn",
    "mars/stn/proto/stnproto_logic.h": "stn",

    "mars/baseevent/base_logic.h": "baseevent",

    "mars/xlog/appender.h": "xlog",
    "mars/xlog/xlogger_interface.h": "xlog",

    "mars/app/app.h": "app",
    "mars/app/app_logic.h": "app",
    "mars/app/app_manager.h": "app",

    "mars/sdt/sdt.h": "sdt",
    "mars/sdt/sdt_logic.h": "sdt",
    "mars/sdt/sdt_manager.h": "sdt",
    "mars/sdt/constants.h": "sdt",
    "mars/sdt/netchecker_profile.h": "sdt",

    "mars/zstd/lib/zstd.h": "",
}

WIN_COPY_EXT_FILES = {
    "mars/comm/platform_comm.h": "comm",
    "mars/comm/windows/sys/cdefs.h": "comm/windows/sys",
    "mars/comm/windows/sys/time.h": "comm/windows/sys",
    "mars/comm/windows/zlib/zlib.h": "comm/windows/zlib",
    "mars/comm/windows/zlib/zconf.h": "comm/windows/zlib",
}

UNIX_COPY_EXT_FILES = {
    "mars/comm/platform_comm.h": "comm",
    "mars/comm/xlogger/loginfo_extract.h": "xlog",
}

XLOG_COPY_HEADER_FILES = {
    "mars/comm/verinfo.h": "comm",
    "mars/comm/autobuffer.h": "comm",
    "mars/comm/http.h": "comm",
    "mars/comm/time_utils.h": "comm",
    "mars/comm/strutil.h": "comm",
    "mars/comm/string_cast.h": "comm",
    "mars/comm/comm_data.h": "comm",
    "mars/comm/socket/local_ipstack.h": "comm",
    "mars/comm/socket/nat64_prefix_util.h": "comm",
    "mars/comm/has_member.h": "comm",
    "mars/comm/objc/scope_autoreleasepool.h": "comm",
    "mars/comm/objc/ThreadOperationQueue.h": "comm",
    "mars/comm/xlogger/preprocessor.h": "xlog",
    "mars/comm/xlogger/xloggerbase.h": "xlog",
    "mars/comm/xlogger/xlogger.h": "xlog",
    "mars/xlog/appender.h": "xlog",
    "mars/xlog/xlogger_interface.h": "xlog",
}


class bcolors:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'


def libtool_libs(src_libs, dst_lib):
    src_lib_str = ''
    for l in src_libs:
        src_lib_str = '%s %s' % (src_lib_str, l)

    print(src_lib_str)
    ret = os.system('libtool -static -no_warning_for_no_symbols -o %s %s' % (dst_lib, src_lib_str))
    if ret != 0:
        print('!!!!!!!!!!!libtool %s fail!!!!!!!!!!!!!!!' % dst_lib)
        return False

    return True


def lipo_libs(src_libs, dst_lib):
    src_lib_str = u''
    for l in src_libs:
        src_lib_str = '%s %s' % (src_lib_str, l)

    cmd = 'lipo -create %s -output %s' % (src_lib_str, dst_lib)
    ret = os.system(cmd)
    if ret != 0:
        print('!!!!!!!!!!!lipo_libs %s fail, cmd:%s!!!!!!!!!!!!!!!' % (dst_lib, cmd))
        return False

    return True


def lipo_thin_libs(src_lib, dst_lib, archs):
    tmp_results = []
    for arch in archs:
        if len(archs) == 1:
            tmp_result = dst_lib
        else:
            tmp_result = dst_lib + '.' + arch

        cmd = 'lipo %s -thin %s -output %s' % (src_lib, arch, tmp_result)
        ret = os.system(cmd)
        if ret != 0:
            print('!!!!!!!!!!!lipo_thin_libs %s fail, cmd:%s!!!!!!!!!!!!!!!' % (tmp_result, cmd))
            return False
        tmp_results.append(tmp_result)

    if len(archs) == 1:
        return True
    else:
        return lipo_libs(tmp_results, dst_lib)


GENERATE_DSYM_FILE_CMD = 'dsymutil %s -o %s'


def gen_dwarf_with_dsym(src_dylib, dst_dsym):
    os.system(GENERATE_DSYM_FILE_CMD % (src_dylib, dst_dsym))


def remove_cmake_files(path):
    cmake_files = path + '/CMakeFiles'
    if os.path.exists(cmake_files):
        shutil.rmtree(cmake_files)

    make_files = path + '/Makefile'
    if os.path.isfile(make_files):
        os.remove(make_files)

    cmake_cache = path + '/CMakeCache.txt'
    if os.path.isfile(cmake_cache):
        os.remove(cmake_cache)

    for f in glob.glob(path + '/*.a'):
        os.remove(f)
    for f in glob.glob(path + '/*.so'):
        os.remove(f)


def remove_if_exist(path: str):
    if not os.path.exists(path):
        return
    if os.path.isdir(path):
        shutil.rmtree(path)
    else:
        os.remove(path)


def clean_except(path, except_list):
    for fpath, dirs, fs in os.walk(path):
        in_except = False
        for exc in except_list:
            if exc in fpath:
                in_except = True
                break
        if not in_except:
            remove_cmake_files(fpath)

    if not os.path.exists(path):
        os.makedirs(path)


def clean(path, incremental=False):
    if not incremental:
        for fpath, dirs, fs in os.walk(path):
            remove_cmake_files(fpath)

    if not os.path.exists(path):
        os.makedirs(path)


def clean_windows(path, incremental):
    try:
        if os.path.exists(path) and not incremental:
            shutil.rmtree(path)
        if not os.path.exists(path):
            os.makedirs(path)
    except Exception:
        pass


def copy_windows_pdb(cmake_out, sub_folder, config, dst_folder):
    for sf in sub_folder:
        # src_file = "%s/%s/" %(cmake_out, sf)
        # dirs = glob.glob(src_file + "*.dir")
        # if len(dirs) != 1:
        #     print("Warning: %s path error." %src_file)
        #     continue

        src_file = "%s/%s/%s" % (cmake_out, sf, config)
        pdbs = glob.glob(src_file + "/*.pdb")
        if len(pdbs) != 1:
            print("Warning: %s path error." % src_file)
            continue
        pdb = pdbs[0]
        if os.path.isfile(pdb):
            shutil.copy(pdb, dst_folder)
        else:
            print("%s not exists" % pdb)


def is_different_file(file1: str, file2: str) -> bool:
    assert os.path.exists(file1)
    if not os.path.exists(file2):
        return True
    md51: str = hashlib.md5(open(file1, 'rb').read()).hexdigest()
    md52: str = hashlib.md5(open(file2, 'rb').read()).hexdigest()
    return md51 != md52


# dst是文件的路径，不是文件所在的文件夹的路径
def copy_file(src, dst):
    assert os.path.isfile(src), src

    if dst.rfind("/") != -1 and not os.path.exists(dst[:dst.rfind("/")]):
        os.makedirs(dst[:dst.rfind("/")])
    if dst.rfind("\\") != -1 and not os.path.exists(dst[:dst.rfind("\\")]):
        os.makedirs(dst[:dst.rfind("\\")])

    if is_different_file(src, dst):
        shutil.copy(src, dst)


def copy_file_mapping(header_file_mappings: Dict[str, str], header_files_src_base: str, header_files_dst_end: str):
    for (src, dst) in header_file_mappings.items():
        copy_file(os.path.join(header_files_src_base, src),
                  os.path.join(header_files_dst_end, dst, src[src.rfind("/") + 1:]))


# dst为复制后文件夹的路径，而非文件夹的父目录的路径。
# 若复制前dst非空，本函数不会清空dst文件夹中的内容。
def copy_folder(src: str, dst: str):
    assert os.path.exists(src), src
    assert os.path.isdir(src), src
    if not os.path.exists(dst):
        os.makedirs(dst)

    for name in os.listdir(src):
        absolute_src_file: str = os.path.join(src, name)
        absolute_dst_file: str = os.path.join(dst, name)
        if os.path.isdir(absolute_src_file):
            copy_folder(absolute_src_file, absolute_dst_file)
        else:
            copy_file(absolute_src_file, absolute_dst_file)


def make_static_framework(src_lib, dst_framework, header_file_mappings, header_files_src_base='./'):
    if os.path.exists(dst_framework):
        shutil.rmtree(dst_framework)

    os.makedirs(dst_framework)
    shutil.copy(src_lib, dst_framework)

    framework_path = dst_framework + '/Headers'
    for (src, dst) in header_file_mappings.items():
        copy_file(os.path.join(header_files_src_base, src),
                  os.path.join(framework_path, dst, src[src.rfind("/") + 1:]))

    return True


def check_ndk_env():
    try:
        ndk_path = os.environ['NDK_ROOT']
    except KeyError:
        print("Error: ndk does not exist or you do not set it into NDK_ROOT.")
        return False

    if ndk_path is not None and ndk_path.strip():
        print("ndk path:%s" % ndk_path)

    if not ndk_path:
        print("Error: ndk does not exist or you do not set it into NDK_ROOT.")
        return False

    if not os.path.isfile(os.path.join(ndk_path, "source.properties")):
        print("Error: source.properties does not exist, make sure ndk's version==r16b")
        return False

    ndk_revision = None

    f = open(os.path.join(ndk_path, "source.properties"))
    line = f.readline()
    while line:
        if line.startswith("Pkg.Revision") and len(line.split("=")) == 2:
            ndk_revision = line.split("=")[1].strip()
        line = f.readline()

    f.close()

    if not ndk_revision or len(ndk_revision) < 4:
        print("Error: parse source.properties fail")
        return False

    if ndk_revision[:4] >= "16.1":
        return True

    print("Error: make sure ndk's version >= r16b")
    return False


html_css = '''
<style type="text/css">
.description table {
    margin: 10px 0 15px 0;
    border-collapse: collapse;
    font-family: Helvetica, "Hiragino Sans GB", Arial, sans-serif;
    font-size: 11px;
    line-height: 16px;
    color: #737373;
    background-color: white;
    margin: 10px 12px 10px 12px;
}
.description td,th { border: 1px solid #ddd; padding: 3px 10px; }
.description th { padding: 5px 10px; }
.description a { color: #0069d6; }
.description a:hover { color: #0050a3; text-decoration: none; }
.description h5 { font-size: 14px; }
</style>
'''

html_table_template = '''
<div class="description">
<h5>{title}</h5>
<table>
<thead>
<tr>
<th align="left">KEY</th>
<th align="left">VALUE</th>
</tr>
</thead>
{table_rows}
</table>
</div>
'''

html_row_template = '''
<tr>
<td align="left">{key}</td>
<td align="left">{value}</td>
</tr>
'''


def parse_as_git(path):
    curdir = os.getcwd()
    os.chdir(path)
    revision = os.popen('git rev-parse --short HEAD').read().strip()
    path = os.popen('git rev-parse --abbrev-ref HEAD').read().strip()
    url = ''
    os.chdir(curdir)

    return revision, path, url


def gen_mars_revision_file(version_file_path, tag=''):
    revision, path, url = parse_as_git(version_file_path)

    timestamp = int(time.time())
    build_time = time.strftime("%Y-%m-%d %H:%M:%S", time.localtime(timestamp))

    contents = '''
#ifndef Mars_verinfo_h
#define Mars_verinfo_h

#define MARS_REVISION "%s"
#define MARS_PATH "%s"
#define MARS_URL "%s"
#define MARS_BUILD_TIME "%s"
#define MARS_TAG "%s"
#define MARS_BUILD_TIMESTAMP %u

#endif
''' % (revision, path, url, build_time, tag, timestamp)

    with open('%s/verinfo.h' % version_file_path, 'wb') as f:
        f.write(contents.encode())
        f.flush()

    version_data = {
        'PublicComponent': {
            'Branch': path,
            'Revision': revision,
            'BuildTag': tag,
            'BuildTime': build_time
        }
    }

    output = '[[==BUILD_DESCRIPTION==]]Revision: %s %s' % (
        version_data['PublicComponent']['Revision'],
        '&nbsp;' * 18)
    html = html_css
    data_type = 'PublicComponent'
    html_rows = ''
    for key in sorted(version_data[data_type].keys()):
        html_rows += html_row_template.format(key=key, value=version_data[data_type][key])

    html += html_table_template.format(title=data_type, table_rows=html_rows)

    output += html

    print(''.join(output.splitlines()))


def check_vs_env(bat_path: str) -> bool:
    if (bat_path == None):
        return False
    p = subprocess.Popen(bat_path)
    p.wait()
    if p.returncode != 0:
        print('!!!!!!!!!!! %s fail!!!!!!!!!!!!!!!' % (bat_path))
        return False

    return True


def merge_win_static_libs(src_libs: List[str], dst_lib: str, lib_exe_path: str) -> bool:
    assert lib_exe_path != None
    assert src_libs.count != 0
    assert dst_lib != None
    lib_cmd_list: list[str] = [lib_exe_path, '/OUT:' + dst_lib]
    lib_cmd_list.extend(src_libs)
    lib_cmd: str = ' '.join(lib_cmd_list)

    p = subprocess.Popen(lib_cmd)
    p.wait()
    if p.returncode != 0:
        print('!!!!!!!!!!! %s fail!!!!!!!!!!!!!!!' % lib_cmd)
        return False

    return True


import glob

if __name__ == '__main__':
    lipo_thin_libs(u'/Users/garry/Documents/gitcode/mmnet/mars/openssl/openssl_lib_iOS/libcrypto.a',
                   u'/Users/garry/Documents/gitcode/mmnet/mars/openssl/openssl_lib_iOS/libcrypto_test.a',
                   ['x86_64', 'arm64'])
