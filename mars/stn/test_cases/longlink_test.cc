/*
* Created on 2015-03-06
*	  Author: 闫国跃
*
*/

#include "util_func.h"

#ifdef LONGLINK_TEST

#include "gtest/gtest.h"
#include "thread/thread.h"

#include "../../test/include/hook.h"
#include "../../test/include/mock_spy.h"

#include "../MMNetSource.h"
#include "../longlink.h"
#include "../../mmcomm/new_getdns.h"
#include "../../mmcomm/active_logic.h"
#include "../../../comm/utils.h"
#include "../../../log/appender.h"
#include "../../../comm/xlogger/xloggerbase.h"

#include "TestUtils.h"


class LongLinkScene : public BaseScene
{
public:
	LongLinkScene() {}
	~LongLinkScene(){}
	void Reset() 
	{
		ipPortItems.clear();
		expectValues.clear();
		longlinkSceneName.clear();
		expectUseTime = 0;
	}
public:
	std::string longlinkSceneName;
	std::vector<IPPortItem> ipPortItems;
	std::vector<std::pair<int, int>> expectValues;
	uint64_t expectUseTime;
};


static std::vector<LongLinkScene> sg_longLinkScenes;
static int InitLongLinkScenes()
{

	LongLinkScene scene;
	IPPortItem item;
	item.eSourceType = kIPSourceDebug;

	// case 0
	scene.longlinkSceneName = "(IP1����, Port1������, ����ʧ��);(IP2����, Port2������, ����ʧ��)";
	item.strIP = "127.0.0.1"; //ok
	item.nPort = 1234; //not ok
	scene.ipPortItems.push_back(item);

	item.nPort = 2345; //not ok
	scene.ipPortItems.push_back(item);
	scene.expectUseTime = 5000;

	scene.expectValues.push_back(std::make_pair(2, MMLongLink2::EConnecting));
	scene.expectValues.push_back(std::make_pair(4, MMLongLink2::EConnectFailed));
	sg_longLinkScenes.push_back(scene);

	// case 1
	scene.Reset();
	scene.longlinkSceneName = "(IP1������, Port1����, ����ʧ��);(IP2������, Port2����, ����ʧ��)";
	item.strIP = "127.0.0.111"; //not ok
	item.nPort = 8080; //ok
	scene.ipPortItems.push_back(item);

	item.nPort = 8001; //ok
	scene.ipPortItems.push_back(item);
	scene.expectUseTime = 5000;
	
	scene.expectValues.push_back(std::make_pair(2, MMLongLink2::EConnecting));
	scene.expectValues.push_back(std::make_pair(4, MMLongLink2::EConnectFailed));
	sg_longLinkScenes.push_back(scene);
	
	//case 2
	scene.Reset();
	scene.longlinkSceneName = "(IP1����, Port1������, ����ʧ��);(IP2����, Port2����, ���ӳɹ�)";
	item.strIP = "127.0.0.1"; //ok
	item.nPort = 1234; //not ok
	scene.ipPortItems.push_back(item);
	
	item.nPort = 8001; //ok
	scene.ipPortItems.push_back(item);
	scene.expectUseTime = 4020;

	scene.expectValues.push_back(std::make_pair(2, MMLongLink2::EConnecting));
	scene.expectValues.push_back(std::make_pair(4, MMLongLink2::EConnected));
	sg_longLinkScenes.push_back(scene);

	//case 3
	scene.Reset();
	scene.longlinkSceneName = "(IP1����, Port1������, ����ʧ��);(IP2����, Port2����, ���ӳɹ�)";
	item.strIP = "127.0.0.1"; //ok
	item.nPort = 8080; //ok
	scene.ipPortItems.push_back(item);
	scene.expectUseTime = 20;

	scene.expectValues.push_back(std::make_pair(2, MMLongLink2::EConnected));
	sg_longLinkScenes.push_back(scene);

	//case 4
	scene.Reset();
	scene.longlinkSceneName = "(IP1����, Port1������, ����ʧ��);(IP2����, Port2����, ������ʱ)";
	item.strIP = "127.0.0.1"; //ok
	item.nPort = 1234; //not ok
	scene.ipPortItems.push_back(item);

	item.nPort = 8000; //connect����, no nooping echo/nooping timeout
	scene.ipPortItems.push_back(item);
	scene.expectUseTime = 5000;

	scene.expectValues.push_back(std::make_pair(1, MMLongLink2::EConnecting));
	scene.expectValues.push_back(std::make_pair(6, MMLongLink2::EConnectFailed));
	sg_longLinkScenes.push_back(scene);


	//case 5
	scene.Reset();
	scene.longlinkSceneName = "(IP1����, Port1����, ������ʱ);(IP2����, Port2����, ���ӳɹ�)";
	item.strIP = "127.0.0.1"; //ok
	item.nPort = 8000; //connect����, no nooping echo/nooping timeout
	scene.ipPortItems.push_back(item);

	item.nPort = 8002; //ok
	scene.ipPortItems.push_back(item);
	scene.expectUseTime = 4020;

	scene.expectValues.push_back(std::make_pair(2, MMLongLink2::EConnecting));
	scene.expectValues.push_back(std::make_pair(4, MMLongLink2::EConnected));
	sg_longLinkScenes.push_back(scene);

	//case 6
	scene.Reset();
	scene.longlinkSceneName = "(IP1����, Port1����, ������ʱ);(IP2����, Port1������, ����ʧ��)";
	item.strIP = "127.0.0.1"; //ok
	item.nPort = 8000; //connect����, no nooping echo/nooping timeout
	scene.ipPortItems.push_back(item);

	item.nPort = 1234; //not ok
	scene.ipPortItems.push_back(item);
	scene.expectUseTime = 5020;

	scene.expectValues.push_back(std::make_pair(1, MMLongLink2::EConnecting));
	scene.expectValues.push_back(std::make_pair(5, MMLongLink2::EConnectFailed));
	sg_longLinkScenes.push_back(scene);


	//case 7
	scene.Reset();
	scene.longlinkSceneName = "(IP1����, Port1����, ������ʱ);(IP2����, Port2������, ����ʧ��);(IP3����, Port3����, ���ӳɹ�)";
	item.strIP = "127.0.0.1"; //ok
	item.nPort = 8000; //connect����, no nooping echo/nooping timeout
	scene.ipPortItems.push_back(item);

	item.nPort = 1234; //not ok
	scene.ipPortItems.push_back(item);

	item.nPort = 8003; //ok
	scene.ipPortItems.push_back(item);
	scene.expectUseTime = 8020;

	scene.expectValues.push_back(std::make_pair(1, MMLongLink2::EConnecting));
	scene.expectValues.push_back(std::make_pair(5, MMLongLink2::EConnecting));
	scene.expectValues.push_back(std::make_pair(4, MMLongLink2::EConnected));
	sg_longLinkScenes.push_back(scene);

	//case 8
	scene.Reset();
	scene.longlinkSceneName = "(IP1����, Port1������, ����ʧ��);(IP2����, Port2����, ������ʱ);(IP3����, Port3������, ����ʧ��);(IP4����, Port4����, ������ʱ);(IP5����, Port5����, ���ӳɹ�)";
	item.strIP = "127.0.0.1"; //ok
	item.nPort = 2345; //not ok
	scene.ipPortItems.push_back(item);

	item.nPort = 8000; //connect����, no nooping echo/nooping timeout
	scene.ipPortItems.push_back(item);

	item.nPort = 1234; //not ok
	scene.ipPortItems.push_back(item);

	item.nPort = 7999; //connect����, no nooping echo/nooping timeout
	scene.ipPortItems.push_back(item);

	item.nPort = 8004; //ok
	scene.ipPortItems.push_back(item);
	scene.expectUseTime = 16020;

	scene.expectValues.push_back(std::make_pair(1, MMLongLink2::EConnecting));
	scene.expectValues.push_back(std::make_pair(5, MMLongLink2::EConnecting));
	scene.expectValues.push_back(std::make_pair(4, MMLongLink2::EConnecting));
	scene.expectValues.push_back(std::make_pair(4, MMLongLink2::EConnecting));
	scene.expectValues.push_back(std::make_pair(4, MMLongLink2::EConnected));
	sg_longLinkScenes.push_back(scene);
	

	return sg_longLinkScenes.size();
}

