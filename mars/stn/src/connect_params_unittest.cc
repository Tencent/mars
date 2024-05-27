
#include "connect_params.h"

#include <vector>

#include "gtest/gtest.h"
#include "mars/boost/filesystem.hpp"
#include "mars/boost/system/system_error.hpp"
#include "mars/comm/tinyxml2.h"
#include "special_ini.h"
#include "stn/config.h"
#include "stn/stn.h"

using namespace mars::stn;
using namespace tinyxml2;
using namespace boost;

TEST(ConnectParams, parseFullxml) {
    const char* xml = R"(
    <?xml version="1.0" encoding="utf-8"?>
    <connstrategy  cachetimeout="604800">
		<portlist type="long">
			<port>80</port>
			<port>443</port>
			<port>8080</port>
			<port>5000</port>
		</portlist>
			<portlist type="short">
			<port>80</port>
		</portlist>
			<portlist type="quic">
			<port>80</port>
		</portlist>
		<connctrl type="long" interval="2500" conntimeout="2500" maxconnnum="3">
		</connctrl>
		<connctrl type="short" interval="2500" conntimeout="3000" maxconnnum="3">
			<conntimeout property="ipv4">2000</conntimeout>
			<conntimeout property="ipv6">2500</conntimeout>
		</connctrl>
		<connctrl type="quic" interval="1000" conntimeout="1000" maxconnnum="3" zerortttimeout="255">
			<conntimeout property="ipv6">1500</conntimeout>
			<zerortttimeout property="ipv4">300</zerortttimeout>
			<zerortttimeout property="ipv6">400</zerortttimeout>
		</connctrl>
	</connstrategy>
)";

    filesystem::path inipath = filesystem::absolute("./connect_params1.ini");
    SpecialINI ini(inipath.c_str());

    ASSERT_TRUE(ini.Select("default") || ini.Create("default"));  // default network
    ASSERT_TRUE(!ini.CurrentSection().empty());

    tinyxml2::XMLDocument xmldoc;
    EXPECT_EQ(xmldoc.Parse(xml), XML_SUCCESS);

    auto* node = xmldoc.FirstChildElement("connstrategy");
    EXPECT_NE(node, nullptr);

    ConnectParams cp;
    ASSERT_TRUE(cp.UpdateFromXML(node, ini));

    auto strategy = cp.CurrentStrategy();
    EXPECT_TRUE(strategy.expire_timetick > 0);

    // tcp short
    const auto& tcp_short_ports = strategy.ports[TCP_SHORTLINK];
    EXPECT_EQ(tcp_short_ports.from_source, FROM_SERVER);
    EXPECT_EQ(tcp_short_ports.ports.size(), 1);
    EXPECT_EQ(tcp_short_ports.ports.front(), 80);

    // quic short
    const auto& quic_short_ports = strategy.ports[QUIC_SHORTLINK];
    EXPECT_EQ(quic_short_ports.from_source, FROM_SERVER);
    EXPECT_EQ(quic_short_ports.ports.size(), 1);
    EXPECT_EQ(quic_short_ports.ports.front(), 80);

    // tcp long
    const auto& tcp_long_ports = strategy.ports[TCP_LONGLINK];
    EXPECT_EQ(tcp_long_ports.from_source, FROM_SERVER);
    EXPECT_EQ(tcp_long_ports.ports.size(), 4);
    EXPECT_EQ(tcp_long_ports.ports.front(), 80);
    EXPECT_EQ(tcp_long_ports.ports[2], 8080);

    // quic long
    const auto& quic_long_ports = strategy.ports[QUIC_LONGLINK];
    EXPECT_EQ(quic_long_ports.from_source, FROM_DEFAULT);
    EXPECT_EQ(quic_long_ports.ports.size(), 1);
    EXPECT_EQ(quic_long_ports.ports.front(), 80);

    // CTRL
    const auto& tcp_short_ctrl = strategy.ctrls[TCP_SHORTLINK];
    EXPECT_EQ(tcp_short_ctrl.from_source, FROM_SERVER);
    EXPECT_EQ(tcp_short_ctrl.interval_ms, 2500);
    EXPECT_EQ(tcp_short_ctrl.maxconn, 3);
    EXPECT_EQ(tcp_short_ctrl.ipv4_timeout_ms, 2000);
    EXPECT_EQ(tcp_short_ctrl.ipv6_timeout_ms, 2500);
    EXPECT_EQ(tcp_short_ctrl.ipv4_zerortt_check_ms, 300);
    EXPECT_EQ(tcp_short_ctrl.ipv6_zerortt_check_ms, 400);

    const auto& tcp_long_ctrl = strategy.ctrls[TCP_LONGLINK];
    EXPECT_EQ(tcp_long_ctrl.from_source, FROM_SERVER);
    EXPECT_EQ(tcp_long_ctrl.interval_ms, 2500);
    EXPECT_EQ(tcp_long_ctrl.maxconn, 3);
    EXPECT_EQ(tcp_long_ctrl.ipv4_timeout_ms, 2500);
    EXPECT_EQ(tcp_long_ctrl.ipv6_timeout_ms, 2500);
    EXPECT_EQ(tcp_short_ctrl.ipv4_zerortt_check_ms, 300);
    EXPECT_EQ(tcp_short_ctrl.ipv6_zerortt_check_ms, 400);

    const auto& quic_short_ctrl = strategy.ctrls[QUIC_SHORTLINK];
    EXPECT_EQ(quic_short_ctrl.from_source, FROM_SERVER);
    EXPECT_EQ(quic_short_ctrl.interval_ms, 1000);
    EXPECT_EQ(quic_short_ctrl.maxconn, 3);
    EXPECT_EQ(quic_short_ctrl.ipv4_timeout_ms, 1000);
    EXPECT_EQ(quic_short_ctrl.ipv6_timeout_ms, 1500);
    EXPECT_EQ(quic_short_ctrl.ipv4_zerortt_check_ms, 300);
    EXPECT_EQ(quic_short_ctrl.ipv6_zerortt_check_ms, 400);

    const auto& quic_long_ctrl = strategy.ctrls[QUIC_LONGLINK];
    EXPECT_EQ(quic_long_ctrl.from_source, FROM_DEFAULT);
    EXPECT_EQ(quic_long_ctrl.interval_ms, 250);
    EXPECT_EQ(quic_long_ctrl.maxconn, 4);
    EXPECT_EQ(quic_long_ctrl.ipv4_timeout_ms, 1000);
    EXPECT_EQ(quic_long_ctrl.ipv6_timeout_ms, 1000);
    EXPECT_EQ(quic_long_ctrl.ipv4_zerortt_check_ms, 300);
    EXPECT_EQ(quic_long_ctrl.ipv6_zerortt_check_ms, 400);

    ini.Save();
}

