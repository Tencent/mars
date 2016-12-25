#ifndef STREAMCDN_SRC_CDN_DEFINE_H_
#define STREAMCDN_SRC_CDN_DEFINE_H_

#pragma once
#include<string>
#include <vector>

#include "mars/comm/autobuffer.h"

namespace mars{
namespace cdn{
    enum CdnRetCode {
        //cdn client errcode
        
        kRetSuccess = 0,
        
        //network err
        kBaseNetworkError                   = -10000,
        kPackError                     		= -10001,       /// 组包失败
        kUnpackError                   = -10002,     /// 解包失败
        kRetryLimit                    = -10003,      /// 重试超出限制
        kNetError                      = -10004,   /// 客户端网络异常
        kPackTimeout                   = -10005,  /// 包超时（入队列起）
        kAuthError                     = -10006, /// 登录失败
        kParserError                   = -10007, /// 解析失败
        kBeenCanceled                 = -10008,    /// 被取消
        kTimeoutQueue                  = -10011, //队列超时
        kTimeoutTransmit               = -10012, //传输超时
        kInvalidSvrResp               = -10013,
        kRequestSkeyFailed            = -10014,
        kGetFileMd5Failed             = -10015,
        kPackRequestFailed            = -10016,
        kRemoteClosed				  = -10017,	//remote closed. (received 0 bytes)
        kRemoteReset				  	  = -10018, //remote reset.
        kFlowLimit                      = -10019,
        
        kErrInternalNetworkChanged               = -11000,
        kErrInternalVerifyFailed                 = -11001,
        kErrInternalVerifyTimeout                = -11002,
        kErrUnSupportMethod               = -11003,
        
        //endof network err
        
        //scene err
        kErrBaseScene                     = -20000,
        kErrLogicBegin                    = -20000,
        
        //不可重试的错误码
        kErrNotInit                       = -20001, //CDNcom没有init
        kErrInvalidRootPath              = -20002, //根目录非法
        kErrInvalidParam                   = -20003, //参数错误
        kErrUnavalible                    = -20004, //cdn不可用，没有设置dnsinfo或者没有init
        //endof 不可重试的错误码
        
        kErrMediaIsUploading             = -21005, //正在上传，重复请求
        kErrMediaIsDownloading           = -21006, //正在下载，重复请求
        kErrMediaNotExist                = -21007, //已经出错或者成功,不能cancel
        kErrMediaCompleted                = -21008, //已经出错或者成功,不能cancel
        kErrMediaOpenFailed              = -21009, //打开媒体文件失败
        kErrGetMediaLengthFailed        = -21000, //获取文件长度失败
        kErrGetFstblockDataFailed         = -21011, //获取第一块数据失败
        kErrGetNextblockDataFailed         = -21012, //获取下一块数据失败
        kErrEncryptFailed                 = -21013, //上传加密文件buf失败
        kErrDecryptFailed                 = -21014, //下载解密文件buf失败
        kErrInvalidContextInfo            = -21015, //上下文信息有错误
        kErrGetDataFailed                = -21016, //获取文件数据失败
        kErrInvalidMediaLength            = -21017, //上层给的文件长度和cdn的密文长度差不在1-16之间
        kErrContextStatus            = -21018, //状态机错误
        kErrReachMaxTimes                = -21019, //下载数据，循环次数异常
        kErrWriteMediaFileFailed        = -21020, //下载数据，写媒体文件失败
        kErrReadMediaFileFailed         = -21021, //上传数据，读取媒体文件失败
        kErrContextInfoOpenFailed        = -21022, //打开CONTEXTINFO文件失败
        kErrContextInfoReadFailed        = -21023, //读取CONTEXTINFO文件失败
        kErrGetThumbFileLengthFailed   = -21024, //缩略图大小不合法
        kErrOpenThumbFileFailed         = -21025, //打开缩略图失败
        kErrReadThumbFileFailed         = -21026, //读取缩略图失败
        kErrNoReturnFileID               = -21027, //没有返回fileid
        kErrNoReturnFileID2              = -21028, //没有返回fileid
        kErrClientFileTooLarge                 = -21029, //文件太大
        kErrNoSkeyStatus                  = -21030,  //skey结果未返回
        kErrSkeyVerifyFailed               = -21031,  //skey校验失败
        kErrSkeyVerifyTimeout            = -21032,  //skey校验超时
        kErrPrepareDecodeFailed          = -21033,  //decode prepare resp failed
        kErrSkeySysError1                 = -21034, // skeyresp = -1,
        kErrSkeySysError13                = -21035, // skeyresp = -13,
        kErrSkeyLogicError                = -21036, // skeyresp !=0 的其它情况
        kErrFileDataMd5NotMatch          = -21037,
        
        
        kErrLogicEnd              = -22000,
        
