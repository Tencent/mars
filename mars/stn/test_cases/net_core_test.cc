#include "util_func.h"

#ifdef NETCORE_TEST

#include "gtest/gtest.h"
#include "thread/thread.h"

#include "singleton.h"

#include "../../test/include/hook.h"
#include "../../test/include/mock_spy.h"

#include "../../../network/net_core.h"
#include "../../mmcomm/new_getdns.h"
#include "../../mmcomm/active_logic.h"

TEST(MMNetCore_test, init)
{
#ifdef _WIN32 
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2,2), &wsaData);
#endif
}

TEST(MMNetCore_test, newdns)
{
//    MMNewGetDns::SetDebugHostInfo("127.0.0.1", 8000);
//    EXPECT_NE((long)CMMNetCore::Singleton(), NULL);
//    CMMNetCore::Singleton()->GetNetSource().GetNewGetDnsCache().GetNewDns().startGetDns(0, 0, 0, 20*1000);
    
//    sleep(30);
//    CMMNetCore::Release();
//    SINGLETON_RELEASE_ALL();
	//for coverage
	CMMNetCore::Singleton()->ClearTask();
	CMMNetCore::Singleton()->GetNetSource();
	CMMNetCore::Singleton()->MakeSureLongLinkConnect();
	CMMNetCore::Singleton()->OnIDCChange();
	CMMNetCore::Singleton()->OnNetworkChange();
	CMMNetCore::Singleton()->RedoTasks();
	CNetCmd cmd;
	cmd.reqCmdID = 100;
	cmd.respCmdID = 1001;
	cmd.networkType = 3;
	cmd.cmduser_firstpkgtimeout = 10 * 1000;
	cmd.cmduser_retrycount = 0;
	CMMNetCore::Singleton()->StartTask(1, cmd);
	CMMNetCore::Singleton()->StopTask(1);
}

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
	bool Mine_Target1(std::vector<IPPortItem>& _IPPortItems);
	bool Mine_Target2(std::vector<IPPortItem>& _IPPortItems);

	static bool (CDetour::* Real_Target1)(std::vector<IPPortItem>& _IPPortItems);
	static bool (CDetour::* Real_Target2)(std::vector<IPPortItem>& _IPPortItems);
	// Class shouldn't have any member variables or virtual functions.



};

bool CDetour::Mine_Target1(std::vector<IPPortItem>& _IPPortItems)
{
	(this->*Real_Target1)(_IPPortItems);
	return true;
}

bool CDetour::Mine_Target2(std::vector<IPPortItem>& _IPPortItems)
{
	(this->*Real_Target2)(_IPPortItems);
	return true;
}

bool (CDetour::* CDetour::Real_Target1)(std::vector<IPPortItem>& _IPPortItems) = (bool (CDetour::*)(std::vector<IPPortItem>& _IPPortItems)) &CMMNetSource::GetLongLinkItems;
bool (CDetour::* CDetour::Real_Target2)(std::vector<IPPortItem>& _IPPortItems) = (bool (CDetour::*)(std::vector<IPPortItem>& _IPPortItems)) &CMMNetSource::GetMixLongLinkItems;


TEST(MMNetCore_test, longlinkconnect_test)
{
    MMNewGetDns::SetDebugHostInfo("127.0.0.1", 9001);
    CMMNetSource::SetDebugLongIP("");
	CMMNetSource::SetDebugLongPort(0);

	bool (CDetour::* Mine_Target1)(std::vector<IPPortItem>& _IPPortItems) = &CDetour::Mine_Target1;
	bool (CDetour::* Mine_Target2)(std::vector<IPPortItem>& _IPPortItems) = &CDetour::Mine_Target2;

	                                                      
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&(PVOID&)CDetour::Real_Target1, *(PBYTE*)&Mine_Target1);
	DetourAttach(&(PVOID&)CDetour::Real_Target2, *(PBYTE*)&Mine_Target2);
	LONG l = DetourTransactionCommit();


    LongLinkObserver2 longlinkobs;                                                                                                                                                                                                                                                                                                                      
    CMMNetSource netsource;
    ActiveLogic activelogic;
    
    netsource.m_getdns2Cache.m_dns.startGetDns(0, 0, 0, 3*1000);
	ThreadUtil::sleep(3);
    
    MMLongLink2 longlink(longlinkobs, &netsource, activelogic);
    longlink.MakeSureConnected();
	ThreadUtil::sleep(4);
    EXPECT_NE(MMLongLink2::EConnectFailed, longlink.m_connectstatus);

	DetourTransactionBegin();  
	DetourUpdateThread(GetCurrentThread());  
	DetourDetach(&(PVOID&)CDetour::Real_Target1, *(PBYTE*)&Mine_Target1);
	DetourDetach(&(PVOID&)CDetour::Real_Target2, *(PBYTE*)&Mine_Target2);
	l = DetourTransactionCommit(); 
}

TEST(MMNetCore_test, longlinkconnect_suc_test)
{
	LongLinkObserver2 longlinkobs;                                                                                                                                                                                                                                                                                                                      
    CMMNetSource netsource;
    ActiveLogic activelogic;
}

#endif