static int sg_size = InitLongLinkScenes();
static int sg_index = 0;

const static char* const CONNECT_STATUS_CHARS[] = {
		"EConnectIdle",
		"EConnecting",
		"EConnected",
		"EDisConnected",
		"EConnectFailed"
};

class LongLinkObserver2 : public MLongLinkObserver2
{
public:

    virtual void OnStartSendTime(unsigned int _nSeq) {}
	virtual void OnResponse(ErrCmdType _type, int _errcode, int _cmdid, unsigned int _seq, AutoBuffer& _body, const LongLinkConnectionInfo& _info) {}
    virtual void OnPkgRecvTime(unsigned int _nSeq, unsigned int _cachedsize, unsigned int _totalsize) {}
};

class CDetour /* add ": public CMember" to enable access to member variables... */
{
public:
	bool Mine_GetLongLinkItems(std::vector<IPPortItem>& _IPPortItems);
	bool Mine_GetMixLongLinkItems(std::vector<IPPortItem>& _IPPortItems);

	void Mine_ReportLongIP(bool _isSuccess, const std::string& _ip, unsigned int _port);


	static bool (CDetour::* Real_GetLongLinkItems)(std::vector<IPPortItem>& _IPPortItems);
	static bool (CDetour::* Real_GetMixLongLinkItems)(std::vector<IPPortItem>& _IPPortItems);
	static void (CDetour::* Real_ReportLongIP)(bool _isSuccess, const std::string& _ip, unsigned int _port);

};


