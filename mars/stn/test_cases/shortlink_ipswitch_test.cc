/*
* Created on��2015-05-11
*	  Author: elviswu
*
*/
#if 0
#include <stdlib.h>
#include "UtilFunc.h"

#define  SHORTLINK_IPSWITCH_TEST
#ifdef SHORTLINK_IPSWITCH_TEST

#include "gtest/gtest.h"

#include "TestUtils.h"
#include "GtestEx.h"
#include "../../../comm/singleton.h"
#include "../../test/include/hook.h"
#include "../../test/include/mock_spy.h"
#include "../../../network/MMNetCore.h"

#include "../../../network/MMShortLink.h"
#include "../../../network/MMShortLinkTaskManager.h"
#include "../../../network/MMHostInfoLogic.h"
#include "../../../mmcomm/MMNewGetDns.h"
#include "../../../mmcomm/ActiveLogic.h"

#include "../MmNetComm.h"

#include "../../../log/appender.h"
#include "../../../comm/xlogger/xloggerbase.h"
#include "MockServerControlUtils.h"
#include "HookUtil.h"

#define DEF_TASK_RETRY_COUNT (2)
#define NEWDNS_SERVER_IP	"127.0.0.89"
#define NEWDNS_SERVER_PORT	8181


struct STCmdShortLinkTask
{
	STCmdShortLinkTask(const int _nHashCode, const CNetCmd& _netCmd)
		:nHashCode(_nHashCode), netCmd(_netCmd), nStartTime(::gettickcount())
		,maxretrycount(0<=_netCmd.cmduser_retrycount? _netCmd.cmduser_retrycount:DEF_TASK_RETRY_COUNT)
		,init_firstpkg_timeout( 0<_netCmd.cmduser_firstpkgtimeout? _netCmd.cmduser_firstpkgtimeout:0)
		,tasktimeout(ComputeTaskTimeout(_netCmd))
		,nLastRecvTime(0),useProxy(true)
	{
		nSendSize = 0;
		nStartSendTime = 0;
		nReadWriteTimeOut = 0;
		nFirstPkgTimeOut = 0;
		remainretrycount = maxretrycount;

		pWorker = 0;
		nRetryInterval = 0;
		nRetryStartTime = 0;

		recv_totalsize = 0;
		recv_cachedsize = 0;
	}

	~STCmdShortLinkTask()
	{
		xassert2(NULL==pWorker, "@%p", pWorker);
	}

	const int nHashCode;
	const CNetCmd netCmd;
	const unsigned long long nStartTime; //ms
	const int maxretrycount;
	const int init_firstpkg_timeout;
	const unsigned long tasktimeout;

	int nSendSize;
	unsigned long long nLastRecvTime;//ms
	unsigned long long nStartSendTime;	//ms

	unsigned long nReadWriteTimeOut;	//ms
	unsigned long nFirstPkgTimeOut; // ms
	int remainretrycount;
	bool useProxy;

	MMShortLink* pWorker;
	unsigned long nRetryInterval;	//ms
	unsigned long long nRetryStartTime; //ms

	unsigned int recv_totalsize;
	unsigned int recv_cachedsize;

private:
	static unsigned long ComputeTaskTimeout(const CNetCmd& _netCmd)
	{
		int readwritetimeout = 15*1000;
		if (0 < _netCmd.cmduser_firstpkgtimeout)
			readwritetimeout = 	_netCmd.cmduser_firstpkgtimeout + 15*1000;

		int trycount = DEF_TASK_RETRY_COUNT;
		if (0 <= _netCmd.cmduser_retrycount)
			trycount = _netCmd.cmduser_retrycount;

		trycount++;

		int task_timeout = (readwritetimeout + 5*1000)*trycount;
		if (0 < _netCmd.cmduser_expectfinishtime &&  _netCmd.cmduser_expectfinishtime < task_timeout)
			task_timeout = _netCmd.cmduser_expectfinishtime;

		return  task_timeout;
	}
};



class IpSwitchTimeItem
{
public:
	unsigned long long startTime;
	unsigned long long findHardcodeIpTime;
	unsigned long long connectHardcodeIpTime;
	unsigned long long switchBackNewdnsIpTime;

