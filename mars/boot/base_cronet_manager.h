//
// Created by Changpeng Pan on 2022/9/26.
//

#ifndef MMNET_BASE_CRONET_MANAGER_H
#define MMNET_BASE_CRONET_MANAGER_H

namespace cronetios {

class BaseCronetManager {
 public:
    virtual ~BaseCronetManager(){}
    virtual void Init() = 0;
    virtual void UnInit() = 0;

 public:
    virtual int GetRandomInt();
    virtual bool GenRandom(char* _data);

    // CronetGlobalCallback
    virtual bool DoDnsResolve(const std::string& _host, std::vector<std::string>& _result);
    virtual bool UseHttpDnsAndLocalDns();
    virtual void NotifyWeakOrGoodNet(bool _weak);
    // CronetGlobalCallback

    virtual void SetGlobalCallback(std::shared_ptr<CronetGlobalCallback> _cb);
    virtual bool DoDnsResolveCallback(const std::string& _host, std::vector<std::string>& _ip_result);
    virtual bool UseDnsCallback();
    virtual bool HasSystemProxy();
    virtual void notifyCronetWeaknet(bool _weak);
    virtual void SetAppPath(const std::string& _path);
    virtual CreateRequestResult StartCronetRequest(CronetTaskProfile::CronetTaskParams& _params, bool _need_generator_id);
    virtual CreateRequestResult StartCronetUploadRequest(CronetTaskProfile::CronetTaskParams& _params, bool _need_generator_id);
    virtual void CancelCronetTask(const std::string& _task_id);
    virtual void SetGoodNetNotifyInterval(uint32_t _interval);
    virtual void OnReEnterCronet();
};
}
#endif  // MMNET_BASE_CRONET_MANAGER_H
