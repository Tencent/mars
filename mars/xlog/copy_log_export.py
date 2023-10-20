#!/usr/bin/env python

import os
import shutil

HEADER_FILE_SUFFIX = ".h"

XLOG_SRC_PATH = "../comm/xlogger"
XLOG_EXPORT_PATH = "export_include/xlogger/"

def cpHeaderFiles():
	for dirpath, dirnames, filenames in os.walk(XLOG_SRC_PATH):
		for filename in filenames:
			if os.path.splitext(filename)[1] == HEADER_FILE_SUFFIX:
				shutil.copy(os.path.join(dirpath, filename), XLOG_EXPORT_PATH)
	
	
def main():
	cpHeaderFiles()
	
if __name__ == "__main__":
	main()