TEST(ConnectParams, parsePartxml) {
    const char* xml = R"(
    <?xml version="1.0" encoding="utf-8"?>
    <connstrategy  cachetimeout="604800">
		<portlist type="long">
			<port>80</port>
			<port>443</port>
			<port>8080</port>
			<port>5000</port>
		</portlist>
			<portlist type="short">
			<port>80</port>
		</portlist>
			<portlist type="quic">
			<port>80</port>
		</portlist>
		<connctrl type="long" interval="2500" conntimeout="2500" maxconnnum="3">
		</connctrl>
		<connctrl type="short" interval="2500" conntimeout="3000" maxconnnum="3">
		</connctrl>
		<connctrl type="quic" interval="1000" conntimeout="1000" maxconnnum="3" zerortttimeout="255">
		</connctrl>
	</connstrategy>
)";

    filesystem::path inipath = filesystem::absolute("./connect_params2.ini");
    SpecialINI ini(inipath.c_str());

    ASSERT_TRUE(ini.Select("default") || ini.Create("default"));  // default network
    ASSERT_TRUE(!ini.CurrentSection().empty());

    tinyxml2::XMLDocument xmldoc;
    EXPECT_EQ(xmldoc.Parse(xml), XML_SUCCESS);

    auto* node = xmldoc.FirstChildElement("connstrategy");
    EXPECT_NE(node, nullptr);

    ConnectParams cp;
    ASSERT_TRUE(cp.UpdateFromXML(node, ini));

    auto strategy = cp.CurrentStrategy();
    EXPECT_TRUE(strategy.expire_timetick > 0);

    // tcp short
    const auto& tcp_short_ports = strategy.ports[TCP_SHORTLINK];
    EXPECT_EQ(tcp_short_ports.from_source, FROM_SERVER);
    EXPECT_EQ(tcp_short_ports.ports.size(), 1);
    EXPECT_EQ(tcp_short_ports.ports.front(), 80);

    // quic short
    const auto& quic_short_ports = strategy.ports[QUIC_SHORTLINK];
    EXPECT_EQ(quic_short_ports.from_source, FROM_SERVER);
    EXPECT_EQ(quic_short_ports.ports.size(), 1);
    EXPECT_EQ(quic_short_ports.ports.front(), 80);

    // tcp long
    const auto& tcp_long_ports = strategy.ports[TCP_LONGLINK];
    EXPECT_EQ(tcp_long_ports.from_source, FROM_SERVER);
    EXPECT_EQ(tcp_long_ports.ports.size(), 4);
    EXPECT_EQ(tcp_long_ports.ports.front(), 80);
    EXPECT_EQ(tcp_long_ports.ports[2], 8080);

    // quic long
    const auto& quic_long_ports = strategy.ports[QUIC_LONGLINK];
    EXPECT_EQ(quic_long_ports.from_source, FROM_DEFAULT);
    EXPECT_EQ(quic_long_ports.ports.size(), 1);
    EXPECT_EQ(quic_long_ports.ports.front(), 80);

    // CTRL
    const auto& tcp_short_ctrl = strategy.ctrls[TCP_SHORTLINK];
    EXPECT_EQ(tcp_short_ctrl.from_source, FROM_SERVER);
    EXPECT_EQ(tcp_short_ctrl.interval_ms, 2500);
    EXPECT_EQ(tcp_short_ctrl.maxconn, 3);
    EXPECT_EQ(tcp_short_ctrl.ipv4_timeout_ms, 3000);
    EXPECT_EQ(tcp_short_ctrl.ipv6_timeout_ms, 3000);
    EXPECT_EQ(tcp_short_ctrl.ipv4_zerortt_check_ms, 300);
    EXPECT_EQ(tcp_short_ctrl.ipv6_zerortt_check_ms, 400);

    const auto& tcp_long_ctrl = strategy.ctrls[TCP_LONGLINK];
    EXPECT_EQ(tcp_long_ctrl.from_source, FROM_SERVER);
    EXPECT_EQ(tcp_long_ctrl.interval_ms, 2500);
    EXPECT_EQ(tcp_long_ctrl.maxconn, 3);
    EXPECT_EQ(tcp_long_ctrl.ipv4_timeout_ms, 2500);
    EXPECT_EQ(tcp_long_ctrl.ipv6_timeout_ms, 2500);
    EXPECT_EQ(tcp_short_ctrl.ipv4_zerortt_check_ms, 300);
    EXPECT_EQ(tcp_short_ctrl.ipv6_zerortt_check_ms, 400);

    const auto& quic_short_ctrl = strategy.ctrls[QUIC_SHORTLINK];
    EXPECT_EQ(quic_short_ctrl.from_source, FROM_SERVER);
    EXPECT_EQ(quic_short_ctrl.interval_ms, 1000);
    EXPECT_EQ(quic_short_ctrl.maxconn, 3);
    EXPECT_EQ(quic_short_ctrl.ipv4_timeout_ms, 1000);
    EXPECT_EQ(quic_short_ctrl.ipv6_timeout_ms, 1000);
    EXPECT_EQ(quic_short_ctrl.ipv4_zerortt_check_ms, 255);
    EXPECT_EQ(quic_short_ctrl.ipv6_zerortt_check_ms, 255);

    const auto& quic_long_ctrl = strategy.ctrls[QUIC_LONGLINK];
    EXPECT_EQ(quic_long_ctrl.from_source, FROM_DEFAULT);
    EXPECT_EQ(quic_long_ctrl.interval_ms, 250);
    EXPECT_EQ(quic_long_ctrl.maxconn, 4);
    EXPECT_EQ(quic_long_ctrl.ipv4_timeout_ms, 1000);
    EXPECT_EQ(quic_long_ctrl.ipv6_timeout_ms, 1000);
    EXPECT_EQ(quic_long_ctrl.ipv4_zerortt_check_ms, 300);
    EXPECT_EQ(quic_long_ctrl.ipv6_zerortt_check_ms, 400);

    ini.Save();
}

