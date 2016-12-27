#!/usr/bin/env python

import sys
import os
import glob
import shutil


MARS_OPEN_LIBRARIES_PATH = os.path.join(os.path.split(os.path.realpath(__file__))[0], "../../../../mars/libraries")
sys.path.append(MARS_OPEN_LIBRARIES_PATH)

from build_apple import *
from mars_utils import *

SAMPLE_LIBS_DIRECTORY = "samplelibs"
proj = APPLE_PROJECTS[0]

SAMPLE_LIBS_PATH = RELATIVE_PATH + "libraries/" + SAMPLE_LIBS_DIRECTORY + "/" + os.path.splitext(os.path.split(proj.path)[1])[0] + proj.other_cflags + "/" + proj.framework_name
DES_PATH =  os.path.join(os.path.split(os.path.realpath(__file__))[0], "MarsLib/" +  proj.framework_name)

def main():

    if not check_python_version():
        exit("python env error")

    if build_apple(proj, SAMPLE_LIBS_DIRECTORY):
        pass
    else:
        exit("build mars fail!")

    if os.path.exists(DES_PATH):
        shutil.rmtree(DES_PATH)
    shutil.copytree(SAMPLE_LIBS_PATH, DES_PATH)



if __name__ == "__main__":
    
    main()
   
