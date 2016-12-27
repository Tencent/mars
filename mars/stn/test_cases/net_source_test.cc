/*
* Created on 2015-03-16
*	  Author: 闫国跃
*
*/


#include "util_func.h"

#ifdef NETSOURCE_TEST

#include "gtest/gtest.h"
#include "thread/thread.h"


#include "../../test/include/mock_spy.h"

#include "../../../network/MMNetSource.h"

#include "../../../log/appender.h"
#include "../../../comm/xlogger/xloggerbase.h"

#include "TestUtils.h"
#include "GtestEx.h"




#define FIND_IPPORTITEM(iter, iPPortItemVector, iPPortItem) \
	do { \
		for (iter=iPPortItemVector.begin();iter!=iPPortItemVector.end();++iter) \
		{ \
			if (iter->strIP==iPPortItem.strIP && iter->nPort==iPPortItem.nPort && iter->eSourceType==5) \
			{ \
				break; \
			} \
		} \
	} while (false)





//..\httpfile.py 127.0.0.88:8080

//history record's num < 5
//ReportLongIP false, successRate reduce
//TEST(MMNetSourceTest, test0)
PUBC_TEST(MMNetSourceTest, test0)
{
	UtilFunc::execute_cmd();
	m_testCaseInfo->m_TestCaseName = "history record's num < 5, ReportLongIP false, successRate reduce";

//	xlogger_SetLevel(ELevelDebug);
//	appender_open(EAppednerSync, "C:\\log", "GTEST");

	
	UtilFunc::del_files(CONFIG_PATH);


	MMNewGetDns::SetDebugHostInfo("127.0.0.88", 8180);

	CMMNetSource::SetDebugLongIP("");
	CMMNetSource::SetDebugLongPort(0);

	CMMNetSource netsource;

	std::vector<IPPortItem> iPPortItemVector;
	netsource.GetMixLongLinkItems(iPPortItemVector);
	ASSERT_TRUE(!iPPortItemVector.empty());

	PRINT_IPPORTITEMS(iPPortItemVector);
	IPPortItem ipPortItem = iPPortItemVector.front();

	BaseScene baseScene;
	baseScene.setEventName("report ipPortItem.strIP:ipPortItem.nPort false one times");

	netsource.ReportLongIP(false, ipPortItem.strIP, ipPortItem.nPort);

	iPPortItemVector.clear();
	netsource.GetMixLongLinkItems(iPPortItemVector);
	PRINT_IPPORTITEMS(iPPortItemVector);

	std::vector<IPPortItem>::iterator iter;
	FIND_IPPORTITEM(iter, iPPortItemVector, ipPortItem);

	if (iter != iPPortItemVector.end())
	{
		PUBC_EXPECT_EQ(0.875f, iter->successRate, &baseScene);
	} else {
		EXPECT_TRUE(false);
	}

	netsource.ReportLongIP(false, ipPortItem.strIP, ipPortItem.nPort);

	iPPortItemVector.clear();
	netsource.GetMixLongLinkItems(iPPortItemVector);
	PRINT_IPPORTITEMS(iPPortItemVector);

	FIND_IPPORTITEM(iter, iPPortItemVector, ipPortItem);


	if (iter != iPPortItemVector.end())
	{
		PUBC_EXPECT_EQ(0.75f, iter->successRate, &baseScene);
	} else {
		EXPECT_TRUE(false);
	}

}

