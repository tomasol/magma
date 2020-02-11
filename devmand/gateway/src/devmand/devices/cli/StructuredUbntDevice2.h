// Copyright (c) 2016-present, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant
// of patent rights can be found in the PATENTS file in the same directory.

#pragma once

#define LOG_WITH_GLOG
#include <magma_logging.h>

#include <devmand/Application.h>
#include <devmand/channels/cli/Channel.h>
#include <devmand/channels/cli/Command.h>
#include <devmand/channels/cli/CliHttpServer.h>
#include <devmand/channels/cli/ReadCachingCli.h>
#include <devmand/devices/Device.h>
#include <devmand/devices/cli/translation/PluginRegistry.h>

namespace devmand {
namespace devices {
namespace cli {

using namespace devmand::channels::cli;

class StructuredUbntDevice2 : public Device {
 public:
  StructuredUbntDevice2(
      Application& application,
      const Id _id,
      bool readonly_,
      const std::shared_ptr<Channel> _channel,
      const std::shared_ptr<ModelRegistry> mreg,
      std::unique_ptr<ReaderRegistry>&& _rReg,
      const std::shared_ptr<CliCache> _cmdCache,
      const std::shared_ptr<CliHttpServer> _httpServer,
      std::unique_ptr<std::thread>&& _httpThread
          );
  StructuredUbntDevice2() = delete;
  virtual ~StructuredUbntDevice2() = default;
  StructuredUbntDevice2(const StructuredUbntDevice2&) = delete;
  StructuredUbntDevice2& operator=(const StructuredUbntDevice2&) = delete;
  StructuredUbntDevice2(StructuredUbntDevice2&&) = delete;
  StructuredUbntDevice2& operator=(StructuredUbntDevice2&&) = delete;

  static std::unique_ptr<devices::Device> createDevice(
      Application& app,
      const cartography::DeviceConfig& deviceConfig);

  // visible for testing
  static std::unique_ptr<devices::Device> createDeviceWithEngine(
      Application& app,
      const cartography::DeviceConfig& deviceConfig,
      Engine& engine);

 public:
  std::shared_ptr<Datastore> getOperationalDatastore() override;

 protected:
  void setIntendedDatastore(const folly::dynamic& config) override;

 private:
  std::shared_ptr<Channel> channel;
  std::shared_ptr<CliCache> cmdCache;
  std::shared_ptr<ModelRegistry> mreg;
  std::unique_ptr<ReaderRegistry> rReg;
  std::shared_ptr<CliHttpServer> httpServer; // TODO: Should be singleton
  std::unique_ptr<std::thread> httpThread;
};

} // namespace cli
} // namespace devices
} // namespace devmand
