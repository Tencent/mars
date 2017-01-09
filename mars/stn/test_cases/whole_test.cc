/*
* Created on:2015-04-10
*	  Author:闫国跃
*
*/
#if 0
#include <stdlib.h>
#include "UtilFunc.h"

#define WHOLE_TEST
#ifdef WHOLE_TEST
#include "gtest/gtest.h"

#include "TestUtils.h"
#include "GtestEx.h"

#include "../../../comm/singleton.h"
#include "../../test/include/hook.h"
#include "../../test/include/mock_spy.h"

#include "../../../network/MMNetCore.h"
#include "../../../network/MMLongLinkTaskManager.h"
#include "../../../network/MMNetSourceTimerCheck.h"
#include "../../../network/MMHostInfoLogic.h"
#include "../../../mmcomm/MMNewGetDns.h"
#include "../../../mmcomm/ActiveLogic.h"

#include "../MmNetComm.h"

#include "../../../log/appender.h"
#include "../../../comm/xlogger/xloggerbase.h"

#include "MockServerControlUtils.h"

#define NEWDNS_SERVER_IP	"127.0.0.89"
#define NEWDNS_SERVER_PORT	8182

#define SOCKET int
#define TLongLinkStatus MMLongLink2::TLongLinkStatus
using std::string;
namespace{
class CDetour /* add ": public CMember" to enable access to member variables... */
{
public:
	bool Mine_GetLongLinkItems(std::vector<IPPortItem>& _IPPortItems);
	bool Mine_GetMixLongLinkItems(std::vector<IPPortItem>& _IPPortItems);
	
	void Mine_ReportLongIP(bool _isSuccess, const std::string& _ip, unsigned int _port);

	//1:
	bool Mine_MakeSureConnected(bool* _newone=NULL);
	SOCKET Mine_RunConnect();
	void Mine_ConnectStatus(TLongLinkStatus _status);
	bool Mine_TryConnect(const string& _host);

	static bool (CDetour::* Real_GetLongLinkItems)(std::vector<IPPortItem>& _IPPortItems);
	static bool (CDetour::* Real_GetMixLongLinkItems)(std::vector<IPPortItem>& _IPPortItems);
	static void (CDetour::* Real_ReportLongIP)(bool _isSuccess, const std::string& _ip, unsigned int _port);

	//2:
	static  bool (CDetour::* Real_MakeSureConnected)(bool* _newone);
	static  SOCKET (CDetour::* Real_RunConnect)();
	static  void (CDetour::* Real_ConnectStatus)(TLongLinkStatus _status);
	static bool (CDetour::* Real_TryConnect)(const string& _host);
};

static Condition sg_Condition; //global
static bool sg_isEnd = false; //global
static bool sg_readyToSwitchBackToSatelliteIdc =false;//global
static bool sg_readyToSwitchBackToNewdnsIP =false;//global
static int sg_TestType = 0; //global
//sg_testType:
//0:	hardcode ip/newdnsip switch      
//1:	satellite IDC/main IDC switch,only one main IDC IP restore  
//2:	satellite IDC/main IDC switch,all main IDC IPs restore  
//3:	satellite IDC/main IDC switch /back to satellite IDC, all main IDC and satellite IDC IPs restore

static std::vector<IPPortItem> sg_restoreSatelliteIdcIPs;//global
static std::vector<IPPortItem> sg_restoreNewdnsIPs; //global

class TestTimeItem
{
  public:
	unsigned long long startTime;  
	unsigned long long findHardcodeIpTime;
	unsigned long long connectHardcodeIpTime;
	unsigned long long connectNewdnsIpTime;
	unsigned long long connectSatelliteIdcNewdnsIpTime;
	unsigned long long connectMainIdcNewdnsIpTime;
	unsigned long long switchBackToSatelliteIdcTime;
	TestTimeItem()
	{
		startTime = 0;  
		findHardcodeIpTime = 0;
		connectHardcodeIpTime = 0;
		connectNewdnsIpTime = 0;
		connectSatelliteIdcNewdnsIpTime = 0;
		connectMainIdcNewdnsIpTime = 0;
		switchBackToSatelliteIdcTime = 0;
	}
};
static std::vector<TestTimeItem> sg_lstTotalTimeItems;//global

class TestCostItem
{
  public:
	unsigned long long findHardcodeIPcost;
	unsigned long long connectHardcodeIPcost;
	unsigned long long connNewdnsIPcost; 
	unsigned long long ConnectSatelliteNewdnsIpCost;
	unsigned long long ConnectMainNewdnsIpCost;
	unsigned long long SwitchBackToSatelliteIdcCost;