//history record's num < 5
//ReportLongIP false three times, ip is baned
PUBC_TEST(MMNetSourceTest, test1)
{
	m_testCaseInfo->m_TestCaseName = "history record's num < 5, ReportLongIP false three times, ip is baned";
	UtilFunc::del_files(CONFIG_PATH);

	MMNewGetDns::SetDebugHostInfo("127.0.0.88", 8180);
	CMMNetSource netsource;

	std::vector<IPPortItem> iPPortItemVector;
	netsource.GetMixLongLinkItems(iPPortItemVector);


	ASSERT_TRUE(!iPPortItemVector.empty());

	IPPortItem ipPortItem = iPPortItemVector.front();

	BaseScene baseScene;
	baseScene.setEventName("report ipPortItem.strIP:ipPortItem.nPort false three times");

	netsource.ReportLongIP(false, ipPortItem.strIP, ipPortItem.nPort);
	netsource.ReportLongIP(false, ipPortItem.strIP, ipPortItem.nPort);
	netsource.ReportLongIP(false, ipPortItem.strIP, ipPortItem.nPort);
	
	iPPortItemVector.clear();
	netsource.GetMixLongLinkItems(iPPortItemVector);

	std::vector<IPPortItem>::iterator iter;
	FIND_IPPORTITEM(iter, iPPortItemVector, ipPortItem);

	PUBC_EXPECT_TRUE(iter==iPPortItemVector.end(), &baseScene);
}


//ReportLongIP true five times, ip will have history record
PUBC_TEST(MMNetSourceTest, test2)
{
	m_testCaseInfo->m_TestCaseName = "ReportLongIP true five times, ip will have history record";
	UtilFunc::del_files(CONFIG_PATH);

	MMNewGetDns::SetDebugHostInfo("127.0.0.88", 8180);
	CMMNetSource netsource;

	std::vector<IPPortItem> iPPortItemVector;
	netsource.GetMixLongLinkItems(iPPortItemVector);


	ASSERT_TRUE(!iPPortItemVector.empty());

	IPPortItem ipPortItem = iPPortItemVector.front();

	BaseScene baseScene;
	baseScene.setEventName("report ipPortItem.strIP:ipPortItem.nPort true five times");

	netsource.ReportLongIP(true, ipPortItem.strIP, ipPortItem.nPort);
	netsource.ReportLongIP(true, ipPortItem.strIP, ipPortItem.nPort);
	netsource.ReportLongIP(true, ipPortItem.strIP, ipPortItem.nPort);
	netsource.ReportLongIP(true, ipPortItem.strIP, ipPortItem.nPort);
	netsource.ReportLongIP(true, ipPortItem.strIP, ipPortItem.nPort);
	
	iPPortItemVector.clear();
	netsource.GetMixLongLinkItems(iPPortItemVector);

	PUBC_EXPECT_TRUE(iPPortItemVector.front().strIP == ipPortItem.strIP && iPPortItemVector.front().nPort == ipPortItem.nPort, &baseScene);
}

//history record's num >= 5, banlist is empty, ReportLongIP false, successRate reduce
PUBC_TEST(MMNetSourceTest, test3)
{
	m_testCaseInfo->m_TestCaseName = "history record's num >= 5, banlist is empty, ReportLongIP false, successRate reduce";
	UtilFunc::del_files(CONFIG_PATH);

	MMNewGetDns::SetDebugHostInfo("127.0.0.88", 8180);
	CMMNetSource netsource;

	std::vector<IPPortItem> iPPortItemVector;
	netsource.GetMixLongLinkItems(iPPortItemVector);


	ASSERT_TRUE(!iPPortItemVector.empty());

	IPPortItem ipPortItem = iPPortItemVector.front();

	BaseScene baseScene;
	baseScene.setEventName("report ipPortItem.strIP:ipPortItem.nPort true five times, false one time, clear banlist");

	netsource.ReportLongIP(true, ipPortItem.strIP, ipPortItem.nPort);
	netsource.ReportLongIP(true, ipPortItem.strIP, ipPortItem.nPort);
	netsource.ReportLongIP(true, ipPortItem.strIP, ipPortItem.nPort);
	netsource.ReportLongIP(true, ipPortItem.strIP, ipPortItem.nPort);
	netsource.ReportLongIP(true, ipPortItem.strIP, ipPortItem.nPort);
	
	iPPortItemVector.clear();
	netsource.GetMixLongLinkItems(iPPortItemVector);

	netsource.ReportLongIP(false, ipPortItem.strIP, ipPortItem.nPort);
	
	netsource.m_ipportstrategy.m_BanFailList.clear();
	
	iPPortItemVector.clear();
	netsource.GetMixLongLinkItems(iPPortItemVector);
	
	
	PUBC_EXPECT_TRUE(iPPortItemVector.front().strIP == ipPortItem.strIP && iPPortItemVector.front().nPort == ipPortItem.nPort, &baseScene);

	baseScene.eventName = "";

	PUBC_EXPECT_GT(1.0f, iPPortItemVector.front().successRate, &baseScene);
}