bool CDetour::Mine_GetLongLinkItems(std::vector<IPPortItem>& _IPPortItems)
{
	printf("ASSERT!!!!!\n");
	return true;
}

bool CDetour::Mine_GetMixLongLinkItems(std::vector<IPPortItem>& _IPPortItems)
{
	if (sg_index >= sg_size)
	{
		printf("out of boundery, sg_inedx:%d, sg_size:%d line:%d\n", sg_index, sg_size, __LINE__);
		return false;
	}

	_IPPortItems = sg_longLinkScenes.at(sg_index).ipPortItems;

	return true;
}

void CDetour::Mine_ReportLongIP(bool _isSuccess, const std::string& _ip, unsigned int _port)
{
	
}


void __OnLongLinkNetworkError(int _line, ErrCmdType _eErrType, int _nErrCode, const std::string& _ip, unsigned int _port)
{
	//printf("%s:%u, err(%d, %d), line:%d\n", _ip.c_str(), _port, _eErrType, _nErrCode, _line);
}

bool (CDetour::* CDetour::Real_GetLongLinkItems)(std::vector<IPPortItem>& _IPPortItems) = (bool (CDetour::*)(std::vector<IPPortItem>& _IPPortItems)) &CMMNetSource::GetLongLinkItems;
bool (CDetour::* CDetour::Real_GetMixLongLinkItems)(std::vector<IPPortItem>& _IPPortItems) = (bool (CDetour::*)(std::vector<IPPortItem>& _IPPortItems)) &CMMNetSource::GetMixLongLinkItems;
void (CDetour::* CDetour::Real_ReportLongIP)(bool _isSuccess, const std::string& _ip, unsigned int _port) = (void (CDetour::*)(bool _isSuccess, const std::string& _ip, unsigned int _port)) &CMMNetSource::ReportLongIP;

static uint64_t sg_lastChangeTime = gettickcount();
static uint64_t sg_nowChangeTime = gettickcount();
static void OnLongLinkStatuChanged(MMLongLink2::TLongLinkStatus _connectStatus)
{
	sg_lastChangeTime = sg_nowChangeTime;
	sg_nowChangeTime = gettickcount();
//	printf("status:%d %ld\n", _connectStatus, gettickcount());
}

