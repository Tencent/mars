// win32test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <sys/stat.h>

#include "mars/xlog/appender.h"
#include "mars/xlog/xlogger.h"
#include "mars/baseevent/base_logic.h"
#include "app_callback.h"
#include "cdn_callback.h"
#include "stn_callback.h"

#define RSA_VERSION "1"
#define RSAPUBKEY_E "010001"
#define RSAPUBKEY_N "BFEDFFB5EA28509F9C89ED83FA7FDDA8881435D444E984D53A98AD8E9410F1145EDD537890E10456190B22E6E5006455EFC6C12E41FDA985F38FBBC7213ECB810E3053D4B8D74FFBC70B4600ABD728202322AFCE1406046631261BD5EE3D44721082FEAB74340D73645DC0D02A293B962B9D47E4A64100BD7524DE00D9D3B5C1"

using namespace mars::stn;

void InitCDN()
{
	mars::cdn::Init("e:\\temp");
	mars::cdn::SetRSAParams(RSA_VERSION, RSAPUBKEY_N, RSAPUBKEY_E);

	const char* testip = "10.219.19.48";
	//const char* testip = "101.227.149.196";
	const unsigned char constAuthKey[] = {
		0x30, 0x3A, 0x02, 0x01, 0x01, 0x04, 0x33, 0x30,
		0x31, 0x02, 0x01, 0x01, 0x02, 0x01, 0x00, 0x02,
		0x03, 0x01, 0xE2, 0x40, 0x02, 0x03, 0x0F, 0x42,
		0x41, 0x02, 0x04, 0x03, 0x03, 0x02, 0x01, 0x02,
		0x04, 0xA8, 0xC3, 0x11, 0xAC, 0x02, 0x01, 0x01,
		0x02, 0x01, 0x00, 0x02, 0x01, 0x00, 0x02, 0x04,
		0x5D, 0x9B, 0xFF, 0x6E, 0x02, 0x04, 0xF6, 0x55,
		0x4C, 0xC2, 0x04, 0x00
	};

	AutoBuffer authkey;
	authkey.Write(constAuthKey, sizeof(constAuthKey));
	mars::cdn::SetDebugIP(testip, 123456, authkey);
}

long long getfilesize(const char* path)
{
	struct _stat temp;
	if (_stat(path, &temp) == 0)
	{
		return temp.st_size;
	}

	return 0;
}

void testDownload()
{
	const char* savepath = "e:\\temp\\test-down.jpg";
	//
	mars::cdn::C2CDownloadRequest request;
	request.fileid = "3034020100042d302b020100020301e24002033d11fd0204c495e365020457467420040a75706c6f6164746573740201000201000400";
	request.filekey = "downloadtest";
	request.filetype = mars::cdn::kMediaTypeImage;
	request.file_size = 86745;
	request.aeskey = "014d28e2c8d24b088d95146edcaf9264";
	request.savepath = savepath;
	request.is_persist_file = false;
	request.is_tinyvideo_file = false;
	request.transfor_timeout_seconds = 60;
	request.queue_timeout_seconds = 60;

	int ret = mars::cdn::StartC2CDownload(request);
	if (ret != 0){
		xerror2(TSF"start download failed. err:%d", ret);
	}
}

void testUpload()
{
	const char* savepath = "e:\\temp\\test.jpg";

	mars::cdn::C2CUploadRequest request;
	request.filekey = "uploadtest";
	request.touser = "test";
	request.filetype = mars::cdn::kMediaTypeImage;
	request.thumb_filepath = "e:\\temp\\test-thumb.jpg";
	request.filepath = savepath;
	request.file_size = getfilesize(savepath);
	request.transfor_timeout_seconds = 60;
	request.queue_timeout_seconds = 60;

	int ret = mars::cdn::StartC2CUpload(request);
	if (ret != 0){
		xerror2(TSF"start download failed. err:%d", ret);
	}
}

void testSendTask() 
{
	Task task;
	task.channel_select = Task::kChannelShort;
	task.host = "weixin.qq.com";
	task.cgi = "/cgi-bin/hello";
	task.cmdid = 1;
	task.channel_strategy = Task::kChannelNormalStrategy;
	task.need_authed = false;

	StartTask(task);

}


int _tmain(int argc, _TCHAR* argv[])
{
	xlogger_SetLevel(kLevelDebug);
	appender_open(kAppednerAsync, "e:\\temp\\test", "marsexample");
	appender_set_console_log(true);

	std::vector<uint16_t> ports;
	ports.push_back(80);

	mars::stn::SetLonglinkSvrAddr("weixin.qq.com", ports);
	//debug longlink ip
	//mars::stn::SetLonglinkSvrAddr("localhost", ports, "127.0.0.1");
	mars::stn::SetShortlinkSvrAddr(80);
	//debug shortlink ip
	//mars::stn::SetShortlinkSvrAddr(80, "127.0.0.1");
	std::vector<std::string> backup_ips;
	backup_ips.push_back("0.0.0.0");
	mars::stn::SetBackupIPs("localhost", backup_ips);	//host-backup_ip map

	mars::cdn::SetCallback(new mars::cdn::CdnCallBack());
	mars::app::SetCallback(new mars::app::AppCallBack());
	mars::stn::SetCallback(new mars::stn::StnCallBack());

	mars::baseevent::OnCreate();

	mars::baseevent::OnForeground(true);

	testSendTask();

	InitCDN();

	testDownload();

	//testUpload();

	::Sleep(5 * 60 * 1000);

	return 0;
}

