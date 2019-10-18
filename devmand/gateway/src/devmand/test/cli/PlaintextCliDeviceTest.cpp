// Copyright (c) 2019-present, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant
// of patent rights can be found in the PATENTS file in the same directory.

#include <devmand/channels/cli/Cli.h>
#include <devmand/devices/State.h>
#include <devmand/devices/cli/PlaintextCliDevice.h>
#include <devmand/Application.h>
#include <gtest/gtest.h>
#include <folly/executors/IOThreadPoolExecutor.h>
#include <boost/algorithm/string/trim.hpp>

namespace devmand {
namespace test {
namespace cli {

using namespace devmand::channels::cli;
using namespace devmand::devices;
using namespace devmand::devices::cli;

class PlaintextCliDeviceTest : public ::testing::Test {
 public:
  PlaintextCliDeviceTest() = default;
  virtual ~PlaintextCliDeviceTest() = default;
  PlaintextCliDeviceTest(const PlaintextCliDeviceTest&) = delete;
  PlaintextCliDeviceTest& operator=(const PlaintextCliDeviceTest&) = delete;
  PlaintextCliDeviceTest(PlaintextCliDeviceTest&&) = delete;
  PlaintextCliDeviceTest& operator=(PlaintextCliDeviceTest&&) = delete;
};

    TEST_F(PlaintextCliDeviceTest, checkEcho) {
        devmand::Application app;
        cartography::DeviceConfig deviceConfig;
        devmand::cartography::ChannelConfig chnlCfg;
        std::map<std::string, std::string> kvPairs;
        kvPairs.insert(std::make_pair("stateCommand", "show interfaces brief"));
        chnlCfg.kvPairs = kvPairs;
        deviceConfig.channelConfigs.insert(std::make_pair("cli", chnlCfg));

        std::unique_ptr<devices::Device> dev = PlaintextCliDevice::createDevice(
                app, deviceConfig);

        std::shared_ptr<State> state = dev->getState();
        const folly::dynamic& stateResult = state->collect().get();

        std::stringstream buffer;
        buffer << stateResult["show interfaces brief"];
        EXPECT_EQ("show interfaces brief", buffer.str());
    }

    TEST_F(PlaintextCliDeviceTest, ubiquiti) {
        devmand::Application app;
        cartography::DeviceConfig deviceConfig;
        devmand::cartography::ChannelConfig chnlCfg;
        std::map<std::string, std::string> kvPairs;
        kvPairs.insert(std::make_pair("stateCommand", "show mac access-lists"));
        kvPairs.insert(std::make_pair("host", "10.19.0.245"));
        kvPairs.insert(std::make_pair("port", "22"));
        kvPairs.insert(std::make_pair("username", "ubnt"));
        kvPairs.insert(std::make_pair("password", "ubnt"));
        chnlCfg.kvPairs = kvPairs;
        deviceConfig.channelConfigs.insert(std::make_pair("cli", chnlCfg));

        std::unique_ptr<devices::Device> dev = PlaintextCliDevice::createDevice(
                app, deviceConfig);

        std::shared_ptr<State> state = dev->getState();
        const folly::dynamic& stateResult = state->collect().get();

        std::stringstream buffer;
        buffer << stateResult[kvPairs.at("stateCommand")];
        EXPECT_EQ("No ACLs are configured", boost::algorithm::trim_copy(buffer.str()));
    }

} // namespace cli
} // namespace test
} // namespace devmand
