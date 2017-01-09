#include <string>

std::string kHostReport = "support.weixin.qq.com";
std::string kHostReportIDC = "";
#ifdef __cplusplus
extern "C" {
#endif
int doEcdsaVerify(const unsigned char* _pubkey, int len_key, const char* _content, int len_ctn, const char* _strSig, int len_sig)
{
	return 1;
}
#ifdef __cplusplus
}

std::string getAppPrivatePath()
{
	return "C:\\";
}

struct XLoggerInfo_t;
void printConsoleLog(const XLoggerInfo_t* _info, const char* _log)
{

}
#endif