	void reset()
	{
		startTime = 0;
		findHardcodeIpTime = 0;
		connectHardcodeIpTime = 0;
		switchBackNewdnsIpTime = 0;
	}
};
class IpSwitchTimeCost
{
public:
	unsigned long long findHardcodeIpCost;
	unsigned long long connectHardcodeIpCost;
	unsigned long long switchBackNewdnsIpCost;
};
class IpSwitchParam
{
public:
	bool isForeGround;
	bool isStartTask;
	unsigned long long startTaskInterval;

	std::vector<std::pair<std::string, std::string> > idcHostTable;
	std::vector<std::string> eventNames;

	IpSwitchTimeCost expectMaxCostTime;

	IpSwitchParam()
	{
		isForeGround = true;
		isStartTask = true;
		startTaskInterval = 30*1000; //30s
	}

};


static void startAllNewdnsIpServer()
{
	startAllMainIdcServer();
	startAllSatelliteIdcServer();
}
static void banAllNewdnsIP()
{
	static bool isAlreadyBanned = false;
	if(!isAlreadyBanned)
	{
		for(int i=0; i<3; ++i)
		{
			CMMNetCore::Singleton()->GetNetSource().ReportShortIP(false, "127.0.50.1", "short.weixin.qq.com");
			CMMNetCore::Singleton()->GetNetSource().ReportShortIP(false, "127.0.50.2", "short.weixin.qq.com");
			CMMNetCore::Singleton()->GetNetSource().ReportShortIP(false, "127.0.50.3", "short.weixin.qq.com");

			CMMNetCore::Singleton()->GetNetSource().ReportShortIP(false, "127.0.51.1", "sz2tjextshort.weixin.qq.com");
			CMMNetCore::Singleton()->GetNetSource().ReportShortIP(false, "127.0.51.2", "sz2tjextshort.weixin.qq.com");
			CMMNetCore::Singleton()->GetNetSource().ReportShortIP(false, "127.0.51.3", "sz2tjextshort.weixin.qq.com");
		}
		isAlreadyBanned = true;
	}
}
//---------------------------------------global variables begin---------------------------------------
static Condition sg_Condition; 
static Condition sg_ConditionEndOK;
static bool sg_isEnd = false; 
static bool sg_readySwitch = false;
static int sg_TestType = 0;
static IpSwitchTimeItem sg_TimeItem;
static bool sg_isAlreadyFindHardcodeIp = false;
static bool sg_isAlreadyConnHardcodeIp = false;
//---------------------------------------global variables end------------------------------------------






//-----------------------------begin hook----------------------------------------------
//---step1  Declare hook function  (unsigned int& port, std::string& strProxy, std::string& _host)
DECLARE_HOOK_BEGIN
	DECLARE_HOOK_FUNCTION3(CMMNetSource, GetShortLinkProxyInfo, bool, unsigned int&, std::string&, std::string&);
	DECLARE_HOOK_FUNCTION2(CMMNetSource, GetShortLinkIPs, bool, const std::vector<std::string>&, std::vector<IPPortItem>&);
	DECLARE_HOOK_FUNCTION3(CMMNetSource, ReportShortIP, void, bool, const std::string&, const std::string&);
	DECLARE_HOOK_FUNCTION0(MMShortLink, __MakeSocketPrepared, SOCKET);
DECLARE_HOOK_END


///---step2	Describe which function(the functions DECLARED above) should be Hook
ATTACH_HOOK_BEGIN
	ATTACH_HOOK_FUNCTION3(CMMNetSource, GetShortLinkProxyInfo, bool, unsigned int&, std::string&, std::string&);
	ATTACH_HOOK_FUNCTION2(CMMNetSource, GetShortLinkIPs, bool, const std::vector<std::string>&, std::vector<IPPortItem>&);
	ATTACH_HOOK_FUNCTION3(CMMNetSource, ReportShortIP, void, bool, const std::string&, const std::string&);
	//ATTACH_HOOK_FUNCTION0(MMShortLink, __MakeSocketPrepared, SOCKET);
ATTACH_HOOK_END


	//-----step3 Describe which function(the functions DECLARED above) should be UnHook
DETACH_HOOK_BEGIN
	DETACH_HOOK_FUNCTION3(CMMNetSource, GetShortLinkProxyInfo, bool, unsigned int&, std::string&, std::string&);
	DETACH_HOOK_FUNCTION2(CMMNetSource, GetShortLinkIPs, bool, const std::vector<std::string>&, std::vector<IPPortItem>&);
	ATTACH_HOOK_FUNCTION3(CMMNetSource, ReportShortIP, void,  bool, const std::string&, const std::string&);
	//DETACH_HOOK_FUNCTION0(MMShortLink, __MakeSocketPrepared, SOCKET);
