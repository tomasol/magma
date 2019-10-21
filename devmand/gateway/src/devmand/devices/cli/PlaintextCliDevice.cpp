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
#include <devmand/channels/cli/Cli.h>
#include <devmand/devices/State.h>
#include <devmand/devices/cli/PlaintextCliDevice.h>
#include <devmand/channels/cli/SshSessionAsync.h>
#include <devmand/channels/cli/PromptAwareCli.h>

namespace devmand {
namespace devices {
namespace cli {

using namespace devmand::channels::cli;
using namespace devmand::channels::cli::sshsession;

shared_ptr<IOThreadPoolExecutor> executor = std::make_shared<folly::IOThreadPoolExecutor>(10);

std::unique_ptr<devices::Device> PlaintextCliDevice::createDevice(
    Application& app,
    const cartography::DeviceConfig& deviceConfig) {
    const auto& channelConfigs = deviceConfig.channelConfigs;
    const auto& plaintextCliKv = channelConfigs.at("cli").kvPairs;
    //crate session
    const std::shared_ptr<SshSessionAsync> &session = std::make_shared<SshSessionAsync>(executor);
    //TODO opening SSH connection
    session->openShell(
            deviceConfig.ip,
            std::stoi( plaintextCliKv.at("port")),
            plaintextCliKv.at("username"),
            plaintextCliKv.at("password")).get();
    //TODO create CLI - how to create a CLI stack?
    const shared_ptr <PromptAwareCli> &cli = std::make_shared<PromptAwareCli>(session, CliFlavour());
    //TODO initialize CLI
    cli->initializeCli();
    //TODO resolve prompt needs to happen
    cli->resolvePrompt();
    const std::shared_ptr<Channel> &channel = std::make_shared<Channel>(cli);
    return std::make_unique<devices::cli::PlaintextCliDevice>(
      app, deviceConfig.id, plaintextCliKv.at("stateCommand"), channel);
}

PlaintextCliDevice::PlaintextCliDevice(
    Application& application,
    const Id& id_,
    const std::string& _stateCommand,
    const std::shared_ptr<Channel> & _channel)
    : Device(application, id_),
      channel(_channel),
      stateCommand(Command::makeReadCommand(_stateCommand)) {
}

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