	TestCostItem()
	{
		findHardcodeIPcost = 170*1000;
		connectHardcodeIPcost = 6*4*1000 + 500;//6 ComplexConnect intervals
		connNewdnsIPcost = 2*2.5*60*1000 + 500;//2 TimerCheck
		ConnectMainNewdnsIpCost = 7000;
		ConnectSatelliteNewdnsIpCost = 7000;
		SwitchBackToSatelliteIdcCost = 7000;
	}

};

void printAllTime(int line)
{
	std::vector<TestTimeItem>::iterator iter = sg_lstTotalTimeItems.begin();
	int index = 0;
	for(; iter!=sg_lstTotalTimeItems.end();++iter)
	{
		printf("\n &&&line=%d:index=%d, startTime=%llu,findHardcodeIpTime=%llu,connectHardcodeIpTime=%llu,connectNewdnsIpTime=%llu \n",line, index, iter->startTime, iter->findHardcodeIpTime, iter->connectHardcodeIpTime, iter->connectNewdnsIpTime);
		printf("findHardcodeIPcost=%llu,connectHardcodeIPconst=%llu,connectNewdnsIPcost=%llu\n", iter->findHardcodeIpTime-iter->startTime, iter->connectHardcodeIpTime-iter->findHardcodeIpTime, iter->connectNewdnsIpTime-iter->connectHardcodeIpTime);
		index++;
	}
}
void printTimeAndCost(int _line, int testType, const TestTimeItem& _item)
{
	//time
	if(0 ==testType)
	{
		printf("\n ~~~line=%d: startTime=%llu,findHC=%llu,connHC=%llu,connND=%llu\n",_line,_item.startTime, _item.findHardcodeIpTime, _item.connectHardcodeIpTime, _item.connectNewdnsIpTime);
	}
	else if(3 == testType)
	{
		printf("\n ~~~line=%d:connSatellite=%llu,connMain=%llu,backSatellite=%llu\n",_line, _item.connectSatelliteIdcNewdnsIpTime, _item.connectMainIdcNewdnsIpTime, _item.switchBackToSatelliteIdcTime);
	}
	//cost
	if(0 ==testType)
	{
		printf("cost:findHC=%llu,connHC=%llu,connND=%llu\n", _item.findHardcodeIpTime-_item.startTime, _item.connectHardcodeIpTime-_item.findHardcodeIpTime, _item.connectNewdnsIpTime-_item.connectHardcodeIpTime);
	}
	else if(3 == testType)
	{
		printf("cost:connSatellite=%llu,connMain=%llu,backSatellite=%llu\n", _item.connectSatelliteIdcNewdnsIpTime-_item.startTime, _item.connectMainIdcNewdnsIpTime-_item.connectSatelliteIdcNewdnsIpTime, 
			_item.switchBackToSatelliteIdcTime-_item.connectMainIdcNewdnsIpTime);
	}

	
}

bool CDetour::Mine_GetLongLinkItems(std::vector<IPPortItem>& _IPPortItems)
{
	(this->*Real_GetLongLinkItems)(_IPPortItems);
	PRINT_IPPORTITEMS(_IPPortItems);

	for (std::vector<IPPortItem>::iterator iter=_IPPortItems.begin();iter!=_IPPortItems.end();++iter)
	{
		if (EIPSourceNewDns != iter->eSourceType)
		{
			std::vector<TestTimeItem>::iterator iterTimeItem = sg_lstTotalTimeItems.end() - 1;
			static int count = 1;
			if(0==iterTimeItem->findHardcodeIpTime)	
			{
				count = 1;
				iterTimeItem->findHardcodeIpTime = ::gettickcount();
			}
			else
			{
				printf("count:%d:findHardCodeIP, timeNow=%llu, findHardcodeIpTime=%llu \n", ++count, ::gettickcount(), iterTimeItem->findHardcodeIpTime);
			}
			//sg_isEnd = true;
			printf("line:%d, eSourceType=%d\n",__LINE__, iter->eSourceType);
	//		printAllTime(__LINE__);
			break;
		}
	}
	return true;
	
}

bool CDetour::Mine_GetMixLongLinkItems(std::vector<IPPortItem>& _IPPortItems)
{

	(this->*Real_GetMixLongLinkItems)(_IPPortItems);
	PRINT_IPPORTITEMS(_IPPortItems);

	for (std::vector<IPPortItem>::iterator iter=_IPPortItems.begin();iter!=_IPPortItems.end();++iter)
	{
		if (EIPSourceNewDns != iter->eSourceType)
		{
			std::vector<TestTimeItem>::iterator iterTimeItem = sg_lstTotalTimeItems.end() - 1;
			if(0==iterTimeItem->findHardcodeIpTime)	iterTimeItem->findHardcodeIpTime = ::gettickcount();
			
		//	sg_isEnd = true;
			printf("line:%d, eSourceType=%d\n",__LINE__, iter->eSourceType);
	//		printAllTime(__LINE__);
			break;
		}
	}
	return true;
}

void CDetour::Mine_ReportLongIP(bool _isSuccess, const std::string& _ip, unsigned int _port)
{
	//every ip complexConnect failed, will be called here
	static int times = 0;
	(this->*Real_ReportLongIP)(_isSuccess, _ip, _port);
	printf("%s:line:%d, isSuccess:%d, ip:%s:%u\n", __FUNCTION__, __LINE__, _isSuccess,_ip.c_str(), _port);
	times++;
	printf("@@@@@@@@@@@ReportLongIP:  times=%d\n", times);
	if(!_isSuccess)
	{
		IPPortItem tempItem;
		tempItem.strIP = _ip;
		tempItem.nPort = _port;
		killNormalEcho(tempItem);
	}
}

//3:
std::vector <unsigned long long>  makeSureConnCostTime;  //global
bool  CDetour::Mine_MakeSureConnected(bool* _newone){
	static int times = 0;
	unsigned long long startTime = ::gettickcount();
	bool ret = (this->*Real_MakeSureConnected)(_newone);
	unsigned long long endTime = ::gettickcount();
	makeSureConnCostTime.push_back(endTime - startTime);

	times++;
//	printf("MakeSureConnected: costTime=%llu, times=%d\n",endTime - startTime, times);
	return ret;
}

std::vector <unsigned long long>  runConnCostTime;  //global
SOCKET  CDetour::Mine_RunConnect(){
	static int times = 0;
	unsigned long long startTime = ::gettickcount();
	SOCKET sock = (this->*Real_RunConnect)();
	unsigned long long endTime = ::gettickcount();
	runConnCostTime.push_back(endTime - startTime);

	times++;
	printf("@@@@@@@@@@@RunConnect: costTime=%llu, times=%d\n",endTime - startTime, times);
	return sock;
}



static bool isLonglinkNewdnsIP()
{
	return EIPSourceNewDns == CMMNetCore::Singleton()->m_longLinkTaskManager->LongLinkChannel().m_connInfo.ipType;
}
static bool isLonglinkHardCodeIP()
{
	return EIPSourceHardcode == CMMNetCore::Singleton()->m_longLinkTaskManager->LongLinkChannel().m_connInfo.ipType;
}
static bool isSatelliteIdcNewdnsIP()
{
	bool ret = true;
	if(EIPSourceNewDns != CMMNetCore::Singleton()->m_longLinkTaskManager->LongLinkChannel().m_connInfo.ipType)
	{
		ret = false;
	}
	const char *strIP = CMMNetCore::Singleton()->m_longLinkTaskManager->LongLinkChannel().m_connInfo.ip.c_str();
	if(0 != strncmp("127.0.50.", strIP, 9))
	{
		ret = false;
	}
	return ret;
}
static bool isMainIdcNewdnsIP()
{
	bool ret = true;
	if(EIPSourceNewDns != CMMNetCore::Singleton()->m_longLinkTaskManager->LongLinkChannel().m_connInfo.ipType)
	{
		ret = false;
	}
	const char *strIP = CMMNetCore::Singleton()->m_longLinkTaskManager->LongLinkChannel().m_connInfo.ip.c_str();
	if(0 != strncmp("127.0.51.", strIP, 9))
	{
		ret = false;
	}
	return ret;
}
static void printLonglinkConnInfo()
{
	const static char* const ITEM_DELIMITER = ":";
	LongLinkConnectionInfo connInfo = CMMNetCore::Singleton()->m_longLinkTaskManager->LongLinkChannel().m_connInfo;
	printf("-----\n iptype:%d,ipIndex:%d, ip:%s, port:%d, connTime:%llu, cost:%lu, host:%s\n", connInfo.ipType, connInfo.ipIndex, connInfo.ip.c_str(),connInfo.port,connInfo.connTime,connInfo.cost,connInfo.host.c_str());
	printf("connErrCode:%d, localIp:%s,external_ip:%s\n",connInfo.connErrCode,connInfo.localIp.c_str(),connInfo.external_ip.c_str());

	std::stringstream stream;
	for(unsigned int i=0;i<connInfo.ipItems.size();i++)
    {
		stream << connInfo.ipItems[i].strIP << ITEM_DELIMITER << connInfo.ipItems[i].nPort << ITEM_DELIMITER << connInfo.ipItems[i].strHost
				<< ITEM_DELIMITER << IPSourceTypeString[connInfo.ipItems[i].eSourceType];

		if(i != connInfo.ipItems.size()-1)
        {
			stream << "|\n";
		}
	}
	printf("%s\n-----\n",stream.str().c_str());
}


void CDetour::Mine_ConnectStatus(TLongLinkStatus _status)
{
	(this->*Real_ConnectStatus)(_status);
	printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~status %d\n",_status);

	if((MMLongLink2::EConnected != _status)) return;

	if(0 == sg_TestType)
	{
		if(isLonglinkHardCodeIP())
		{
			std::vector<TestTimeItem>::iterator iter = sg_lstTotalTimeItems.end() - 1;
			static int count = 1;
			if(0==iter->connectHardcodeIpTime)
			{
				count = 1;
				iter->connectHardcodeIpTime = ::gettickcount();
				printTimeAndCost(__LINE__, sg_TestType, *iter);
			}
			else
			{
				printf("count:%d:ConnectHardCodeIP, timeNow=%llu, connectHardcodeIpTime=%llu\n", ++count, ::gettickcount(), iter->connectHardcodeIpTime);
			}
			printLonglinkConnInfo();
			sg_readyToSwitchBackToNewdnsIP = true;
			printf("sg_testType==0;sg_readyToSwitchBackToNewdnsIP=true\n");
			int startSuccCount = startNormalEchoServerList(sg_restoreNewdnsIPs);//here
			if(startSuccCount != sg_restoreNewdnsIPs.size())
			{
				printf("line:%d, error:startSuccCount is not equal with size.\n", __LINE__);
			}
		}
		else if(isLonglinkNewdnsIP())
		{
			if(!sg_readyToSwitchBackToNewdnsIP)
			{
				printf("~~~~error: must be connect HardcodeIP before newdnsIP\n");
			}
			std::vector<TestTimeItem>::iterator iter = sg_lstTotalTimeItems.end() - 1;
			if(0==iter->connectNewdnsIpTime)	
			{
				iter->connectNewdnsIpTime = ::gettickcount();
			}
			printLonglinkConnInfo();
			sg_isEnd = true;
			sg_Condition.notifyOne();
		}
	}
	else if(sg_TestType > 0)
	{
		if(isSatelliteIdcNewdnsIP())
		{
			std::vector<TestTimeItem>::iterator iter = sg_lstTotalTimeItems.end() - 1;
			if(0==iter->connectSatelliteIdcNewdnsIpTime)	iter->connectSatelliteIdcNewdnsIpTime = ::gettickcount();
			if(3 == sg_TestType)
			{
				if(sg_readyToSwitchBackToSatelliteIdc)
				{
					std::vector<TestTimeItem>::iterator iter = sg_lstTotalTimeItems.end() - 1;
					if(0==iter->switchBackToSatelliteIdcTime)	iter->switchBackToSatelliteIdcTime = ::gettickcount();
					printLonglinkConnInfo();
					sg_isEnd = true;
					printf("sg_TestType=3;sg_readyToSwitchBackToSatelliteIdc==true;sg_isEnd==true\n");
					sg_Condition.notifyOne();
				}
				else
				{
					printf("connect satellite IDC:\n");
					printTimeAndCost(__LINE__, sg_TestType, *(sg_lstTotalTimeItems.end() - 1));
					
					//Should be kill all IDC SERVERs
					killAllSatelliteIdcServer();
					startAllMainIdcServer();
				}
			}
			else
			{
				printf("error sg_TestType.\n");
			}
			return;
		}
		else if( isMainIdcNewdnsIP())
		{
			std::vector<TestTimeItem>::iterator iter = sg_lstTotalTimeItems.end() - 1;
			if(0==iter->connectMainIdcNewdnsIpTime)	iter->connectMainIdcNewdnsIpTime = ::gettickcount();
			printTimeAndCost(__LINE__, sg_TestType, *iter);
			if(3 == sg_TestType)
			{
				sg_readyToSwitchBackToSatelliteIdc = true;
				sg_Condition.notifyOne();//for add to sg_removeFromBanListItems without delay
			}
			return ;
		}
	}
}

bool CDetour::Mine_TryConnect(const string& _host)
{
	printf("_host=%s\n",_host.c_str());
	//(this->*Real_TryConnect)(_host);
	if(3 == sg_TestType&&sg_readyToSwitchBackToSatelliteIdc)
	{
		std::vector<IPPortItem>::iterator iter = sg_restoreSatelliteIdcIPs.begin();
		if(sg_restoreSatelliteIdcIPs.end()==iter)
		{
			printf("line:%d, error:sg_restoreSatelliteIdcIPs is empty.\n", __LINE__);
		}
		for(;iter!=sg_restoreSatelliteIdcIPs.end(); ++iter)
		{
			CMMNetCore::Singleton()->m_netsource->RemoveLongBanIP(iter->strIP);
			startNormalEchoServer(iter->strIP.c_str());  //here
		}
	}else if(0 == sg_TestType&&sg_readyToSwitchBackToNewdnsIP)
	{
		std::vector<IPPortItem>::iterator iter = sg_restoreNewdnsIPs.begin();
		if(sg_restoreNewdnsIPs.end()==iter)
		{
			printf("line:%d, error:sg_restoreNewdnsIPs is empty.\n", __LINE__);
		}
		for(;iter!=sg_restoreNewdnsIPs.end(); ++iter)
		{
			CMMNetCore::Singleton()->m_netsource->RemoveLongBanIP(iter->strIP);
		}
	}
    return true;
}


bool (CDetour::* CDetour::Real_GetLongLinkItems)(std::vector<IPPortItem>& _IPPortItems) = (bool (CDetour::*)(std::vector<IPPortItem>& _IPPortItems)) &CMMNetSource::GetLongLinkItems;
bool (CDetour::* CDetour::Real_GetMixLongLinkItems)(std::vector<IPPortItem>& _IPPortItems) = (bool (CDetour::*)(std::vector<IPPortItem>& _IPPortItems)) &CMMNetSource::GetMixLongLinkItems;
void (CDetour::* CDetour::Real_ReportLongIP)(bool _isSuccess, const std::string& _ip, unsigned int _port) = (void (CDetour::*)(bool _isSuccess, const std::string& _ip, unsigned int _port)) &CMMNetSource::ReportLongIP;
//4:
bool (CDetour::* CDetour::Real_MakeSureConnected)(bool* _newone)=(bool (CDetour::*)(bool* _newone)) &MMLongLink2::MakeSureConnected;


//-------------private start
void * p1 = (void *)DetourFindFunction("test.exe","MMLongLink2::__RunConnect");
SOCKET (CDetour::* CDetour::Real_RunConnect)()=(SOCKET (CDetour::* &)()) p1;
void * p2 = (void *)DetourFindFunction("test.exe","MMLongLink2::__ConnectStatus");
void (CDetour::* CDetour::Real_ConnectStatus)(TLongLinkStatus _status)=(void (CDetour::* &)(TLongLinkStatus )) p2;
void * p3 = (void *)DetourFindFunction("test.exe","CMMNetSourceTimerCheck::__TryConnnect");
bool (CDetour::* CDetour::Real_TryConnect)(const string& _host)=(bool (CDetour::* &)(const string&)) p3;
//-------------private end

static bool (CDetour::* Mine_Target1)(std::vector<IPPortItem>& _IPPortItems) = &CDetour::Mine_GetLongLinkItems;
static bool (CDetour::* Mine_Target2)(std::vector<IPPortItem>& _IPPortItems) = &CDetour::Mine_GetMixLongLinkItems;
static void (CDetour::* Mine_Target3)(bool _isSuccess, const std::string& _ip, unsigned int _port) = &CDetour::Mine_ReportLongIP;
//5:
static bool (CDetour::* Mine_Target4)(bool* _newone) = &CDetour::Mine_MakeSureConnected;



static void Hook()
{
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&(PVOID&)CDetour::Real_GetLongLinkItems, *(PBYTE*)&Mine_Target1);
	DetourAttach(&(PVOID&)CDetour::Real_GetMixLongLinkItems, *(PBYTE*)&Mine_Target2);
	DetourAttach(&(PVOID&)CDetour::Real_ReportLongIP, *(PBYTE*)&Mine_Target3);
	//6:
	DetourAttach(&(PVOID&)CDetour::Real_MakeSureConnected, *(PBYTE*)&Mine_Target4);


