#!/usr/bin/env python

import sys
import os
import glob
import time
import getpass
import time

from mars_utils import *

LIBRARIES_PATH = os.path.split(os.path.realpath(__file__))[0]


class Project:
    def __init__(self, path, platform, platform_folders, des_lib_name, framework_name, other_cflags):
        self.path = path
        self.platform = platform
        self.platform_folders = platform_folders
        self.des_lib_name = des_lib_name
        self.framework_name = framework_name
        self.other_cflags = other_cflags

iphone_project = Project(RELATIVE_PATH + "mars-open-iphone.xcodeproj", "iphone", set(["Release-iphoneos", "Release-iphonesimulator"]), "mars", "mars.framework", "")
iphone_xlog_project = Project(RELATIVE_PATH + "mars-log-iphone.xcodeproj", "iphone", set(["Release-iphoneos", "Release-iphonesimulator"]), "mars", "mars.framework", "")
iphone_project_with_bitcode = Project(RELATIVE_PATH + "mars-open-iphone.xcodeproj", "iphone", set(["Release-iphoneos", "Release-iphonesimulator"]), "mars", "mars.framework", "-fembed-bitcode")
mac_project = Project(RELATIVE_PATH + "mars-open-mac.xcodeproj", "macosx", set(["Release"]), "mars", "mars.framework", "")

APPLE_PROJECTS = (iphone_project, iphone_project_with_bitcode, iphone_xlog_project, mac_project)


def get_child_project(project_path):
    PROJECT_FILE = project_path + "/project.pbxproj"
    BEGIN_FILEREFERENCE = "Begin PBXFileReference section"
    END_FILEREFERENCE = "End PBXFileReference section"
    XCODE_PROJ_SUFFIX = "xcodeproj"
    FOLDER_BEGIN = "path = "
    FOLDER_END = "/"

    projects = []
    folders = []
    file = open(PROJECT_FILE)
    is_filereference_section = False
    if not file:
        printf("open project file fail,build failed!!!")
        return (folders, projects)

    for line in file:
        if END_FILEREFERENCE in line:
            break

        if is_filereference_section and XCODE_PROJ_SUFFIX in line:
            folder = line[line.find(FOLDER_BEGIN) + len(FOLDER_BEGIN):]
            project = folder[folder.find(FOLDER_END) + len(FOLDER_END):folder.find(".xcodeproj")]
            folder = folder[:folder.find(FOLDER_END)]
            if "\"" in folder:
                folder = folder[1:]
            if os.path.exists(RELATIVE_PATH + folder):
                folders.append(folder)
                projects.append(project)
            else:
                print("\033[0;31;40m%s not exist\033[0m" % (folder))

        if BEGIN_FILEREFERENCE in line:
            is_filereference_section = True

    file.close()
    return (folders, projects)

def build_apple(project, save_path):
    gen_revision_file(save_path, save_path)

    print('##############display system info####################')
    xcode_version = os.popen("xcodebuild -version").read()
    print(xcode_version)

    xcode_sdks = os.popen("xcodebuild -showsdks").read()
    print(xcode_sdks)

    print('#####################################################')

    targets = []

    while project.platform in xcode_sdks:
        xcode_sdks = xcode_sdks[xcode_sdks.find(project.platform):]
        target = xcode_sdks[:xcode_sdks.find("\n")]
        xcode_sdks = xcode_sdks[xcode_sdks.find("\n"):]
        targets.append(target)

        ret = os.system("xcodebuild clean -sdk %s -configuration Release -project %s" %(target, project.path))
        if ret:
            print("\033[0;31;40m!!!!clean %s failed!!!\033[0m" %(target))
            return False

        if "simulator" in target:
            ret = os.system("xcodebuild  -sdk %s -configuration Release -project %s" %(target, project.path))
        else:
            ret = os.system('xcodebuild %s -sdk %s -configuration Release -project %s' %(('' if not project.other_cflags else  'OTHER_CFLAGS="' + project.other_cflags + '"'), target, project.path))

        if ret:
            print("\033[0;31;40m!!!!build %s failed!!!\033[0m" %(target))
            return False

    
    save_path = os.path.join(LIBRARIES_PATH, save_path + "/" + os.path.splitext(os.path.split(project.path)[1])[0] + project.other_cflags)
    if not os.path.exists(save_path):
        os.makedirs(save_path)


    print("**************merging libs, wait for a monment!!!***********************")

    cpu_num = 0

    (child_folders, child_projects) = get_child_project(project.path)
 
    for i, cf in enumerate(child_folders):
        print("merging %s..........." %(cf))
        for pf in project.platform_folders:
            obj_folders = "%s%s/build/%s.build/%s/%s.build/Objects-normal"  %(RELATIVE_PATH, cf, child_projects[i], pf, cf)
            cpu_folders = os.listdir(obj_folders)
            if cpu_num == 0:
                cpu_num = len(cpu_folders)

            if cpu_num != len(cpu_folders):
                print("\033[0;31;40m!!!ERROR: the quantity of valid architectures is different, project:%s, platform:%s!!!\033[0m" %(cf, pf))
                return False

            for cpu_folder in cpu_folders:
                cpu_lib_path = "%s/%s.a" %(save_path, cpu_folder)
                if not os.path.isfile(cpu_lib_path):
                    os.system("ar -cur %s %s/%s/*.o 2>/dev/null" %(cpu_lib_path, obj_folders, cpu_folder))
                else:
                    os.system("ar -q %s %s/%s/*.o 2>/dev/null" %(cpu_lib_path, obj_folders, cpu_folder))


    framework_path = save_path + "/" + project.framework_name
    if not os.path.exists(framework_path):
        os.mkdir(framework_path)

    
    print("ranlib ing..............")
    os.system("ranlib %s/*.a 2>/dev/null" %(save_path))
    print("lipo ing..............")
    os.system("lipo -create '%s/'*.a -output %s/%s" %(save_path, framework_path, project.des_lib_name))
    print("rming tmp files........")
    os.system("rm -r '%s/'*.a" %(save_path))
    copy_files(RELATIVE_PATH, framework_path + "/Headers", save_path, APPLE_COPY_EXT_FILES, child_folders)

    return True
      

def main():
    
    if not check_python_version():
        return

    if 2 <= len(sys.argv):
        prefix = sys.argv[1]
    else:
        prefix = raw_input("input prefix for save directory. like `trunk`,`br`,`tag`: ").strip()
    
    save_path = prefix + "_[%s]@%s@%s" % (time.strftime('%Y-%m-%d_%H.%M', time.localtime()), get_revision(RELATIVE_PATH), getpass.getuser())
    
    while True:
        num = raw_input("\033[0;33mEnter menu:\n1. build mars for iphone.\n2. build mars for iphone with bitcode.\n3. build xlog for iphone\n4. build mars for macosx.\n5. build all.\n6. exit.\033[0m\n").strip()
        if num == "1" or num == "2" or num == "3" or num == "4":
            build_apple(APPLE_PROJECTS[int(num)-1], save_path)
            return
        elif num == "5":
            for project in APPLE_PROJECTS:
                build_apple(project, save_path)
            return
        elif num == "6":
            print("exit!")
            return
        else:
            continue;

if __name__ == "__main__":
    before_time = time.time()
    
    main()

    after_time = time.time()

    print("use time:%d s" % (int(after_time - before_time)))
