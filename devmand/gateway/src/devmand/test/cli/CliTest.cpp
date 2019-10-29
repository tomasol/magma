// Copyright (c) 2019-present, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant
// of patent rights can be found in the PATENTS file in the same directory.

#include <boost/algorithm/string/trim.hpp>
#include <devmand/channels/cli/Channel.h>
#include <devmand/channels/cli/Cli.h>
#include <devmand/channels/cli/QueuedCli.h>
#include <folly/executors/CPUThreadPoolExecutor.h>
#include <folly/executors/ThreadedExecutor.h>
#include <gtest/gtest.h>
#include <devmand/cartography/DeviceConfig.h>
#include <devmand/devices/cli/PlaintextCliDevice.h>
#include <devmand/devices/Device.h>
#include <devmand/devices/State.h>
#include <devmand/Application.h>
#include <chrono>
#include <magma_logging.h>

namespace devmand {
namespace test {
namespace cli {

using namespace devmand::channels::cli;
using devmand::cartography::DeviceConfig;
using devmand::cartography::ChannelConfig;
using devmand::devices::Device;
using devmand::devices::State;
using devmand::devices::cli::PlaintextCliDevice;
using devmand::Application;

class CliTest : public ::testing::Test {
 public:
  CliTest() = default;
  virtual ~CliTest() = default;
  CliTest(const CliTest&) = delete;
  CliTest& operator=(const CliTest&) = delete;
  CliTest(CliTest&&) = delete;
  CliTest& operator=(CliTest&&) = delete;
};

DeviceConfig getConfig();
DeviceConfig getConfig() {
    DeviceConfig deviceConfig;
    ChannelConfig chnlCfg;
    std::map<std::string, std::string> kvPairs;
    kvPairs.insert(std::make_pair("stateCommand", "echo 123"));
    kvPairs.insert(std::make_pair("port", "22"));
    kvPairs.insert(std::make_pair("username", "root"));
    kvPairs.insert(std::make_pair("password", "root"));
    chnlCfg.kvPairs = kvPairs;
    deviceConfig.channelConfigs.insert(std::make_pair("cli", chnlCfg));
    deviceConfig.ip = "localhost";
    deviceConfig.id = "ubuntu-test-device";
    return deviceConfig;
}

TEST_F(CliTest, PlaintextCliDevices) {
        Application app;

        std::vector<std::unique_ptr<Device>> ds;
        for (int i = 0; i < 10; i++) {
            ds.push_back(std::move(PlaintextCliDevice::createDevice(app, getConfig())));
        }

        for (const auto& dev : ds) {
            std::shared_ptr<State> state = dev->getState();
            auto t1 = std::chrono::high_resolution_clock::now();
            const folly::dynamic& stateResult = state->collect().get();
            auto t2 = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();
            MLOG(MDEBUG) << "Retrieving state took: " << duration << " mu.";
            std::stringstream buffer;

            buffer << stateResult["echo 123"];
            EXPECT_EQ("123", boost::algorithm::trim_copy(buffer.str()));
        }
}

TEST_F(CliTest, queuedCli) {
  QueuedCli qcli(std::make_shared<AsyncEchoCli>(
    std::make_shared<folly::ThreadedExecutor>()),3,1);

  std::vector<std::string> results;
    results.push_back("one");
    results.push_back("two");
    results.push_back("three");
    results.push_back("four");
    results.push_back("five");
    results.push_back("six");
    results.push_back("seven");

  // create requests
  std::vector<Command> cmds;
  cmds.push_back(Command::makeReadCommand("one"));
  cmds.push_back(Command::makeReadCommand("two"));
  cmds.push_back(Command::makeReadCommand("three"));
  cmds.push_back(Command::makeReadCommand("four"));
  cmds.push_back(Command::makeReadCommand("five"));
  cmds.push_back(Command::makeReadCommand("six"));
  cmds.push_back(Command::makeReadCommand("seven"));

  // send requests
  std::vector<folly::Future<std::string>> futures;
  for (const auto& cmd : cmds) {
    DLOG(INFO) << "test exec '" << cmd << "'\n";
    futures.push_back(qcli.executeAndRead(cmd));
  }

  // collect values
  const std::vector<folly::Try<std::string>>& values =
      collectAll(futures.begin(), futures.end()).get();

  // check values
  EXPECT_EQ(values.size(), results.size());
  for (unsigned int i = 0; i < values.size(); ++i) {
    EXPECT_EQ(boost::algorithm::trim_copy(values[i].value()), results[i]);
  }
}

TEST_F(CliTest, queuedCliMT) {
  const int loopcount = 10;
  folly::CPUThreadPoolExecutor executor(8);

  QueuedCli qcli(std::make_shared<AsyncEchoCli>(
    std::make_shared<folly::ThreadedExecutor>()));

  // create requests
  Command cmd = Command::makeReadCommand("hello");
  std::vector<folly::Future<std::string>> futures;
  for (int i = 0; i < loopcount; ++i) {
    DLOG(INFO) << "test exec '" << cmd << "'\n";
    futures.push_back(folly::via(&executor, [&, i]() { return qcli.executeAndRead(cmd); }));
  }

  // collect values
  const std::vector<folly::Try<std::string>>& values =
      collectAll(futures.begin(), futures.end()).get();

  // check values
  EXPECT_EQ(values.size(), loopcount);
  for (auto v : values) {
    EXPECT_EQ(boost::algorithm::trim_copy(v.value()), "hello");
  }
}

TEST_F(CliTest, api) {
  std::string foo("foo");
  Command cmd = Command::makeReadCommand(foo);
  EXPECT_EQ("foo", cmd.toString());
  foo.clear();
  EXPECT_EQ("foo", cmd.toString());
  cmd.toString().clear();
  EXPECT_EQ("foo", cmd.toString());

  const auto mockCli = std::make_shared<EchoCli>();
  folly::Future<std::string> future = mockCli->executeAndRead(cmd);
  EXPECT_EQ("foo", std::move(future).get());

  Channel cliChannel(std::make_shared<EchoCli>());
  folly::Future<std::string> futureFromChannel = cliChannel.executeAndRead(cmd);
  EXPECT_EQ("foo", std::move(futureFromChannel).get());
}

} // namespace cli
} // namespace test
} // namespace devmand