TEST(ConnectParams, parsePartServerxml) {
    const char* xml = R"(
    <?xml version="1.0" encoding="utf-8"?>
    <connstrategy  cachetimeout="604800">
		<portlist type="long">
			<port>80</port>
			<port>443</port>
		</portlist>
		<portlist type="short">
			<port>80</port>
		</portlist>
		<connctrl type="long" interval="2500">
		</connctrl>
		<connctrl type="short" conntimeout="5000" maxconnnum="6" zerortttimeout="1255">
		</connctrl>
		<connctrl type="quic" interval="300" conntimeout="1000">
		</connctrl>
	</connstrategy>
)";

    filesystem::path inipath = filesystem::absolute("./connect_params3.ini");
    SpecialINI ini(inipath.c_str());

    ASSERT_TRUE(ini.Select("default") || ini.Create("default"));  // default network
    ASSERT_TRUE(!ini.CurrentSection().empty());

    tinyxml2::XMLDocument xmldoc;
    EXPECT_EQ(xmldoc.Parse(xml), XML_SUCCESS);

    auto* node = xmldoc.FirstChildElement("connstrategy");
    EXPECT_NE(node, nullptr);

    ConnectParams cp;
    ASSERT_TRUE(cp.UpdateFromXML(node, ini));

    auto strategy = cp.CurrentStrategy();
    EXPECT_TRUE(strategy.expire_timetick > 0);

    // tcp short
    const auto& tcp_short_ports = strategy.ports[TCP_SHORTLINK];
    EXPECT_EQ(tcp_short_ports.from_source, FROM_SERVER);
    EXPECT_EQ(tcp_short_ports.ports.size(), 1);
    EXPECT_EQ(tcp_short_ports.ports.front(), 80);

    // quic short
    const auto& quic_short_ports = strategy.ports[QUIC_SHORTLINK];
    EXPECT_EQ(quic_short_ports.from_source, FROM_DEFAULT);
    EXPECT_EQ(quic_short_ports.ports.size(), 1);
    EXPECT_EQ(quic_short_ports.ports.front(), 80);

    // tcp long
    const auto& tcp_long_ports = strategy.ports[TCP_LONGLINK];
    EXPECT_EQ(tcp_long_ports.from_source, FROM_SERVER);
    EXPECT_EQ(tcp_long_ports.ports.size(), 2);
    EXPECT_EQ(tcp_long_ports.ports.front(), 80);
    EXPECT_EQ(tcp_long_ports.ports[1], 443);

    // quic long
    const auto& quic_long_ports = strategy.ports[QUIC_LONGLINK];
    EXPECT_EQ(quic_long_ports.from_source, FROM_DEFAULT);
    EXPECT_EQ(quic_long_ports.ports.size(), 1);
    EXPECT_EQ(quic_long_ports.ports.front(), 80);

    // CTRL
    const auto& tcp_short_ctrl = strategy.ctrls[TCP_SHORTLINK];
    EXPECT_EQ(tcp_short_ctrl.from_source, FROM_PART_SERVER);
    EXPECT_EQ(tcp_short_ctrl.interval_ms, 2500);
    EXPECT_EQ(tcp_short_ctrl.maxconn, 6);
    EXPECT_EQ(tcp_short_ctrl.ipv4_timeout_ms, 5000);
    EXPECT_EQ(tcp_short_ctrl.ipv6_timeout_ms, 5000);
    EXPECT_EQ(tcp_short_ctrl.ipv4_zerortt_check_ms, 1255);
    EXPECT_EQ(tcp_short_ctrl.ipv6_zerortt_check_ms, 1255);

    const auto& tcp_long_ctrl = strategy.ctrls[TCP_LONGLINK];
    EXPECT_EQ(tcp_long_ctrl.from_source, FROM_PART_SERVER);
    EXPECT_EQ(tcp_long_ctrl.interval_ms, 2500);
    EXPECT_EQ(tcp_long_ctrl.maxconn, 3);
    EXPECT_EQ(tcp_long_ctrl.ipv4_timeout_ms, kLonglinkConnTimeout);
    EXPECT_EQ(tcp_long_ctrl.ipv6_timeout_ms, kLonglinkConnTimeout);
    EXPECT_EQ(tcp_short_ctrl.ipv4_zerortt_check_ms, 1255);
    EXPECT_EQ(tcp_short_ctrl.ipv6_zerortt_check_ms, 1255);

    const auto& quic_short_ctrl = strategy.ctrls[QUIC_SHORTLINK];
    EXPECT_EQ(quic_short_ctrl.from_source, FROM_PART_SERVER);
    EXPECT_EQ(quic_short_ctrl.interval_ms, 300);
    EXPECT_EQ(quic_short_ctrl.maxconn, 3);
    EXPECT_EQ(quic_short_ctrl.ipv4_timeout_ms, 1000);
    EXPECT_EQ(quic_short_ctrl.ipv6_timeout_ms, 1000);
    EXPECT_EQ(quic_short_ctrl.ipv4_zerortt_check_ms, 300);
    EXPECT_EQ(quic_short_ctrl.ipv6_zerortt_check_ms, 400);

    const auto& quic_long_ctrl = strategy.ctrls[QUIC_LONGLINK];
    EXPECT_EQ(quic_long_ctrl.from_source, FROM_DEFAULT);
    EXPECT_EQ(quic_long_ctrl.interval_ms, 250);
    EXPECT_EQ(quic_long_ctrl.maxconn, 4);
    EXPECT_EQ(quic_long_ctrl.ipv4_timeout_ms, 1000);
    EXPECT_EQ(quic_long_ctrl.ipv6_timeout_ms, 1000);
    EXPECT_EQ(quic_long_ctrl.ipv4_zerortt_check_ms, 300);
    EXPECT_EQ(quic_long_ctrl.ipv6_zerortt_check_ms, 400);

    ini.Save();
}

