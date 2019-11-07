// Copyright (c) 2019-present, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant
// of patent rights can be found in the PATENTS file in the same directory.

#pragma once

#include <devmand/channels/cli/Cli.h>
#include <folly/futures/Future.h>
#include <folly/Executor.h>
#include <folly/executors/SerialExecutor.h>

namespace devmand::channels::cli {

using namespace std;
using namespace folly;

class QueuedCli2 : public Cli {
 private:
  shared_ptr<Cli> cli;
  Executor::KeepAlive<SerialExecutor> serialExecutorKeepAlive;

 public:

  QueuedCli2(shared_ptr<Cli> _cli, const shared_ptr<Executor>& _parentExecutor);

  QueuedCli2() = delete;

  QueuedCli2(const QueuedCli2 &) = delete;

  Future<string> executeAndRead(const Command &cmd) override;

  Future<string> executeAndSwitchPrompt(const Command &cmd) override;

 private:
  Future<string> executeSomething(const string &prefix, const Command &cmd, const function<Future<string>()>& innerFunc);
};
}
