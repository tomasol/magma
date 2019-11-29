// Copyright (c) 2019-present, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant
// of patent rights can be found in the PATENTS file in the same directory.

#include <devmand/channels/cli/Command.h>
#include <devmand/channels/cli/KeepaliveCli.h>
#include <magma_logging.h>

namespace devmand::channels::cli {

using devmand::channels::cli::Command;
using namespace std;
using namespace folly;

shared_ptr<KeepaliveCli> KeepaliveCli::make(
    string id,
    shared_ptr<Cli> cli,
    shared_ptr<folly::Executor> parentExecutor,
    shared_ptr<folly::ThreadWheelTimekeeper> timekeeper,
    chrono::milliseconds heartbeatInterval,
    string keepAliveCommand,
    chrono::milliseconds backoffAfterKeepaliveTimeout) {
  const shared_ptr<KeepaliveCli>& result =
      shared_ptr<KeepaliveCli>(new KeepaliveCli(
          id,
          cli,
          parentExecutor,
          timekeeper,
          heartbeatInterval,
          move(keepAliveCommand),
          backoffAfterKeepaliveTimeout));
  return result;
}

KeepaliveCli::KeepaliveCli(
    string _id,
    shared_ptr<Cli> _cli,
    shared_ptr<Executor> _parentExecutor,
    shared_ptr<ThreadWheelTimekeeper> _timekeeper,
    chrono::milliseconds _heartbeatInterval,
    string _keepAliveCommand,
    chrono::milliseconds _backoffAfterKeepaliveTimeout) {
  keepaliveParameters = shared_ptr<KeepaliveParameters>(new KeepaliveParameters{
      /* id */ _id,
      /* cli */ _cli,
      /* timekeeper */ _timekeeper,
      /* parentExecutor */ _parentExecutor,
      /* serialExecutorKeepAlive */
      SerialExecutor::create(
          Executor::getKeepAliveToken(_parentExecutor.get())),
      /* keepAliveCommand */ move(_keepAliveCommand),
      /* heartbeatInterval */ _heartbeatInterval,
      /* backoffAfterKeepaliveTimeout */ _backoffAfterKeepaliveTimeout,
      /* shutdown */ {false}});

  MLOG(MDEBUG) << "[" << _id << "] "
               << "initialized";
  triggerSendKeepAliveCommand(keepaliveParameters);
}

KeepaliveCli::~KeepaliveCli() {
  keepaliveParameters->shutdown = true;
  MLOG(MDEBUG) << "[" << keepaliveParameters->id << "] "
               << "~KeepaliveCli";
  while (keepaliveParameters.use_count() >
         1) { // TODO cancel currently running future
    MLOG(MDEBUG) << "[" << keepaliveParameters->id << "] "
                 << "~KeepaliveCli sleeping";
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
  MLOG(MDEBUG) << "[" << keepaliveParameters->id << "] "
               << "~KeepaliveCli done";
}

void KeepaliveCli::triggerSendKeepAliveCommand(
    shared_ptr<KeepaliveParameters> keepaliveParameters) {
  if (keepaliveParameters->shutdown) {
    MLOG(MDEBUG) << "[" << keepaliveParameters->id << "] "
                 << "triggerSendKeepAliveCommand shutting down";
    return;
  }
  MLOG(MDEBUG) << "[" << keepaliveParameters->id << "] "
               << "triggerSendKeepAliveCommand";

  via(keepaliveParameters->serialExecutorKeepAlive)
      .thenValue([params = keepaliveParameters](auto) {
        ReadCommand cmd = ReadCommand::create(params->keepAliveCommand, false);
        MLOG(MDEBUG)
            << "[" << params->id << "] (" << cmd.getIdx() << ") "
            << "triggerSendKeepAliveCommand executing keepalive command";

        return params->cli->executeRead(cmd);
      })
      .thenValue([params = keepaliveParameters](auto) -> SemiFuture<Unit> {
        MLOG(MDEBUG) << "[" << params->id << "] "
                     << "Creating sleep future";
        return futures::sleep(
            params->heartbeatInterval, params->timekeeper.get());
      })
      .thenValue([keepaliveParameters](auto) -> Unit {
        MLOG(MDEBUG) << "[" << keepaliveParameters->id << "] "
                     << "Woke up after sleep";
        triggerSendKeepAliveCommand(keepaliveParameters);
        return Unit{};
      })
      .thenError(
          folly::tag_t<std::exception>{},
          [params =
               keepaliveParameters](std::exception const& e) -> Future<Unit> {
            MLOG(MINFO)
                << "[" << params->id << "] "
                << "Got error running keepalive, backing off "
                << e.what(); // FIXME: real exception is not propagated here

            return futures::sleep(
                       params->backoffAfterKeepaliveTimeout,
                       params->timekeeper.get())
                .via(params->serialExecutorKeepAlive)
                .thenValue([params](auto) -> Unit {
                  MLOG(MDEBUG) << "[" << params->id << "] "
                               << "Woke up after backing off";
                  triggerSendKeepAliveCommand(params);
                  return Unit{};
                });
          });

  MLOG(MDEBUG) << "[" << keepaliveParameters->id << "] "
               << "triggerSendKeepAliveCommand() done";
}

Future<string> KeepaliveCli::executeRead(const ReadCommand cmd) {
  return keepaliveParameters->cli->executeRead(cmd);
}

Future<string> KeepaliveCli::executeWrite(const WriteCommand cmd) {
  return keepaliveParameters->cli->executeWrite(cmd);
}

} // namespace devmand::channels::cli
