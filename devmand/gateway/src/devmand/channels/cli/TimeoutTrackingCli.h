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
#include <folly/futures/ThreadWheelTimekeeper.h>

namespace devmand::channels::cli {

using namespace std;
using namespace folly;
using devmand::channels::cli::Command;

// CLI layer that should be instanciated below QueuedCli. It throws FutureTimeout if Future returned by
// underlying layer does not return result within specified time period (timeout).
class TimeoutTrackingCli : public Cli {

 public:
  TimeoutTrackingCli(
          shared_ptr<Cli> _cli,
          shared_ptr<folly::ThreadWheelTimekeeper> _timekeeper,
          std::chrono::milliseconds _timeoutInterval = std::chrono::milliseconds(10 * 1000));

  folly::Future<std::string> executeAndRead(const Command &cmd) override;

  folly::Future<std::string> executeAndSwitchPrompt(const Command &cmd) override;

 private:
  shared_ptr<Cli> cli; // underlying cli layer
  shared_ptr<folly::ThreadWheelTimekeeper> timekeeper;
  std::chrono::milliseconds timeoutInterval;

  Future<string> executeSomething(
          const Command &cmd,
          const string &loggingPrefix,
          const function<Future<string>()>& innerFunc);

};

}
