#pragma once

#include "mars/cdn/cdn.h"
#include "mars/cdn/cdn_logic.h"

namespace mars {
	namespace cdn {
	
class CdnCallBack : public Callback
{
public:

	CdnCallBack() {}
	~CdnCallBack() {}

	virtual void OnProgressChanged(const std::string& filekey, size_t completed_length, size_t total_length);

	virtual void OnC2CUploadCompleted(const std::string& filekey, const C2CUploadResult& result);

	virtual void OnC2CDownloadCompleted(const std::string& filekey, const C2CDownloadResult& result);

	virtual void OnCheckFileIDCompleted(const std::string& filekey, const CheckFileIDResult& result);

};

	}
}