DETACH_HOOK_END


	///------step4 Implement the hook function for the real function

IMPLEMENT_MINE_HOOK_FUNCITON3(CMMNetSource, GetShortLinkProxyInfo, bool, unsigned int&, port, std::string&, strProxy, std::string&, _host)
{
	(this->*Real_GetShortLinkProxyInfo)(port, strProxy, _host);
	return false;
}

IMPLEMENT_MINE_HOOK_FUNCITON3(CMMNetSource, ReportShortIP, void, bool, _isSuccess, const std::string&, _ip, const std::string&, _host)
{
	(this->*Real_ReportShortIP)(_isSuccess, _ip, _host);
	printf("******connect %s: ip=%s, host=%s************************\n", _isSuccess?"success":"failed", _ip.c_str(), _host.c_str());
}

IMPLEMENT_MINE_HOOK_FUNCITON2(CMMNetSource, GetShortLinkIPs, bool, const std::vector<std::string>&, _hostlist, std::vector<IPPortItem>&, _IPPortItems)
{
	if(1==sg_TestType)
	{
		banAllNewdnsIP();
	}
	(this->*Real_GetShortLinkIPs)(_hostlist, _IPPortItems);
	PRINT_IPPORTITEMS(_IPPortItems);
	std::vector<IPPortItem>::iterator iter=_IPPortItems.begin();
	for (; iter!=_IPPortItems.end(); ++iter)
	{
		if(EIPSourceHardcode == iter->eSourceType && !sg_isAlreadyFindHardcodeIp)
		{
			sg_isAlreadyFindHardcodeIp = true;
			sg_TimeItem.findHardcodeIpTime = ::gettickcount();
		}
	}
	return true;
}

//returnType (CDetour::* CDetour::Real_##functionName)() = (returnType (CDetour::*)())  DetourFindFunction("PublicComponent.lib", #className::functionName); \
//IMPLEMENT_MINE_HOOK_FUNCITON0(MMShortLink, __MakeSocketPrepared, SOCKET)
void * p = (void *)DetourFindFunction("test.exe","MMShortLink::__MakeSocketPrepared");
SOCKET (CDetour::* CDetour::Real___MakeSocketPrepared)() = (SOCKET (CDetour::* &)()) p;//DetourFindFunction("test.exe","MMShortLink, __MakeSocketPrepared");
SOCKET CDetour::Mine___MakeSocketPrepared()
{
	if(!sg_isEnd)
	{
		SOCKET sockfd = (this->*Real___MakeSocketPrepared)();
		if(sockfd != INVALID_SOCKET)
		{
			std::list<STCmdShortLinkTask> temp_lstCmd = CMMNetCore::Singleton()->m_shortLinkTaskManager->m_lstCmd;
			std::list<STCmdShortLinkTask>::iterator iter = temp_lstCmd.begin();
			for(; iter!=temp_lstCmd.end(); ++iter)
			{
				if(0 !=iter->pWorker)
				{
					printf("*****sg_isAlreadyConnHardcodeIp=%d;temp_lstCmd.size=%d***************************\n", sg_isAlreadyConnHardcodeIp, temp_lstCmd.size());
					printf("pWorker=0x%x\n", iter->pWorker);
					ShortLinkConnectionInfo connInfo = iter->pWorker->ShortLinkConnectInfo();
					if(EIPSourceHardcode == connInfo.ipType && !sg_isAlreadyConnHardcodeIp)
					{
						sg_isAlreadyConnHardcodeIp = true;
						sg_TimeItem.connectHardcodeIpTime = ::gettickcount();
						startAllNewdnsIpServer();
						sg_readySwitch = true;
						printf("********************sg_readySwitch==true;connInfo.ip=%s************************\n", connInfo.ip.c_str());
						break;
					}
					if(sg_readySwitch && EIPSourceNewDns == connInfo.ipType)
					{
						sg_TimeItem.switchBackNewdnsIpTime = ::gettickcount();
						sg_isEnd = true;
						printf("********************sg_isEnd==true;connInfo.ip=%s********************************\n", connInfo.ip.c_str());
						sg_Condition.notifyOne();
						break;
					}
				}
			}
		
		}
		else
		{
			printf("*************sockfd == INVALID_SOCKET!!!!!!!!******************\n");
		}
		return sockfd;
	}
	else //sg_isEnd == true
	{
		std::list<STCmdShortLinkTask> temp_lstCmd = CMMNetCore::Singleton()->m_shortLinkTaskManager->m_lstCmd;
		std::list<STCmdShortLinkTask>::iterator iter = temp_lstCmd.begin();
		for(; iter!=temp_lstCmd.end(); ++iter)
		{
			if(0 !=iter->pWorker)
			{
				iter->pWorker->Cancel();
			}
		}
		sg_ConditionEndOK.notifyOne();
	}

	return -1;
}

