// Copyright (c) 2016-present, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant
// of patent rights can be found in the PATENTS file in the same directory.

#include <devmand/channels/cli/IoConfigurationBuilder.h>
#include <devmand/devices/Datastore.h>
#include <devmand/devices/cli/StructuredUbntDevice.h>
#include <devmand/devices/cli/UbntInterfacePlugin.h>
#include <devmand/devices/cli/UbntStpPlugin.h>
#include <devmand/devices/cli/UbntNetworksPlugin.h>
#include <devmand/devices/cli/schema/ModelRegistry.h>
#include <devmand/devices/cli/translation/PluginRegistry.h>
#include <devmand/devices/cli/translation/ReaderRegistry.h>
#include <devmand/devices/cli/translation/WriterRegistry.h>
#include <folly/json.h>
#include <memory>

namespace devmand {
namespace devices {
namespace cli {

using namespace devmand::channels::cli;
using namespace std;
using namespace folly;

std::unique_ptr<devices::Device> StructuredUbntDevice::createDevice(
    Application& app,
    const cartography::DeviceConfig& deviceConfig) {
  return createDeviceWithEngine(app, deviceConfig, app.getCliEngine());
}

unique_ptr<devices::Device> StructuredUbntDevice::createDeviceWithEngine(
    Application& app,
    const cartography::DeviceConfig& deviceConfig,
    Engine& engine) {
  IoConfigurationBuilder ioConfigurationBuilder(deviceConfig, engine);
  auto cmdCache = ReadCachingCli::createCache();
  auto treeCache = make_shared<TreeCache>(
      ioConfigurationBuilder.getConnectionParameters()->flavour);
  const std::shared_ptr<Channel>& channel = std::make_shared<Channel>(
      deviceConfig.id, ioConfigurationBuilder.createAll(cmdCache, treeCache));

  PluginRegistry pReg;
  pReg.registerPlugin(make_shared<UbntInterfacePlugin>(
      engine.getModelRegistry()->getBindingContext(Model::OPENCONFIG_2_4_3)));
  pReg.registerPlugin(make_shared<UbntStpPlugin>(
      engine.getModelRegistry()->getBindingContext(Model::OPENCONFIG_2_4_3)));
  pReg.registerPlugin(make_shared<UbntNetworksPlugin>(
      engine.getModelRegistry()->getBindingContext(Model::OPENCONFIG_2_4_3)));

  shared_ptr<DeviceContext> deviceCtx = pReg.getDeviceContext({"ubnt", "*"});

  ReaderRegistryBuilder rRegBuilder{
      engine.getModelRegistry()->getSchemaContext(Model::OPENCONFIG_2_4_3)};
  deviceCtx->provideReaders(rRegBuilder);
  WriterRegistryBuilder wRegBuilder{
      engine.getModelRegistry()->getSchemaContext(Model::OPENCONFIG_2_4_3)};
  deviceCtx->provideWriters(wRegBuilder);

  return std::make_unique<StructuredUbntDevice>(
      app,
      deviceConfig.id,
      deviceConfig.readonly,
      channel,
      engine.getModelRegistry(),
      rRegBuilder.build(),
      wRegBuilder.build(),
      cmdCache,
      treeCache);
}

StructuredUbntDevice::StructuredUbntDevice(
    Application& application,
    const Id id_,
    bool readonly_,
    const shared_ptr<Channel> _channel,
    const std::shared_ptr<ModelRegistry> _mreg,
    std::unique_ptr<ReaderRegistry>&& _rReg,
    std::unique_ptr<WriterRegistry>&& _wReg,
    const shared_ptr<CliCache> _cmdCache,
    const shared_ptr<TreeCache> _treeCache)
    : Device(application, id_, readonly_),
      channel(_channel),
      cmdCache(_cmdCache),
      treeCache(_treeCache),
      mreg(_mreg),
      rReg(forward<unique_ptr<ReaderRegistry>>(_rReg)),
      wReg(forward<unique_ptr<WriterRegistry>>(_wReg)),
      configCache(make_unique<devmand::channels::cli::datastore::Datastore>(
          DatastoreType::config,
          app.getCliEngine().getModelRegistry()->getSchemaContext(
              Model::OPENCONFIG_2_4_3))) {}

void StructuredUbntDevice::reconcile(DeviceAccess& access) {
  MLOG(MINFO) << "[" << id << "] "
              << "Reconciling";
  auto reconcileTx = configCache->newTx();
  try {
    dynamic reconciledData = rReg->readConfiguration("/", access).get();
    MLOG(MINFO) << "[" << id << "] "
                << "Reconciled with: " << reconciledData;
    reconcileTx->merge("/", reconciledData);
    reconcileTx->print();
  } catch (DatastoreException& e) {
    reconcileTx->abort();
    throw runtime_error(
        "Invalid configuration for device: " + id + " due to: " + e.what());
  } catch (runtime_error& e) {
    reconcileTx->abort();
    throw runtime_error(
        "Unable to reconcile device: " + id + " due to: " + e.what());
  }
  if (!reconcileTx->isValid()) {
    reconcileTx->abort();
    throw runtime_error(
        "Unable to reconcile device: " + id +
        " due to: Reconciled configuration is not valid");
  }
  reconcileTx->commit();
}

void StructuredUbntDevice::setIntendedDatastore(const dynamic& config) {
  MLOG(MINFO) << "[" << id << "] "
              << "Writing config";

  // Reset cache
  cmdCache->wlock()->clear();
  treeCache->clear(); // FIXME this is not threadsafe

  DeviceAccess access = DeviceAccess(channel, id, getCPUExecutor());
  reconcile(access);

  // Apply new config
  auto tx = configCache->newTx();
  try {
    tx->merge("/", config);
  } catch (DatastoreException& e) {
    tx->abort();
    throw runtime_error("Invalid configuration for device: " + id);
  }

  if (!tx->isValid()) {
    tx->abort();
    throw runtime_error("Invalid configuration for device: " + id);
  }

  MLOG(MINFO) << "[" << id << "] "
              << "Calculating diff";
  vector<DiffPath> diffPaths;
  for (auto& regPath : wReg->getWriterPaths()) {
    // TODO support subtree writers
    diffPaths.push_back(DiffPath(regPath, false, false));
  }
  auto diff = tx->diff(diffPaths);

  // TODO check unhandled paths
  // TODO check if there are any updates and if not log and abort tx

  MLOG(MINFO) << "[" << id << "] "
              << "Submitting to device";
  wReg->write(diff, access);
  tx->commit();

  MLOG(MINFO) << "[" << id << "] "
              << "Config written successfully";
}

shared_ptr<Datastore> StructuredUbntDevice::getOperationalDatastore() {
  MLOG(MINFO) << "[" << id << "] "
              << "Retrieving state";

  // Reset cache
  cmdCache->wlock()->clear();
  treeCache->clear(); // FIXME this is not threadsafe

  auto state = Datastore::make(*reinterpret_cast<MetricSink*>(&app), getId());
  state->setStatus(true);
  //  return state;
  DeviceAccess access = DeviceAccess(channel, id, getCPUExecutor());

  state->addRequest(
      rReg->readState(Path::ROOT, access).thenValue([state](auto v) {
        state->update([&v](auto& lockedState) { lockedState.merge_patch(v); });
      }));

  return state;
}
} // namespace cli
} // namespace devices
} // namespace devmand
