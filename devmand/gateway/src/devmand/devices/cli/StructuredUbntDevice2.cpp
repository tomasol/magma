// Copyright (c) 2016-present, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant
// of patent rights can be found in the PATENTS file in the same directory.

#include <devmand/channels/cli/IoConfigurationBuilder.h>
#include <devmand/devices/Datastore.h>
#include <devmand/devices/cli/StructuredUbntDevice2.h>
#include <devmand/devices/cli/UbntInterfacePlugin.h>
#include <devmand/devices/cli/schema/ModelRegistry.h>
#include <devmand/devices/cli/translation/PluginRegistry.h>
#include <devmand/devices/cli/translation/ReaderRegistry.h>
#include <folly/json.h>
#include <memory>
#include <devmand/channels/cli/CliHttpServer.h>

namespace devmand {
namespace devices {
namespace cli {

using namespace devmand::channels::cli;
using namespace std;
using namespace folly;

std::unique_ptr<devices::Device> StructuredUbntDevice2::createDevice(
    Application& app,
    const cartography::DeviceConfig& deviceConfig) {
  return createDeviceWithEngine(app, deviceConfig, app.getCliEngine());
}

unique_ptr<devices::Device> StructuredUbntDevice2::createDeviceWithEngine(
    Application& app,
    const cartography::DeviceConfig& deviceConfig,
    Engine& engine) {
  IoConfigurationBuilder ioConfigurationBuilder(deviceConfig, engine);
  auto cmdCache = ReadCachingCli::createCache();
  const std::shared_ptr<Channel>& channel = std::make_shared<Channel>(
      deviceConfig.id, ioConfigurationBuilder.createAll(cmdCache));

  // TODO make configurable singleton
  auto dummyTxResolver = [](const string token, bool configDS, bool readCurrentTx, Path path)
      -> dynamic  {
    (void)token;(void)configDS;(void)readCurrentTx;(void)path;
    throw runtime_error("Not implemented");
  };
  auto cliResolver = [channel](const string token) -> shared_ptr<Channel> {
    if (token == "secret") {
      return channel;
    }
    throw runtime_error("Wrong token");
  };
  shared_ptr<CliHttpServer> httpServer = make_shared<CliHttpServer>("0.0.0.0", 4000, cliResolver, dummyTxResolver);
  unique_ptr<std::thread> httpThread = make_unique<std::thread>([httpServer]() {
    httpServer->listen();
  });
  while (not httpServer->is_running()) {
    this_thread::sleep_for(chrono::milliseconds(10));
  }

  PluginRegistry pReg;
  pReg.registerPlugin(make_shared<UbntInterfacePlugin>(
      engine.getModelRegistry()->getBindingContext(Model::OPENCONFIG_0_1_6)));
  shared_ptr<DeviceContext> deviceCtx = pReg.getDeviceContext({"ubnt", "*"});

  ReaderRegistryBuilder rRegBuilder{
      engine.getModelRegistry()->getSchemaContext(Model::OPENCONFIG_0_1_6)};
  deviceCtx->provideReaders(rRegBuilder);

  return std::make_unique<StructuredUbntDevice2>(
      app,
      deviceConfig.id,
      deviceConfig.readonly,
      channel,
      engine.getModelRegistry(),
      rRegBuilder.build(),
      cmdCache,
      httpServer,
      move(httpThread)
      );
}

StructuredUbntDevice2::StructuredUbntDevice2(
    Application& application,
    const Id id_,
    bool readonly_,
    const shared_ptr<Channel> _channel,
    const std::shared_ptr<ModelRegistry> _mreg,
    std::unique_ptr<ReaderRegistry>&& _rReg,
    const shared_ptr<CliCache> _cmdCache,
    const std::shared_ptr<CliHttpServer> _httpServer,
    std::unique_ptr<std::thread>&& _httpThread

    )
    : Device(application, id_, readonly_),
      channel(_channel),
      cmdCache(_cmdCache),
      mreg(_mreg),
      rReg(forward<unique_ptr<ReaderRegistry>>(_rReg)),
      httpServer(_httpServer),
      httpThread(forward<unique_ptr<std::thread>>(_httpThread)) {}

void StructuredUbntDevice2::setIntendedDatastore(const dynamic& config) {
  (void)config;
}

shared_ptr<Datastore> StructuredUbntDevice2::getOperationalDatastore() {
  MLOG(MINFO) << "[" << id << "] "
              << "Retrieving state";

  // Reset cache
  cmdCache->wlock()->clear();

  auto state = Datastore::make(*reinterpret_cast<MetricSink*>(&app), getId());
  state->setStatus(true);

  DeviceAccess access = DeviceAccess(channel, id);

  state->addRequest(
      rReg->readState(Path::ROOT, access).thenValue([state](auto v) {
        state->update([&v](auto& lockedState) { lockedState.merge_patch(v); });
      }));

  return state;
}

} // namespace cli
} // namespace devices
} // namespace devmand
