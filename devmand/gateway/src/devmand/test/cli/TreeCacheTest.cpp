//#define LOG_WITH_GLOG

#include <devmand/channels/cli/Cli.h>
#include <devmand/channels/cli/TreeCacheCli.h>
#include <devmand/test/TestUtils.h>
#include <devmand/test/cli/utils/Log.h>
#include <gtest/gtest.h>
#include <magma_logging.h>

namespace devmand::channels::cli {
using namespace std;

namespace testdata {

static constexpr const char* SH_RUN_UBIQUITI =
    R"template(*Ubiquiti Edgeswitch outdoor configs:*

hostname "MPK14-11-Switch"
network protocol none
network parms 192.168.0.35 255.255.255.0 0.0.0.0
network ipv6 address autoconfig
vlan database
vlan 10,101,302,312,345,307
vlan name 101 "MGMT"
exit

network mgmt_vlan 101
configure
sntp server "10.128.203.238"
sntp server "1.ntp.vip.facebook.com"
clock timezone -7 minutes 0 zone "PST"
ip name server 2620:10d:c097::c0de
username "ubnt" password asdasdasdasdasdasdasdasdasd level 15 encrypted
line console
exit

line ssh
exit

snmp-server sysname "MPK10-06-Switch"
!

interface 0/1
description 'Primary-1'
switchport mode access
switchport access vlan 307
exit


interface 0/2
description 'Secodary-3'
switchport mode access
switchport access vlan 307
exit



interface 0/3
description 'Secondary-2'
switchport mode access
switchport access vlan 307
exit



interface 0/4
description 'Secondary-1'
switchport mode access
switchport access vlan 307
exit



interface 0/5
description 'Primary-2'
switchport mode access
switchport access vlan 307
exit



interface 0/6
switchport mode access
switchport access vlan 307
exit

interface 0/7
switchport mode access
switchport access vlan 101
exit

interface 0/9
description 'SOMA-AP'
switchport mode access
switchport access vlan 101
exit



interface 0/10
description 'MAGMA-CPE'
switchport mode access
switchport access vlan 345
exit


interface 0/14
description 'Ruckus-AP'
switchport mode access
switch access vlan 100
exit

interface 0/15
description 'NanoBeam'
spanning-tree bpdufilter
switchport mode trunk
switchport trunk native vlan 101
switchport trunk allowed vlan 101,307,312,345
poe opmode passive24v
exit

interface 0/16
description 'PDU'
switchport mode access
switchport access vlan 101
exit

copy system:running-config nvram:startup-config

)template";

static constexpr const char* SH_RUN_CISCO = R"template(Building configuration...
Current configuration : 3781 bytes
!
interface FastEthernet0
 ip address 192.1.12.2 255.255.255.0
 no ip directed-broadcast (default)
 ip nat outside
 duplex auto
 speed auto
!
line con 0
 password cisco123
 transport preferred all
 transport output all
line aux 0
 transport preferred all
 transport output all
!
)template";

static constexpr const char* SH_RUN_INT_GI4 = R"template(interface 0/14
description 'Ruckus-AP'
switchport mode access
switch access vlan 100
exit
)template";

static constexpr const char* SH_RUN_TWO_IFC = R"template(

 foo


interface Loopback99
 description bla
!



interface Bundle-Ether103.100
 description TOOL_TEST
 ethernet cfm
  mep domain DML3 service 504 mep-id 1
   cos 6
  !
 !
!
  )template";

