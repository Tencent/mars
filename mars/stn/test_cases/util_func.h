// Tencent is pleased to support the open source community by making Mars available.
// Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

// Licensed under the MIT License (the "License"); you may not use this file except in 
// compliance with the License. You may obtain a copy of the License at
// http://opensource.org/licenses/MIT

// Unless required by applicable law or agreed to in writing, software distributed under the License is
// distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
// either express or implied. See the License for the specific language governing permissions and
// limitations under the License.

/*
* Created on 2015-04-01
*	  Author: yanguoyue
*
*/
#ifndef __UTILFUNC__
#define __UTILFUNC__ 

#include "thread/thread.h"
#include "../../../log/windows/dirent.h"


#include <string>
#include <vector>
#include <direct.h>
#include "../../comm/ini.h"
#include "test_constants.h"

#define PRINT_IPPORTITEMS(iPPortItemVector) \
	do { \
	printf("-------------" __FUNCTION__ "---------------------\n"); \
	for (std::vector<IPPortItem>::iterator iter=iPPortItemVector.begin();iter!=iPPortItemVector.end(); ++iter) \
		{ \
		printf("%s\t%s:%d\ttype:%d\trate:%f\n", (*iter).strHost.c_str(), (*iter).strIP.c_str(), (*iter).nPort, (*iter).eSourceType, (*iter).successRate); \
		} \
	} while (false)


class UtilFunc 
{
public:
	static void del_files(const std::string& forderPath)
	{
		DIR* dir = opendir(forderPath.c_str());
		if(NULL == dir) { return; }

		struct dirent* entry = NULL;
		while((entry = readdir(dir), NULL!=entry))
		{
			std::string fullFileName = forderPath;
			fullFileName += "/";
			fullFileName += entry->d_name;
			remove(fullFileName.c_str());
		}
		closedir(dir);
	}


	static void execute_cmd()
	{

		static bool init = false;

		if (init)
		{
			return;
		}
		init = true;

		for (size_t i=0; i< sizeof(LONGLINKTASKMANAGER_TEST_CMD)/sizeof(LONGLINKTASKMANAGER_TEST_CMD[0]); ++i)
		{
			FILE* f = _popen(LONGLINKTASKMANAGER_TEST_CMD[i], "rt");
			if(NULL == f)   
			{
				printf("popen:%s error!\n", LONGLINKTASKMANAGER_TEST_CMD[i]);
				continue;
			}
		}

		for (size_t i=0; i< sizeof(LONGLINK_TEST_CMD)/sizeof(LONGLINK_TEST_CMD[0]); ++i)
		{
			FILE* f = _popen(LONGLINK_TEST_CMD[i], "rt");
			if(NULL == f)   
			{
				printf("popen:%s error!\n", LONGLINK_TEST_CMD[i]);
				continue;
			}
		}

		_chdir(NETSOURCE_CHDIR_DIR);
		FILE* f = _popen(NETSOURCE_TEST_CMD , "rt");
		if(NULL == f)   
		{
			printf("popen error!\n");
			
		}

		ThreadUtil::sleep(10);

	}


	static bool save_wording_to_ini(const char* _key, const char* _value, const char* _sectionName, const char* _iniFileName)
	{
		INI ini(_iniFileName, false);
		ini.Parse();
		if (!ini.Select(_sectionName))
		{
			if (!ini.Create(_sectionName))
			{
				printf("create selection=%s err", _sectionName);
				return false;
			}
		}
		ini.Set(_key, _value);
		ini.Save();
		return true;
	}
	static  std::string get_wording_from_ini(const char* _key,const char* _sectionName, const char* _iniFileName)
	{
		INI ini(_iniFileName, false);
		ini.Parse();
		std::string value(ini.Get(_sectionName, _key, ""));
		return value;
	}

};

#define NETCORE_TEST
#define NETSOURCE_TEST
#define SMARTHEARTBEAT_TEST
#define LONGLINK_TEST
#define LONGLINK_TASKMANAGER_TEST

//#define WHOLE_TEST
#define HOST_ORDER_TEST
#endif
