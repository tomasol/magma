// Copyright (c) 2016-present, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant
// of patent rights can be found in the PATENTS file in the same directory.

#include <iostream>
#include <stdexcept>

#include <folly/Format.h>

#include <devmand/channels/cli/Channel.h>
#include <devmand/channels/cli/IoConfigurationBuilder.h>
#include <devmand/channels/cli/Cli.h>
#include <devmand/devices/State.h>
#include <devmand/devices/cli/PlaintextCliDevice.h>

namespace devmand {
namespace devices {
namespace cli {

using namespace devmand::channels::cli;
using namespace devmand::channels::cli::sshsession;

std::unique_ptr<devices::Device> PlaintextCliDevice::createDevice(
    Application& app,
    const cartography::DeviceConfig& deviceConfig) {

  IoConfigurationBuilder ioConfigurationBuilder;
  const std::shared_ptr<Channel>& channel = std::make_shared<Channel>(ioConfigurationBuilder.getIo(
          ioConfigurationBuilder.createSSH(deviceConfig)));
  return std::make_unique<devices::cli::PlaintextCliDevice>(
      app,
      deviceConfig.id,
      deviceConfig.channelConfigs.at("cli").kvPairs.at("stateCommand"),
      channel);
}

PlaintextCliDevice::PlaintextCliDevice(
    Application& application,
    const Id& id_,
    const std::string& _stateCommand,
    const std::shared_ptr<Channel>& _channel)
    : Device(application, id_),
      channel(_channel),
      stateCommand(Command::makeReadCommand(_stateCommand)) {}

std::shared_ptr<State> PlaintextCliDevice::getState() {
  LOG(INFO) << "[" << this << "] "
            << "Retrieving state";

  auto state = State::make(*reinterpret_cast<MetricSink*>(&app), getId());

  state->addRequest(channel->executeAndRead(stateCommand)
                        .thenValue([state, cmd = stateCommand](std::string v) {
                          state->setStatus(true);
                          state->update([&v, &cmd](auto& lockedState) {
                            lockedState[cmd.toString()] = v;
                          });
                        }));
  return state;
}

} // namespace cli
} // namespace devices
} // namespace devmand
