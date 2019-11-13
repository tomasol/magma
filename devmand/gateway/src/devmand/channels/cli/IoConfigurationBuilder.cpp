// Copyright (c) 2019-present, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant
// of patent rights can be found in the PATENTS file in the same directory.

#include <devmand/channels/cli/IoConfigurationBuilder.h>
#include <devmand/channels/cli/PromptAwareCli.h>
#include <devmand/channels/cli/QueuedCli.h>
#include <devmand/channels/cli/ReadCachingCli.h>
#include <devmand/channels/cli/SshSessionAsync.h>
#include <devmand/channels/cli/SshSession.h>
#include <folly/Singleton.h>
#include <devmand/channels/cli/SshSocketReader.h>
#include <magma_logging.h>
#include <folly/container/EvictingCacheMap.h>
#include <devmand/channels/cli/KeepaliveCli.h>
#include <devmand/channels/cli/TimeoutTrackingCli.h>

namespace devmand::channels::cli {

using devmand::channels::cli::IoConfigurationBuilder;
using devmand::channels::cli::SshSocketReader;
using devmand::channels::cli::sshsession::SshSession;
using devmand::channels::cli::sshsession::SshSessionAsync;
using devmand::channels::cli::sshsession::readCallback;
using folly::IOThreadPoolExecutor;
using folly::EvictingCacheMap;
using std::string;
using std::make_shared;


IoConfigurationBuilder::IoConfigurationBuilder() :
        executor(std::make_shared<IOThreadPoolExecutor>(10)),
        timekeeper(std::make_shared<ThreadWheelTimekeeper>() // TODO: set to nullptr when running with folly to use singleton
                ) {

}

shared_ptr<Cli> IoConfigurationBuilder::createSSH(const DeviceConfig &deviceConfig) {
  MLOG(MDEBUG) << "Creating CLI ssh device for " << deviceConfig.id << " (host: " << deviceConfig.ip << ")";
  const auto &plaintextCliKv = deviceConfig.channelConfigs.at("cli").kvPairs;
  // crate session
  const std::shared_ptr<SshSessionAsync> &session =
          std::make_shared<SshSessionAsync>(executor);
  // TODO opening SSH connection
  session->openShell(
                  deviceConfig.ip,
                  std::stoi(plaintextCliKv.at("port")),
                  plaintextCliKv.at("username"),
                  plaintextCliKv.at("password"))
          .get();

  shared_ptr<CliFlavour> cl = plaintextCliKv.find("flavour") != plaintextCliKv.end()
                              ? CliFlavour::create(plaintextCliKv.at("flavour")) : CliFlavour::create("");

  // TODO create CLI - how to create a CLI stack?
  shared_ptr<PromptAwareCli> cli = std::make_shared<PromptAwareCli>(session, cl);

  // TODO initialize CLI
  cli->initializeCli();
  // TODO resolve prompt needs to happen
  cli->resolvePrompt();
  //TODO create async data reader
  event *sessionEvent = SshSocketReader::getInstance().addSshReader(readCallback, session->getSshFd(), session.get());
  session->setEvent(sessionEvent);
  return cli;
}

shared_ptr<Cli> IoConfigurationBuilder::getIo(shared_ptr<Cli> underlyingCliLayer) {

  // create caching cli
  const shared_ptr<ReadCachingCli> &ccli = std::make_shared<ReadCachingCli>(underlyingCliLayer, ReadCachingCli::createCache());
  // create timeout tracker
  const shared_ptr<TimeoutTrackingCli> &ttcli = std::make_shared<TimeoutTrackingCli>(ccli, timekeeper, executor);
  // create Queued cli
  const shared_ptr<QueuedCli> &qcli = std::make_shared<QueuedCli>(ttcli, executor);
  // create keepalive cli
  const shared_ptr<KeepaliveCli> kacli = std::make_shared<KeepaliveCli>(qcli, executor, timekeeper);
  return kacli;
}
}