	DetourTransactionCommit();
}
#include "comm/singleton.h"
static void UnHook()
{
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());  
	DetourDetach(&(PVOID&)CDetour::Real_GetLongLinkItems, *(PBYTE*)&Mine_Target1);
	DetourDetach(&(PVOID&)CDetour::Real_GetMixLongLinkItems, *(PBYTE*)&Mine_Target2);
	DetourDetach(&(PVOID&)CDetour::Real_ReportLongIP, *(PBYTE*)&Mine_Target3);
	//7:
	DetourDetach(&(PVOID&)CDetour::Real_MakeSureConnected, *(PBYTE*)&Mine_Target4);

	DetourTransactionCommit();
}

static void HookPrivateMemberFn()
{

	SOCKET (CDetour::* Mine_Target1)() = &CDetour::Mine_RunConnect;
	void (CDetour::* Mine_Target2)(TLongLinkStatus ) = &CDetour::Mine_ConnectStatus;
	bool (CDetour::* Mine_Target3)(const std::string& ) = &CDetour::Mine_TryConnect;
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&(PVOID&)CDetour::Real_RunConnect, *(PBYTE*)&Mine_Target1);
	DetourAttach(&(PVOID&)CDetour::Real_ConnectStatus, *(PBYTE*)&Mine_Target2);
	DetourAttach(&(PVOID&)CDetour::Real_TryConnect, *(PBYTE*)&Mine_Target3);
	DetourTransactionCommit();

}

static void UnHookPrivateMemberFn()
{

	SOCKET (CDetour::* Mine_Target1)() = &CDetour::Mine_RunConnect;
	void (CDetour::* Mine_Target2)(TLongLinkStatus ) = &CDetour::Mine_ConnectStatus;
	bool (CDetour::* Mine_Target3)(const std::string& ) = &CDetour::Mine_TryConnect;
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourDetach(&(PVOID&)CDetour::Real_RunConnect, *(PBYTE*)&Mine_Target1);
	DetourDetach(&(PVOID&)CDetour::Real_ConnectStatus, *(PBYTE*)&Mine_Target2);
	DetourDetach(&(PVOID&)CDetour::Real_TryConnect, *(PBYTE*)&Mine_Target3);
	DetourTransactionCommit();

}

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

static void ChangeBindFunc(CMMNetCore* _netcore)
{
	_netcore->m_longLinkTaskManager->funCallback = boost::bind(&CallBack, 0, _1, _2, _3, _4, _5, _6);
	_netcore->m_longLinkTaskManager->funbuf2Resp = buf2Resp;
}


class WholeTestParam
{
  public:
	bool isForeGround;
	unsigned long long timeAccelerateMilliSec;
	bool isStartTask;
	unsigned long long startTaskInterval;
	unsigned long long expectMaxTime;
	int index;
	int testType;
	TestCostItem expectMaxCostTime;
	std::vector<std::string> eventNames;
	std::vector<IPPortItem> restoreSatelliteIdcIPs;
	std::vector<IPPortItem> restoreNewdnsIPs;
	std::vector<std::pair<std::string, std::string> > idcHostTable;

