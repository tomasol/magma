// Copyright (c) 2016-present, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant
// of patent rights can be found in the PATENTS file in the same directory.

#include <iostream>
#include <stdexcept>
#include <utility>

#include <folly/Format.h>

#include <devmand/channels/cli/Channel.h>
#include <devmand/channels/cli/Cli.h>
#include <devmand/devices/State.h>
#include <devmand/devices/cli/PlaintextCliDevice.h>

namespace devmand {
namespace devices {
namespace cli {

using namespace devmand::channels::cli;

std::unique_ptr<devices::Device> PlaintextCliDevice::createDevice(
    Application& app,
    const cartography::DeviceConfig& deviceConfig) {
    const auto& channelConfigs = deviceConfig.channelConfigs;
    const auto& plaintextCliKv = channelConfigs.at("cli").kvPairs;
  return std::make_unique<devices::cli::PlaintextCliDevice>(
      app, deviceConfig.id, plaintextCliKv.at("stateCommand"));
}

PlaintextCliDevice::PlaintextCliDevice(
    Application& application,
    const Id& id_,
    const std::string& _stateCommand)
    : Device(application, id_),
      channel(std::make_shared<EchoCli>()),
      stateCommand(Command::makeReadCommand(_stateCommand)) {}

std::shared_ptr<State> PlaintextCliDevice::getState() {
  LOG(INFO) << "[" << this << "] "
            << "Retrieving state";

  auto state = State::make(app, *this);

  state->addRequest(channel.executeAndRead(stateCommand)
                        .thenValue([state, cmd = stateCommand](auto v) {
                          state->setStatus(true);
                          state->update()[cmd.toString()] = v;
                        }));
  return state;
}

} // namespace cli
} // namespace devices
} // namespace devmand