        //end of scene err
        
        //end of cdn client errcode
        
        
        //cdn server errcode  //cdn sever的错误码不可重试
        kErrBase =    -5000000,
        
        kErrTaskState = -5103002,
        
        kErrEncodeFailed =     -5103003,
        kErrDecodeFailed =     -5103004,
        
        kErrFrontIPNoMatch = -5103008,
        kErrParamNotExist = -5103009,
        kErrHttpBodyLen = -5103010,
        kErrInvalidAuthKey = -5103011,
        kErrUploadBusy = -5103012,
        kErrCcdMQOverflow =  -5103013,
        kErrDccMQOverflow =  -5103014,
        kErrParamInvalid =  -5103015,
        kErrSetDiskCache = -5103016,
        kErrNoData = -5103017,
        kErrFrontGetBusy = -5103018,
        kErrFatherNotExist = -5103019,
        kErrMemOverflow = -5103020,
        kErrZoneHttpBusy = -5103021,
        kErrNoZoneIP = -5103022,
        kErrPostBackBusy = -5103023,
        kErrDiskCacheData = -5103024,
        kErrPostBackKeyNotExist = -5103025,
        kErrDiskCacheGet = -5103026,
        kErrCompressDataBig = -5103027,
        kErrRSAVerNotExist = -5103028,
        kErrCreateRSAFile =  -5103029,
        kErrWriteRSAFile =  -5103030,
        kErrRSADecrypt = -5103031,
        kErrFrontFileExist = -5103032, // 续传时，文件已传完
        kErrAESEncrypt = -5103033,
        kErrAESDecrypt = -5103034,
        kErrWriteDiskBusy = -5103035,
        kErrGetDiskCache = -5103036,
        kErrCompressBusy = -5103037,
        kErrReqDownloadBusy = -5103038,
        kErrBackSrcFrontHttpBusy = -5103039,
        kErrBackSrcZoneHttpBusy = -5103040,
        kErrNoMidImgLen = -5103041,
        kErrPostBackZoneFileExist = -5102047,
        kErrServerFileTooLarge = -5103058,
        kErrServerFileExpired = -5103059,
        kErrDownloadRedirect = -5103100,
        
        //these 3 error code can retry
        kErrInvalidOffset =  -5103205, //offset不匹配
        kErrSendDataOnInvalidConn = -5103202, //连接以发生错误但是还在续传数据
        kErrFileMd5NotEqual = -5103211,
        kErrAckRetransNoCompleteFile = -5103216,
        kErrDataChecksumNotEqual = -5103215,
        
        kErrForbidIllegalContent = -5103087, //非法内容拒绝下载
        
        //需要延迟通知失败的容灾错误码...
        kErrDownLoadVideoControl = -5103073,
        kErrOverloadControl = -5103084,        //任务失败，并且延迟通知到APP(10s)
        kErrUploadFlowControl = -5103081,
        kErrDownloadFlowControl = -5103082,
        kErrPostBackFlowControl = -5103083,
        kErrFlowControl = -5103210,
    	kErrImageStorageOverload = -999,
    };
    
    enum EMediaType {
        kMediaTypeAny = 0,
        kMediaTypeFullSizeImage = 1,   // need RSA(AESKey)原图，
        kMediaTypeImage = 2,//中图
        kMediaTypeThumbImage = 3,
        kMediaTypeVideo = 4,
        kMediaTypeFile  = 5,//app附件
        kMediaTypeTinyVideo = 6,//微视频
        kMediaTypeBigFile = 7,
        