	WholeTestParam()
	{ 
		this->isForeGround = false;
		this->timeAccelerateMilliSec = 0;
		this->isStartTask = false;
		this->startTaskInterval = 0;
		this->expectMaxTime = 60*1000;
		this->index = 0;
		this->testType = -1;
	}
	WholeTestParam(bool isForeGround,bool isStartTask, unsigned long long startTaskInterval, unsigned long long expectMaxTime,unsigned long long timeAccelerateMilliSec=0)
	{ 
		this->isForeGround = isForeGround;
		if(isForeGround) this->timeAccelerateMilliSec = timeAccelerateMilliSec;
		this->isStartTask = isStartTask;
		this->startTaskInterval = startTaskInterval;
		this->expectMaxTime = expectMaxTime;
		this->index = 0;
		expectMaxCostTime.findHardcodeIPcost = expectMaxTime;
		this->testType = -1;
	}
};

static bool isTimeOut(unsigned long long elapseTime)
{
	return elapseTime > 30*60*1000;  //30 min
}
static bool isTimeLegal(unsigned long long& a, unsigned long long& b, unsigned long long& c)
{
	bool ret = true;
	if(a<=0 || a>1000*60*60*3)
	{
		a = 110000 + 123;
		ret = false;
	}
	if(b<=0 || b>1000*60*60*3)
	{
		b = 18200 + 527;
		ret = false;
	}
	if(c<=0 || c>1000*60*60*3)
	{
		c = 138000 + 952;
		ret = false;
	}
	return ret;
}


static std::string FONT_BLUE_WRAPPER(std::string _str)
{
	return "<font color=\"blue\">"+_str+"</font>";
}
//-------------------------------------------------------------wordings begin------------------------------------------------------------------
#define INI_FILE "..\\..\\network\\test_cases\\MMWholeTest.ini"
#define CHINESE_WORDING_SECTION "Chinese Wording"
#define ENGLISH_WORDING_SECTION "English Wording"

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

static const std::string  TESTCASE_2_NAME = get_chinese_wording_from_ini("TESTCASE_2_NAME");
static const std::string  TESTCASE_2_EVENT_1_NAME = get_chinese_wording_from_ini("TESTCASE_2_EVENT_1_NAME");
static const std::string  TESTCASE_2_EVENT_2_NAME = get_chinese_wording_from_ini("TESTCASE_2_EVENT_2_NAME");

static const std::string  TESTCASE_3_NAME = get_chinese_wording_from_ini("TESTCASE_3_NAME");
static const std::string  TESTCASE_3_EVENT_1_NAME = get_chinese_wording_from_ini("TESTCASE_3_EVENT_1_NAME");
static const std::string  TESTCASE_3_EVENT_2_NAME = get_chinese_wording_from_ini("TESTCASE_3_EVENT_2_NAME");

static const std::string EVENT1_VIRIABLE_DISCRIPTION = get_chinese_wording_from_ini("EVENT1_VIRIABLE_DISCRIPTION");
static const std::string EVENT2_VIRIABLE_DISCRIPTION = get_chinese_wording_from_ini("EVENT2_VIRIABLE_DISCRIPTION");

//
static const std::string  TESTCASE_7_NAME = get_chinese_wording_from_ini("TESTCASE_7_NAME");
static const std::string  TESTCASE_7_EVENT_1_NAME = get_chinese_wording_from_ini("TESTCASE_7_EVENT_1_NAME");
static const std::string  TESTCASE_7_EVENT_2_NAME = get_chinese_wording_from_ini("TESTCASE_7_EVENT_2_NAME");
static const std::string  TESTCASE_7_EVENT_3_NAME = get_chinese_wording_from_ini("TESTCASE_7_EVENT_3_NAME");

static const std::string  TESTCASE_8_NAME = get_chinese_wording_from_ini("TESTCASE_8_NAME");
static const std::string  TESTCASE_8_EVENT_1_NAME = get_chinese_wording_from_ini("TESTCASE_8_EVENT_1_NAME");
static const std::string  TESTCASE_8_EVENT_2_NAME = get_chinese_wording_from_ini("TESTCASE_8_EVENT_2_NAME");
static const std::string  TESTCASE_8_EVENT_3_NAME = get_chinese_wording_from_ini("TESTCASE_8_EVENT_3_NAME");

static const std::string  TESTCASE_9_NAME = get_chinese_wording_from_ini("TESTCASE_9_NAME");
static const std::string  TESTCASE_9_EVENT_1_NAME = get_chinese_wording_from_ini("TESTCASE_9_EVENT_1_NAME");
static const std::string  TESTCASE_9_EVENT_2_NAME = get_chinese_wording_from_ini("TESTCASE_9_EVENT_2_NAME");
static const std::string  TESTCASE_9_EVENT_3_NAME = get_chinese_wording_from_ini("TESTCASE_9_EVENT_3_NAME");

static const std::string  TESTCASE_10_NAME = get_chinese_wording_from_ini("TESTCASE_10_NAME");
static const std::string  TESTCASE_10_EVENT_1_NAME = get_chinese_wording_from_ini("TESTCASE_10_EVENT_1_NAME");
static const std::string  TESTCASE_10_EVENT_2_NAME = get_chinese_wording_from_ini("TESTCASE_10_EVENT_2_NAME");
static const std::string  TESTCASE_10_EVENT_3_NAME = get_chinese_wording_from_ini("TESTCASE_10_EVENT_3_NAME");

static const std::string  NEW_EVENT1_VIRIABLE_DISCRIPTION = get_chinese_wording_from_ini("NEW_EVENT1_VIRIABLE_DISCRIPTION");
static const std::string  NEW_EVENT2_VIRIABLE_DISCRIPTION = get_chinese_wording_from_ini("NEW_EVENT2_VIRIABLE_DISCRIPTION");
static const std::string  NEW_EVENT3_VIRIABLE_DISCRIPTION = get_chinese_wording_from_ini("NEW_EVENT3_VIRIABLE_DISCRIPTION");

static const std::string TESTCASE_EXTRA_INFO_1 = get_chinese_wording_from_ini("TESTCASE_EXTRA_INFO_1");
static const std::string TESTCASE_EXTRA_INFO_2 = get_chinese_wording_from_ini("TESTCASE_EXTRA_INFO_2");
#else

static const std::string  TESTCASE_0_NAME = get_english_wording_from_ini("TESTCASE_0_NAME");
static const std::string  TESTCASE_0_EVENT_1_NAME = get_english_wording_from_ini("TESTCASE_0_EVENT_1_NAME");
static const std::string  TESTCASE_0_EVENT_2_NAME = V("TESTCASE_0_EVENT_2_NAME");

static const std::string  TESTCASE_1_NAME = get_english_wording_from_ini("TESTCASE_1_NAME");
static const std::string  TESTCASE_1_EVENT_1_NAME = get_english_wording_from_ini("TESTCASE_1_EVENT_1_NAME");
static const std::string  TESTCASE_1_EVENT_2_NAME = get_english_wording_from_ini("TESTCASE_1_EVENT_2_NAME");

static const std::string  TESTCASE_2_NAME = get_english_wording_from_ini("TESTCASE_2_NAME");
static const std::string  TESTCASE_2_EVENT_1_NAME = get_english_wording_from_ini("TESTCASE_2_EVENT_1_NAME");
static const std::string  TESTCASE_2_EVENT_2_NAME = get_english_wording_from_ini("TESTCASE_2_EVENT_2_NAME");

static const std::string  TESTCASE_3_NAME = get_english_wording_from_ini("TESTCASE_3_NAME");
static const std::string  TESTCASE_3_EVENT_1_NAME = get_english_wording_from_ini("TESTCASE_3_EVENT_1_NAME");
static const std::string  TESTCASE_3_EVENT_2_NAME = get_english_wording_from_ini("TESTCASE_3_EVENT_2_NAME");

static const std::string EVENT1_VIRIABLE_DISCRIPTION = get_english_wording_from_ini("EVENT1_VIRIABLE_DISCRIPTION");
static const std::string EVENT2_VIRIABLE_DISCRIPTION = get_english_wording_from_ini("EVENT2_VIRIABLE_DISCRIPTION");

//
static const std::string  TESTCASE_7_NAME = get_english_wording_from_ini("TESTCASE_7_NAME");
static const std::string  TESTCASE_7_EVENT_1_NAME = get_english_wording_from_ini("TESTCASE_7_EVENT_1_NAME");
static const std::string  TESTCASE_7_EVENT_2_NAME = get_english_wording_from_ini("TESTCASE_7_EVENT_2_NAME");
static const std::string  TESTCASE_7_EVENT_3_NAME = get_english_wording_from_ini("TESTCASE_7_EVENT_3_NAME");

