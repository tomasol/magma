// Copyright (c) 2016-present, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant
// of patent rights can be found in the PATENTS file in the same directory.

#include <devmand/channels/cli/Cli.h>
#include <devmand/channels/cli/IoConfigurationBuilder.h>
#include <devmand/devices/State.h>
#include <devmand/devices/cli/ModelRegistry.h>
#include <devmand/devices/cli/ParsingUtils.h>
#include <devmand/devices/cli/StructuredUbntDevice.h>
#include <folly/futures/Future.h>
#include <folly/json.h>
#include <ydk_ietf/iana_if_type.hpp>
#include <ydk_openconfig/openconfig_interfaces.hpp>
#include <memory>
#include <regex>
#include <unordered_map>

namespace devmand {
namespace devices {
namespace cli {

using namespace devmand::channels::cli;
using namespace std;

// TODO get out of here, this will be a shared single instance for all
// structured devices
auto mreg = std::unique_ptr<ModelRegistry>(new ModelRegistry());

using Ifcs = openconfig::openconfig_interfaces::Interfaces;
using Ifc = openconfig::openconfig_interfaces::Interfaces::Interface;
using AdminState = Ifc::State::AdminStatus;
using OperState = Ifc::State::OperStatus;

static const auto shutdown = regex("shutdown");
static const auto description = regex(R"(description '?(.+?)'?)");
static const auto mtu = regex(R"(mtu (.+))");

static void parseConfig(
    Channel& channel,
    const string& ifcId,
    shared_ptr<Ifc::Config>& cfg) {
  const Command cmd =
      Command::makeReadCommand("show running-config interface " + ifcId);
  string output = channel.executeAndRead(cmd).get();

  cfg->name = ifcId;
  parseLeaf(output, mtu, cfg->mtu, 1, toUI16);
  parseLeaf<string>(output, description, cfg->description);
  cfg->enabled = true;
  parseLeaf<bool>(output, shutdown, cfg->enabled, 0, [](auto str) {
    return "shutdown" == str;
  });

  if (regex_match(ifcId, regex(R"(\d+/\d+)"))) {
    cfg->type = ietf::iana_if_type::EthernetCsmacd();
  } else {
    cfg->type = ietf::iana_if_type::Other();
  }
}

static const auto inOct =
    regex(R"(Total Packets Received \(Octets\).*?(\d+).*)");
static const auto inUPkt = regex(R"(Unicast Packets Received.*?(\d+).*)");
static const auto inMPkt = regex(R"(Multicast Packets Received.*?(\d+).*)");
static const auto inBPkt = regex(R"(Broadcast Packets Received.*?(\d+).*)");
static const auto inDisc = regex(R"(Receive Packets Discarded.*?(\d+).*)");
static const auto inErrors =
    regex(R"(Total Packets Received with MAC Errors.*?(\d+).*)");

static const auto outOct =
    regex(R"(Total Packets Transmitted \(Octets\).*?(\d+).*)");
static const auto outUPkt = regex(R"(Unicast Packets Transmitted.*?(\d+).*)");
static const auto outMPkt = regex(R"(Multicast Packets Transmitted.*?(\d+).*)");
static const auto outBPkt = regex(R"(Broadcast Packets Transmitted.*?(\d+).*)");
static const auto outDisc = regex(R"(Transmit Packets Discarded.*?(\d+).*)");
static const auto outErrors = regex(R"(Total Transmit Errors.*?(\d+).*)");

static const auto counterReset =
    regex(R"(Time Since Counters Last Cleared.*?([^.]+).*)");

static void parseEthernetCounters(
    const string& output,
    shared_ptr<Ifc::State::Counters>& ctr) {
  // FIXME there is a bug in YDK
  // https://github.com/CiscoDevNet/ydk-cpp/blob/58f8dc4f6d078ec8c672c96bf8db7ecb84d14007/core/ydk/src/json_subtree_codec.cpp#L227
  // they assume any number to be an int, bigger numbers such as uint64 below
  // fail the encoding process

  parseLeaf(output, inOct, ctr->in_octets, 1, toUI64);
  parseLeaf(output, inUPkt, ctr->in_unicast_pkts, 1, toUI64);
  parseLeaf(output, inMPkt, ctr->in_multicast_pkts, 1, toUI64);
  parseLeaf(output, inBPkt, ctr->in_broadcast_pkts, 1, toUI64);
  parseLeaf(output, inDisc, ctr->in_discards, 1, toUI64);
  parseLeaf(output, inErrors, ctr->in_errors, 1, toUI64);

  parseLeaf(output, outOct, ctr->out_octets, 1, toUI64);
  parseLeaf(output, outUPkt, ctr->out_unicast_pkts, 1, toUI64);
  parseLeaf(output, outMPkt, ctr->out_multicast_pkts, 1, toUI64);
  parseLeaf(output, outBPkt, ctr->out_broadcast_pkts, 1, toUI64);
  parseLeaf(output, outDisc, ctr->out_discards, 1, toUI64);
  parseLeaf(output, outErrors, ctr->out_errors, 1, toUI64);

  parseLeaf<string>(output, counterReset, ctr->last_clear, 1);
}

static const auto mtuState = regex(R"(Max Frame Size.*?(\d+).*)");

static void parseState(
    Channel& channel,
    const string& ifcId,
    shared_ptr<Ifc::State>& state,
    const shared_ptr<Ifc::Config>& cfg) {
  const Command cmdState =
      Command::makeReadCommand("show interfaces description");
  string stateOut = channel.executeAndRead(cmdState).get();
  const auto regexIfcState = regex(ifcId + R"(\s+(\S+)\s+(\S+)\s*(.*))");

  state->name = cfg->name.get();
  state->type = cfg->type.get();
  state->enabled = cfg->enabled.get();
  parseLeaf<string>(stateOut, regexIfcState, state->description, 3);

  parseValue(stateOut, regexIfcState, 1, [&state](string value) {
    if ("Enable" == value) {
      state->admin_status = AdminState::UP;
    } else if ("Disable" == value) {
      state->admin_status = AdminState::DOWN;
    }
  });

  parseValue(stateOut, regexIfcState, 2, [&state](string value) {
    if ("Up" == value) {
      state->oper_status = OperState::UP;
    } else if ("Down" == value) {
      state->oper_status = OperState::DOWN;
    }
  });

  if (cfg->type.get() == ietf::iana_if_type::EthernetCsmacd().to_string()) {
    const Command cmdStateEth =
        Command::makeReadCommand("show interface ethernet " + ifcId);
    string outputStateEth = channel.executeAndRead(cmdStateEth).get();
    parseLeaf(outputStateEth, mtuState, state->mtu, 1, toUI16);
    parseEthernetCounters(outputStateEth, state->counters);
  }
}

static shared_ptr<Ifc> parseInterface(Channel& channel, const string& ifcId) {
  auto ifc = make_shared<Ifc>();
  ifc->name = ifcId;
  parseConfig(channel, ifcId, ifc->config);
  parseState(channel, ifcId, ifc->state, ifc->config);
  return ifc;
}

static shared_ptr<Ifcs> parseIfcs(Channel& channel) {
  auto cmd = Command::makeReadCommand("show interfaces description");
  string output = channel.executeAndRead(cmd).get();
  regex ifcIdRegex(R"(^(\S+)\s+(\S+)\s+(\S+)\s*(.*))");

  auto interfaces = make_shared<Ifcs>();
  for (auto& ifcId : parseKeys<string>(output, ifcIdRegex, 1, 4)) {
    interfaces->interface.append(parseInterface(channel, ifcId));
  }
  return interfaces;
}

unique_ptr<devices::Device> StructuredUbntDevice::createDevice(
    Application& app,
    const cartography::DeviceConfig& deviceConfig) {
  IoConfigurationBuilder ioConfigurationBuilder(deviceConfig);
  const std::shared_ptr<Channel>& channel =
      std::make_shared<Channel>(ioConfigurationBuilder.getIo());

  return unique_ptr<StructuredUbntDevice>(
      new StructuredUbntDevice(app, deviceConfig.id, channel));
}

StructuredUbntDevice::StructuredUbntDevice(
    Application& application,
    const Id& id_,
    const shared_ptr<Channel>& _channel)
    : Device(application, id_), channel(_channel) {}

shared_ptr<State> StructuredUbntDevice::getState() {
  LOG(INFO) << "[" << this << "] "
            << "Retrieving state";

  auto state = State::make(*reinterpret_cast<MetricSink*>(&app), getId());
  state->setStatus(true);

  auto& bundle = mreg->getBundle(Model::OPENCONFIG_0_1_6);
  auto ifcs = parseIfcs(*channel);

  // TODO the conversion here is: Object -> Json -> folly:dynamic
  // the json step is unnecessary
  string json = bundle.encode(*ifcs);
  folly::dynamic dynamicIfcs = folly::parseJson(json);
  state->update([&dynamicIfcs](folly::dynamic& lockedState) {
    lockedState.merge_patch(dynamicIfcs);
  });

  return state;
}

} // namespace cli
} // namespace devices
} // namespace devmand
