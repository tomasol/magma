// Copyright (c) 2019-present, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant
// of patent rights can be found in the PATENTS file in the same directory.

#include <boost/algorithm/string/trim.hpp>
#include <folly/executors/ThreadedExecutor.h>
#include <gtest/gtest.h>
#include <devmand/cartography/DeviceConfig.h>
#include <devmand/devices/cli/PlaintextCliDevice.h>
#include <devmand/devices/Device.h>
#include <devmand/devices/State.h>
#include <devmand/Application.h>
#include <chrono>
#include <magma_logging.h>
#include <devmand/channels/cli/CliFlavour.h>

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
using devmand::channels::cli::UBIQUITI;

class RealCliDeviceTest : public ::testing::Test {
 public:
  RealCliDeviceTest() = default;
  virtual ~RealCliDeviceTest() = default;
  RealCliDeviceTest(const RealCliDeviceTest&) = delete;
  RealCliDeviceTest& operator=(const RealCliDeviceTest&) = delete;
  RealCliDeviceTest(RealCliDeviceTest&&) = delete;
  RealCliDeviceTest& operator=(RealCliDeviceTest&&) = delete;
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

TEST_F(RealCliDeviceTest, tenPlaintextCliDevices) {
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

TEST_F(RealCliDeviceTest, ubiquiti) {
        devmand::Application app;
        cartography::DeviceConfig deviceConfig;
        devmand::cartography::ChannelConfig chnlCfg;
        std::map<std::string, std::string> kvPairs;
        kvPairs.insert(std::make_pair("stateCommand", "show mac access-lists"));
        kvPairs.insert(std::make_pair("port", "22"));
        kvPairs.insert(std::make_pair("username", "ubnt"));
        kvPairs.insert(std::make_pair("password", "ubnt"));
        kvPairs.insert(std::make_pair("flavour", UBIQUITI));
        chnlCfg.kvPairs = kvPairs;
        deviceConfig.channelConfigs.insert(std::make_pair("cli", chnlCfg));
        deviceConfig.ip = "10.19.0.245";
        deviceConfig.id = "ubiquiti-test-device";
        std::unique_ptr<devices::Device> dev =
                PlaintextCliDevice::createDevice(app, deviceConfig);

        std::shared_ptr<State> state = dev->getState();
        const folly::dynamic& stateResult = state->collect().get();

        std::stringstream buffer;
        buffer << stateResult[kvPairs.at("stateCommand")];
        EXPECT_EQ(
                "No ACLs are configured", boost::algorithm::trim_copy(buffer.str()));
}

} // namespace cli
} // namespace test
} // namespace devmand