//history record's num >= 5, banlist is empty, ReportLongIP true, successRate increase
PUBC_TEST(MMNetSourceTest, test4)
{
	m_testCaseInfo->m_TestCaseName = "history record's num >= 5, banlist is empty, ReportLongIP true, successRate increase";

	UtilFunc::del_files(CONFIG_PATH);

	MMNewGetDns::SetDebugHostInfo("127.0.0.88", 8180);
	CMMNetSource netsource;

	std::vector<IPPortItem> iPPortItemVector;
	netsource.GetMixLongLinkItems(iPPortItemVector);


	ASSERT_TRUE(!iPPortItemVector.empty());

	IPPortItem ipPortItem = iPPortItemVector.front();

	netsource.ReportLongIP(false, ipPortItem.strIP, ipPortItem.nPort);
	netsource.ReportLongIP(true, ipPortItem.strIP, ipPortItem.nPort);
	netsource.ReportLongIP(true, ipPortItem.strIP, ipPortItem.nPort);
	netsource.ReportLongIP(true, ipPortItem.strIP, ipPortItem.nPort);
	netsource.ReportLongIP(true, ipPortItem.strIP, ipPortItem.nPort);
	
	netsource.m_ipportstrategy.m_BanFailList.clear();
	
	iPPortItemVector.clear();
	netsource.GetMixLongLinkItems(iPPortItemVector);

	BaseScene baseScene;
	baseScene.setEventName("report ipPortItem.strIP:ipPortItem.nPort false one times, true four times, clear banlist");

	//EXPECT_TRUE(iPPortItemVector.front().strIP == ipPortItem.strIP && iPPortItemVector.front().nPort == ipPortItem.nPort);
	PUBC_EXPECT_TRUE(iPPortItemVector.front().strIP == ipPortItem.strIP && iPPortItemVector.front().nPort == ipPortItem.nPort, &baseScene);
	
	float beforeRate = iPPortItemVector.front().successRate;

	netsource.ReportLongIP(true, ipPortItem.strIP, ipPortItem.nPort);
	
	netsource.m_ipportstrategy.m_BanFailList.clear();
	
	iPPortItemVector.clear();
	netsource.GetMixLongLinkItems(iPPortItemVector);

	baseScene.setEventName("report ipPortItem.strIP:ipPortItem.nPort %d one times", true);

	PUBC_EXPECT_TRUE(iPPortItemVector.front().strIP == ipPortItem.strIP && iPPortItemVector.front().nPort == ipPortItem.nPort, &baseScene);
	
	baseScene.eventName.clear();
	PUBC_EXPECT_LT(beforeRate, iPPortItemVector.front().successRate, &baseScene);
	
}

//history record's num >= 5, banlist is empty, ReportLongIP false three times, ip is baned
PUBC_TEST(MMNetSourceTest, test5)
{
	m_testCaseInfo->m_TestCaseName = "history record's num >= 5, banlist is empty, ReportLongIP false three times, ip is baned";
	UtilFunc::del_files(CONFIG_PATH);

	MMNewGetDns::SetDebugHostInfo("127.0.0.88", 8180);
	CMMNetSource netsource;

	std::vector<IPPortItem> iPPortItemVector;
	netsource.GetMixLongLinkItems(iPPortItemVector);


	ASSERT_TRUE(!iPPortItemVector.empty());

	IPPortItem ipPortItem = iPPortItemVector.front();

	netsource.ReportLongIP(true, ipPortItem.strIP, ipPortItem.nPort);
	netsource.ReportLongIP(true, ipPortItem.strIP, ipPortItem.nPort);
	netsource.ReportLongIP(true, ipPortItem.strIP, ipPortItem.nPort);
	netsource.ReportLongIP(true, ipPortItem.strIP, ipPortItem.nPort);
	netsource.ReportLongIP(true, ipPortItem.strIP, ipPortItem.nPort);

	
	netsource.ReportLongIP(false, ipPortItem.strIP, ipPortItem.nPort);
	netsource.ReportLongIP(false, ipPortItem.strIP, ipPortItem.nPort);
	netsource.ReportLongIP(false, ipPortItem.strIP, ipPortItem.nPort);
	
	iPPortItemVector.clear();
	netsource.GetMixLongLinkItems(iPPortItemVector);

	BaseScene baseScene;
	baseScene.setEventName("report ipPortItem.strIP:ipPortItem.nPort true five times, false three times");

	std::vector<IPPortItem>::iterator iter;
	FIND_IPPORTITEM(iter, iPPortItemVector, ipPortItem);

	//EXPECT_EQ(iPPortItemVector.end(), iter);
	PUBC_EXPECT_EQ(iPPortItemVector.end(), iter, &baseScene);
}


