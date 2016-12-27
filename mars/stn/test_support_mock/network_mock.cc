#include <vector>

#include "net_comm.h"

class MMNewSpeedTest {
public:
	MMNewSpeedTest();
	~MMNewSpeedTest();
    
public:
	void StartAsyncSpeedTest(const char* _buffer, int _len);
};
MMNewSpeedTest::MMNewSpeedTest()
{
}
MMNewSpeedTest::~MMNewSpeedTest()
{

}
void MMNewSpeedTest::StartAsyncSpeedTest(const char* _buffer, int _len)
{
}

class CMMSpeedTest {

public:
	CMMSpeedTest();
	~CMMSpeedTest();

public:
	void StartAsyncSpeedTest(const std::vector<STTestIpPort>& vectIpPort);
};

CMMSpeedTest::CMMSpeedTest()
{
}
CMMSpeedTest::~CMMSpeedTest()
{

}
void CMMSpeedTest::StartAsyncSpeedTest(const std::vector<STTestIpPort>& vectIpPort)
{
}

#include "../error_log_report.h_"
ErrorLogReport::ErrorLogReport()
:m_nLastBackgroundReportTime(0)
,m_pFile(NULL)
,m_ini("c:\\errlog.txt")
{
	m_pReportThread = NULL;
}

ErrorLogReport::~ErrorLogReport()
{
	if(NULL != m_pReportThread)
	{
		delete m_pReportThread;
		m_pReportThread = NULL;
	}

	if(NULL != m_pFile)
	{
		fclose(m_pFile);
		m_pFile = NULL;
	}
}
#include "longlink_speed_test.h"

CMMLongLinkSpeedTestItem::CMMLongLinkSpeedTestItem(const std::string& _ip, unsigned int _port)
: m_ip(_ip)
, m_port(_port)
, m_socket(-1)
, m_state(ELongLinkSpeedTestConnecting)
, m_beforeConnectTime(0)
, m_afterConnectTime(0)
{
}

CMMLongLinkSpeedTestItem::~CMMLongLinkSpeedTestItem() 
{
	CloseSocket();
}

void CMMLongLinkSpeedTestItem::HandleFDISSet(SocketSelect& _sel)
{
	xverbose_function();
}

void CMMLongLinkSpeedTestItem::HandleSetFD(SocketSelect& _sel) 
{
}

int CMMLongLinkSpeedTestItem::GetSocket() {
	return m_socket;
}

std::string CMMLongLinkSpeedTestItem::GetIP() {
	return m_ip;
}

unsigned int CMMLongLinkSpeedTestItem::GetPort() {
	return m_port;
}

unsigned long CMMLongLinkSpeedTestItem::GetConnectTime() {
	return m_afterConnectTime - m_beforeConnectTime;
}

int CMMLongLinkSpeedTestItem::GetState() {
	return m_state;
}

void CMMLongLinkSpeedTestItem::CloseSocket() {
	if (m_socket > 0) {
		::socket_close(m_socket);
		m_socket = -1;
		m_socket = -1;
	}
}
