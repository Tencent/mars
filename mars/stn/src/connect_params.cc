#include "connect_params.h"

#include <cstdint>
#include <mutex>
#include <string>
#include <vector>

#include "mars/comm/ini.h"
#include "mars/comm/strutil.h"
#include "mars/comm/tinyxml2.h"
#include "mars/comm/xlogger/xlogger.h"
#include "special_ini.h"
#include "stn/config.h"

namespace mars {
namespace stn {

const char* LabelConfigFrom[] = {"DEFAULT", "FROM_SERVER", "FROM_PREVIOUS_STATE", "FROM_PART_SERVER"};
const char* LabelLinkType[] = {"TCP_LONGLINK", "TCP_SHORTLINK", "QUIC_LONGLINK", "QUIC_SHORTLINK"};

namespace {
// <connstrategy  cachetimeout="604800">

// <portlist type="long">
// <port>80</port>
// <port>443</port>
// <port>8080</port>
// <port>5000</port>
// </portlist>
// <portlist type="short">
// <port>80</port>
// </portlist>
// <portlist type="quic">
// <port>80</port>
// </portlist>

// <connctrl type="long" interval="2500" conntimeout="2500" maxconnnum="3">
// </connctrl>
// <connctrl type="short" interval="2500" conntimeout="3000" maxconnnum="3">
// <conntimeout property="ipv4">2000</conntimeout>
// <conntimeout property="ipv6">2500</conntimeout>
// </connctrl>
// <connctrl type="quic" interval="1000" conntimeout="250" maxconnnum="3">
// <conntimeout property="ipv6">400</conntimeout>
// </connctrl>

// </connstrategy>
bool parse_connect_port_xml(tinyxml2::XMLElement* elem, ConnectStrategy& strategy) {
    // port config
    auto* portlist = elem->FirstChildElement("portlist");
    while (portlist) {
        int link_type = TCP_SHORTLINK;
        if (portlist->Attribute("type", "long")) {
            link_type = TCP_LONGLINK;
        } else if (portlist->Attribute("type", "quic")) {
            link_type = QUIC_SHORTLINK;
        }

        ConnectPorts conncfg;
        conncfg.from_source = FROM_PART_SERVER;

        auto* portelem = portlist->FirstChildElement("port");
        while (portelem) {
            int port = 0;
            if (portelem->QueryIntText(&port) == tinyxml2::XML_SUCCESS && port > 0) {
                conncfg.ports.emplace_back(static_cast<uint16_t>(port));
            }
            portelem = portelem->NextSiblingElement("port");
        }

        if (!conncfg.ports.empty()) {
            conncfg.from_source = FROM_SERVER;
        }

        // done.
        strategy.ports[link_type] = conncfg;

        xinfo2(TSF "xml.port %_ ports %_", LabelLinkType[link_type], strutil::join_to_string(conncfg.ports));

        portlist = portlist->NextSiblingElement("portlist");
    }
    return true;
}

bool parse_connect_timeout_xml(tinyxml2::XMLElement* elem, ConnectStrategy& strategy) {
    // port config
    auto* ctrlelem = elem->FirstChildElement("connctrl");
    while (ctrlelem) {
        int link_type = TCP_SHORTLINK;
        if (ctrlelem->Attribute("type", "long")) {
            link_type = TCP_LONGLINK;
        } else if (ctrlelem->Attribute("type", "quic")) {
            link_type = QUIC_SHORTLINK;
        }

        ConnectCtrl ctrl;
        ctrl.from_source = FROM_PART_SERVER;

        unsigned interval = 0;
        if (ctrlelem->QueryUnsignedAttribute("interval", &interval) == tinyxml2::XML_SUCCESS && interval > 0) {
            ctrl.interval_ms = interval;
        }
        unsigned timeout_ms = 0;
        if (ctrlelem->QueryUnsignedAttribute("conntimeout", &timeout_ms) == tinyxml2::XML_SUCCESS && timeout_ms > 0) {
            ctrl.ipv4_timeout_ms = timeout_ms;
            ctrl.ipv6_timeout_ms = timeout_ms;
        }
        unsigned maxconnnum = 0;
        if (ctrlelem->QueryUnsignedAttribute("maxconnnum", &maxconnnum) == tinyxml2::XML_SUCCESS && maxconnnum > 0) {
            ctrl.maxconn = maxconnnum;
        }
        unsigned zerortt_ms = 0;
        if (ctrlelem->QueryUnsignedAttribute("zerortttimeout", &zerortt_ms) == tinyxml2::XML_SUCCESS
            && zerortt_ms > 0) {
            ctrl.ipv4_zerortt_check_ms = zerortt_ms;
            ctrl.ipv6_zerortt_check_ms = zerortt_ms;
        }

        // connect timeout
        auto* subelem = ctrlelem->FirstChildElement("conntimeout");
        while (subelem) {
            if (subelem->QueryUnsignedText(&timeout_ms) == tinyxml2::XML_SUCCESS && timeout_ms > 0) {
                if (subelem->Attribute("property", "ipv4")) {
                    ctrl.ipv4_timeout_ms = timeout_ms;
                } else if (subelem->Attribute("property", "ipv6")) {
                    ctrl.ipv6_timeout_ms = timeout_ms;
                } else {
                    // not v4 nor v6
                }
            }
            subelem = subelem->NextSiblingElement("conntimeout");
        }

        // 0rtt check timeout
        subelem = ctrlelem->FirstChildElement("zerortttimeout");
        while (subelem) {
            if (subelem->QueryUnsignedText(&zerortt_ms) == tinyxml2::XML_SUCCESS && zerortt_ms > 0) {
                if (subelem->Attribute("property", "ipv4")) {
                    ctrl.ipv4_zerortt_check_ms = zerortt_ms;
                } else if (subelem->Attribute("property", "ipv6")) {
                    ctrl.ipv6_zerortt_check_ms = zerortt_ms;
                } else {
                    // not v4 nor v6
                }
            }
            subelem = subelem->NextSiblingElement("zerortttimeout");
        }

        if (interval > 0 && timeout_ms > 0 && maxconnnum > 0 && (link_type != QUIC_SHORTLINK || zerortt_ms > 0)) {
            ctrl.from_source = FROM_SERVER;
        }

        // done.
        strategy.ctrls[link_type] = ctrl;

        xinfo2(TSF "xml.ctrl %_ interval %_ ipv4 %_ ipv6 %_ maxconn %_ 0rttv4 %_ 0rttv6 %_",
               LabelLinkType[link_type],
               ctrl.interval_ms,
               ctrl.ipv4_timeout_ms,
               ctrl.ipv6_timeout_ms,
               ctrl.maxconn,
               ctrl.ipv4_zerortt_check_ms,
               ctrl.ipv6_zerortt_check_ms);

        ctrlelem = ctrlelem->NextSiblingElement("connctrl");
    }
    return true;
}

bool set_ports_from_ini_keyvalue(INI& ini, const char* key, ConnectPorts& conn) {
    std::vector<std::string> ports;
    std::string svalue = ini.Get(key);
    strutil::SplitToken(svalue, ",", ports);
    if (ports.empty()) {
        return false;
    }

    conn.ports.clear();
    for (const auto& sport : ports) {
        conn.ports.emplace_back(static_cast<uint16_t>(atoi(sport.c_str())));
    }

    xinfo2(TSF "get ini.port %_ ports %_", key, strutil::join_to_string(conn.ports));

    conn.from_source = FROM_PREVIOUS_STATE;
    return true;
}

bool set_ports_ini_keyvalue(INI& ini, const char* key, ConnectPorts& conn) {
    if (conn.ports.empty()) {
        return false;
    }

    std::string value = strutil::join_to_string(conn.ports, ",");
    if (value.empty()) {
        xassert2(false);
        return false;
    }

    return ini.Set(key, value);
}

bool parse_ini_unsigned_value(INI& ini, const std::string& key, unsigned& out) {
    std::string svalue = ini.Get(key);
    auto value = static_cast<unsigned>(atoi(svalue.c_str()));
    if (value > 0) {
        out = value;
        return true;
    }
    return false;
}

bool set_ctrl_from_ini_keyvalue(INI& ini, const char* key, ConnectCtrl& ctrl) {
    std::string skey(key);

    if (parse_ini_unsigned_value(ini, skey + ".interval", ctrl.interval_ms)
        && parse_ini_unsigned_value(ini, skey + ".ipv4.timeout", ctrl.ipv4_timeout_ms)
        && parse_ini_unsigned_value(ini, skey + ".ipv6.timeout", ctrl.ipv6_timeout_ms)
        && parse_ini_unsigned_value(ini, skey + ".maxconn", ctrl.maxconn)
        && parse_ini_unsigned_value(ini, skey + ".ipv4.0rtt", ctrl.ipv4_zerortt_check_ms)
        && parse_ini_unsigned_value(ini, skey + ".ipv6.0rtt", ctrl.ipv6_zerortt_check_ms)) {
        ctrl.from_source = FROM_PREVIOUS_STATE;

        xinfo2(TSF "get ini.ctrl %_ interval %_ ipv4 %_ ipv6 %_ maxconn %_ 0rttv4 %_ 0rttv6 %_",
               key,
               ctrl.interval_ms,
               ctrl.ipv4_timeout_ms,
               ctrl.ipv6_timeout_ms,
               ctrl.maxconn,
               ctrl.ipv4_zerortt_check_ms,
               ctrl.ipv6_zerortt_check_ms);
        return true;
    }

    return false;
}

bool set_ctrl_ini_keyvalue(INI& ini, const char* key, ConnectCtrl& ctrl) {
    std::string skey(key);

    bool ret = ini.Set(skey + ".interval", ctrl.interval_ms) && ini.Set(skey + ".ipv4.timeout", ctrl.ipv4_timeout_ms)
               && ini.Set(skey + ".ipv6.timeout", ctrl.ipv6_timeout_ms) && ini.Set(skey + ".maxconn", ctrl.maxconn)
               && ini.Set(skey + ".ipv4.0rtt", ctrl.ipv4_zerortt_check_ms)
               && ini.Set(skey + ".ipv6.0rtt", ctrl.ipv6_zerortt_check_ms);

    xinfo2(TSF "set ini.ctrl %_ interval %_ ipv4 %_ ipv6 %_ maxconn %_ 0rttv4 %_ 0rttv6 %_",
           key,
           ctrl.interval_ms,
           ctrl.ipv4_timeout_ms,
           ctrl.ipv6_timeout_ms,
           ctrl.maxconn,
           ctrl.ipv4_zerortt_check_ms,
           ctrl.ipv6_zerortt_check_ms);
    return ret;
}

};  // namespace

ConnectParams::ConnectParams() {
    _InitDefaultStrategy();
}

void ConnectParams::SetIniFilePath(const std::string& inifile) {
    xinfo2(TSF "inifile %_", inifile);
    inifile_ = inifile;
}

bool ConnectParams::LoadFromINIFile(const std::string& section) {
    xassert2(!inifile_.empty());
    if (inifile_.empty()) {
        xwarn2(TSF "inifile not set");
        return false;
    }

    SpecialINI ini(inifile_);
    if (!ini.Select(section)) {
        xwarn2(TSF "section %_ not exists.", section);
        std::lock_guard<std::mutex> lock(mutex_);
        _InitDefaultStrategy();
        return false;
    }

    return LoadFromINI(ini);
}

bool ConnectParams::LoadFromINI(SpecialINI& ini) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!_LoadFromINIImpl(ini)) {
        _InitDefaultStrategy();
        return false;
    }