TEST(ConnectParams, loadiniTestwifi) {
    filesystem::path inipath = filesystem::absolute("./connect_params_wifi.ini");
    ConnectParams cp;
    cp.SetIniFilePath(inipath.c_str());
    ASSERT_TRUE(cp.LoadFromINIFile("testwifi"));

    auto strategy = cp.CurrentStrategy();

    // tcp short
    const auto& tcp_short_ports = strategy.ports[TCP_SHORTLINK];
    EXPECT_EQ(tcp_short_ports.from_source, FROM_PREVIOUS_STATE);
    EXPECT_EQ(tcp_short_ports.ports.size(), 1);
    EXPECT_EQ(tcp_short_ports.ports.front(), 801);

    // quic short
    const auto& quic_short_ports = strategy.ports[QUIC_SHORTLINK];
    EXPECT_EQ(quic_short_ports.from_source, FROM_PREVIOUS_STATE);
    EXPECT_EQ(quic_short_ports.ports.size(), 1);
    EXPECT_EQ(quic_short_ports.ports.front(), 1080);

    // tcp long
    const auto& tcp_long_ports = strategy.ports[TCP_LONGLINK];
    EXPECT_EQ(tcp_long_ports.from_source, FROM_PREVIOUS_STATE);
    EXPECT_EQ(tcp_long_ports.ports.size(), 4);
    EXPECT_EQ(tcp_long_ports.ports.front(), 8022);
    EXPECT_EQ(tcp_long_ports.ports[2], 8080);

    // quic long
    const auto& quic_long_ports = strategy.ports[QUIC_LONGLINK];
    EXPECT_EQ(quic_long_ports.from_source, FROM_PREVIOUS_STATE);
    EXPECT_EQ(quic_long_ports.ports.size(), 1);
    EXPECT_EQ(quic_long_ports.ports.front(), 80);

    // CTRL
    const auto& tcp_short_ctrl = strategy.ctrls[TCP_SHORTLINK];
    EXPECT_EQ(tcp_short_ctrl.from_source, FROM_PREVIOUS_STATE);
    EXPECT_EQ(tcp_short_ctrl.interval_ms, 2500);
    EXPECT_EQ(tcp_short_ctrl.maxconn, 3);
    EXPECT_EQ(tcp_short_ctrl.ipv4_timeout_ms, 12000);
    EXPECT_EQ(tcp_short_ctrl.ipv6_timeout_ms, 22500);
    EXPECT_EQ(tcp_short_ctrl.ipv4_zerortt_check_ms, 300);
    EXPECT_EQ(tcp_short_ctrl.ipv6_zerortt_check_ms, 400);

    const auto& tcp_long_ctrl = strategy.ctrls[TCP_LONGLINK];
    EXPECT_EQ(tcp_long_ctrl.from_source, FROM_PREVIOUS_STATE);
    EXPECT_EQ(tcp_long_ctrl.interval_ms, 2500);
    EXPECT_EQ(tcp_long_ctrl.maxconn, 3);
    EXPECT_EQ(tcp_long_ctrl.ipv4_timeout_ms, 2500);
    EXPECT_EQ(tcp_long_ctrl.ipv6_timeout_ms, 2500);
    EXPECT_EQ(tcp_long_ctrl.ipv4_zerortt_check_ms, 1300);
    EXPECT_EQ(tcp_long_ctrl.ipv6_zerortt_check_ms, 400);

    const auto& quic_short_ctrl = strategy.ctrls[QUIC_SHORTLINK];
    EXPECT_EQ(quic_short_ctrl.from_source, FROM_PREVIOUS_STATE);
    EXPECT_EQ(quic_short_ctrl.interval_ms, 1000);
    EXPECT_EQ(quic_short_ctrl.maxconn, 3);
    EXPECT_EQ(quic_short_ctrl.ipv4_timeout_ms, 1000);
    EXPECT_EQ(quic_short_ctrl.ipv6_timeout_ms, 1500);
    EXPECT_EQ(quic_short_ctrl.ipv4_zerortt_check_ms, 300);
    EXPECT_EQ(quic_short_ctrl.ipv6_zerortt_check_ms, 4100);

    const auto& quic_long_ctrl = strategy.ctrls[QUIC_LONGLINK];
    EXPECT_EQ(quic_long_ctrl.from_source, FROM_PREVIOUS_STATE);
    EXPECT_EQ(quic_long_ctrl.interval_ms, 150);
    EXPECT_EQ(quic_long_ctrl.maxconn, 8);
    EXPECT_EQ(quic_long_ctrl.ipv4_timeout_ms, 10001);
    EXPECT_EQ(quic_long_ctrl.ipv6_timeout_ms, 10100);
    EXPECT_EQ(quic_long_ctrl.ipv4_zerortt_check_ms, 1300);
    EXPECT_EQ(quic_long_ctrl.ipv6_zerortt_check_ms, 4002);
}

