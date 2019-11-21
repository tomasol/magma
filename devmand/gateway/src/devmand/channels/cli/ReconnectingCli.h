// Copyright (c) 2019-present, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant
// of patent rights can be found in the PATENTS file in the same directory.

#pragma once

#include <boost/thread/mutex.hpp>
#include <devmand/channels/cli/Cli.h>
#include <devmand/channels/cli/Command.h>
#include <folly/Executor.h>
#include <folly/executors/SerialExecutor.h>
#include <folly/futures/Future.h>
#include <folly/futures/ThreadWheelTimekeeper.h>

namespace devmand {
namespace channels {
namespace cli {

using namespace std;
using namespace folly;
using boost::mutex;
using devmand::channels::cli::Cli;
using devmand::channels::cli::Command;

class ReconnectingCli : public Cli,
                        public enable_shared_from_this<ReconnectingCli> {
 public:
  static shared_ptr<ReconnectingCli> make(
      string id,
      shared_ptr<Executor> executor,
      function<shared_ptr<Cli>()>&& createCliStack,
      shared_ptr<Timekeeper> timekeeper,
      chrono::milliseconds quietPeriod = chrono::seconds(5));

  ~ReconnectingCli() override;

  folly::Future<std::string> executeAndRead(const Command& cmd) override;

  folly::Future<std::string> execute(const Command& cmd) override;

 private:
  struct ReconnectParameters {
    string id;

    atomic<bool> isReconnecting;

    atomic<bool> shutdown;

    shared_ptr<Executor> executor;

    function<shared_ptr<Cli>()> createCliStack;

    mutex cliMutex;

    // guarded by mutex
    shared_ptr<Cli> maybeCli;

    std::chrono::milliseconds quietPeriod;

    shared_ptr<Timekeeper> timekeeper;
  };

  shared_ptr<ReconnectParameters> reconnectParameters;

  ReconnectingCli(
      string id,
      shared_ptr<Executor> executor,
      function<shared_ptr<Cli>()>&& createCliStack,
      shared_ptr<Timekeeper> timekeeper,
      chrono::milliseconds quietPeriod);

  Future<string> executeSomething(
      const string&& loggingPrefix,
      const function<Future<string>(shared_ptr<Cli>)>& innerFunc,
      const string&& loggingSuffix);

  static void triggerReconnect(shared_ptr<ReconnectParameters> params);
};
} // namespace cli
} // namespace channels
} // namespace devmand