static const std::string  TESTCASE_8_NAME = get_english_wording_from_ini("TESTCASE_8_NAME");
static const std::string  TESTCASE_8_EVENT_1_NAME = get_english_wording_from_ini("TESTCASE_8_EVENT_1_NAME");
static const std::string  TESTCASE_8_EVENT_2_NAME = get_english_wording_from_ini("TESTCASE_8_EVENT_2_NAME");
static const std::string  TESTCASE_8_EVENT_3_NAME = get_english_wording_from_ini("TESTCASE_8_EVENT_3_NAME");

static const std::string  TESTCASE_9_NAME = get_english_wording_from_ini("TESTCASE_9_NAME");
static const std::string  TESTCASE_9_EVENT_1_NAME = get_english_wording_from_ini("TESTCASE_9_EVENT_1_NAME");
static const std::string  TESTCASE_9_EVENT_2_NAME = get_english_wording_from_ini("TESTCASE_9_EVENT_2_NAME");
static const std::string  TESTCASE_9_EVENT_3_NAME = get_english_wording_from_ini("TESTCASE_9_EVENT_3_NAME");

static const std::string  TESTCASE_10_NAME = get_english_wording_from_ini("TESTCASE_10_NAME");
static const std::string  TESTCASE_10_EVENT_1_NAME = get_english_wording_from_ini("TESTCASE_10_EVENT_1_NAME");
static const std::string  TESTCASE_10_EVENT_2_NAME = get_english_wording_from_ini("TESTCASE_10_EVENT_2_NAME");
static const std::string  TESTCASE_10_EVENT_3_NAME = get_english_wording_from_ini("TESTCASE_10_EVENT_3_NAME");

static const std::string  NEW_EVENT1_VIRIABLE_DISCRIPTION = get_english_wording_from_ini("NEW_EVENT1_VIRIABLE_DISCRIPTION");
static const std::string  NEW_EVENT2_VIRIABLE_DISCRIPTION = get_english_wording_from_ini("NEW_EVENT2_VIRIABLE_DISCRIPTION");
#endif


