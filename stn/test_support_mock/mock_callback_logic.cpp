/** * created on : 2012-11-28 * author : 叶润桂
 */

#include <string>
#include <vector>

class AutoBuffer;

int onTaskEnd( const int errType, const int errCode, const char* const errMsg, const int hashCode, const AutoBuffer& cookies) { return 0; }
void onPush(int seq, int cmd, const AutoBuffer& data) {}
void onOOBNotify(unsigned int uiType, unsigned int uiTime) { }
void onOOBNotify(std::string& xml) { }
void onRequestDoSync() {}
bool req2Buf( const int hashCode, AutoBuffer& encodeBuffer) { return true; }
int buf2Resp( const int hashCode, const AutoBuffer& decodeBuffer, AutoBuffer& autoBuffer) { return -1; }
void sessionTimeout() {}
bool makeSureAuth() { return true;}
bool isLogoned() { return false; }
void getSyncCheckInfo(int& uin, AutoBuffer& syncKeyBuffer, AutoBuffer& md5Buffer) {}


void reportFlowData(const int wifiRecvDataSize, const int wifiSendDataSize, int mobileRecvDataSize, int mobileSendDataSize) { }
void reportNetConnectInfo(int netstatus, bool blonglinkbeconnected) {}
void longLinkConnectInfo(int netstatus) {}
void shortLinkConnectState(bool _isConnectSuc) {}
void reportCrashStatistics(const char* _raw, const char* _type) {}

std::string getUserIDCLocale()
{
    std::string loc = "CN";
    return loc;
}

int getStatisticsNetType()
{
    return 0;
}


int getLongLinkIdentifyCheckBuffer(AutoBuffer& identifyReqBuf, int& reqCmdId, int& respCmdId, AutoBuffer& hashCodeBuffer)
{
    return 2;
}

bool onLongLinkIdentifyResp(const AutoBuffer& buffer, const AutoBuffer& hashCodeBuffer)
{
    return true;
}


void ReportKV(unsigned int logId, const char* value, bool isImportant, bool isReportNow){}
void ReportIDKey(unsigned int ID, unsigned int key, unsigned int value, bool isImportant) {}
void reportGroupIDKey(const std::vector<std::vector<unsigned int> > & lstIdKeyInfo, bool isImportant) {}

// trace route
void startTraceroute(const char* ip) { }