static void HookPrivate()
{
	SOCKET (CDetour::* Mine_Target1)() = &CDetour::Mine___MakeSocketPrepared;
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&(PVOID&)CDetour::Real___MakeSocketPrepared, *(PBYTE*)&Mine_Target1);
	DetourTransactionCommit();
}

static void UnHookPrivate()
{
	SOCKET (CDetour::* Mine_Target1)() = &CDetour::Mine___MakeSocketPrepared;
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourDetach(&(PVOID&)CDetour::Real___MakeSocketPrepared, *(PBYTE*)&Mine_Target1);
	DetourTransactionCommit();
}
//----------------------------end hook-------------------------------------


//----------------------------begin callback change------------------------
static int CallBack(int _from, ErrCmdType _eErrType, int _nErrCode, const AutoBuffer& _cookies, int _nHashCode, const CNetCmd& _cmd, unsigned int _taskcosttime)
{
	printf("from:%d, errType:%d errCode:%d hashCode:%d, costTime:%u\n", _from, _eErrType, _nErrCode, _nHashCode, _taskcosttime);

	//sg_Condition.notifyOne();

	return 0;
}

static int buf2Resp(
	const int hashCode,
	const AutoBuffer& decodeBuffer,
	AutoBuffer& autoBuffer)
{
	return 0;
}
static bool req2Buf( const int hashCode, AutoBuffer& encodeBuffer) 
{ 
	char tempBuf[128] = {0};
	snprintf(tempBuf, 128, "random:%llu", ::gettickcount());//randombuf , escape antiAvalancheCheck
	encodeBuffer.Write(tempBuf);
	return true; 
}
static void ChangeBindFunc(CMMNetCore* _netcore)
{
	_netcore->m_shortLinkTaskManager->funCallback = boost::bind(&CallBack, 0, _1, _2, _3, _4, _5, _6);
	_netcore->m_shortLinkTaskManager->funbuf2Resp = buf2Resp;
	_netcore->m_shortLinkTaskManager->funreq2Buf = req2Buf;
}
//-----------------------------end callback change--------------------------


//-----------------------------wordings begin-------------------------------------
#define INI_FILE "..\\..\\network\\test_cases\\MMShortLinkIpSwitchTest.ini"
#define CHINESE_WORDING_SECTION "Shortlink Chinese Wording"
#define ENGLISH_WORDING_SECTION "Shortlink English Wording"

static std::string get_english_wording_from_ini(const char* _key)
{
	return UtilFunc::get_wording_from_ini(_key,ENGLISH_WORDING_SECTION, INI_FILE);
}
static std::string get_chinese_wording_from_ini(const char* _key)
{
	return UtilFunc::get_wording_from_ini(_key,CHINESE_WORDING_SECTION, INI_FILE);
}
#define USE_CHINESE_WORDING
#ifdef USE_CHINESE_WORDING

static const std::string  TESTCASE_0_NAME = get_chinese_wording_from_ini("TESTCASE_0_NAME");
static const std::string  TESTCASE_0_EVENT_1_NAME = get_chinese_wording_from_ini("TESTCASE_0_EVENT_1_NAME");
static const std::string  TESTCASE_0_EVENT_2_NAME = get_chinese_wording_from_ini("TESTCASE_0_EVENT_2_NAME");

static const std::string  TESTCASE_1_NAME = get_chinese_wording_from_ini("TESTCASE_1_NAME");
static const std::string  TESTCASE_1_EVENT_1_NAME = get_chinese_wording_from_ini("TESTCASE_1_EVENT_1_NAME");
static const std::string  TESTCASE_1_EVENT_2_NAME = get_chinese_wording_from_ini("TESTCASE_1_EVENT_2_NAME");

