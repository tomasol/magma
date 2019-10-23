// Copyright (c) 2016-present, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant
// of patent rights can be found in the PATENTS file in the same directory.

#include <boost/algorithm/string/trim.hpp>
#include <devmand/channels/cli/Cli.h>
#include <devmand/devices/State.h>
#include <devmand/devices/cli/ModelRegistry.h>
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
    } else if (cmd.toString().find(" | include Max Frame Size") != string::npos) {
      return "Max Frame Size................................. 1518";
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
using AdminState = Ifcs::Interface::State::AdminStatus;
using OperState = Ifcs::Interface::State::OperStatus;

template <typename T>
using postProcessor = std::function<T(string)>;

template <typename T>
static vector<T> parseKeys(
    const string& output,
    const regex& pattern,
    const uint groupToExtract,
    const int skipLines = 0,
    const postProcessor<T> postProcess = [](auto str) { return str; }) {
  std::stringstream ss(output);
  std::string line;
  vector<T> retval;
  int counter = 0;

  while (std::getline(ss, line, '\n')) {
    counter++;
    if (counter <= skipLines) {
      continue;
    }

    boost::algorithm::trim(line);
    std::smatch match;
    if (std::regex_search(line, match, pattern) and
        match.size() > groupToExtract) {
      T processed = postProcess(match[groupToExtract]);
      retval.push_back(processed);
    }
  }

  return retval;
}

static void parseValue(
    const string& output,
    const regex& pattern,
    const uint groupToExtract,
    const std::function<void(string)> setter) {
  std::stringstream ss(output);
  std::string line;

  while (std::getline(ss, line, '\n')) {
    boost::algorithm::trim(line);
    std::smatch match;
    if (std::regex_match(line, match, pattern) and
        match.size() > groupToExtract and match[groupToExtract].length() > 0) {
      setter(match[groupToExtract]);
    }
  }
}

static void parseConfig(
    Channel& channel,
    const string& ifcId,
    shared_ptr<Ifcs::Interface::Config>& cfg) {
  const Command cmd =
      Command::makeReadCommand("show running-config interface " + ifcId);
  string output = channel.executeAndRead(cmd).get();

  cfg->name = ifcId;
  cfg->enabled = true;
  parseValue(output, regex("shutdown"), 0, [&cfg](string value) {
    cfg->enabled = "shutdown" == value;
  });

  // TODO support other types
  cfg->type = regex_match(ifcId, regex(R"(\d+/\d+)"))
      ? (ietf::iana_if_type::IanaInterfaceType)
            ietf::iana_if_type::EthernetCsmacd()
      : (ietf::iana_if_type::IanaInterfaceType)ietf::iana_if_type::Other();

  parseValue(
      output, regex(R"(description '?(.+?)'?)"), 1, [&cfg](string value) {
        cfg->description = value;
      });

  parseValue(output, regex(R"(mtu (.+))"), 1, [&cfg](string value) {
    cfg->mtu = stoi(value);
  });
}

static void parseState(
    Channel& channel,
    const string& ifcId,
    shared_ptr<Ifcs::Interface::State>& state,
    const shared_ptr<Ifcs::Interface::Config>& cfg) {
  const Command cmdState =
      Command::makeReadCommand("show interfaces description");
  string outputState = channel.executeAndRead(cmdState).get();

  state->name = cfg->name.get();
  state->type = cfg->type.get();
  state->enabled = cfg->enabled.get();

  const auto regexIfcState = regex(ifcId + R"(\s+(\S+)\s+(\S+)\s*(.*))");
  parseValue(outputState, regexIfcState, 1, [&state](string value) {
    if ("Enable" == value) {
      state->admin_status = AdminState::UP;
    } else if ("Disable" == value) {
      state->admin_status = AdminState::DOWN;
    }
  });

  parseValue(outputState, regexIfcState, 2, [&state](string value) {
    if ("Up" == value) {
      state->oper_status = OperState::UP;
    } else if ("Down" == value) {
      state->oper_status = OperState::DOWN;
    }
  });

  parseValue(outputState, regexIfcState, 3, [&state](string value) {
    state->description = value;
  });

  YLeaf et(YType::identityref, "type");
  et.set(ietf::iana_if_type::EthernetCsmacd());

  if (cfg->type == et) {
    const Command cmdStateMtu = Command::makeReadCommand(
        "show interface ethernet " + ifcId + " | include Max Frame Size");
    string outputStateMtu = channel.executeAndRead(cmdStateMtu).get();
    parseValue(outputStateMtu, regex(R"(.+?(\d+).*)"), 1, [&state](string value) {
      state->mtu = stoi(value);
    });
  }
}

static shared_ptr<Ifcs::Interface> parseInterface(
    Channel& channel,
    const string& ifcId) {
  auto ifc = make_shared<Ifcs::Interface>();
  ifc->name = ifcId;
  parseConfig(channel, ifcId, ifc->config);
  parseState(channel, ifcId, ifc->state, ifc->config);
  return ifc;
}

static shared_ptr<Ifcs> parseIfcs(Channel& channel) {
  auto cmd = Command::makeReadCommand("show interfaces description");
  string output = channel.executeAndRead(cmd).get();
  regex ifcIdRegex(R"(^(\S+)\s+(\S+)\s+(\S+)\s*(.*))");
  vector<string> ids = parseKeys<string>(output, ifcIdRegex, 1, 3);

  auto interfaces = make_shared<Ifcs>();
  for (auto& ifcId : ids) {
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
