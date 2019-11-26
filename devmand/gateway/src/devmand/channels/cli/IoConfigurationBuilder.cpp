// Copyright (c) 2019-present, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant
// of patent rights can be found in the PATENTS file in the same directory.

#define LOG_WITH_GLOG

#include <magma_logging.h>

#include <devmand/channels/cli/IoConfigurationBuilder.h>
#include <devmand/channels/cli/KeepaliveCli.h>
#include <devmand/channels/cli/PromptAwareCli.h>
#include <devmand/channels/cli/QueuedCli.h>
#include <devmand/channels/cli/ReadCachingCli.h>
#include <devmand/channels/cli/ReconnectingCli.h>
#include <devmand/channels/cli/SshSession.h>
#include <devmand/channels/cli/SshSessionAsync.h>
#include <devmand/channels/cli/SshSocketReader.h>
#include <devmand/channels/cli/TimeoutTrackingCli.h>
#include <folly/Singleton.h>

namespace devmand {
namespace channels {
namespace cli {

using devmand::channels::cli::IoConfigurationBuilder;
using devmand::channels::cli::SshSocketReader;
using devmand::channels::cli::sshsession::readCallback;
using devmand::channels::cli::sshsession::SshSession;
using devmand::channels::cli::sshsession::SshSessionAsync;
using folly::EvictingCacheMap;

IoConfigurationBuilder::IoConfigurationBuilder(
    const DeviceConfig& deviceConfig,
    channels::cli::Engine& engine) {
  const std::map<std::string, std::string>& plaintextCliKv =
      deviceConfig.channelConfigs.at("cli").kvPairs;

  connectionParameters = makeConnectionParameters(
      deviceConfig.id,
      deviceConfig.ip,
      plaintextCliKv.at("username"),
      plaintextCliKv.at("password"),
      plaintextCliKv.find("flavour") != plaintextCliKv.end()
          ? plaintextCliKv.at("flavour")
          : "",
      std::stoi(plaintextCliKv.at("port")),
      loadTimeout(
          plaintextCliKv,
          configKeepAliveIntervalSeconds,
          defaultKeepaliveInterval),
      loadTimeout(
          plaintextCliKv,
          configMaxCommandTimeoutSeconds,
          defaultCommandTimeout),
      engine.getTimekeeper(),
      engine.getExecutor(Engine::executorRequestType::sshCli),
      engine.getExecutor(Engine::executorRequestType::paCli),
      engine.getExecutor(Engine::executorRequestType::rcCli),
      engine.getExecutor(Engine::executorRequestType::ttCli),
      engine.getExecutor(Engine::executorRequestType::qCli),
      engine.getExecutor(Engine::executorRequestType::rCli),
      engine.getExecutor(Engine::executorRequestType::kaCli));
}

IoConfigurationBuilder::~IoConfigurationBuilder() {
  MLOG(MDEBUG) << "~IoConfigurationBuilder";
}

shared_ptr<Cli> IoConfigurationBuilder::createAll(
    shared_ptr<CliCache> commandCache) {
  return createAllUsingFactory(commandCache);
}

chrono::seconds IoConfigurationBuilder::loadTimeout(
    const std::map<std::string, std::string>& plaintextCliKv,
    const string& configKey,
    chrono::seconds defaultValue) {
  if (plaintextCliKv.find(configKey) != plaintextCliKv.end()) {
    return chrono::seconds(stoi(plaintextCliKv.at(configKey)));
  } else {
    return defaultValue;
  }
}

shared_ptr<Cli> IoConfigurationBuilder::createAllUsingFactory(
    shared_ptr<CliCache> commandCache) {
  function<SemiFuture<shared_ptr<Cli>>()> cliFactory =
      [params = connectionParameters, commandCache]() {
        return createPromptAwareCli(params).thenValue(
            [params, commandCache](shared_ptr<Cli> sshCli) -> shared_ptr<Cli> {
              MLOG(MDEBUG) << "[" << params->id << "] "
                           << "Creating cli layers rcclli, ttcli, qcli";
              // create caching cli
              const shared_ptr<ReadCachingCli>& rccli =
                  std::make_shared<ReadCachingCli>(
                      params->id, sshCli, commandCache, params->rcExecutor);
              // create timeout tracker
              shared_ptr<TimeoutTrackingCli> ttcli = TimeoutTrackingCli::make(
                  params->id,
                  rccli,
                  params->timekeeper,
                  params->ttExecutor,
                  params->cmdTimeout);
              // create Queued cli
              shared_ptr<QueuedCli> qcli =
                  QueuedCli::make(params->id, ttcli, params->qExecutor);
              return qcli;
            });
      };

  // create reconnecting cli that uses cliFactory to establish ssh connection
  shared_ptr<ReconnectingCli> rcli = ReconnectingCli::make(
      connectionParameters->id,
      connectionParameters->rExecutor,
      move(cliFactory),
      connectionParameters->timekeeper);
  // create keepalive cli
  shared_ptr<KeepaliveCli> kaCli = KeepaliveCli::make(
      connectionParameters->id,
      rcli,
      connectionParameters->kaExecutor,
      connectionParameters->timekeeper,
      connectionParameters->kaTimeout);
  return kaCli;
}

Future<shared_ptr<Cli>> IoConfigurationBuilder::createPromptAwareCli(
    shared_ptr<ConnectionParameters> params) {
  MLOG(MDEBUG) << "Creating CLI ssh device for " << params->id << " ("
               << params->ip << ":" << params->port << ")";

  // create session
  std::shared_ptr<SshSessionAsync> session =
      std::make_shared<SshSessionAsync>(params->id, params->sshExecutor);
  // open SSH connection

  MLOG(MDEBUG) << "[" << params->id << "] "
               << "Opening shell";
  // TODO: do this using future chaining
  session
      ->openShell(params->ip, params->port, params->username, params->password)
      .get();

  MLOG(MDEBUG) << "[" << params->id << "] "
               << "Setting flavour";
  shared_ptr<CliFlavour> cl = params->flavour;

  // create CLI
  shared_ptr<PromptAwareCli> cli =
      PromptAwareCli::make(params->id, session, cl, params->paExecutor);
  MLOG(MDEBUG) << "[" << params->id << "] "
               << "Initializing cli";
  // initialize CLI
  // TODO: do this using future chaining:
  //  via(executor.get())
  //      .thenValue([params, cli, session](...) -> SemiFuture<shared_ptr<Cli>>
  //      {

  cli->initializeCli(params->password).get();
  // resolve prompt needs to happen
  MLOG(MDEBUG) << "[" << params->id << "] "
               << "Resolving prompt";
  // TODO: do this using future chaining
  cli->resolvePrompt().get();

  MLOG(MDEBUG) << "[" << params->id << "] "
               << "Creating async data reader";
  event* sessionEvent = SshSocketReader::getInstance().addSshReader(
      readCallback, session->getSshFd(), session.get());
  session->setEvent(sessionEvent);
  MLOG(MDEBUG) << "[" << params->id << "] "
               << "SSH layer configured";
  return makeFuture(cli);
}

shared_ptr<IoConfigurationBuilder::ConnectionParameters>
IoConfigurationBuilder::makeConnectionParameters(
    string id,
    string hostname,
    string username,
    string password,
    string flavour,
    int port,
    chrono::seconds kaTimeout,
    chrono::seconds cmdTimeout,
    shared_ptr<Timekeeper> timekeeper,
    shared_ptr<Executor> sshExecutor,
    shared_ptr<Executor> paExecutor,
    shared_ptr<Executor> rcExecutor,
    shared_ptr<Executor> ttExecutor,
    shared_ptr<Executor> qExecutor,
    shared_ptr<Executor> rExecutor,
    shared_ptr<Executor> kaExecutor) {
  shared_ptr<IoConfigurationBuilder::ConnectionParameters>
      connectionParameters =
          make_shared<IoConfigurationBuilder::ConnectionParameters>();
  connectionParameters->id = id;
  connectionParameters->ip = hostname;
  connectionParameters->username = username;
  connectionParameters->password = password;
  connectionParameters->port = port;
  connectionParameters->flavour = CliFlavour::create(flavour);
  connectionParameters->kaTimeout = kaTimeout;
  connectionParameters->cmdTimeout = cmdTimeout;

  connectionParameters->timekeeper = timekeeper;
  connectionParameters->sshExecutor = sshExecutor;
  connectionParameters->paExecutor = paExecutor;
  connectionParameters->rcExecutor = rcExecutor;
  connectionParameters->ttExecutor = ttExecutor;
  connectionParameters->qExecutor = qExecutor;
  connectionParameters->rExecutor = rExecutor;
  connectionParameters->kaExecutor = kaExecutor;

  return connectionParameters;
}

} // namespace cli
} // namespace channels
} // namespace devmand