void test(TestCaseInfo& _info)
{
	if (sg_index >= sg_size)
	{
		printf("out of boundery, sg_inedx:%d, sg_size:%d \n", sg_index, sg_size);
		return;
	}

	sg_lastChangeTime = sg_nowChangeTime = gettickcount();

	LongLinkObserver2 longlinkobs;                                                                                                                                                                                                                                                                                                                      
	CMMNetSource netsource;
	ActiveLogic activelogic;
	MMLongLink2 longlink(longlinkobs, &netsource, activelogic);

	longlink.SignalConnection.connect(boost::bind(&OnLongLinkStatuChanged, _1));

	longlink.FuncNetworkReport = boost::bind(&__OnLongLinkNetworkError, _1, _2, _3, _4, _5);;
	longlink.MakeSureConnected();
	
	_info.m_isPassed = true;

	for (size_t i=0;i<sg_longLinkScenes.at(sg_index).expectValues.size();++i)
	{
		
		std::pair<int, int> expectValue =  sg_longLinkScenes.at(sg_index).expectValues.at(i);
		ThreadUtil::sleep(expectValue.first);

			
		EXPECT_EQ(expectValue.second, longlink.m_connectstatus) << "case name:"<< _info.m_TestCaseName
			<<", expectValue index:" << i << ", sleep:" << expectValue.first << ", longlink expect:" << expectValue.second << ", longlink status:" << longlink.m_connectstatus;

		sg_longLinkScenes.at(sg_index).testVariableName = "longlink.m_connectstatus";

		char name[256] = {0};
		snprintf(name, sizeof(name), "sleep:%d", expectValue.first);
		sg_longLinkScenes.at(sg_index).eventName = name;
		char tmpValue[128] = {0};
		snprintf(tmpValue, 128, "%s", CONNECT_STATUS_CHARS[expectValue.second]);
		sg_longLinkScenes.at(sg_index).expectValue = tmpValue;
		memset(tmpValue, 0, sizeof(tmpValue));

		snprintf(tmpValue, sizeof(tmpValue), "%s", CONNECT_STATUS_CHARS[longlink.m_connectstatus]);
		sg_longLinkScenes.at(sg_index).realValue = tmpValue;

		bool flag = true;
		if (expectValue.second != longlink.m_connectstatus)
		{
			_info.m_isPassed = flag = false;
		}
		_info.output_formatter(&sg_longLinkScenes.at(sg_index), flag);
	}

	_info.m_WorkerExpectTimeCost = sg_longLinkScenes.at(sg_index).expectUseTime;
	_info.m_WorkerRealTimeCost = sg_nowChangeTime - sg_lastChangeTime;
	uint64_t diff = (sg_nowChangeTime - sg_lastChangeTime)>sg_longLinkScenes.at(sg_index).expectUseTime?(sg_nowChangeTime - sg_lastChangeTime)-sg_longLinkScenes.at(sg_index).expectUseTime:sg_longLinkScenes.at(sg_index).expectUseTime-(sg_nowChangeTime - sg_lastChangeTime);
	EXPECT_TRUE(diff<500);

}

TEST(MMLongLink2_test, longlinkconnect_test0)
{
	UtilFunc::execute_cmd();

	static bool (CDetour::* Mine_Target1)(std::vector<IPPortItem>& _IPPortItems) = &CDetour::Mine_GetLongLinkItems;
	static bool (CDetour::* Mine_Target2)(std::vector<IPPortItem>& _IPPortItems) = &CDetour::Mine_GetMixLongLinkItems;
	static void (CDetour::* Mine_Target3)(bool _isSuccess, const std::string& _ip, unsigned int _port) = &CDetour::Mine_ReportLongIP;

//	xlogger_SetLevel(ELevelDebug);
//	appender_open(EAppednerSync, "C:\\log", "GTEST");
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&(PVOID&)CDetour::Real_GetLongLinkItems, *(PBYTE*)&Mine_Target1);
	DetourAttach(&(PVOID&)CDetour::Real_GetMixLongLinkItems, *(PBYTE*)&Mine_Target2);
	DetourAttach(&(PVOID&)CDetour::Real_ReportLongIP, *(PBYTE*)&Mine_Target3);

	LONG l = DetourTransactionCommit();

	for (sg_index=0;sg_index<sg_size;++sg_index) 
	{
		TestCaseInfo info(sg_longLinkScenes.at(sg_index).longlinkSceneName);
		info.m_TestCaseGroupName = "MMLongLink2_test";
		test(info);
	}

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());  
	DetourDetach(&(PVOID&)CDetour::Real_GetLongLinkItems, *(PBYTE*)&Mine_Target1);
	DetourDetach(&(PVOID&)CDetour::Real_GetMixLongLinkItems, *(PBYTE*)&Mine_Target2);
	DetourDetach(&(PVOID&)CDetour::Real_ReportLongIP, *(PBYTE*)&Mine_Target3);

	l = DetourTransactionCommit(); 
	
}

#endif
