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

KeepaliveCli::KeepaliveCli(shared_ptr<Cli> _cli,
                           shared_ptr<Executor> _parentExecutor,
                           shared_ptr<ThreadWheelTimekeeper> _timekeeper,
                           Command &&_keepAliveCommand,
                           chrono::milliseconds _heartbeatInterval) :
        cli(_cli),
        serialExecutorKeepAlive(
                SerialExecutor::create(Executor::getKeepAliveToken(_parentExecutor.get()))),
        timekeeper(_timekeeper),
        keepAliveCommand(_keepAliveCommand),
        heartbeatInterval(_heartbeatInterval) {
  assert(_keepAliveCommand.skipCache());
  sendKeepAliveCommand();
}

void KeepaliveCli::sendKeepAliveCommand() {
  MLOG(MDEBUG) << "sendKeepAliveCommand()";
  Future<string> result = via(serialExecutorKeepAlive).thenValue(
          [=](auto) {
            MLOG(MDEBUG) << "sendKeepAliveCommand executing keepalive command";
            return this->executeAndRead(this->keepAliveCommand); });

  scheduleNextPing(move(result));
}

void KeepaliveCli::scheduleNextPing(Future<string> keepAliveCmdFuture) {
  sleepFuture = move(keepAliveCmdFuture).via(serialExecutorKeepAlive)
          .thenValue([=](auto) {
            MLOG(MDEBUG) << "Creating sleep future";
            return futures::sleep(this->heartbeatInterval, timekeeper.get());
          }).thenValue([=](auto) {
            MLOG(MDEBUG) << "Woke up after sleep";
          });
  // FIXME: moving sleepFuture
  move(sleepFuture).thenValue([=](auto) {
    this->sendKeepAliveCommand();
  });
}

KeepaliveCli::~KeepaliveCli() {
  MLOG(MDEBUG) << "~KeepaliveCli: Cancelling future keepalive";
  sleepFuture.cancel(); // if sleeping, wake up
}

Future<string> KeepaliveCli::executeAndRead(const Command &cmd) {
  return cli->executeAndRead(cmd);
}

Future<string> KeepaliveCli::execute(const Command &cmd) {
  return cli->execute(cmd);
}

}