        kMediaTypeFavoriteFile   = 10001,//收藏文件
        kMediaTypeFavoriteVideo  = 10002,//收藏视频（流媒体视频上传时还需要指定isStreamMedia）
        kMediaTypeExposeImage    = 11000, //举报证据图片
        
        kMediaTypeBackupFile   = 20001,//聊天记录备份（C2C 20001~29999）
        
        kMediaTypeFriends = 20201,  //朋友圈
        kMediaTypeFriendsVideo = 20202,    //朋友圈视频
        kMediaTypeShop = 20301, //C2C个人小店
        kMediaTypeNearEvent  = 20310,   //附近的CDN图片
        kMediaTypeSmartHwPage = 20321,    //硬件公众号封面

	    kMediaTypeAppImage = 20301,    //微信应用图片类型
    	kMediaTypeAppVideo = 20302,    //微信应用视频类型
	    kMediaTypeAppFile = 20303,     //微信应用文件类型
	    kMediaTypeHWDevice = 20322,  //硬件近场转发
    	kMediaTypeHWDeviceFile = 20303,   //硬件转发文件				
    };
    
    enum ECDNScene {
        kCdnSceneC2C = 0,
        kCdnSceneFavorite = 1,
        kCdnSceneEmoji = 2,
        kCdnSceneStore = 3,
        kCdnSceneFriends = 4,
    };
    
    enum EBizType{
        kBizUnknown = -1,
        kBizAny = 0,
        kBizC2C = 1,
        kBizFavorite = 2,
        kBizSns = 3,
        kBizApp = 4,
    };
    
    enum EBizOpType{
        kOPUnknown = -1,
        kOPAny = 0,
        kOPUpload = 1,
        kOPDownload = 2,
    };
    enum EAppType {
        kAppTypeUnknown = -1,
        kAppTypeAny = 0,
        //C2C
        kAppTypeC2C = 1,
    	kAppTypeC2CGroupChat = 2,
        
        //favorite
        kAppTypeFavorite = 10,
        
        //sns
        kAppTypeSingleJpeg = 100,
        kAppTypeMultiJpeg = 101,
        kAppTypeVideo = 102,
        kAppTypeSingleWebp = 103,
        kAppTypeMultiWebp = 104,
        
        //appmsg
        kAppTypeShop = 200,
        kAppTypeHwPage = 200,
        kAppTypeNearEvent = 201,
    	kAppTypeYunStorage = 202,
    };
    
    enum ECDNComOptype {
        kCdnComOptypeUpload = 1,
        kCdnComOptypeDownload = 2,
        kCdnComOptypeResumeUpload = 3,
        kCdnComOptypeResumeDownload = 4,
        
        kCdnComOptypeStreamUpload = 5,
        kCdnComOptypeStreamDownload = 6,
        kCdnComOptypeResumeStreamUpload = 7,
        kCdnComOptypeResumeStreamDownload = 8,
        
        kCdnComOptypeStreamOnlyUpload = 9,
        kCdnComOptypeResumeStreamOnlyUpload = 10,
        
        kCdnComOptypeOCDownloadUseNewDns = 11,
        kCdnComOptypeOCDownloadUseSysDns = 12,
    };
    
    enum ECDNComPriority {
        kCdnComPriorityLow = 1,//收藏等低优先级用这个
        kCdnComPriorityMiddle = 2,//普通媒体用这个优先级
        
        //只有下载缩略图专用的优先级，上传缩略图不需要
        //高优先级下载不受队列限制，目前同时下载媒体数为2.
        kCdnComPriorityHigh = 3,
    };
    
    enum UploadHitCacheType {
        kNoHitCache = 0,
        kHitFileID = 1,
        kHitUploadCheckMd5 = 2,
        kHitUploadWithMd5 = 3,
    };
    
    enum NetWorkType {
        kNetWorkType2G = 1,
        kNetWorkType3G = 2,
        kNetWorkTypeWiFi = 3,
    };
    
    //new cdn interface
    struct CDNComDnsInfo
    {
        unsigned int ver;
        unsigned int uin;
        int network_type;//参看枚举NetWorkType
        std::string frontip1;
        std::string frontip2;
        std::string zoneip1;
        std::string zoneip2;
        AutoBuffer authkey;
        std::vector<unsigned short> ports;
        
        CDNComDnsInfo();
    };
    