static constexpr const char* SH_RUN_UBNT_REAL =
    "\r\n\r\n!Current Configuration:\r\n!\r\n!System Description \"EdgeSwitch 16-Port 150W, 1.8.1.5145168, Linux 3.6.5-1b505fb7, 1.1.0.5102011\"\r\n!System Software Version \"1.8.1.5145168\"\r\n!System Up Time          \"89 days 8 hrs 9 mins 32 secs\"\r\n!Additional Packages     QOS,IPv6 Management,Routing\r\n!Current SNTP Synchronized Time: SNTP Last Attempt Status Is Not Successful\r\n!\r\nnetwork protocol none\r\nnetwork parms 10.19.0.245 255.255.255.0 10.19.0.1\r\nvlan database\r\nvlan 77,99,787\r\nvlan name 99 \"testing\"\r\nvlan name 787 \"another\"\r\nvlan routing 33 1\r\nexit\r\n\r\nip ssh protocol 1 2\r\nconfigure\r\nip name server 10.19.0.1\r\nline console\r\nexit\r\n\r\nline telnet\r\nexit\r\n\r\nline ssh\r\nexit\r\n\r\n!\r\n\r\ninterface 0/5\r\nshutdown\r\ndescription 'testing'\r\nexit\r\n\r\n\r\n\r\ninterface 0/7\r\nswitchport access vlan 787\r\nexit\r\n\r\n\r\n\r\ninterface 0/8\r\nswitchport trunk allowed vlan 77,787\r\nexit\r\n\r\n\r\n\r\ninterface 0/9\r\nswitchport trunk native vlan 77\r\nswitchport trunk allowed vlan 787\r\nexit\r\n\r\n\r\n\r\ninterface 0/10\r\nswitchport mode trunk\r\nswitchport trunk allowed vlan 33\r\nexit\r\n\r\n\r\n\r\ninterface 0/11\r\nswitchport access vlan 77\r\nswitchport trunk allowed vlan 1-100\r\nexit\r\n\r\n\r\n\r\ninterface vlan 33\r\nshutdown\r\ndescription 'routedVlan'\r\nrouting\r\nexit\r\n\r\n\r\nexit\r\n\r\n\r\n";
} // namespace testdata

static const char* showRunningCommand = "show running-config";

static const shared_ptr<CliFlavour> ubiquitiFlavour =
    CliFlavour::create(UBIQUITI);

static const shared_ptr<CliFlavour> ciscoFlavour = CliFlavour::create("cisco");

class TreeCacheTest : public ::testing::Test {
 protected:
  unique_ptr<TreeCache> tested_ubiquiti;
  unique_ptr<TreeCache> tested_cisco;

  void SetUp() override {
    devmand::test::utils::log::initLog();

    tested_ubiquiti = make_unique<TreeCache>(ubiquitiFlavour);
    tested_ubiquiti->update(testdata::SH_RUN_UBIQUITI);
    EXPECT_EQ(15, tested_ubiquiti->size());

    tested_cisco = make_unique<TreeCache>(ciscoFlavour);
    tested_cisco->update(testdata::SH_RUN_CISCO);
    EXPECT_EQ(3, tested_cisco->size());
  }
};

TEST_F(TreeCacheTest, createSectionPattern_noIndent) {
  auto content_chars = R"template(section1
foo
exit
section2
bar
baz
exit
)template";
  string content(content_chars);

  string sectionPattern = tested_ubiquiti->createSectionPattern(0);
  EXPECT_EQ("\n(\\S[^]*?\nexit\n)", sectionPattern);
  regex regexMatch = regex(sectionPattern);
  smatch sm;

  EXPECT_TRUE(tested_ubiquiti->hasNextSection(regexMatch, sm, content));
  EXPECT_EQ(sm[1], "section1\nfoo\nexit\n");

  EXPECT_TRUE(tested_ubiquiti->hasNextSection(regexMatch, sm, content));
  EXPECT_EQ(sm[1], "section2\nbar\nbaz\nexit\n");

  EXPECT_FALSE(tested_ubiquiti->hasNextSection(regexMatch, sm, content));
}

TEST_F(TreeCacheTest, createSectionPattern_noNewLineAtStart) {
  string content = "section1\nfoo\nexit\n";
  regex regexMatch = regex(tested_ubiquiti->createSectionPattern(0));
  smatch sm;

  EXPECT_TRUE(tested_ubiquiti->hasNextSection(regexMatch, sm, content));
  EXPECT_EQ(sm[1], "section1\nfoo\nexit\n");
  EXPECT_FALSE(tested_ubiquiti->hasNextSection(regexMatch, sm, content));
}

TEST_F(TreeCacheTest, createSectionPattern_noNewLine) {
  string content = "section1";
  regex regexMatch = regex(tested_ubiquiti->createSectionPattern(0));
  smatch sm;

  EXPECT_FALSE(tested_ubiquiti->hasNextSection(regexMatch, sm, content));
}

TEST_F(TreeCacheTest, createSectionPattern_indent) {
  string content(testdata::SH_RUN_TWO_IFC);

  string sectionPattern = tested_ubiquiti->createSectionPattern(' ', "!", 0);

  regex regexMatch = regex(sectionPattern);
  smatch sm;

  EXPECT_TRUE(tested_ubiquiti->hasNextSection(regexMatch, sm, content));
  EXPECT_EQ(
      sm[1],
      "interface Loopback99\n"
      " description bla\n"
      "!\n");
  EXPECT_TRUE(tested_ubiquiti->hasNextSection(regexMatch, sm, content));
  EXPECT_EQ(
      sm[1],
      "interface Bundle-Ether103.100\n"
      " description TOOL_TEST\n"
      " ethernet cfm\n"
      "  mep domain DML3 service 504 mep-id 1\n"
      "   cos 6\n"
      "  !\n"
      " !\n"
      "!\n");
  EXPECT_FALSE(tested_ubiquiti->hasNextSection(regexMatch, sm, content));
  EXPECT_FALSE(tested_ubiquiti->hasNextSection(regexMatch, sm, content));
}

