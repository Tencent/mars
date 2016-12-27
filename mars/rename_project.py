#!/usr/bin/env python

try:
	import xml.etree.cElementTree as ET
except ImportError:
	import xml.etree.ElementTree as ET

import os
import re

DELIMITER = "_"
PROJECT_FILE_NAME = ".project"
CPROJECT_FILE_NAME = ".cproject"

NAME_TAG_XPATH = "./name"
PROJECT_DEPEND_TAG_XPATH = "./projects/project"
CPROJECT_DEPEND_TAG_XPATH = "./storageModule/cconfiguration/storageModule/externalSettings"
CONTAINER_KEY = "containerId"
COLON = ";"

XML_DECLARATION_PATTERN = re.compile(r"^<\s*\?[\s\S]*\?\s*>")
MAIN_MENU_TEXT = "Input suffix or Press Enter to del suffix:"
SECOND_MENU_TEXT = "\nPlease input suffix:"

def listProjectFile():
	folders = os.listdir(os.curdir)

	fileList = []
	for f in folders:
		if os.path.isdir(f) and os.path.isfile(os.path.join(f, PROJECT_FILE_NAME)):
			fileList.append(os.path.join(f, PROJECT_FILE_NAME))
		
	
	return fileList
	
def listCProjectFile():
	folders = os.listdir(os.curdir)

	fileList = []
	for f in folders:
		if os.path.isdir(f) and os.path.isfile(os.path.join(f, CPROJECT_FILE_NAME)):
			fileList.append(os.path.join(f, CPROJECT_FILE_NAME))
		
	
	return fileList

def nameAddSuffix(_suffix):

	fileList = listProjectFile()

	for pf in fileList:
		tree = ET.ElementTree(file = pf)
		for elem in tree.iterfind(NAME_TAG_XPATH):
			elem.text = elem.text + DELIMITER +  _suffix
			
		for elem in tree.iterfind(PROJECT_DEPEND_TAG_XPATH):
			elem.text = elem.text + DELIMITER +  _suffix
		
		tree.write(pf);
	
	fileList = listCProjectFile()
	
	for pf in fileList:
		tree = ET.ElementTree(file = pf)
		for elem in tree.iterfind(CPROJECT_DEPEND_TAG_XPATH):
			value = elem.get(CONTAINER_KEY, None)
			if value == None:
				continue
				
			elem.set(CONTAINER_KEY, value[:value.find(COLON)] + DELIMITER +  _suffix + COLON)
		
		f = open(pf, "rb")
		match = XML_DECLARATION_PATTERN.match(f.read())
		f.close();
		f = open(pf, "wb")
		xml = ET.tostring(tree.getroot(), encoding="utf-8")
		if match:
			xml = match.group() + xml
		f.write(xml)
		f.close()

def nameDelSuffix():

	fileList = listProjectFile()

	for pf in fileList:
		tree = ET.ElementTree(file = pf)
		for elem in tree.iterfind(NAME_TAG_XPATH):
			index = elem.text.find(DELIMITER)
			if index < 0:
				continue
			
			elem.text = elem.text[:elem.text.find(DELIMITER)]
			
		for elem in tree.iterfind(PROJECT_DEPEND_TAG_XPATH):
			index = elem.text.find(DELIMITER)
			if index < 0:
				continue
			
			elem.text = elem.text[:elem.text.find(DELIMITER)]
		
		tree.write(pf, xml_declaration=True);
			
	fileList = listCProjectFile()
	
	for pf in fileList:

		tree = ET.ElementTree(file = pf)
		for elem in tree.iterfind(CPROJECT_DEPEND_TAG_XPATH):
			value = elem.get(CONTAINER_KEY, None)
			if value == None:
				continue
			
			elem.set(CONTAINER_KEY, value[:value.find(DELIMITER)] + COLON)
		
		f = open(pf, "rb")
		match = XML_DECLARATION_PATTERN.match(f.read())
		f.close();
		f = open(pf, "wb")
		xml = ET.tostring(tree.getroot(), encoding="utf-8")
		if match:
			xml = match.group() + xml
		f.write(xml)
		f.close()
	

def displayName():
	print "\nProject Name:"

	fileList = listProjectFile()

	for pf in fileList:
		tree = ET.ElementTree(file = pf)
		for elem in tree.iterfind(NAME_TAG_XPATH):
			print elem.text


def main():
	userInput = raw_input(MAIN_MENU_TEXT).strip()

	nameDelSuffix()
	if len(userInput) > 0:
		nameAddSuffix(userInput)

	
	
	displayName()

	print("\n\nexecute end!")
	raw_input("Press Enter to exit")

if __name__ == "__main__":
	main()