//history record's num >= 5, banlist is not empty, ReportLongIP false, successRate reduce
PUBC_TEST(MMNetSourceTest, test6)
{
	m_testCaseInfo->m_TestCaseName = "history record's num >= 5, banlist is not empty, ReportLongIP false, successRate reduce";
	UtilFunc::del_files(CONFIG_PATH);

	MMNewGetDns::SetDebugHostInfo("127.0.0.88", 8180);
	CMMNetSource netsource;

	std::vector<IPPortItem> iPPortItemVector;
	netsource.GetMixLongLinkItems(iPPortItemVector);


	ASSERT_TRUE(!iPPortItemVector.empty());

	IPPortItem ipPortItem = iPPortItemVector.front();

	netsource.ReportLongIP(true, ipPortItem.strIP, ipPortItem.nPort);
	netsource.ReportLongIP(true, ipPortItem.strIP, ipPortItem.nPort);
	netsource.ReportLongIP(true, ipPortItem.strIP, ipPortItem.nPort);
	netsource.ReportLongIP(true, ipPortItem.strIP, ipPortItem.nPort);
	netsource.ReportLongIP(true, ipPortItem.strIP, ipPortItem.nPort);
	
	iPPortItemVector.clear();
	netsource.GetMixLongLinkItems(iPPortItemVector);

	netsource.ReportLongIP(false, ipPortItem.strIP, ipPortItem.nPort);
	
	iPPortItemVector.clear();
	netsource.GetMixLongLinkItems(iPPortItemVector);

	std::vector<IPPortItem>::iterator iter;
	FIND_IPPORTITEM(iter, iPPortItemVector, ipPortItem);
	
	BaseScene baseScene;
	baseScene.setEventName("report ipPortItem.strIP:ipPortItem.nPort true five times, false one times");

	PUBC_EXPECT_GT(1.0f, iter->successRate, &baseScene);
}

//history record's num >= 5, banlist is not empty, ReportLongIP true, successRate increase
PUBC_TEST(MMNetSourceTest, test7)
{
	m_testCaseInfo->m_TestCaseName = "history record's num >= 5, banlist is not empty, ReportLongIP true, successRate increase";
	UtilFunc::del_files(CONFIG_PATH);

	MMNewGetDns::SetDebugHostInfo("127.0.0.88", 8180);
	CMMNetSource netsource;

	std::vector<IPPortItem> iPPortItemVector;
	netsource.GetMixLongLinkItems(iPPortItemVector);


	ASSERT_TRUE(!iPPortItemVector.empty());

	IPPortItem ipPortItem = iPPortItemVector.front();

	netsource.ReportLongIP(false, ipPortItem.strIP, ipPortItem.nPort);
	netsource.ReportLongIP(false, ipPortItem.strIP, ipPortItem.nPort);
	netsource.ReportLongIP(true, ipPortItem.strIP, ipPortItem.nPort);
	netsource.ReportLongIP(true, ipPortItem.strIP, ipPortItem.nPort);
	netsource.ReportLongIP(true, ipPortItem.strIP, ipPortItem.nPort);
	netsource.ReportLongIP(true, ipPortItem.strIP, ipPortItem.nPort);
	netsource.ReportLongIP(true, ipPortItem.strIP, ipPortItem.nPort);
	netsource.ReportLongIP(true, ipPortItem.strIP, ipPortItem.nPort);
	
	
	iPPortItemVector.clear();
	netsource.GetMixLongLinkItems(iPPortItemVector);

	//PRINT_IPPORTITEMS(iPPortItemVector);

	std::vector<IPPortItem>::iterator iter;
	FIND_IPPORTITEM(iter, iPPortItemVector, ipPortItem);

	float beforeRate = iter->successRate;

	netsource.ReportLongIP(true, ipPortItem.strIP, ipPortItem.nPort);
	
	iPPortItemVector.clear();
	netsource.GetMixLongLinkItems(iPPortItemVector);
	//PRINT_IPPORTITEMS(iPPortItemVector);

	FIND_IPPORTITEM(iter, iPPortItemVector, ipPortItem);

	BaseScene baseScene;
	baseScene.setEventName("report ipPortItem.strIP:ipPortItem.nPort false two times, true six times(get beforeRate), true one times");

	PUBC_EXPECT_GT(iter->successRate, beforeRate, &baseScene);
	
}