    return true;
}

bool ConnectParams::_LoadFromINIImpl(SpecialINI& ini) {
    std::string section = ini.CurrentSection();
    xassert2(!section.empty());
    if (section.empty()) {
        xwarn2(TSF "empty current section");
        return false;
    }

    time_t now = time(nullptr);
    unsigned expire_timetick = ini.Get("strategy.expire", 0);
    xinfo2(TSF "try load section %_ name %_ expire %_ now %_", section, ini.Get("name"), expire_timetick, now);
    if (expire_timetick == 0 || static_cast<unsigned>(now) > expire_timetick) {
        return false;
    }

    // ports
    bool ports_ok = set_ports_from_ini_keyvalue(ini, "strategy.tcp.long.ports", strategy_.ports[TCP_LONGLINK])
                    && set_ports_from_ini_keyvalue(ini, "strategy.tcp.short.ports", strategy_.ports[TCP_SHORTLINK])
                    && set_ports_from_ini_keyvalue(ini, "strategy.quic.long.ports", strategy_.ports[QUIC_LONGLINK])
                    && set_ports_from_ini_keyvalue(ini, "strategy.quic.short.ports", strategy_.ports[QUIC_SHORTLINK]);

    // ctrl
    if (ports_ok) {
        return set_ctrl_from_ini_keyvalue(ini, "strategy.tcp.long", strategy_.ctrls[TCP_LONGLINK])
               && set_ctrl_from_ini_keyvalue(ini, "strategy.tcp.short", strategy_.ctrls[TCP_SHORTLINK])
               && set_ctrl_from_ini_keyvalue(ini, "strategy.quic.long", strategy_.ctrls[QUIC_LONGLINK])
               && set_ctrl_from_ini_keyvalue(ini, "strategy.quic.short", strategy_.ctrls[QUIC_SHORTLINK]);
    }
    return false;
}

