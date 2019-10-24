// Copyright (c) 2016-present, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant
// of patent rights can be found in the PATENTS file in the same directory.

#include <devmand/channels/cli/Cli.h>
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

auto mreg = std::unique_ptr<ModelRegistry>(new ModelRegistry());

class UbntFakeCli : public Cli {
 public:
  folly::Future<string> executeAndRead(const Command& cmd) override {
    (void)cmd;
    if (cmd.toString() == "show interfaces description") {
      return "\n"
             "Interface  Admin      Link    Description\n"
             "---------  ---------  ------  ----------------------------------------------------------------\n"
             "0/1        Enable     Up\n"
             "0/2        Enable     Down    Some descr\n"
             "0/3        Enable     Down\n"
             "0/4        Enable     Down\n"
             "0/5        Enable     Down\n"
             "0/6        Enable     Down\n"
             "0/7        Enable     Down\r\n"
             "0/8        Enable     Down\n"
             "0/9        Enable     Down\n"
             "0/10       Enable     Down\n"
             "0/11       Enable     Down\n"
             "0/12       Enable     Down\n"
             "0/13       Enable     Down\n"
             "0/14       Enable     Down\n"
             "0/15       Enable     Down\n"
             "0/16       Enable     Down\n"
             "0/17       Enable     Down\n"
             "0/18       Enable     Down\n"
             "3/1        Enable     Down\n"
             "3/2        Enable     Down\n"
             "3/3        Enable     Down\n"
             "3/4        Enable     Down\n"
             "3/5        Enable     Down\n"
             "3/6        Enable     Down\n";
    } else if (cmd.toString().find("show running-config interface ") == 0) {
      string ifcId = cmd.toString().substr(
          string("show running-config interface ").size() - 1);

      return "\n"
             "!Current Configuration:\n"
             "!\n"
             "interface  " +
          ifcId +
          "\n"
          "description 'This is ifc " +
          ifcId +
          "'\n"
          "mtu 1500\n"
          "exit\n"
          "";
    } else if (cmd.toString().find("show interface ethernet") == 0) {
      return "\n"
             "Total Packets Received (Octets)................ 42706681\n"
             "Packets Received 64 Octets..................... 2522468\n"
             "Packets Received 65-127 Octets................. 235916\n"
             "Packets Received 128-255 Octets................ 2334087\n"
             "Packets Received 256-511 Octets................ 810085\n"
             "Packets Received 512-1023 Octets............... 1079612\n"
             "Packets Received 1024-1518 Octets.............. 1847904\n"
             "Packets Received > 1518 Octets................. 0\n"
             "Packets RX and TX 64 Octets.................... 2855454\n"
             "Packets RX and TX 65-127 Octets................ 315435\n"
             "Packets RX and TX 128-255 Octets............... 2346308\n"
             "Packets RX and TX 256-511 Octets............... 811155\n"
             "Packets RX and TX 512-1023 Octets.............. 1086055\n"
             "Packets RX and TX 1024-1518 Octets............. 1873514\n"
             "Packets RX and TX 1519-2047 Octets............. 0\n"
             "Packets RX and TX 2048-4095 Octets............. 0\n"
             "Packets RX and TX 4096-9216 Octets............. 0\n"
             "\n"
             "Total Packets Received Without Errors.......... 8830072\n"
             "Unicast Packets Received....................... 293117\n"
             "Multicast Packets Received..................... 5769311\n"
             "Broadcast Packets Received..................... 2767644\n"
             "                  \n"
             "Receive Packets Discarded...................... 0\n"
             "\n"
             "Total Packets Received with MAC Errors......... 0\n"
             "Jabbers Received............................... 0\n"
             "Fragments Received............................. 0\n"
             "Undersize Received............................. 0\n"
             "Alignment Errors............................... 0\n"
             "FCS Errors..................................... 0\n"
             "Overruns....................................... 0\n"
             "\n"
             "Total Received Packets Not Forwarded........... 0\n"
             "802.3x Pause Frames Received................... 0\n"
             "Unacceptable Frame Type........................ 0\n"
             "\n"
             "Total Packets Transmitted (Octets)............. 72669652\n"
             "Packets Transmitted 64 Octets.................. 332986\n"
             "Packets Transmitted 65-127 Octets.............. 79519\n"
             "Packets Transmitted 128-255 Octets............. 12221\n"
             "Packets Transmitted 256-511 Octets............. 1070\n"
             "Packets Transmitted 512-1023 Octets............ 6443\n"
             "Packets Transmitted 1024-1518 Octets........... 25610\n"
             "Packets Transmitted > 1518 Octets.............. 0\n"
             "Max Frame Size................................. 1518\n"
             "\n"
             "Total Packets Transmitted Successfully......... 457849\n"
             "Unicast Packets Transmitted.................... 125182\n"
             "Multicast Packets Transmitted.................. 325039\n"
             "Broadcast Packets Transmitted.................. 7628\n"
             "\n"
             "Transmit Packets Discarded..................... 0\n"
             "\n"
             "Total Transmit Errors.......................... 0\n"
             "\n"
             "Total Transmit Packets Discarded............... 0\n"
             "Single Collision Frames........................ 0\n"
             "Multiple Collision Frames...................... 0\n"
             "Excessive Collision Frames..................... 0\n"
             "\n"
             "802.3x Pause Frames Transmitted................ 0\n"
             "GVRP PDUs received............................. 0\n"
             "GVRP PDUs Transmitted.......................... 0\n"
             "GVRP Failed Registrations...................... 0\n"
             "GMRP PDUs Received............................. 0\n"
             "GMRP PDUs Transmitted.......................... 0\n"
             "GMRP Failed Registrations...................... 0\n"
             "                  \n"
             "STP BPDUs Transmitted.......................... 304922\n"
             "STP BPDUs Received............................. 2\n"
             "RSTP BPDUs Transmitted......................... 0\n"
             "RSTP BPDUs Received............................ 0\n"
             "MSTP BPDUs Transmitted......................... 5\n"
             "MSTP BPDUs Received............................ 0\n"
             "\n"
             "EAPOL Frames Transmitted....................... 0\n"
             "EAPOL Start Frames Received.................... 0\n"
             "\n"
             "Load Interval.................................. 5\n"
             "Bits Per Second Received....................... 38272\n"
             "Bits Per Second Transmitted.................... 4552\n"
             "Packets Per Second Received.................... 14\n"
             "Packets Per Second Transmitted................. 1\n"
             "\n"
             "Time Since Counters Last Cleared............... 7 day 1 hr 24 min 53 sec";
    } else if (cmd.toString() == "show interfaces description") {
      return "\n"
             "Interface  Admin      Link    Description\n"
             "---------  ---------  ------  ----------------------------------------------------------------\n"
             "0/1        Enable     Up\n"
             "0/2        Enable     Down\n"
             "0/3        Enable     Down\n"
             "0/4        Enable     Down\n"
             "0/5        Disable    Down    testing\n"
             "0/6        Enable     Down\n"
             "0/7        Enable     Down\n"
             "0/8        Enable     Down\n"
             "0/9        Enable     Down\n"
             "0/10       Enable     Down\n"
             "0/11       Enable     Down\n"
             "0/12       Enable     Down\n"
             "0/13       Enable     Down\n"
             "0/14       Enable     Down\n"
             "0/15       Enable     Down\n"
             "0/16       Enable     Down\n"
             "0/17       Enable     Down\n"
             "0/18       Enable     Down\n"
             "3/1        Enable     Down\n"
             "3/2        Enable     Down\n"
             "3/3        Enable     Down\n"
             "3/4        Enable     Down\n"
             "3/5        Enable     Down\n"
             "3/6        Enable     Down\n"
             "";
    }

    return "";
  }

  folly::Future<string> executeAndSwitchPrompt(const Command& cmd) override {
    (void)cmd;
    return folly::Future<string>("");
  }
};

unique_ptr<devices::Device> StructuredUbntDevice::createDevice(
    Application& app,
    const cartography::DeviceConfig& deviceConfig) {
  //  const auto& channelConfigs = deviceConfig.channelConfigs;
  //  const auto& plaintextCliKv = channelConfigs.at("cli").kvPairs;

  auto cli = make_shared<UbntFakeCli>();
  auto channel = make_shared<Channel>(cli);
  return unique_ptr<StructuredUbntDevice>(
      new StructuredUbntDevice(app, deviceConfig.id, channel));
}

StructuredUbntDevice::StructuredUbntDevice(
    Application& application,
    const Id& id_,
    const shared_ptr<Channel>& _channel)
    : Device(application, id_), channel(_channel) {}

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
  for (auto& ifcId : parseKeys<string>(output, ifcIdRegex, 1, 3)) {
    interfaces->interface.append(parseInterface(channel, ifcId));
  }
  return interfaces;
}

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
