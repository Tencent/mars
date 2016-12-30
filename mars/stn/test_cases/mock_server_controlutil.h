// Tencent is pleased to support the open source community by making Mars available.
// Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

// Licensed under the MIT License (the "License"); you may not use this file except in 
// compliance with the License. You may obtain a copy of the License at
// http://opensource.org/licenses/MIT

// Unless required by applicable law or agreed to in writing, software distributed under the License is
// distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
// either express or implied. See the License for the specific language governing permissions and
// limitations under the License.

#ifndef __MOCK_SERVER_CONTROL_UTILS_H__
#define __MOCK_SERVER_CONTROL_UTILS_H__
#include <string>
#include <vector>
class ServerStatusItem
{
  public:
	std::string strIP;
	uint16_t nPort;
	bool isRunning;
	ServerStatusItem(const char* ip, uint16_t port, bool isRunning)
	{
		strIP =  ip;
		nPort = port;
		this->isRunning = isRunning;
	}
	ServerStatusItem(const char* ip, uint16_t port)
	{
		strIP =  ip;
		nPort = port;
		this->isRunning = false;
	}
};

static std::vector<ServerStatusItem>  sg_ServerStatusTable;
class MockServerControlUtils
{
	//startxxxϵ�к�������������server������status��־λ     killXXXϵ�к����ǹر�server���������־λ    stopXXXϵ�к����Ƿ���8888 TASK�� �������־λ
public:
static void setServerStatus(std::string _ip, uint16_t _port, bool& _isRunning)//_isRunning������������� ���Ϊserver��oldStatus
{
	bool oldStatus = false;
	std::vector<ServerStatusItem>::iterator iter = sg_ServerStatusTable.begin();
	for(;iter!=sg_ServerStatusTable.end();++iter)
	{
		if(iter->strIP==_ip && iter->nPort==_port)
		{
			oldStatus = iter->isRunning;
			iter->isRunning = _isRunning;
			break;
		}
	}

	if(iter == sg_ServerStatusTable.end())
	{
		ServerStatusItem tempItem(_ip.c_str(), _port, _isRunning);
		sg_ServerStatusTable.push_back(tempItem);
	}

	_isRunning = oldStatus;
}
//����һ��server������sg_ServerStatusTable���ö�Ӧ��־λ
static bool startNormalEcho(const IPPortItem& _ipPortItem)
{
	char tmpCmd[1024] = {0};
	snprintf(tmpCmd, sizeof(tmpCmd), START_NORMAL_ECHO_CMD, _ipPortItem.strIP.c_str(), _ipPortItem.nPort);
	bool isRunning = true;
	setServerStatus( _ipPortItem.strIP, _ipPortItem.nPort, isRunning);
	if(isRunning)
	{
		printf("server(%s:%u) is already running\n",_ipPortItem.strIP.c_str(), _ipPortItem.nPort);
		return true;
	}
	printf("line:%d,tempCmd=%s\n",__LINE__, tmpCmd);
	FILE* f = _popen(tmpCmd , "rt");
	if(NULL == f)   
	{
		printf("popen error!\n");
		return false;

	}
	return true;
}
//�ر�һ��server�� ����sg_ServerStatusTable�����Ӧ��־λ, ����ֵ��ʾ�Ƿ�رճɹ�
static bool killNormalEcho(const IPPortItem& _ipPortItem)
{
	char KILL_MSG[16] = "kill";
	bool isRunning = false;
	setServerStatus( _ipPortItem.strIP, _ipPortItem.nPort, isRunning);
	if(!isRunning)
	{
		printf("stopping server(%s:%u) is not running\n",_ipPortItem.strIP.c_str(), _ipPortItem.nPort);
		return true;
	}
	SOCKET sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	bool bRet = false;
	do 
	{
		if(sockfd<0)
		{
			isRunning = true;
			setServerStatus( _ipPortItem.strIP, _ipPortItem.nPort, isRunning);//û�������ص�  �ָ�statusΪ running
			printf("^^^^^^^^^^^^^close server(%s:%u) failed^^^^^<sockfd=%d, socket_errno:%d>^^^^^^^\n", _ipPortItem.strIP.c_str(), _ipPortItem.nPort, sockfd, WSAGetLastError());
			bRet = false;
			break;
		}
		struct sockaddr_in addr={0};
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr =inet_addr(_ipPortItem.strIP.c_str());
		addr.sin_port = (uint16_t)htons(_ipPortItem.nPort);

		int ret = connect(sockfd, (struct sockaddr *)&addr, sizeof(addr));
		if(ret < 0)
		{
			isRunning = true;
			setServerStatus( _ipPortItem.strIP, _ipPortItem.nPort, isRunning);//û�������ص�  �ָ�statusΪ running
			printf("close echo_nomal server error.\n");
			printf("^^^^^^^^^^^^^close server(%s:%u) failed^^^^^<connect failed>^^^^^^^\n", _ipPortItem.strIP.c_str(), _ipPortItem.nPort);
			bRet = false;
			break;
		}
		else
		{
			std::string closeMsg(KILL_MSG);
			int sendSize = send(sockfd, closeMsg.c_str(), closeMsg.size(), 0);
			if(sendSize != closeMsg.size())
			{
				isRunning = true;
				setServerStatus( _ipPortItem.strIP, _ipPortItem.nPort, isRunning);//û�������ص�  �ָ�statusΪ running
				printf("sendSize=%d, closeMsg.size=%d, is not equal!\n", sendSize, closeMsg.size());
				printf("^^^^^^^^^^^^^close server(%s:%u) failed^^^^^<send failed>^^^^^^^\n", _ipPortItem.strIP.c_str(), _ipPortItem.nPort);
				bRet = false;
				break;
			}
			else
			{
				printf("^^^^^^^^^^^^^close server(%s:%u) success^^^^^^^^^^^^\n", _ipPortItem.strIP.c_str(), _ipPortItem.nPort);
				bRet = true;
				break;
			}
		}
	} while (false);
	
	if(sockfd!=INVALID_SOCKET)
	{
		::closesocket(sockfd);
		sockfd = INVALID_SOCKET;
	}
	return bRet;
}
static int startNormalEchoServer(const char* ip)
{
	int succCount = 0;
	IPPortItem tempItem;
	tempItem.strIP = ip;
	tempItem.nPort = 80;
	if(startNormalEcho(tempItem))
		succCount++;
	tempItem.nPort = 443;
	if(startNormalEcho(tempItem))
		succCount++;

	return succCount;
}
static int killNormalEchoServer(const char* ip)
{
	int succCount = 0;
	IPPortItem tempItem;
	tempItem.strIP = ip;
	tempItem.nPort = 80;
	if(killNormalEcho(tempItem))
		succCount++;
	tempItem.nPort = 443;
	if(killNormalEcho(tempItem))
		succCount++;

	return succCount;
}
static int startNormalEchoServerList(const std::vector<IPPortItem>& _serverList)
{
	int succCount = 0;
	std::vector<IPPortItem>::const_iterator iter = _serverList.begin();
	for(; iter!=_serverList.end(); ++iter)
	{
		bool isSucc = startNormalEcho(*iter);
		if(isSucc)
			succCount++;
	}
	return succCount++;
}
static int killNormalEchoServerList(const std::vector<IPPortItem>& _serverList)
{
	int succCount = 0;
	std::vector<IPPortItem>::const_iterator iter = _serverList.begin();
	for(; iter!=_serverList.end(); ++iter)
	{
		bool isSucc = killNormalEcho(*iter);
		if(isSucc)
			succCount++;
	}
	return succCount++;
}
static void startAllMainIdcServer()
{
	char* IPs[3] = {"127.0.51.1", "127.0.51.2", "127.0.51.3"};
	unsigned int ports[2] = {80, 443};
	for(int i=0; i<2; ++i)
	{
		for(int j=0; j<3; ++j)
		{
			IPPortItem ipPortItem;
			ipPortItem.strIP =  IPs[j];
			ipPortItem.nPort = ports[i];
			startNormalEcho(ipPortItem);
		}
	}
}
static void killAllMainIdcServer()
{
	char* IPs[3] = {"127.0.51.1", "127.0.51.2", "127.0.51.3"};
	unsigned int ports[2] = {80, 443};
	for(int i=0; i<2; ++i)
	{
		for(int j=0; j<3; ++j)
		{
			IPPortItem ipPortItem;
			ipPortItem.strIP =  IPs[j];
			ipPortItem.nPort = ports[i];
			killNormalEcho(ipPortItem);
		}
	}
}
static void startAllSatelliteIdcServer()
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
			startNormalEcho(ipPortItem);
		}
	}
}
static void killAllSatelliteIdcServer()
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
			killNormalEcho(ipPortItem);
		}
	}
}
/*
static void startEchoServer(const IPPortItem& _ipPortItem)
{
	char tmpCmd[1024] = {0};
	snprintf(tmpCmd, sizeof(tmpCmd), ECHO_NOOPING_RESP_THREE, _ipPortItem.strIP.c_str(), _ipPortItem.nPort);
	printf("line:%d,tempCmd=%s, ip=%s\n",__LINE__, tmpCmd, _ipPortItem.strIP.c_str());
	FILE* f = _popen(tmpCmd , "rt");
	if(NULL == f)   
	{
		printf("popen error!\n");

	}

}
static void startEchoServer(const std::string& ip, unsigned int port)
{
	IPPortItem ipPortItem;
	ipPortItem.strIP =  ip;
	ipPortItem.nPort = port;
	startEchoServer(ipPortItem);
}
*/
static bool isAllServerClosed()
{
	bool isAllClosed = true;
	std::vector<ServerStatusItem>::iterator iter = sg_ServerStatusTable.begin();
	for(;iter!=sg_ServerStatusTable.end();++iter)
	{
		if(true == iter->isRunning)
		{
			isAllClosed = false;
			break;
		}
	}
	return isAllClosed;
}