TEST_F(TreeCacheTest, readConfigurationToMap_ubiquiti) {
  string content(testdata::SH_RUN_INT_GI4);
  map<vector<string>, string> actual =
      tested_ubiquiti->readConfigurationToMap(content);
  EXPECT_EQ(actual.size(), 1);
  map<vector<string>, string> expected;

  vector<string> key = {"interface", "0/14"};
  string value =
      "interface 0/14\n"
      "description 'Ruckus-AP'\n"
      "switchport mode access\n"
      "switch access vlan 100\n"
      "exit\n";
  expected.insert(make_pair(key, value));
  EXPECT_EQ(actual, expected);
}

TEST_F(TreeCacheTest, readConfigurationToMap_cisco) {
  string content(testdata::SH_RUN_TWO_IFC);
  map<vector<string>, string> actual =
      tested_cisco->readConfigurationToMap(content);
  EXPECT_EQ(actual.size(), 2);

  map<vector<string>, string> expected;
  {
    vector<string> key = {"interface", "Loopback99"};
    string value =
        "interface Loopback99\n"
        " description bla\n"
        "!\n";
    expected.insert(make_pair(key, value));
  }
  {
    vector<string> key = {"interface", "Bundle-Ether103.100"};
    string value =
        "interface Bundle-Ether103.100\n"
        " description TOOL_TEST\n"
        " ethernet cfm\n"
        "  mep domain DML3 service 504 mep-id 1\n"
        "   cos 6\n"
        "  !\n"
        " !\n"
        "!\n";
    expected.insert(make_pair(key, value));
  }
  EXPECT_EQ(actual, expected);
}

TEST_F(TreeCacheTest, splitSupportedCommand) {
  Optional<pair<string, string>> split = tested_ubiquiti->splitSupportedCommand(
      "show running-config interface 0/14");
  EXPECT_TRUE(split);
  EXPECT_EQ("show running-config", split->first);
  EXPECT_EQ("interface 0/14", split->second);
}

// public api tests:

TEST_F(TreeCacheTest, parseUnknownCommand) {
  auto nothing = tested_ubiquiti->parseCommand("foo");
  EXPECT_FALSE(nothing);
}

TEST_F(TreeCacheTest, getWholeConfig_expectException) {
  Optional<pair<string, vector<string>>> maybeCmd =
      tested_ubiquiti->parseCommand(showRunningCommand);
  EXPECT_TRUE(maybeCmd);
  EXPECT_EQ(maybeCmd->second.size(), 0);
  EXPECT_THROW(tested_ubiquiti->getSection(maybeCmd.value()), runtime_error);
}

TEST_F(TreeCacheTest, getParticularIfc) {
  Optional<pair<string, vector<string>>> maybeCmd =
      tested_ubiquiti->parseCommand("sh run interface 0/14");
  EXPECT_TRUE(maybeCmd);
  EXPECT_EQ(maybeCmd->second.size(), 2);
  Optional<string> wholeConfig = tested_ubiquiti->getSection(maybeCmd.value());
  EXPECT_TRUE(wholeConfig);
  EXPECT_EQ(testdata::SH_RUN_INT_GI4, wholeConfig.value());
}

TEST_F(TreeCacheTest, parseSH_RUN_UBNT_REAL) {
  tested_ubiquiti->clear();
  tested_ubiquiti->update(testdata::SH_RUN_UBNT_REAL);
  EXPECT_EQ(tested_ubiquiti->size(), 11);
  EXPECT_EQ(
      tested_ubiquiti->toString(),
      "(11)["
      "{!},"
      "{!Current}{Configuration:},"
      "{interface}{0/10},"
      "{interface}{0/11},"
      "{interface}{0/7},"
      "{interface}{0/8},"
      "{interface}{0/9},"
      "{interface}{vlan}{33},"
      "{ip}{ssh}{protocol}{1}{2},"
      "{line}{ssh},{line}{telnet},"
      "]");
}

} // namespace devmand::channels::cli