static const std::string EVENT1_VIRIABLE_DISCRIPTION = get_chinese_wording_from_ini("EVENT1_VIRIABLE_DISCRIPTION");
static const std::string EVENT2_VIRIABLE_DISCRIPTION = get_chinese_wording_from_ini("EVENT2_VIRIABLE_DISCRIPTION");
#else

static const std::string  TESTCASE_0_NAME = get_english_wording_from_ini("TESTCASE_0_NAME");
static const std::string  TESTCASE_0_EVENT_1_NAME = get_english_wording_from_ini("TESTCASE_0_EVENT_1_NAME");
static const std::string  TESTCASE_0_EVENT_2_NAME = get_english_wording_from_ini("TESTCASE_0_EVENT_2_NAME");

static const std::string  TESTCASE_1_NAME = get_english_wording_from_ini("TESTCASE_1_NAME");
static const std::string  TESTCASE_1_EVENT_1_NAME = get_english_wording_from_ini("TESTCASE_1_EVENT_1_NAME");
static const std::string  TESTCASE_1_EVENT_2_NAME = get_english_wording_from_ini("TESTCASE_1_EVENT_2_NAME");
#endif


//-----------------------------wordings end---------------------------------------

static void shortIpSwitchTest(TestCaseInfo *m_testCaseInfo, const IpSwitchParam& param)
{
	sg_TimeItem.startTime = ::gettickcount();

	SINGLETON_STRONG(ActiveLogic).OnForeground(param.isForeGround);
	SINGLETON_STRONG(ActiveLogic).m_isactive = true;
	UtilFunc::del_files(CONFIG_PATH);
	xlogger_SetLevel(ELevelDebug);
	appender_open(EAppednerSync, "C:\\log", "GTEST");
	Hook();
	HookPrivate();
	ChangeBindFunc(CMMNetCore::Singleton());

	MMNewGetDns::SetDebugHostInfo(NEWDNS_SERVER_IP, NEWDNS_SERVER_PORT);

	if(param.idcHostTable.empty())
	{
		std::vector<std::pair<std::string, std::string> > idchostTable;
		idchostTable.push_back(std::make_pair("short.weixin.qq.com", "short.weixin.qq.com"));
		idchostTable.push_back(std::make_pair("short.weixin.qq.com", "sz2tjextshort.weixin.qq.com"));

		CMMNetCore::Singleton()->GetNetSource().GetHostRedirect().SaveIDCHostMapping(idchostTable);
	}
	else
	{
		CMMNetCore::Singleton()->GetNetSource().GetHostRedirect().SaveIDCHostMapping(param.idcHostTable);
	}
	
	unsigned long long before = gettickcount();


	for (int i=0;i<1000;++i)
	{
		printf("-----------------------\n");
		CNetCmd netcmd;
		netcmd.reqCmdID = 100;
		netcmd.respCmdID = 1001;
		netcmd.uri = "http://test.com";
		netcmd.networkType = 1; //only for shortlink
		//	netcmd.cmduser_expectfinishtime = 70 * 1000;

		if(param.isStartTask)
		{
			CMMNetCore::Singleton()->StartTask(9527+i, netcmd);
			sg_Condition.wait(param.startTaskInterval);
		}
		else
		{
			sg_Condition.wait(60*1000);
		}

		BaseScene baseScene;
		unsigned long long elapseTime = ::gettickcount() - before;

		if(sg_isEnd)
		{
			if(0==sg_TestType || 1==sg_TestType)
			{
				IpSwitchTimeItem timeItem = sg_TimeItem;
				unsigned long long FindHardcodeIpCost = timeItem.findHardcodeIpTime - timeItem.startTime;
				unsigned long long ConnectHardcodeIpCost = timeItem.connectHardcodeIpTime - timeItem.findHardcodeIpTime;
				unsigned long long SwitchBackNewdnsIpCost = timeItem.switchBackNewdnsIpTime - timeItem.connectHardcodeIpTime;

				if(3 == param.eventNames.size())
				{
				//	baseScene.setEventName(param.eventNames.at(0).c_str());
				//	baseScene.setTestVariableDescription("switch to HardcodeIP cost");
					//ConnectSatelliteNewdnsIpCost /= 1000; //ms to s
				//	double fFindHardcodeIpCost = ((double)FindHardcodeIpCost)/1000;
				//	PUBC_EXPECT_GT((double)param.expectMaxCostTime.findHardcodeIpCost/1000, fFindHardcodeIpCost, &baseScene);
					baseScene.setEventName(param.eventNames.at(1).c_str());
#ifdef USE_CHINESE_WORDING
					baseScene.setTestVariableDescription(EVENT1_VIRIABLE_DISCRIPTION);
#endif
					//ConnectMainNewdnsIpCost /= 1000; //ms to s
					double fConnectHardcodeIpCost = ((double)ConnectHardcodeIpCost)/1000;
					PUBC_EXPECT_GT((double)param.expectMaxCostTime.connectHardcodeIpCost/1000, fConnectHardcodeIpCost, &baseScene);
					baseScene.setEventName(param.eventNames.at(2).c_str());
#ifdef USE_CHINESE_WORDING
					baseScene.setTestVariableDescription(EVENT2_VIRIABLE_DISCRIPTION);
#endif
					//SwitchBackToSatelliteIdcCost /= 1000;//ms to s
					double fSwitchBackNewdnsIpCost = ((double)SwitchBackNewdnsIpCost)/1000;
					PUBC_EXPECT_GT((double)param.expectMaxCostTime.switchBackNewdnsIpCost/1000, fSwitchBackNewdnsIpCost, &baseScene);

				}
				else
				{
					printf("\nxxxx-wrong eventNames size.\n");
				}
				break;
			}
		}//if(sg_isEnd)


	}
	printf("~~~~~~~~~~~~~~~~~~~~sg_isEnd~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	killAllServer();//should be close all servers
	sg_TimeItem.reset();
	sg_ConditionEndOK.wait();
	sg_readySwitch = false;
	sg_isEnd = false;
	sg_isAlreadyFindHardcodeIp = false;
	sg_isAlreadyConnHardcodeIp = false;
	CMMNetCore::Release();
	SINGLETON_RELEASE_ALL();
	UnHookPrivate();
	UnHook();
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
	isWSAStartupOK=WSAStartup(wVer,&wsaData);//get the version of Windows sockets dll
	printf("!!!!!!!!!!!!!!!!!!isWSAStartupOK=%s!!!!!!!!!!!!!!!!!!!\n", 0==isWSAStartupOK?"true":"false");
	if(isWSAStartupOK!=0)
	{
		BaseScene baseScene;
		baseScene.setEventName("is WSAStartup ok");
		PUBC_EXPECT_EQ(0,isWSAStartupOK,&baseScene);
	}
#endif 
}

PUBC_TEST(MMShortLinkIpSwitchTest, test0)
{
	InitWinSock(m_testCaseInfo);
	sg_TestType = 0;
	m_testCaseInfo->m_TestCaseName = TESTCASE_0_NAME;

	bool ret = false;
	int startCount = 0;
	while(!ret && startCount++ < 10)
	{
		ret = startNewdnsMockServer(NEWDNS_SERVER_IP, NEWDNS_SERVER_PORT);
	}
	
	ThreadUtil::sleep(10);

	IpSwitchParam param;
	param.expectMaxCostTime.findHardcodeIpCost = 1*1000;
	param.expectMaxCostTime.connectHardcodeIpCost = 24*1000;
	param.expectMaxCostTime.switchBackNewdnsIpCost = 5*1000;
	param.eventNames.push_back("findHardcodeIP");
	param.eventNames.push_back(TESTCASE_0_EVENT_1_NAME);
	param.eventNames.push_back(TESTCASE_0_EVENT_2_NAME);
	shortIpSwitchTest(m_testCaseInfo, param);

}


PUBC_TEST(MMShortLinkIpSwitchTest, test1)
{
	sg_TestType = 1;
	m_testCaseInfo->m_TestCaseName = TESTCASE_1_NAME;

	IpSwitchParam param;
	param.expectMaxCostTime.findHardcodeIpCost = 1*1000;
	param.expectMaxCostTime.connectHardcodeIpCost = 24*1000;
	param.expectMaxCostTime.switchBackNewdnsIpCost = 6*60*1000; //6min
	param.eventNames.push_back("findHardcodeIP");
	param.eventNames.push_back(TESTCASE_1_EVENT_1_NAME);
	param.eventNames.push_back(TESTCASE_1_EVENT_2_NAME);
	shortIpSwitchTest(m_testCaseInfo, param);

}









#endif

#endif