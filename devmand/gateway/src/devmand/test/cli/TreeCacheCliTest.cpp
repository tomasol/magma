#define LOG_WITH_GLOG

#include <magma_logging.h>

#include <devmand/cartography/DeviceConfig.h>
#include <devmand/channels/cli/Cli.h>
#include <devmand/channels/cli/TreeCacheCli.h>
#include <devmand/test/cli/utils/Log.h>
#include <folly/executors/CPUThreadPoolExecutor.h>
#include <folly/executors/ThreadedExecutor.h>
#include <folly/futures/ThreadWheelTimekeeper.h>
#include <gtest/gtest.h>
#include <chrono>

namespace devmand::channels::cli {

using namespace std;
using namespace std::chrono_literals;
using folly::CPUThreadPoolExecutor;

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

static constexpr const char* SH_RUN_INT_GI4 = R"template(interface 0/14
description 'Ruckus-AP'
switchport mode access
switch access vlan 100
exit
)template";
} // namespace testdata

static const shared_ptr<CliFlavour> ubiquitiFlavour =
    CliFlavour::create(UBIQUITI);
static const char* showRunningCommand = "show running-config";

// responds to read commands using a pre populated map of commands -> outputs
class MockedCli : public Cli {
 private:
  map<string, string> responseMap;

 public:
  MockedCli(map<string, string> map) : responseMap(map) {}

  SemiFuture<Unit> destroy() override {
    return makeSemiFuture(unit);
  }

  SemiFuture<std::string> executeRead(const ReadCommand cmd) override {
    if (responseMap.count(cmd.raw()) == 1) {
      MLOG(MDEBUG) << "MockedCli.executeRead hit ('" << cmd.raw() << "')";
      return responseMap.at(cmd.raw());
    } else {
      MLOG(MDEBUG) << "MockedCli.executeRead miss ('" << cmd.raw() << "')";
      return Future<string>(runtime_error(cmd.raw()));
    }
  }

  SemiFuture<std::string> executeWrite(const WriteCommand cmd) override {
    return Future<string>(runtime_error(cmd.raw()));
  }

  void clear() {
    responseMap.clear();
  }
};

class TreeCacheCliTest : public ::testing::Test {
 protected:
  shared_ptr<CPUThreadPoolExecutor> testExec =
      make_shared<CPUThreadPoolExecutor>(1);
  shared_ptr<MockedCli> mockedCli;
  shared_ptr<TreeCacheCli> tested_ubiquiti;
  shared_ptr<TreeCache> treeCache;

  void SetUp() override {
    devmand::test::utils::log::initLog();

    map<string, string> mockedResponses;
    mockedResponses.insert(
        make_pair(showRunningCommand, testdata::SH_RUN_UBIQUITI));
    mockedCli = make_shared<MockedCli>(mockedResponses);

    treeCache = make_shared<TreeCache>(ubiquitiFlavour);

    tested_ubiquiti = make_shared<TreeCacheCli>(
        "id", mockedCli, testExec, ubiquitiFlavour, treeCache);
    EXPECT_TRUE(treeCache->isEmpty());
  }

  void TearDown() override {
    MLOG(MDEBUG) << "Waiting for test executor to finish";
    testExec->join();
  }
};

TEST_F(TreeCacheCliTest, checkMockWorks) {
  // just load show running config, should be handled by MockedCli
  string shResult =
      tested_ubiquiti
          ->executeRead(ReadCommand::create(showRunningCommand, false))
          .get();
  EXPECT_EQ(testdata::SH_RUN_UBIQUITI, shResult);
  mockedCli->clear();
  EXPECT_THROW(
      tested_ubiquiti
          ->executeRead(ReadCommand::create(showRunningCommand, false))
          .get(),
      runtime_error);
}

TEST_F(TreeCacheCliTest, getParticularIfc_samePass) {
  // this should execute base command on MockedCli, output will be parsed
  string result = tested_ubiquiti
                      ->executeRead(ReadCommand::create(
                          "show running-config interface 0/14", false))
                      .get();
  EXPECT_EQ(testdata::SH_RUN_INT_GI4, result);
}

TEST_F(
    TreeCacheCliTest,
    getParticularIfc_populateCache_thenGoStraightToTreeCache) {
  // first request will be passed to MockedCli
  string result = tested_ubiquiti
                      ->executeRead(ReadCommand::create(
                          "show running-config interface 0/14", false))
                      .get();
  EXPECT_FALSE(treeCache->isEmpty());
  EXPECT_EQ(testdata::SH_RUN_INT_GI4, result);
  // second request should end in tree cache
  mockedCli->clear();
  result = tested_ubiquiti
               ->executeRead(ReadCommand::create(
                   "show running-config interface 0/14", false))
               .get();
  EXPECT_EQ(testdata::SH_RUN_INT_GI4, result);
}
} // namespace devmand::channels::cli
