// Copyright (c) 2019-present, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant
// of patent rights can be found in the PATENTS file in the same directory.

#pragma once

#include <devmand/channels/cli/Cli.h>
#include <folly/Executor.h>
#include <folly/futures/Future.h>
#include <folly/futures/ThreadWheelTimekeeper.h>
#include <folly/executors/SerialExecutor.h>

namespace devmand::channels::cli {
using namespace std;
using devmand::channels::cli::Command;

// CLI layer that should be above QueuedCli. Periodically schedules keepalive command to prevent dropping
// of inactive connection.
class KeepaliveCli : public Cli {
 public:
  KeepaliveCli(
          shared_ptr <Cli> _cli,
          shared_ptr <folly::Executor> parentExecutor,
          shared_ptr<folly::ThreadWheelTimekeeper> _timekeeper,
          Command &&keepAliveCommand = Command::makeReadCommand("\n", true),
          std::chrono::milliseconds heartbeatInterval = std::chrono::milliseconds(60 * 1000));

  ~KeepaliveCli() override;

  folly::Future<string> executeAndRead(const Command &cmd) override;

  folly::Future<string> execute(const Command &cmd) override;

 private:
  const shared_ptr <Cli> cli; // underlying cli layer
  const folly::Executor::KeepAlive<folly::SerialExecutor> serialExecutorKeepAlive;
  shared_ptr<folly::ThreadWheelTimekeeper> timekeeper;
  const Command keepAliveCommand;
  const chrono::milliseconds heartbeatInterval;

  folly::Future<folly::Unit> sleepFuture; // only accessed from single thread


  void scheduleNextPing(folly::Future<string> keepAliveCmdFuture);

  void sendKeepAliveCommand();
};
}