//history record's num >= 5, banlist is not empty, ReportLongIP false three times, ip is baned
PUBC_TEST(MMNetSourceTest, test8)
{
	m_testCaseInfo->m_TestCaseName = "history record's num >= 5, banlist is not empty, ReportLongIP false three times, ip is baned";
	UtilFunc::del_files(CONFIG_PATH);

	MMNewGetDns::SetDebugHostInfo("127.0.0.88", 8180);
	CMMNetSource netsource;

	std::vector<IPPortItem> iPPortItemVector;
	netsource.GetMixLongLinkItems(iPPortItemVector);


	ASSERT_TRUE(!iPPortItemVector.empty());

	IPPortItem ipPortItem = iPPortItemVector.front();


	netsource.ReportLongIP(true, ipPortItem.strIP, ipPortItem.nPort);
	netsource.ReportLongIP(false, ipPortItem.strIP, ipPortItem.nPort);
	netsource.ReportLongIP(true, ipPortItem.strIP, ipPortItem.nPort);
	netsource.ReportLongIP(true, ipPortItem.strIP, ipPortItem.nPort);
	netsource.ReportLongIP(true, ipPortItem.strIP, ipPortItem.nPort);

	netsource.ReportLongIP(false, ipPortItem.strIP, ipPortItem.nPort);
	netsource.ReportLongIP(false, ipPortItem.strIP, ipPortItem.nPort);
	
	iPPortItemVector.clear();
	netsource.GetMixLongLinkItems(iPPortItemVector);

	std::vector<IPPortItem>::iterator iter;
	FIND_IPPORTITEM(iter, iPPortItemVector, ipPortItem);

	BaseScene baseScene;
	baseScene.setEventName("report ipPortItem.strIP:ipPortItem.nPort true one times, false one times, true three times, false two times");
	PUBC_EXPECT_EQ(iPPortItemVector.end(), iter, &baseScene);

	//for coverage
	netsource.GetNewGetDnsCache().ReportFailIp("123.0.255.1");
	netsource.GetNewGetDnsCache().ForceRefresh();
	netsource.ClearShortLinkProxyInfo();
	std::vector<std::string> iPList;
	iPList.push_back("192.168.1.1");
	netsource.SaveAuthLongIPList("Auth_test_host", iPList);
	std::vector<std::string> portList;
	portList.push_back("999");
	netsource.SaveAuthLongPortList(portList);

	netsource.SaveAuthShortIPList("Auth_test_host", iPList);
	netsource.SaveAuthShortPortList(portList);
	netsource.SetDebugShortIP("192.168.1.1");
	netsource.SetDebugShortPort(80);
	netsource.ClearCache();
	std::string ip;
	netsource.GetLastSvrIP(ip);
	std::vector<IPItem> ipVector;
	netsource.GetInitLongIPs(ipVector);
	netsource.GetInitShortIPs(ipVector);
	netsource.SetDebugShortIP("");
	netsource.SetDebugShortPort(0);
}
#endif