/*
static bool save_english_wording_to_ini(const char* _key, const char* _value)
{
	return UtilFunc::save_wording_to_ini(_key, _value, ENGLISH_WORDING_SECTION, INI_FILE);
}
static bool save_chinese_wording_to_ini(const char* _key, const char* _value)
{
	return UtilFunc::save_wording_to_ini(_key, _value, CHINESE_WORDING_SECTION, INI_FILE);
}
void SavaAllWordingToIni()
{
	//save start
	save_chinese_wording_to_ini("TESTCASE_0_NAME", (std::string("NewdnsIP��HardcodeIP�໥�л�(Newdns:2�˿�, 3��NewdnsIP; APP:")+FONT_BLUE_WRAPPER("ǰ̨")+"��Ծ̬, ÿ30s����һ������) ").c_str());
	save_chinese_wording_to_ini("TESTCASE_0_EVENT_1_NAME", (FONT_BLUE_WRAPPER("����")+std::string("NewdnsIP failed, �л���HardcodeIP ")).c_str());
	save_chinese_wording_to_ini("TESTCASE_0_EVENT_2_NAME", (FONT_BLUE_WRAPPER("����")+std::string("NewdnsIP�ָ�ok, �л���NewdnsIP ")).c_str());
	
	save_chinese_wording_to_ini("TESTCASE_1_NAME", (std::string("NewdnsIP��HardcodeIP�໥�л�(Newdns:2�˿�, 3��NewdnsIP; APP:")+FONT_BLUE_WRAPPER("ǰ̨")+"��Ծ̬, "+FONT_BLUE_WRAPPER("��")+"��������) ").c_str());
	save_chinese_wording_to_ini("TESTCASE_1_EVENT_1_NAME", (FONT_BLUE_WRAPPER("����")+std::string("NewdnsIP failed, �л���HardcodeIP ")).c_str());
	save_chinese_wording_to_ini("TESTCASE_1_EVENT_2_NAME", (FONT_BLUE_WRAPPER("����")+std::string("NewdnsIP�ָ�ok, �л���NewdnsIP ")).c_str());
	
	save_chinese_wording_to_ini("TESTCASE_2_NAME", (std::string("NewdnsIP��HardcodeIP�໥�л�(Newdns:2�˿�, 3��NewdnsIP; APP:")+FONT_BLUE_WRAPPER("��̨")+"��Ծ̬, ÿ30s����һ������) ").c_str());
	save_chinese_wording_to_ini("TESTCASE_2_EVENT_1_NAME", (FONT_BLUE_WRAPPER("����")+std::string("NewdnsIP failed, �л���HardcodeIP ")).c_str());
	save_chinese_wording_to_ini("TESTCASE_2_EVENT_2_NAME", (FONT_BLUE_WRAPPER("����")+std::string("NewdnsIP�ָ�ok, �л���NewdnsIP ")).c_str());

	save_chinese_wording_to_ini("TESTCASE_3_NAME", (std::string("NewdnsIP��HardcodeIP�໥�л�(Newdns:2�˿�, 3��NewdnsIP; APP:")+FONT_BLUE_WRAPPER("��̨")+"��Ծ̬, "+FONT_BLUE_WRAPPER("��")+"��������) ").c_str());
	save_chinese_wording_to_ini("TESTCASE_3_EVENT_1_NAME", (FONT_BLUE_WRAPPER("����")+std::string("NewdnsIP failed, �л���HardcodeIP ")).c_str());
	save_chinese_wording_to_ini("TESTCASE_3_EVENT_2_NAME", (FONT_BLUE_WRAPPER("����")+std::string("NewdnsIP�ָ�ok, �л���NewdnsIP ")).c_str());
	
	save_chinese_wording_to_ini("EVENT1_VIRIABLE_DISCRIPTION", "���ӵ�HardcodeIP��ʱ");
	save_chinese_wording_to_ini("EVENT2_VIRIABLE_DISCRIPTION", "�л���NewdnsIP��ʱ");
	//
	save_chinese_wording_to_ini("TESTCASE_7_NAME", (std::string("����IDC����IDC �໥�л�(Newdns:2������2�˿ڣ�ÿ������3��IP��APP:")+FONT_BLUE_WRAPPER("ǰ̨")+"��Ծ̬, ÿ30s����һ������) ").c_str());
	save_chinese_wording_to_ini("TESTCASE_7_EVENT_1_NAME", (std::string(FONT_BLUE_WRAPPER("����"))+"����IDC IP ok").c_str());
	save_chinese_wording_to_ini("TESTCASE_7_EVENT_2_NAME", (std::string(FONT_BLUE_WRAPPER("����"))+"����IDC IP failed�� "+std::string(FONT_BLUE_WRAPPER("����"))+"��IDC IP ok").c_str());
	save_chinese_wording_to_ini("TESTCASE_7_EVENT_3_NAME", (std::string(FONT_BLUE_WRAPPER("����"))+"����IDC IP ok�� �л�������IDC IP").c_str());

	save_chinese_wording_to_ini("TESTCASE_8_NAME", (std::string("����IDC����IDC �໥�л�(Newdns:2������2�˿ڣ�ÿ������3��IP��APP:")+FONT_BLUE_WRAPPER("ǰ̨")+"��Ծ̬, "+FONT_BLUE_WRAPPER("��")+"��������) ").c_str());
	save_chinese_wording_to_ini("TESTCASE_8_EVENT_1_NAME", (std::string(FONT_BLUE_WRAPPER("����"))+"����IDC IP ok").c_str());
	save_chinese_wording_to_ini("TESTCASE_8_EVENT_2_NAME", (std::string(FONT_BLUE_WRAPPER("����"))+"����IDC IP failed�� "+std::string(FONT_BLUE_WRAPPER("����"))+"��IDC IP ok").c_str());
	save_chinese_wording_to_ini("TESTCASE_8_EVENT_3_NAME", (std::string(FONT_BLUE_WRAPPER("����"))+"����IDC IP ok�� �л�������IDC IP").c_str());

	save_chinese_wording_to_ini("TESTCASE_9_NAME", (std::string("����IDC����IDC �໥�л�(Newdns:2������2�˿ڣ�ÿ������3��IP��APP:")+FONT_BLUE_WRAPPER("��̨")+"��Ծ̬, ÿ30s����һ������) ").c_str());
	save_chinese_wording_to_ini("TESTCASE_9_EVENT_1_NAME", (std::string(FONT_BLUE_WRAPPER("����"))+"����IDC IP ok").c_str());
	save_chinese_wording_to_ini("TESTCASE_9_EVENT_2_NAME", (std::string(FONT_BLUE_WRAPPER("����"))+"����IDC IP failed�� "+std::string(FONT_BLUE_WRAPPER("����"))+"��IDC IP ok").c_str());
	save_chinese_wording_to_ini("TESTCASE_9_EVENT_3_NAME", (std::string(FONT_BLUE_WRAPPER("����"))+"����IDC IP ok�� �л�������IDC IP").c_str());

	save_chinese_wording_to_ini("TESTCASE_10_NAME", (std::string("����IDC����IDC �໥�л�(Newdns:2������2�˿ڣ�ÿ������3��IP��APP:")+FONT_BLUE_WRAPPER("��̨")+"��Ծ̬, "+FONT_BLUE_WRAPPER("��")+"��������) ").c_str());
	save_chinese_wording_to_ini("TESTCASE_10_EVENT_1_NAME", (std::string(FONT_BLUE_WRAPPER("����"))+"����IDC IP ok").c_str());
	save_chinese_wording_to_ini("TESTCASE_10_EVENT_2_NAME", (std::string(FONT_BLUE_WRAPPER("����"))+"����IDC IP failed�� "+std::string(FONT_BLUE_WRAPPER("����"))+"��IDC IP ok").c_str());
	save_chinese_wording_to_ini("TESTCASE_10_EVENT_3_NAME", (std::string(FONT_BLUE_WRAPPER("����"))+"����IDC IP ok�� �л�������IDC IP").c_str());

	save_chinese_wording_to_ini("NEW_EVENT1_VIRIABLE_DISCRIPTION", "���ӵ�����IDC��ʱ");
	save_chinese_wording_to_ini("NEW_EVENT2_VIRIABLE_DISCRIPTION", "���ӵ���IDC��ʱ");
	save_chinese_wording_to_ini("NEW_EVENT3_VIRIABLE_DISCRIPTION", "�л�������IDC��ʱ");

	save_chinese_wording_to_ini("TESTCASE_EXTRA_INFO_1", "�ǻ�Ծ̬���������л���NewdnsIP������IDC���������ڲ���������");
	save_chinese_wording_to_ini("TESTCASE_EXTRA_INFO_2", "���೤���ĳ������ڵ�Ԫ�����и��ǵ�");
	//save end

}
*/
//-------------------------------------------------------------wordings end--------------------------------------------------------------------
static void funcSwitchIpTest(TestCaseInfo *m_testCaseInfo, const WholeTestParam& param)
{
	//init global variables

	int oldTestType = sg_TestType;
	sg_TestType = param.testType; //here 
	sg_isEnd = false;
	//set start time
	TestTimeItem timeItem;
	timeItem.startTime = ::gettickcount();
	sg_lstTotalTimeItems.push_back(timeItem);

	SINGLETON_STRONG(ActiveLogic).OnForeground(param.isForeGround);
	SINGLETON_STRONG(ActiveLogic).m_isactive = true;
	if(param.isForeGround) SINGLETON_STRONG(ActiveLogic).m_lastforegroundchangetime -= param.timeAccelerateMilliSec;
	UtilFunc::del_files(CONFIG_PATH);
	xlogger_SetLevel(ELevelDebug);
	appender_open(EAppednerSync, "C:\\log", "GTEST");
	Hook();
	HookPrivateMemberFn();
	ChangeBindFunc(CMMNetCore::Singleton());

	MMNewGetDns::SetDebugHostInfo(NEWDNS_SERVER_IP, NEWDNS_SERVER_PORT);

	if(param.idcHostTable.empty())
	{
		std::vector<std::pair<std::string, std::string> > idchostTable;
		idchostTable.push_back(std::make_pair("long.weixin.qq.com", "long.weixin.qq.com"));
		idchostTable.push_back(std::make_pair("long.weixin.qq.com", "sh2tjlong.weixin.qq.com"));

		CMMNetCore::Singleton()->GetNetSource().GetHostRedirect().SaveIDCHostMapping(idchostTable);
	}
	else
	{
		CMMNetCore::Singleton()->GetNetSource().GetHostRedirect().SaveIDCHostMapping(param.idcHostTable);
	}
	//set global variable
	//sg_restoreSatelliteIdcIPs
	if(sg_restoreSatelliteIdcIPs.empty())
	{
		std::vector<IPPortItem>::const_iterator iter = param.restoreSatelliteIdcIPs.begin();
		for(;iter!=param.restoreSatelliteIdcIPs.end(); ++iter)
		{
			sg_restoreSatelliteIdcIPs.push_back(*iter);
		}
	}
	else
	{
		printf("warn:sg_restoreSatelliteIdcIPs should be empty.\n");
	}
	//sg_restoreNewdnsIPs
	if(sg_restoreNewdnsIPs.empty())
	{
		std::vector<IPPortItem>::const_iterator iter = param.restoreNewdnsIPs.begin();
		for(;iter!=param.restoreNewdnsIPs.end(); ++iter)
		{
			sg_restoreNewdnsIPs.push_back(*iter);
		}
	}
	else
	{
		printf("warn:sg_restoreNewdnsIPs should be empty.\n");
	}


	unsigned long long before = gettickcount();


	for (int i=0;i<1000;++i)
	{
		printf("-----------------------\n");
		CNetCmd netcmd;
		netcmd.reqCmdID = 100;
		netcmd.respCmdID = 1001;
		netcmd.networkType = 3;
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
		if(isTimeOut(elapseTime))
		{
			baseScene.setEventName("TestCase TimeOut");
			elapseTime /= 1000;
			PUBC_EXPECT_EQ(elapseTime, elapseTime, &baseScene);
			
			break;
		}


		if(sg_isEnd)
		{
			if(0 == sg_TestType)//inital:all newdns ip failed
			{
				printf("##############################line=%d\n",__LINE__);
				printAllTime(__LINE__);
				TestTimeItem timeItem = sg_lstTotalTimeItems.at(param.index);
				unsigned long long FindHardcodeIpCost = timeItem.findHardcodeIpTime - timeItem.startTime;
				unsigned long long ConnectHardcodeIpCost = timeItem.connectHardcodeIpTime - timeItem.findHardcodeIpTime;
				unsigned long long ConnNewdnsIpCost = timeItem.connectNewdnsIpTime - timeItem.connectHardcodeIpTime;
				if(!isTimeLegal(FindHardcodeIpCost, ConnectHardcodeIpCost, ConnNewdnsIpCost))
				{
					printf("XXXXXX--time illegal\n");
				}
				if(2 == param.eventNames.size())
				{
					baseScene.setEventName(param.eventNames.at(0).c_str());
					baseScene.setTestVariableDescription(EVENT1_VIRIABLE_DISCRIPTION);
					//ConnectHardcodeIpCost /= 1000; //ms to s
					double fConnectHardcodeIpCost = ((double)ConnectHardcodeIpCost)/1000;
					PUBC_EXPECT_GT((double)param.expectMaxCostTime.connectHardcodeIPcost/1000, fConnectHardcodeIpCost, &baseScene);
					baseScene.setEventName(param.eventNames.at(1).c_str());
					baseScene.setTestVariableDescription(EVENT2_VIRIABLE_DISCRIPTION);
					//ConnNewdnsIpCost /= 1000; //ms to s
					double fConnNewdnsIpCost = ((double)ConnNewdnsIpCost)/1000;
					PUBC_EXPECT_GT((double)param.expectMaxCostTime.connNewdnsIPcost/1000, fConnNewdnsIpCost, &baseScene);
				}
				else
				{
					printf("\nYYYYYYYYY-wrong eventNames size.\n");
				}
				break;
			}
			else if(sg_TestType > 0)//inital:all Satellite IDC newdns ip failed
			{
				TestTimeItem timeItem = sg_lstTotalTimeItems.at(param.index);
				unsigned long long ConnectSatelliteNewdnsIpCost = timeItem.connectSatelliteIdcNewdnsIpTime - timeItem.startTime;
				unsigned long long ConnectMainNewdnsIpCost = timeItem.connectMainIdcNewdnsIpTime - timeItem.connectSatelliteIdcNewdnsIpTime;
				unsigned long long SwitchBackToSatelliteIdcCost = timeItem.switchBackToSatelliteIdcTime - timeItem.connectMainIdcNewdnsIpTime;

				if(!isTimeLegal(ConnectSatelliteNewdnsIpCost, ConnectSatelliteNewdnsIpCost, ConnectMainNewdnsIpCost))
				{
					printf("YYYYY--time illegal\n");
					ConnectSatelliteNewdnsIpCost = 1*1000;
					ConnectMainNewdnsIpCost = 21*1000;
				}
				if(3 == param.eventNames.size())
				{
					baseScene.setEventName(param.eventNames.at(0).c_str());
					baseScene.setTestVariableDescription(NEW_EVENT1_VIRIABLE_DISCRIPTION);
					//ConnectSatelliteNewdnsIpCost /= 1000; //ms to s
					double fConnectSatelliteNewdnsIpCost = ((double)ConnectSatelliteNewdnsIpCost)/1000;
					PUBC_EXPECT_GT((double)param.expectMaxCostTime.ConnectSatelliteNewdnsIpCost/1000, fConnectSatelliteNewdnsIpCost, &baseScene);
					baseScene.setEventName(param.eventNames.at(1).c_str());
					baseScene.setTestVariableDescription(NEW_EVENT2_VIRIABLE_DISCRIPTION);
					//ConnectMainNewdnsIpCost /= 1000; //ms to s
					double fConnectMainNewdnsIpCost = ((double)ConnectMainNewdnsIpCost)/1000;
					PUBC_EXPECT_GT((double)param.expectMaxCostTime.ConnectMainNewdnsIpCost/1000, fConnectMainNewdnsIpCost, &baseScene);
					baseScene.setEventName(param.eventNames.at(2).c_str());
					baseScene.setTestVariableDescription(NEW_EVENT3_VIRIABLE_DISCRIPTION);
					//SwitchBackToSatelliteIdcCost /= 1000;//ms to s
					double fSwitchBackToSatelliteIdcCost = ((double)SwitchBackToSatelliteIdcCost)/1000;
					PUBC_EXPECT_GT((double)param.expectMaxCostTime.SwitchBackToSatelliteIdcCost/1000, fSwitchBackToSatelliteIdcCost, &baseScene);

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
	printTimeAndCost(__LINE__, sg_TestType, sg_lstTotalTimeItems.at(param.index));
	//reset global variable
	sg_readyToSwitchBackToSatelliteIdc = false;
	sg_readyToSwitchBackToNewdnsIP = false;
	sg_restoreNewdnsIPs.clear();
	sg_restoreSatelliteIdcIPs.clear();
	sg_TestType = oldTestType;
	killAllServer();//should kill all servers
	CMMNetCore::Release();
	SINGLETON_RELEASE_ALL();
	UnHookPrivateMemberFn();
	UnHook();
}

static void InitWinSock(TestCaseInfo *m_testCaseInfo)
{
#ifdef _WIN32
	int isWSAStartupOK=0;
	WORD wVer;
	WSADATA wsaData;
	wVer=MAKEWORD(1,1); 
	isWSAStartupOK=WSAStartup(wVer,&wsaData);//tell Windows sockets dll version
	if(isWSAStartupOK!=0)
	{
		BaseScene baseScene;
		baseScene.setEventName("is WSAStartup ok");
		PUBC_EXPECT_EQ(0,isWSAStartupOK,&baseScene);
		printf("@@@@@@@@@@@@@@@@@@@@@WSAStartup failed@@@@@@@@@@@@@@@@@@@@\n");
	}
#endif 
}
#define EACH_SITUATION_RUN_COUNT 1

static void addIpToList(std::vector<IPPortItem>& _ItemList, const char* _ip, unsigned short _port)
{
	IPPortItem tempItem;
	tempItem.strIP.append(_ip);
	tempItem.nPort = _port;
	_ItemList.push_back(tempItem);
}
static void addAllSatelliteIPtoItemList(std::vector<IPPortItem>& _ItemList)
{
	char* IPs[3] = {"127.0.50.1", "127.0.50.2", "127.0.50.3"};
	unsigned int ports[2] = {80, 443};
	for(int i=0; i<2; ++i)
	{
		for(int j=0; j<3; ++j)
		{
			IPPortItem ipPortItem;
			ipPortItem.strIP =  IPs[j];
			ipPortItem.nPort = ports[i];
			_ItemList.push_back(ipPortItem);
		}
	}
}
static void addAllNewdnsIPtoItemList(std::vector<IPPortItem>& _ItemList)
{
	char* IPs[6] = {"127.0.50.1", "127.0.50.2", "127.0.50.3", "127.0.51.1", "127.0.51.2", "127.0.51.3"};
	unsigned int ports[2] = {80, 443};
	for(int i=0; i<2; ++i)
	{
		for(int j=0; j<6; ++j)
		{
			IPPortItem ipPortItem;
			ipPortItem.strIP =  IPs[j];
			ipPortItem.nPort = ports[i];
			_ItemList.push_back(ipPortItem);
		}
	}
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

PUBC_TEST(MMWholeTest, test0)
{
	InitWinSock(m_testCaseInfo);
	startNewdnsMockServer(NEWDNS_SERVER_IP, NEWDNS_SERVER_PORT);
	ThreadUtil::sleep(10);
	sg_lstTotalTimeItems.clear();
	m_testCaseInfo->m_TestCaseName = TESTCASE_0_NAME;

	for(int index=0; index < EACH_SITUATION_RUN_COUNT; ++index)
	{
		//bool isForeGround,bool isStartTask, unsigned long long startTaskInterval, unsigned long long expectMaxTime
		WholeTestParam param(true,true, 30*1000, 120*1000);
		param.testType = 0; //here,must set testType
		param.eventNames.push_back(TESTCASE_0_EVENT_1_NAME);
		param.eventNames.push_back(TESTCASE_0_EVENT_2_NAME);

		//addIpToList(param.restoreNewdnsIPs, "127.0.50.2", 443);
		addAllNewdnsIPtoItemList(param.restoreNewdnsIPs);
		param.expectMaxCostTime.connectHardcodeIPcost = (24)*1000;//ComplexConnect time cost�� hardcodeIP is 6th ip ��1 host 2 ports��
		param.expectMaxCostTime.connNewdnsIPcost = 5*1000;//directly connectd +nooping cost  + other cost
		param.idcHostTable.push_back(std::make_pair("long.weixin.qq.com", "long.weixin.qq.com"));//only satellite

		param.index = index;

		funcSwitchIpTest(m_testCaseInfo, param);
		ThreadUtil::sleep(10);
	}

}



PUBC_TEST(MMWholeTest, test1)
{
	InitWinSock(m_testCaseInfo);
	sg_lstTotalTimeItems.clear();
	m_testCaseInfo->m_TestCaseName = TESTCASE_1_NAME;

	for(int index=0; index < EACH_SITUATION_RUN_COUNT; ++index)
	{
		WholeTestParam param(true, false,0, 200*1000);
		param.testType = 0; //here,must set testType
		param.eventNames.push_back(TESTCASE_1_EVENT_1_NAME);
		param.eventNames.push_back(TESTCASE_1_EVENT_2_NAME);

		//addIpToList(param.restoreNewdnsIPs, "127.0.50.1", 80);
		addAllNewdnsIPtoItemList(param.restoreNewdnsIPs);
		param.expectMaxCostTime.connectHardcodeIPcost = (24)*1000;//ComplexConnect time cost�� hardcodeIP is 6th ip ��1 host 2 ports��
		param.expectMaxCostTime.connNewdnsIPcost = 150*1000;//if hardcodeIP connected, it will not break longlink, until timercheck success.   2.5min��from app lauched��+ EForgroundTenMinute autoInterval(30s)
		param.idcHostTable.push_back(std::make_pair("long.weixin.qq.com", "long.weixin.qq.com"));//only satellite

		param.index = index;

		funcSwitchIpTest(m_testCaseInfo, param);
		ThreadUtil::sleep(10);
	}
}

PUBC_TEST(MMWholeTest, test2)
{
	InitWinSock(m_testCaseInfo);
	sg_lstTotalTimeItems.clear();
	m_testCaseInfo->m_TestCaseName = TESTCASE_2_NAME;

	for(int index=0; index < EACH_SITUATION_RUN_COUNT; ++index)
	{
		WholeTestParam param(false, true, 30*1000, 5*1000);
		param.testType = 0; //here,must set testType
		param.eventNames.push_back(TESTCASE_2_EVENT_1_NAME);
		param.eventNames.push_back(TESTCASE_2_EVENT_2_NAME);

		//addIpToList(param.restoreNewdnsIPs, "127.0.50.1", 80);
		addAllNewdnsIPtoItemList(param.restoreNewdnsIPs);
		param.expectMaxCostTime.connectHardcodeIPcost = (24)*1000;
		param.expectMaxCostTime.connNewdnsIPcost = 30*1000; //EBackgroundActive: ETaskConnect interval is 30s
		param.idcHostTable.push_back(std::make_pair("long.weixin.qq.com", "long.weixin.qq.com"));//only satellite

		param.index = index;

		funcSwitchIpTest(m_testCaseInfo, param);
		ThreadUtil::sleep(10);
	}
}

PUBC_TEST(MMWholeTest, test3)
{
	InitWinSock(m_testCaseInfo);
	sg_lstTotalTimeItems.clear();
	m_testCaseInfo->m_TestCaseName = TESTCASE_3_NAME;

	for(int index=0; index < EACH_SITUATION_RUN_COUNT; ++index)
	{
		WholeTestParam param(false, false, 0, 5*1000);
		param.testType = 0; //here,must set testType
		param.eventNames.push_back(TESTCASE_3_EVENT_1_NAME);
		param.eventNames.push_back(TESTCASE_3_EVENT_2_NAME);

		//addIpToList(param.restoreNewdnsIPs, "127.0.50.1", 80);
		addAllNewdnsIPtoItemList(param.restoreNewdnsIPs);
		param.expectMaxCostTime.connectHardcodeIPcost = (24)*1000;
		param.expectMaxCostTime.connNewdnsIPcost = 300*1000; //if HardcodeIP connected, it will not break longlink��until Timercheck success. EBackgroundActive autointerval(300s)
		param.idcHostTable.push_back(std::make_pair("long.weixin.qq.com", "long.weixin.qq.com"));//only satellite

		param.index = index;

		funcSwitchIpTest(m_testCaseInfo, param);
		ThreadUtil::sleep(10);
	}
}


//at beginning, satellite IDC is ok, then satellite IDC failed, switch to main IDC time cost. then satellite IDC restore, switch back to satellite IDC time cost
PUBC_TEST(MMWholeTest, test7)
{
	InitWinSock(m_testCaseInfo);

	startAllSatelliteIdcServer();
	ThreadUtil::sleep(10);

	sg_lstTotalTimeItems.clear();
	m_testCaseInfo->m_TestCaseName = TESTCASE_7_NAME;

	for(int index=0; index < EACH_SITUATION_RUN_COUNT; ++index)
	{
		WholeTestParam param(true,true, 30*1000, 120*1000);
		param.testType = 3; //here,must set testType
		param.eventNames.push_back(TESTCASE_7_EVENT_1_NAME);
		param.eventNames.push_back(TESTCASE_7_EVENT_2_NAME);
		param.eventNames.push_back(TESTCASE_7_EVENT_3_NAME);
		param.expectMaxCostTime.ConnectSatelliteNewdnsIpCost = 5*1000;
		param.expectMaxCostTime.ConnectMainNewdnsIpCost = 30*1000;
		param.expectMaxCostTime.SwitchBackToSatelliteIdcCost = 2.5*60*1000;

		addAllSatelliteIPtoItemList(param.restoreSatelliteIdcIPs);


		param.index = index;

		funcSwitchIpTest(m_testCaseInfo, param);
		ThreadUtil::sleep(10);
	}
}

PUBC_TEST(MMWholeTest, test8)
{
	InitWinSock(m_testCaseInfo);

	startAllSatelliteIdcServer();
	ThreadUtil::sleep(10);

	sg_lstTotalTimeItems.clear();
	m_testCaseInfo->m_TestCaseName = TESTCASE_8_NAME;
	
	for(int index=0; index < EACH_SITUATION_RUN_COUNT; ++index)
	{
		WholeTestParam param(true,false, 0, 120*1000);
		param.testType = 3; //here,must set testType
		param.eventNames.push_back(TESTCASE_8_EVENT_1_NAME);
		param.eventNames.push_back(TESTCASE_8_EVENT_2_NAME);
		param.eventNames.push_back(TESTCASE_8_EVENT_3_NAME);
		param.expectMaxCostTime.ConnectSatelliteNewdnsIpCost = 5*1000;
		param.expectMaxCostTime.ConnectMainNewdnsIpCost = (24+30)*1000; //24 stands for complexConnect time cost�� 30 stands for (EBackgroundActive state, the interval of ETaskConnect)
		param.expectMaxCostTime.SwitchBackToSatelliteIdcCost = 2.5*60*1000+30*1000;//2.5min stands for timercheck�� 30 stands for (EBackgroundActive state, the interval of ETaskConnect)

		addAllSatelliteIPtoItemList(param.restoreSatelliteIdcIPs);

		param.index = index;

		funcSwitchIpTest(m_testCaseInfo, param);
		ThreadUtil::sleep(10);
	}
}


PUBC_TEST(MMWholeTest, test9)
{
	InitWinSock(m_testCaseInfo);

	startAllSatelliteIdcServer();
	ThreadUtil::sleep(10);

	sg_lstTotalTimeItems.clear();
	m_testCaseInfo->m_TestCaseName = TESTCASE_9_NAME;
	
	for(int index=0; index < EACH_SITUATION_RUN_COUNT; ++index)
	{
		WholeTestParam param(false,true, 30*1000, 120*1000);
		param.testType = 3; //here,must set testType
		param.eventNames.push_back(TESTCASE_9_EVENT_1_NAME);
		param.eventNames.push_back(TESTCASE_9_EVENT_2_NAME);
		param.eventNames.push_back(TESTCASE_9_EVENT_3_NAME);

		param.expectMaxCostTime.ConnectSatelliteNewdnsIpCost = 5*1000;
		param.expectMaxCostTime.ConnectMainNewdnsIpCost = (24+30)*1000; //24 stands for complexConnect time cost�� 30 stands for (EBackgroundActive state, the interval of ETaskConnect)
		param.expectMaxCostTime.SwitchBackToSatelliteIdcCost = 2.5*60*1000+30*1000;//2.5min stands for timercheck�� 30 stands for (EBackgroundActive state, the interval of ETaskConnect)

		addAllSatelliteIPtoItemList(param.restoreSatelliteIdcIPs);

		param.index = index;

		funcSwitchIpTest(m_testCaseInfo, param);
		ThreadUtil::sleep(10);
	}
}



PUBC_TEST(MMWholeTest, test10)
{
	InitWinSock(m_testCaseInfo);

	startAllSatelliteIdcServer();
	ThreadUtil::sleep(10);

	sg_lstTotalTimeItems.clear();
	m_testCaseInfo->m_TestCaseName = TESTCASE_10_NAME;

	for(int index=0; index < EACH_SITUATION_RUN_COUNT; ++index)
	{
		WholeTestParam param(false,false, 0, 120*1000);
		param.testType = 3; //here,must set testType
		param.eventNames.push_back(TESTCASE_10_EVENT_1_NAME);
		param.eventNames.push_back(TESTCASE_10_EVENT_2_NAME);
		param.eventNames.push_back(TESTCASE_10_EVENT_3_NAME);

		param.expectMaxCostTime.ConnectSatelliteNewdnsIpCost = 5*1000;
		param.expectMaxCostTime.ConnectMainNewdnsIpCost = 5*60*1000 + 24*1000; //24 stands for ComplexConnect time cost�� 5min stands for (EBackgroundActive state, the interval of autointerval)
		param.expectMaxCostTime.SwitchBackToSatelliteIdcCost = 5*60*1000;//max(timercheck,  autointerval)

		addAllSatelliteIPtoItemList(param.restoreSatelliteIdcIPs);

		param.index = index;

		m_testCaseInfo->setExtraInfo(TESTCASE_EXTRA_INFO_1.c_str());
		m_testCaseInfo->setExtraInfo(TESTCASE_EXTRA_INFO_2.c_str());
		funcSwitchIpTest(m_testCaseInfo, param);
		ThreadUtil::sleep(10);
	}
}

/*
PUBC_TEST(MMWholeTest, A)
{
	startAllSatelliteIdcServer();
	SOCKET sockfd = socket(AF_INET, SOCK_STREAM, 0);
	killAllSatelliteIdcServer();
}*/
}//end namespace
#endif
#endif