TEST(ConnectParams, loadiniSpecialwifi) {
    filesystem::path inipath = filesystem::absolute("./connect_params_wifi.ini");
    SpecialINI ini(inipath.c_str());
    ASSERT_TRUE(ini.Select("wifi - 3333 -"));

    ConnectParams cp;
    ASSERT_TRUE(cp.LoadFromINI(ini));

    auto strategy = cp.CurrentStrategy();

    // tcp short
    const auto& tcp_short_ports = strategy.ports[TCP_SHORTLINK];
    EXPECT_EQ(tcp_short_ports.from_source, FROM_PREVIOUS_STATE);
    EXPECT_EQ(tcp_short_ports.ports.size(), 1);
    EXPECT_EQ(tcp_short_ports.ports.front(), 80);

    // quic short
    const auto& quic_short_ports = strategy.ports[QUIC_SHORTLINK];
    EXPECT_EQ(quic_short_ports.from_source, FROM_PREVIOUS_STATE);
    EXPECT_EQ(quic_short_ports.ports.size(), 1);
    EXPECT_EQ(quic_short_ports.ports.front(), 80);

    // tcp long
    const auto& tcp_long_ports = strategy.ports[TCP_LONGLINK];
    EXPECT_EQ(tcp_long_ports.from_source, FROM_PREVIOUS_STATE);
    EXPECT_EQ(tcp_long_ports.ports.size(), 4);
    EXPECT_EQ(tcp_long_ports.ports.front(), 80);
    EXPECT_EQ(tcp_long_ports.ports[2], 8080);

    // quic long
    const auto& quic_long_ports = strategy.ports[QUIC_LONGLINK];
    EXPECT_EQ(quic_long_ports.from_source, FROM_PREVIOUS_STATE);
    EXPECT_EQ(quic_long_ports.ports.size(), 1);
    EXPECT_EQ(quic_long_ports.ports.front(), 80);

    // CTRL
    const auto& tcp_short_ctrl = strategy.ctrls[TCP_SHORTLINK];
    EXPECT_EQ(tcp_short_ctrl.from_source, FROM_PREVIOUS_STATE);
    EXPECT_EQ(tcp_short_ctrl.interval_ms, 2500);
    EXPECT_EQ(tcp_short_ctrl.maxconn, 3);
    EXPECT_EQ(tcp_short_ctrl.ipv4_timeout_ms, 2000);
    EXPECT_EQ(tcp_short_ctrl.ipv6_timeout_ms, 2500);
    EXPECT_EQ(tcp_short_ctrl.ipv4_zerortt_check_ms, 300);
    EXPECT_EQ(tcp_short_ctrl.ipv6_zerortt_check_ms, 400);

    const auto& tcp_long_ctrl = strategy.ctrls[TCP_LONGLINK];
    EXPECT_EQ(tcp_long_ctrl.from_source, FROM_PREVIOUS_STATE);
    EXPECT_EQ(tcp_long_ctrl.interval_ms, 2500);
    EXPECT_EQ(tcp_long_ctrl.maxconn, 3);
    EXPECT_EQ(tcp_long_ctrl.ipv4_timeout_ms, 2500);
    EXPECT_EQ(tcp_long_ctrl.ipv6_timeout_ms, 2500);
    EXPECT_EQ(tcp_long_ctrl.ipv4_zerortt_check_ms, 300);
    EXPECT_EQ(tcp_long_ctrl.ipv6_zerortt_check_ms, 400);

    const auto& quic_short_ctrl = strategy.ctrls[QUIC_SHORTLINK];
    EXPECT_EQ(quic_short_ctrl.from_source, FROM_PREVIOUS_STATE);
    EXPECT_EQ(quic_short_ctrl.interval_ms, 1000);
    EXPECT_EQ(quic_short_ctrl.maxconn, 3);
    EXPECT_EQ(quic_short_ctrl.ipv4_timeout_ms, 1000);
    EXPECT_EQ(quic_short_ctrl.ipv6_timeout_ms, 1500);
    EXPECT_EQ(quic_short_ctrl.ipv4_zerortt_check_ms, 300);
    EXPECT_EQ(quic_short_ctrl.ipv6_zerortt_check_ms, 400);

    const auto& quic_long_ctrl = strategy.ctrls[QUIC_LONGLINK];
    EXPECT_EQ(quic_long_ctrl.from_source, FROM_PREVIOUS_STATE);
    EXPECT_EQ(quic_long_ctrl.interval_ms, 250);
    EXPECT_EQ(quic_long_ctrl.maxconn, 4);
    EXPECT_EQ(quic_long_ctrl.ipv4_timeout_ms, 1000);
    EXPECT_EQ(quic_long_ctrl.ipv6_timeout_ms, 1000);
    EXPECT_EQ(quic_long_ctrl.ipv4_zerortt_check_ms, 300);
    EXPECT_EQ(quic_long_ctrl.ipv6_zerortt_check_ms, 400);
}

TEST(ConnectParams, defaultcfg) {
}

EXPORT_GTEST_SYMBOLS(ConnectParams_unittest)