void ConnectParams::UpdateToINI(SpecialINI& ini) {
    std::lock_guard<std::mutex> lock(mutex_);
    _UpdateToINIImpl(ini);
}

void ConnectParams::_UpdateToINIImpl(SpecialINI& ini) {
    std::string section = ini.CurrentSection();
    xassert2(!section.empty());
    if (section.empty()) {
        return;
    }

    xinfo2(TSF "try save section %_ name %_ expire %_ now %_",
           section,
           ini.Get("name"),
           strategy_.expire_timetick,
           time(nullptr));

    ini.Set("strategy.expire", strategy_.expire_timetick);
    // ports
    set_ports_ini_keyvalue(ini, "strategy.tcp.long.ports", strategy_.ports[TCP_LONGLINK]);
    set_ports_ini_keyvalue(ini, "strategy.tcp.short.ports", strategy_.ports[TCP_SHORTLINK]);
    set_ports_ini_keyvalue(ini, "strategy.quic.long.ports", strategy_.ports[QUIC_LONGLINK]);
    set_ports_ini_keyvalue(ini, "strategy.quic.short.ports", strategy_.ports[QUIC_SHORTLINK]);

    // ctrl
    set_ctrl_ini_keyvalue(ini, "strategy.tcp.long", strategy_.ctrls[TCP_LONGLINK]);
    set_ctrl_ini_keyvalue(ini, "strategy.tcp.short", strategy_.ctrls[TCP_SHORTLINK]);
    set_ctrl_ini_keyvalue(ini, "strategy.quic.long", strategy_.ctrls[QUIC_LONGLINK]);
    set_ctrl_ini_keyvalue(ini, "strategy.quic.short", strategy_.ctrls[QUIC_SHORTLINK]);
}

