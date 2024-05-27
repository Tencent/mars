#pragma once

#include <mutex>
#include <string>
#include <vector>

#include "mars/comm/tinyxml2.h"
#include "special_ini.h"
#include "stn/stn.h"

namespace mars {
namespace stn {

enum {
    FROM_DEFAULT = 0,
    FROM_SERVER = 1,
    FROM_PREVIOUS_STATE = 2,  //.来源于上一次svr配置.
    FROM_PART_SERVER = 3,     // .部份来源于svr.
    FROM_MAX_COUNT,
};
extern const char* LabelConfigFrom[FROM_MAX_COUNT];

enum { TCP_LONGLINK = 0, TCP_SHORTLINK, QUIC_LONGLINK, QUIC_SHORTLINK, MAX_LINK_TYPES };
extern const char* LabelLinkType[MAX_LINK_TYPES];

struct ConnectStrategy {
    time_t expire_timetick = 0;  //.过期时间,秒.
    ConnectPorts ports[MAX_LINK_TYPES];
    ConnectCtrl ctrls[MAX_LINK_TYPES];
};

class ConnectParams {
 public:
    ConnectParams();
    void SetIniFilePath(const std::string& inifile);
    bool LoadFromINIFile(const std::string& section);
    bool LoadFromINI(SpecialINI& ini);
    void UpdateToINI(SpecialINI& ini);

    bool UpdateFromXML(tinyxml2::XMLElement* node, SpecialINI& ini);
    ConnectStrategy CurrentStrategy() const;

 private:
    void _InitDefaultStrategy();
    bool _LoadFromINIImpl(SpecialINI& ini);
    void _UpdateToINIImpl(SpecialINI& ini);

    std::string inifile_;
    ConnectStrategy strategy_;
    mutable std::mutex mutex_;
};

}  // namespace stn
};  // namespace mars