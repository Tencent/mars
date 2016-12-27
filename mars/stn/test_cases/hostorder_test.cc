/*
* Created on��2015-05-11
*	  Author: elviswu
*
*/
#include <stdlib.h>

#include "util_func.h"

#if 1

#ifdef HOST_ORDER_TEST
#include <stdio.h>
#include "gtest/gtest.h"

#include "TestUtils.h"
#include "GtestEx.h"

#include "../../test/include/hook.h"
#include "../../test/include/mock_spy.h"

#include "net_core.h"
#include "host_info_logic.h"
#include "new_getdns.h"
#include "active_logic.h"
#include "../../../comm/singleton.h"
#include "../MmNetComm.h"

#include "../../../log/appender.h"
#include "../../../comm/xlogger/xloggerbase.h"

extern std::string getAppPrivatePath();

static bool startNewdnsMockServer(const char* _ip, uint16_t _port)
{
	_chdir(NETSOURCE_CHDIR_DIR);
	char tmpCmd[1024] = {0};
	snprintf(tmpCmd, sizeof(tmpCmd), START_NEWDNS_SERVER_CMD, _ip, _port);
	printf("line:%d,tempCmd=%s\n",__LINE__, tmpCmd);
	FILE* f = _popen(tmpCmd , "rt");
	if(NULL == f)   
	{
		printf("popen error!\n");
		_chdir("../../msvc"); //change back
		return false;

	}
	_chdir("../../msvc"); //change back
	return true;
}

static void onCreate()
{
	UtilFunc::del_files(getAppPrivatePath()+"host");
	CMMNetCore::Singleton();	
}
static void onDestroy()
{
	CMMNetCore::Release();
	SINGLETON_RELEASE_ALL();
	UtilFunc::del_files(getAppPrivatePath()+"host");
}
PUBC_TEST(HostOrder_test, test0)
{
	m_testCaseInfo->m_TestCaseName = "Longlink Hosts Order by Auth";
	startNewdnsMockServer("127.0.0.81", 8899);
	ThreadUtil::sleep(10);
	
	onCreate();
	MMNewGetDns::SetDebugHostInfo("127.0.0.81", 8899);
	const char* const highPriorityHostFromAuth = "sh2tjlong.weixin.qq.com";
	const char* const lowPriorityHostFromAuth = "long.weixin.qq.com";

	struct IdcHostInfo info1;
	info1.Origin = "long.weixin.qq.com";
	info1.Substitute = "long.weixin.qq.com";
	info1.priority = 1;

	struct IdcHostInfo info2;
	info2.Origin = "long.weixin.qq.com";
	info2.Substitute = "sh2tjlong.weixin.qq.com";
	info2.priority = 2;

	std::vector<IdcHostInfo> idcHostInfoList;
	idcHostInfoList.push_back(info1);
	idcHostInfoList.push_back(info2);

	HostInfoLogic::DispatchHostInfo(idcHostInfoList);

	BaseScene baseScene;
	std::vector<std::string> longHostList;
	CMMNetCore::Singleton()->GetNetSource().GetLongLinkHost(longHostList);
	std::string HighPriorityHostFromAuth(highPriorityHostFromAuth);
	std::string LowPriorityHostFromAuth(lowPriorityHostFromAuth);
	baseScene.setEventName("GetLongLinkHost");
	PUBC_EXPECT_EQ(longHostList.at(0).c_str(), highPriorityHostFromAuth, &baseScene);
	PUBC_EXPECT_EQ(longHostList.at(1).c_str(), lowPriorityHostFromAuth, &baseScene);

	onDestroy();
}



PUBC_TEST(HostOrder_test, test1)
{
	m_testCaseInfo->m_TestCaseName = "Shortlink Hosts Order by Auth";
	startNewdnsMockServer("127.0.0.81", 9988);
	ThreadUtil::sleep(10);

	onCreate();
	MMNewGetDns::SetDebugHostInfo("127.0.0.81", 9988);
	const char* const highPriorityHostFromAuth = "szshort.weixin.qq.com";
	const char* const lowPriorityHostFromAuth = "short.weixin.qq.com";

	struct IdcHostInfo info1;
	info1.Origin = "short.weixin.qq.com";
	info1.Substitute = "short.weixin.qq.com";
	info1.priority = 1;

	struct IdcHostInfo info2;
	info2.Origin = "short.weixin.qq.com";
	info2.Substitute = "szshort.weixin.qq.com";
	info2.priority = 2;

	struct IdcHostInfo info3;
	info3.Origin = "long.weixin.qq.com";
	info3.Substitute = "long.weixin.qq.com";
	info3.priority = 0;

	std::vector<IdcHostInfo> idcHostInfoList;
	idcHostInfoList.push_back(info1);
	idcHostInfoList.push_back(info2);
	idcHostInfoList.push_back(info3);

	HostInfoLogic::DispatchHostInfo(idcHostInfoList);

	BaseScene baseScene;
	std::vector<std::string> shortHostList;
	CMMNetCore::Singleton()->GetNetSource().GetDefShortLinkHost(shortHostList);
	std::string HighPriorityHostFromAuth(highPriorityHostFromAuth);
	std::string LowPriorityHostFromAuth(lowPriorityHostFromAuth);
	baseScene.setEventName("GetDefShortLinkHost");
	PUBC_EXPECT_EQ(shortHostList.at(0).c_str(), highPriorityHostFromAuth, &baseScene);
	PUBC_EXPECT_EQ(shortHostList.at(1).c_str(), lowPriorityHostFromAuth, &baseScene);

	onDestroy();
}
#endif

#endif
