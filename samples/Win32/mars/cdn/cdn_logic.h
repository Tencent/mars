#ifndef _PBC_CDNLIB_H
#define _PBC_CDNLIB_H

#include "../comm/autobuffer.h"
#include "cdn.h"

namespace mars{
namespace cdn{
    
    //callback interface
    class Callback
    {
    public:
        virtual ~Callback(){}
        virtual void OnProgressChanged(const std::string& _filekey, size_t _completed_length, size_t _total_length){}
        virtual void OnC2CUploadCompleted(const std::string& _filekey, const C2CUploadResult& _result){}
        virtual void OnC2CDownloadCompleted(const std::string& _filekey, const C2CDownloadResult& _result){}
        virtual void OnRecvedData(const std::string& _filekey, size_t _offset, const void* _buffer, size_t _length){}
        virtual void OnCheckFileIDCompleted(const std::string& _filekey, const CheckFileIDResult& _result){}
		virtual bool OnDecodeSkeyResponse(const std::string& _filekey, const AutoBuffer& inbuf, AutoBuffer& outbuf) { return false;	 }
    };
    
	
	// init with data path
	int Init(const char* datapath);
    
    // callback
    void SetCallback(Callback* const callback);
    
    // set debug ip. set NULL to clean
    void SetDebugIP(const char* ip, uint32_t uin, const AutoBuffer& authkey);

	//set cdn information, a protobuf buffer
	int SetCdnInfo(const void* infobuffer, size_t length);
    
    //set rsa encryption params
    void SetRSAParams(const char* _rsaver, const char* _publickey_N, const char* _publickey_E);
    
    //set flowlimit, in bytes. default is no limit. <0 means no limit. =0 means forbid all upload/download.
    void SetFlowlimitPerhour(int64_t limit);

	// set touser encrypt key, optional
	int SetTouserCiper(const AutoBuffer& key);

	// start normal c2c upload
    int StartC2CUpload(const C2CUploadRequest& request);

	// start normal c2c download
	int StartC2CDownload(const C2CDownloadRequest& request);
    
    // start fileid check
    int StartFileidCheck(const CheckFileIDRequest& request);

	// cancel
	int Cancel(const std::string& filekey);

	// uninit
	void UnInit();
}
}

#endif