//�ر�����server�������sg_ServerStatusTable
static bool killAllServer()
{
	bool isStopAllServerSucc = true;
	std::vector<ServerStatusItem>::iterator iter = sg_ServerStatusTable.begin();
	for(;iter!=sg_ServerStatusTable.end();++iter)
	{
		if(true == iter->isRunning)
		{
			printf("line=%d, server(%s:%d) is running, stop it!\n",__LINE__, iter->strIP.c_str(), iter->nPort);
			IPPortItem tempItem;
			tempItem.strIP = iter->strIP;
			tempItem.nPort = iter->nPort;
			isStopAllServerSucc = killNormalEcho(tempItem);
			if(!isStopAllServerSucc)
			{
				printf("error: stop server failed\n");
			}
		}
	}
	if(isStopAllServerSucc)
		sg_ServerStatusTable.clear();

	return isStopAllServerSucc;
}
};


#define startNormalEcho				    MockServerControlUtils::startNormalEcho
#define killNormalEcho				    MockServerControlUtils::killNormalEcho
#define startNormalEchoServer		    MockServerControlUtils::startNormalEchoServer
#define killNormalEchoServer		    MockServerControlUtils::killNormalEchoServer
#define startNormalEchoServerList		MockServerControlUtils::startNormalEchoServerList
#define killNormalEchoServerList		MockServerControlUtils::killNormalEchoServerList
#define startAllMainIdcServer			MockServerControlUtils::startAllMainIdcServer
#define startAllSatelliteIdcServer		MockServerControlUtils::startAllSatelliteIdcServer
#define killAllSatelliteIdcServer		MockServerControlUtils::killAllSatelliteIdcServer
#define startEchoServer				    MockServerControlUtils::startEchoServer
#define killAllServer				    MockServerControlUtils::killAllServer



#endif