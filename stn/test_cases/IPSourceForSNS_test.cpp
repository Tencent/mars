
#if 1
#include <stdlib.h>
#include "UtilFunc.h"

#define  IPSourceForSNS_TEST
#ifdef IPSourceForSNS_TEST
#include <vector>
#include <string>
#include "gtest/gtest.h"

#include "TestUtils.h"
#include "GtestEx.h"
#include "../../../comm/singleton.h"
#include "../../../network/net_core.h"
#include "../../../network/longlink_task_manager.h"
#include "../../test/include/hook.h"
#include "../../test/include/mock_spy.h"
#include "../../../network/IPSourceForSNS.h"


#define NEWDNS_SERVER_IP	"127.0.0.119"
#define NEWDNS_SERVER_PORT	8281
extern std::string getAppPrivatePath();
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
static bool startNewdnsMockServer(const char* _ip, unsigned int _port)
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
static void InitWinSock(TestCaseInfo *m_testCaseInfo)
{
#ifdef _WIN32
	int isWSAStartupOK=0;
	WORD wVer;
	WSADATA wsaData;
	wVer=MAKEWORD(1,1); 
	isWSAStartupOK=WSAStartup(wVer,&wsaData);//�ж�Windows sockets dll�汾
	if(isWSAStartupOK!=0)
	{
		BaseScene baseScene;
		baseScene.setEventName("is WSAStartup ok");
		PUBC_EXPECT_EQ(0,isWSAStartupOK,&baseScene);
	}
#endif 
}
PUBC_TEST(IPSourceForSNS_test, test0)
{
	InitWinSock(m_testCaseInfo);
	startNewdnsMockServer(NEWDNS_SERVER_IP, NEWDNS_SERVER_PORT);
	ThreadUtil::sleep(10);
	onCreate();
	MMNewGetDns::SetDebugHostInfo(NEWDNS_SERVER_IP, NEWDNS_SERVER_PORT);
	IPSourceForSNS& ipsourceForSns = CMMNetCore::Singleton()->GetNetSource().GetIPSourceForSns();
	std::vector<std::string> iplist;
	bool ret = ipsourceForSns.GetIP(iplist);
	BaseScene baseScene;
	baseScene.setEventName("GetSnsIp");
	PUBC_EXPECT_FALSE(iplist.empty(), &baseScene);

	if(!iplist.empty())
	{
		baseScene.setEventName("ReportFailIP Once");
		bool isBan = ipsourceForSns.ReportFailIP(iplist.front());
		PUBC_EXPECT_FALSE(isBan, &baseScene);

		baseScene.setEventName("ReportFailIP Twice");
		isBan = ipsourceForSns.ReportFailIP(iplist.front());
		PUBC_EXPECT_TRUE(isBan, &baseScene);
	}
	ipsourceForSns.Clear();
	onDestroy();
}













#endif
#endif
