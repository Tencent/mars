import os
import sys
import time
import platform
import shutil
import getpass

RELATIVE_PATH = os.path.join(os.path.split(os.path.realpath(__file__))[0], "../")

APPLE_COPY_EXT_FILES = {"stn/proto/longlink_packer.h": "longlink_packer.h",
        "stn/proto/longlink_packer.cc": "longlink_packer.cc.rewriteme",
        "stn/proto/stnproto_logic.h": "stnproto_logic.h",
        "comm/socket/local_ipstack.h": "mars.framework/Headers/comm/local_ipstack.h",
        "comm/socket/nat64_prefix_util.h": "mars.framework/Headers/comm/nat64_prefix_util.h",
        "comm/has_member.h" : "mars.framework/Headers/comm/has_member.h",
        "comm/objc/scope_autoreleasepool.h": "mars.framework/Headers/comm/scope_autoreleasepool.h",
        "comm/objc/ThreadOperationQueue.h": "mars.framework/Headers/comm/ThreadOperationQueue.h",
        "log/crypt/log_crypt.cc": "log_crypt.cc.rewriteme",
        "log/crypt/log_crypt.h": "log_crypt.h",
        "stn/proto/shortlink_packer.h": "shortlink_packer.h",
        "stn/proto/shortlink_packer.cc": "shortlink_packer.cc.rewriteme",
        }
        
WIN_COPY_EXT_FILES = {"stn/proto/longlink_packer.h": "longlink_packer.h",
        "stn/proto/stnproto_logic.h": "stnproto_logic.h",
        "stn/proto/longlink_packer.cc__": "longlink_packer.cc.rewriteme",
        "comm/windows/projdef.h": "mars/comm/projdef.h",
        "comm/windows/sys/cdefs.h": "mars/comm/sys/cdefs.h",
        "comm/windows/sys/time.h": "mars/comm/sys/time.h"
        }

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


def parse_as_svn():
    revision = ''
    path = ''
    url = ''

    revision, path, url = '', '', ''
    cmd = 'svn info'
    result = os.popen(cmd).read()
    for line in result.split("\n"):
        if line == "":
            continue
        if line[:3] == "URL":
            values = line.split(": ")
            _len = len(values[1])
            url_full = values[1][:_len - 10]
            url_split = url_full.split("/")
            _count = len(url_split)
            url = url_split[_count - 2] + "/" + url_split[_count - 1]
        if line[:8] == "Revision":
            values = line.split(": ")
            revision = values[1]
        if line[:22] == "Working Copy Root Path":
            values = values = line.split(": ")
            path = values[1]
            print(path)
            path = path.replace('\\', '/')

    return revision, path, url


def parse_as_git():
    revision = os.popen('git rev-parse --short HEAD').read().strip()
    path = os.popen('git rev-parse --abbrev-ref HEAD').read().strip()
    url = ''
    return revision, path, url

def gen_revision_file(save_path, tag):
    now_path = os.path.dirname(os.path.abspath(__file__))
    print (now_path)

    # TRY AS SVN
    revision, path, url = parse_as_svn()

    # TRY AS GIT
    if len(revision) <= 0:
        revision, path, url = parse_as_git()

    build_time = time.strftime("%Y-%m-%d %H:%M:%S", time.localtime(time.time()))
    contents = '''
#ifndef Mars_verinfo_h
#define Mars_verinfo_h

#define MARS_REVISION "%s"
#define MARS_PATH "%s"
#define MARS_URL "%s"
#define MARS_BUILD_TIME "%s"
#define MARS_TAG "%s"

#endif
''' % (revision, path, url, build_time, tag)

    with open('%s/../comm/verinfo.h' % now_path, 'wb') as f:
        f.write(contents)
        f.flush()

  #  if not os.path.exists(save_path):
  #      os.makedirs(save_path)

    version_data = {
        'PublicComponent': {
            'Branch': url,
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

    print (''.join(output.splitlines()))

def get_revision(path):

    revision = os.popen("svn info %s | grep \"Revision\" | awk '{print $2}'" %path).read()
    if revision:
        return "svn-" + revision[:len(revision)-1]

    revision = os.popen("git rev-parse --short HEAD").read()
    if revision:
        return "git-" + revision[:len(revision)-1]
    
    return ""


def copy_files(src_path, dst_header_path, framework_path, ext_files, child_project_folder=[]):
    COPY_HEADER_FILES = {"comm/verinfo.h": "comm",
            "comm/autobuffer.h": "comm",
            "comm/http.h": "comm",
            "comm/time_utils.h": "comm",
            "stn/stn.h": "stn",
            "stn/stn_logic.h": "stn",
            "baseevent/base_logic.h": "baseevent",
            "comm/xlogger/preprocessor.h": "xlog",
            "comm/xlogger/xloggerbase.h": "xlog",
            "comm/xlogger/xlogger.h": "xlog",
            "log/appender.h": "xlog",
            "app/app.h": "app",
            "app/app_logic.h": "app",
            "sdt/sdt.h": "sdt",
            "sdt/sdt_logic.h": "sdt",
            "openssl/export_include/aes_crypt.h": "openssl",
            "openssl/export_include/md5_digest.h": "openssl",
            "openssl/export_include/rsa_crypt.h": "openssl",
            "openssl/export_include/gen_rsa_key_pair.h":"openssl",
            "openssl/export_include/rsa_pss_sha256.h":"openssl",
            "openssl/export_include/ecdsa_verify.h":"openssl",
            "openssl/export_include/ecdh_crypt.h":"openssl",
            }


    for (src, dst) in COPY_HEADER_FILES.items():
        if not os.path.isfile(src_path + "/" + src):
            continue

        if len(child_project_folder) >0 and src[:src.find("/")] not in child_project_folder:
            continue

        if not os.path.exists(dst_header_path + "/" + dst):
            os.makedirs(dst_header_path + "/" + dst)
        shutil.copy(src_path + "/" + src, dst_header_path + "/" + dst)

    for (src, dst) in ext_files.items():
        if dst.rfind("/") != -1 and not os.path.exists(framework_path + "/" + dst[:dst.rfind("/")]):
            os.makedirs(framework_path + "/" + dst[:dst.rfind("/")])
        shutil.copy(src_path + "/" + src, framework_path + "/" + dst)

    #shutil.copy("Readme.md", framework_path)

def check_python_version():
    python_version = sys.version
    if len(python_version) < 3 or python_version[0:3] != "2.7":
        print("Error: python version must be 2.7xxx")
        return False;

    return True


def check_ndk_env():
    system = platform.system()
    path_env = os.getenv("PATH")

    delimiter = ":"

    if "Windows" == system:
        delimiter = ";"
        
    path_array = path_env.split(delimiter)

    ndk_path = None
    for s in path_array:
        if os.path.isfile(os.path.join(s, "ndk-build")) or os.path.isfile(os.path.join(s, "ndk-build.cmd")):
            ndk_path = s

    if not ndk_path:
        print("Error: ndk does not exist or you do not set it into system environment.")
        return False

    if not os.path.isfile(os.path.join(ndk_path, "source.properties")):
        print("Error: source.properties does not exist, make sure ndk's version>=r11c")
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

    if ndk_revision[:4] >= "11.2":
        return True


    print("Error: make sure ndk's version >= r11c")
    return False
    

def check_env():

    return check_python_version() and check_ndk_env()
    


def main(save_path, tag):
    check_env()

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Error: get_revision.py expect 2 arguments")
    else:
        main(sys.argv[1], sys.argv[2])