    //
    struct FileIDCheckItem
    {
        std::string fileid;
        int filetype;
    };

    //upload
    struct C2CUploadRequest
    {
        std::string filekey;    //任务标识，需要保证唯一
        void* user_context;     //用户自定义
        std::string touser;     //目标用户
        int32_t queue_timeout_seconds;  //排队超时时间
        int32_t transfor_timeout_seconds;   //运行时超时时间
        
        int filetype;           //文件类型，见前面EMediaType定义
        std::string filepath;   //文件路径
        std::string thumb_filepath; //缩略图路径. 有效则表示需要发缩略图
        std::string forward_fileid; //转发的fileid
        std::string forward_aeskey; //转发的aeskey
        
        size_t file_size;           // 文件大小
        size_t midfile_size;       // 原图的中图大小
	    std::string filemd5;     // 32byte
    	//
		std::string  wx_signature;   //
		bool check_exist_only;
		AutoBuffer sso_buffer_request;
    };
    
    struct CheckFileIDRequest
    {
        std::string filekey;
        std::string touser;
        void* user_context;
        int32_t queue_timeout_seconds;  //排队超时时间
        int32_t transfor_timeout_seconds;   //运行时超时时间
        
        std::vector<FileIDCheckItem> vFileIdList;
    };
    
    struct C2CUploadResult
    {
        int error_code;
        
        std::string filekey;    //任务标识，需要保证唯一
        void* user_context;     //用户自定义
        std::string touser;     //目标用户

        std::string fileid;     
        std::string aeskey;     //加密的key
        std::string filemd5;    //原文件md5
		std::string thumbfilemd5;		
        std::string transfor_msg;   //传输的一些信息，用于kv上报
        
        size_t file_size;       
        size_t midfile_size;
        size_t thumbfile_size;
        
        UploadHitCacheType hitcache;
    };
    
    struct FileIDCheckResultItem
    {
        FileIDCheckItem request;
        bool exist;
        std::string newfileid;
    };
    
    struct CheckFileIDResult
    {
        std::string filekey;
        int error_code;
        void* user_context;
        
        std::vector<FileIDCheckResultItem> vResult;
    };
    
    
    //download
    struct C2CDownloadRequest
    {
    	C2CDownloadRequest() {
    		Reset();
    	}

    	void Reset() {
    		transfor_timeout_seconds = 60;
			queue_timeout_seconds = 60;

    		filetype = kMediaTypeAny;
    		file_size = 0;
    		limit_rate = 0;
    	}

        std::string filekey;    //任务标识，需要保证唯一
        void* user_context;     //用户自定义
        std::string arg_info;   // 用户自定义
        int32_t queue_timeout_seconds;  //排队超时时间,
        int32_t transfor_timeout_seconds;   //运行时超时时间
        
        int filetype;
        size_t file_size;
        std::string savepath;
        std::string aeskey;
        std::string fileid;
		std::string filemd5;     // 32byte	
								 //Limit the download speed to amount bytes per second, in KBytes
		int limit_rate;     //<=0: no limit;  >0: limit;  default is 0.
		std::string wx_signature;

        bool is_persist_file;   //是否是永久存储，比如收藏
        bool is_tinyvideo_file; //是否是小视频
    };
    
    struct C2CDownloadResult
    {
        std::string filekey;
        void* user_context;
        std::string arg_info;
        
        int error_code;
        size_t file_size;
        std::string fileid;
        std::string transfor_msg;
    };
    
    
    //callback functions
    extern void onProgressChanged(const std::string& _filekey, size_t _completed_length, size_t _total_length);
    extern void onC2CUploadCompleted(const std::string& _filekey, const C2CUploadResult& _result);
    extern void onC2CDownloadCompleted(const std::string& _filekey, const C2CDownloadResult& _result);
    extern void onRecvedData(const std::string& _filekey, size_t _offset, const void* _buffer, size_t _length);
    extern void onCheckFileIDCompleted(const std::string& _filekey, const CheckFileIDResult& _result);
	extern bool onDecodeSkeyResponse(const std::string& _filekey, const AutoBuffer& inbuf, AutoBuffer& outbuf);
}
}

#endif // STREAMCDN_SRC_CDN_DEFINE_H_

