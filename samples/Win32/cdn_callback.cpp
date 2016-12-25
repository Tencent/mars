#include "stdafx.h"
#include "cdn_callback.h"

#include "mars/xlog/appender.h"
#include "mars/xlog/xlogger.h"

namespace mars {
	namespace cdn {
	
		 void CdnCallBack::OnProgressChanged(const std::string& filekey, size_t completed_length, size_t total_length)
		{
			xinfo2(TSF"filekey %_ progress %_/%_", filekey.c_str(), completed_length, total_length);
		}
		 void CdnCallBack::OnC2CUploadCompleted(const std::string& filekey, const C2CUploadResult& result)
		{
			xinfo2(TSF"filekey %_ upload completed, error:%_, fileid : %_, aeskey : %_", filekey.c_str(), result.error_code, result.fileid.c_str(), result.aeskey.c_str());

		}
		 void CdnCallBack::OnC2CDownloadCompleted(const std::string& filekey, const C2CDownloadResult& result)
		{
			xinfo2(TSF"filekey %_ download completed, error:%_, fileid:%_", filekey.c_str(), result.error_code, result.fileid.c_str());

		}
		 void CdnCallBack::OnCheckFileIDCompleted(const std::string& filekey, const CheckFileIDResult& result)
		{
			xinfo2(TSF"filekey %_ checkfileid completed, error:%_", filekey.c_str(), result.error_code);
		}

	}
}