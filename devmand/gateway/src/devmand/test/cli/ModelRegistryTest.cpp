// Copyright (c) 2019-present, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant
// of patent rights can be found in the PATENTS file in the same directory.

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/split.hpp>
#include <devmand/devices/cli/ModelRegistry.h>
#include <gtest/gtest.h>
#include <ydk_ietf/iana_if_type.hpp>
#include <ydk_openconfig/openconfig_interfaces.hpp>
#include <spdlog/spdlog.h>

namespace devmand {
namespace test {
namespace cli {

using namespace devmand::devices::cli;

using OpenconfigInterfaces = openconfig::openconfig_interfaces::Interfaces;
using OpenconfigInterface = OpenconfigInterfaces::Interface;

class ModelRegistryTest : public ::testing::Test {
 protected:
  void SetUp() override {
    // Enable ydk logging
    auto ydk = spdlog::stdout_color_mt("ydk");
    spdlog::set_level(spdlog::level::level_enum::debug);
  }
  void TearDown() override {
    spdlog::drop("ydk");
  }

 protected:
  ModelRegistry mreg;
};

TEST_F(ModelRegistryTest, caching) {
  Bundle& bundleOpenconfig = mreg.getBundle(Model::OPENCONFIG_0_1_6);
  Bundle& bundleOpenconfig2 = mreg.getBundle(Model::OPENCONFIG_0_1_6);
  ASSERT_EQ(&bundleOpenconfig, &bundleOpenconfig2);
  ASSERT_EQ(1, mreg.cacheSize());
}

static shared_ptr<OpenconfigInterface> interfaceCpp() {
  auto interface = make_shared<OpenconfigInterface>();
  interface->name = "loopback1";
  interface->config->name = "loopback1";
  interface->config->type = ietf::iana_if_type::SoftwareLoopback();
  interface->config->mtu = 1500;
  interface->state->ifindex = 1;
  return interface;
}

static shared_ptr<OpenconfigInterfaces> interfacesCpp() {
  auto interfaces = make_shared<OpenconfigInterfaces>();
  interfaces->interface.append(interfaceCpp());
  return interfaces;
}

static const string interfaceJson =
    "{\n"
    "  \"openconfig-interfaces:interfaces\": {\n"
    "    \"interface\": [\n"
    "      {\n"
    "        \"config\": {\n"
    "          \"mtu\": 1500,\n"
    "          \"name\": \"loopback1\",\n"
    "          \"type\": \"iana-if-type:softwareLoopback\"\n"
    "        },\n"
    "        \"name\": \"loopback1\",\n"
    "        \"state\": {\n"
    "          \"ifindex\": 1\n"
    "        }\n"
    "      }\n"
    "    ]\n"
    "  }\n"
    "}";

static const string singleInterfaceJson =
    "{\n"
    "  \"openconfig-interfaces:interface\": {\n"
    "    \"config\": {\n"
    "      \"mtu\": 1500,\n"
    "      \"name\": \"loopback1\",\n"
    "      \"type\": \"iana-if-type:softwareLoopback\"\n"
    "    },\n"
    "    \"name\": \"loopback1\",\n"
    "    \"state\": {\n"
    "      \"ifindex\": 1\n"
    "    }\n"
    "  }\n"
    "}";

static string sortJson(const string& json) {
  std::vector<std::string> lines;
  // Split to lines
  boost::split(lines, json, boost::is_any_of("\n"), boost::token_compress_on);
  // Sort
  std::sort(lines.begin(), lines.end());
  auto joined = boost::algorithm::join(lines, "\n");
  // Remove comma
  boost::replace_all(joined, ",", "");

  return joined;
}

TEST_F(ModelRegistryTest, jsonSerializationTopLevel) {
  Bundle& bundleOpenconfig = mreg.getBundle(Model::OPENCONFIG_0_1_6);

  shared_ptr<OpenconfigInterfaces> originalIfc = interfacesCpp();

  const string& interfaceEncoded = bundleOpenconfig.encode(*originalIfc);
  ASSERT_EQ(sortJson(interfaceJson), sortJson(interfaceEncoded));

  const shared_ptr<Entity> decodedIfcEntity = bundleOpenconfig.decode(
      interfaceEncoded, make_shared<OpenconfigInterfaces>());
  ASSERT_TRUE(*decodedIfcEntity == *originalIfc);
}

TEST_F(ModelRegistryTest, jsonSerializationNested) {
  Bundle& bundleOpenconfig = mreg.getBundle(Model::OPENCONFIG_0_1_6);

  shared_ptr<OpenconfigInterface> originalIfc = interfaceCpp();

  const string& interfaceEncoded = bundleOpenconfig.encode(*originalIfc);
  ASSERT_EQ(sortJson(singleInterfaceJson), sortJson(interfaceEncoded));

  const shared_ptr<Entity> decodedIfcEntity = bundleOpenconfig.decode(
      interfaceEncoded, make_shared<OpenconfigInterface>());
  ASSERT_TRUE(*decodedIfcEntity == *originalIfc);
}

// TODO test thread safety of Bundle and Registry

} // namespace cli
} // namespace test
} // namespace devmand
