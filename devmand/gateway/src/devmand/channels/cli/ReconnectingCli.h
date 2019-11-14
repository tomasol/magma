// Copyright (c) 2019-present, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant
// of patent rights can be found in the PATENTS file in the same directory.

#pragma once


#include <devmand/channels/cli/Cli.h>
#include <devmand/channels/cli/Command.h>
#include <folly/futures/Future.h>
#include <folly/Executor.h>
#include <folly/executors/SerialExecutor.h>
#include <folly/futures/ThreadWheelTimekeeper.h>

namespace devmand::channels::cli {

using namespace std;
using namespace folly;
using devmand::channels::cli::Command;

class ReconnectingCli : public Cli {

 public:
  explicit ReconnectingCli(function<shared_ptr<Cli>()> &&createCliStack);

  folly::Future<std::string> executeAndRead(const Command &cmd) override;

  folly::Future<std::string> execute(const Command &cmd) override;

 private:
  function<shared_ptr<devmand::channels::cli::Cli>()> createCliStack;

  shared_ptr<devmand::channels::cli::Cli> cli;

  Future<string> executeSomething(const Command &cmd, const string &loggingPrefix,
                                  const function<Future<string>()> &innerFunc);

};
}