bool ConnectParams::UpdateFromXML(tinyxml2::XMLElement* node, SpecialINI& ini) {
    std::lock_guard<std::mutex> lock(mutex_);
    xassert2(node != nullptr);
    if (!node) {
        return false;
    }

    unsigned seconds = 0;
    if (node->QueryUnsignedAttribute("cachetimeout", &seconds) == tinyxml2::XML_SUCCESS && seconds > 0) {
        strategy_.expire_timetick = time(nullptr) + seconds;
    }

    if (parse_connect_port_xml(node, strategy_) && parse_connect_timeout_xml(node, strategy_)) {
        _UpdateToINIImpl(ini);
        return true;
    }

    xwarn2(TSF "parse connect strategy failed");
    return false;
}

ConnectStrategy ConnectParams::CurrentStrategy() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return strategy_;
}

void ConnectParams::_InitDefaultStrategy() {
    xinfo_function();
    strategy_.ports[TCP_LONGLINK].from_source = FROM_DEFAULT;
    strategy_.ports[TCP_LONGLINK].ports = {80, 443, 8080, 5000};
    strategy_.ports[TCP_SHORTLINK].from_source = FROM_DEFAULT;
    strategy_.ports[TCP_SHORTLINK].ports = {80};
    strategy_.ports[QUIC_LONGLINK].from_source = FROM_DEFAULT;
    strategy_.ports[QUIC_LONGLINK].ports = {80};
    strategy_.ports[QUIC_SHORTLINK].from_source = FROM_DEFAULT;
    strategy_.ports[QUIC_SHORTLINK].ports = {80};

    strategy_.ctrls[TCP_LONGLINK].from_source = FROM_DEFAULT;
    strategy_.ctrls[TCP_LONGLINK].interval_ms = kLonglinkConnInteral;
    strategy_.ctrls[TCP_LONGLINK].ipv4_timeout_ms = kLonglinkConnTimeout;
    strategy_.ctrls[TCP_LONGLINK].ipv6_timeout_ms = kLonglinkConnTimeout;
    strategy_.ctrls[TCP_LONGLINK].maxconn = kLonglinkConnMax;

    strategy_.ctrls[TCP_SHORTLINK].from_source = FROM_DEFAULT;
    strategy_.ctrls[TCP_SHORTLINK].interval_ms = kShortlinkConnInterval;
    strategy_.ctrls[TCP_SHORTLINK].ipv4_timeout_ms = kShortlinkConnTimeout;
    strategy_.ctrls[TCP_SHORTLINK].ipv6_timeout_ms = kShortlinkConnTimeout;
    strategy_.ctrls[TCP_SHORTLINK].maxconn = kLonglinkConnMax;

    strategy_.ctrls[QUIC_LONGLINK].from_source = FROM_DEFAULT;
    strategy_.ctrls[QUIC_LONGLINK].interval_ms = 250;
    strategy_.ctrls[QUIC_LONGLINK].ipv4_timeout_ms = 1000;
    strategy_.ctrls[QUIC_LONGLINK].ipv6_timeout_ms = 1000;
    strategy_.ctrls[QUIC_LONGLINK].ipv4_zerortt_check_ms = 300;
    strategy_.ctrls[QUIC_LONGLINK].ipv6_zerortt_check_ms = 400;
    strategy_.ctrls[QUIC_LONGLINK].maxconn = 4;

    strategy_.ctrls[QUIC_LONGLINK].from_source = FROM_DEFAULT;
    strategy_.ctrls[QUIC_SHORTLINK].interval_ms = 250;
    strategy_.ctrls[QUIC_SHORTLINK].ipv4_timeout_ms = 1000;
    strategy_.ctrls[QUIC_SHORTLINK].ipv6_timeout_ms = 1000;
    strategy_.ctrls[QUIC_SHORTLINK].ipv4_zerortt_check_ms = 300;
    strategy_.ctrls[QUIC_SHORTLINK].ipv6_zerortt_check_ms = 400;
    strategy_.ctrls[QUIC_SHORTLINK].maxconn = 4;
}

}  // namespace stn

}  // namespace mars