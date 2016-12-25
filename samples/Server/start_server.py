#!/usr/bin/env python

import sys
import os

def main():
	while True:
		num = raw_input("\033[0;33mEnter menu:\n1. start server on Mac, Linux or Cygwin.\n2. start server on Windows.\n3. exit.\033[0m\n")
		if num == "1":
			os.system("chmod +x ./gradlew && ./gradlew :server:startProxyServer")
			return
		elif num == "2":
			os.system("gradlew.bat :server:startProxyServer")
			return
		elif num == "3":
			print("exit!")
			return
		else:
			continue

if __name__ == "__main__":

	main()