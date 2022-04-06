//
// Created by Cpan on 2022/3/10.
//

#ifndef MMNET_MARS_H
#define MMNET_MARS_H

#include "mars/stn/stn_manager.h"
#include "mars/sdt/sdt_manager.h"
#include <string.h>

using namespace mars::stn;
using namespace mars::sdt;

namespace mars {
namespace boot {

class Mars {
public:
  Mars();

  std::function<void ()> on_create_;
  std::function<void ()> on_destroy_;
  std::function<void (bool _is_foreground)> on_foreground_;
  std::function<void (int64_t _id)> on_alarm_;
  std::function<void ()> on_network_change_;
  std::function<void (const char *_tag, int32_t _send, int32_t _recv)> on_network_data_change_;
  std::function<void (int _sig)> on_singal_crash_;
  std::function<void ()> on_exception_crash_;

  void OnInitConfigBeforeOnCreate(int _packer_encoder_version);
  uint64_t LastForeGroundChangeTime();
  bool IsActive();
  void SwitchActiveStateForDebug(bool _active);
  StnManager* GetStnManager();
  SdtManager* GetSdtManager();

public:
    void OnCreate(const std::string& _name);
    void OnDestroy();
    void OnSignalCrash(int _sig);
    void OnExceptionCrash();
    void OnNetworkChange();
    void OnNetworkDataChange(const char *_tag, int32_t _send, int32_t _recv);

private:
    std::string name_;
    bool is_foreground_;
    bool is_active_;
    uint64_t last_foreground_change_time_;
    StnManager *stn_manager_ = nullptr;
    SdtManager *sdt_manager = nullptr;
};

} // namespace boot
} // namespace mars

#endif // MMNET_MARS_H
