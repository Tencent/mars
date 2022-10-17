//
// Created by Changpeng Pan on 2022/10/14.
//

#ifndef MMNET_BASE_IPXX_MANAGER_H
#define MMNET_BASE_IPXX_MANAGER_H


namespace mars {
namespace magicbox {

class Callback;
class IUploadFileProcessObserver;

class BaseIPxxManager {

 public:
    virtual ~BaseIPxxManager() {}
    virtual void Init() = 0;
    virtual void UnInit() = 0;


 public:
    virtual void SetHost(const std::string& _host) = 0;
    virtual void SetCgi(const std::string& _cgi) = 0;
    virtual void SetSumFormat(const std::string& _sum_format) = 0;

    virtual void UploadFile(const char* _file_path,
                    int _file_type,
                    int _product_id,
                    int64_t _uin = -1,
                    std::string _username = "",
                    IUploadFileProcessObserver* _observer = NULL,
                    int _observer_id = 0,
                    const std::string& _extra_info = "",
                    const std::string& _rtxname = "") = 0;
    virtual void UploadLog(int _product_id,
                   IUploadFileProcessObserver* _observer,
                   int _observer_id,
                   std::vector<int> _timespans,
                   bool _is_login,
                   const std::string& _extra_info,
                   int64_t _uin = -1,
                   std::string _username = "",
                   int _begin_hour = 0,
                   int _end_hour = 24,
                   const std::string& _input_prefix_preffix = "86",
                   const std::string& _user_input_suffix = "") = 0;

    virtual void OnIPxx(const std::string& _ipxx_content, int _product_id) = 0;

    virtual void SetFileReportDebugHost(const std::string& _host) = 0;

    virtual void SetCallback(Callback* const callback) = 0;

    virtual void GetUploadLogExtrasInfo(std::string& _extrasinfo, const std::string& _file_path) = 0;
    virtual void SetNewDnsDebugHostInfo(const std::string& _ip, int _port) = 0;
    virtual void GetCrashFilePath(std::string& _file_path, int _timespan) = 0;
    virtual void RecoverLinkAddrs() = 0;

    virtual std::string GetHostReport() = 0;
    virtual std::string GetHostReportIDC() = 0;
    virtual void SetHostReportIDC(const std::string& _host) = 0;

};
}
}

#endif  // MMNET_BASE_IPXX_MANAGER_H
