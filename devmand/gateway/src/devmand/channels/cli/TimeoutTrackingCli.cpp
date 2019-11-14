// Copyright (c) 2019-present, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant
// of patent rights can be found in the PATENTS file in the same directory.

#include <devmand/channels/cli/Command.h>
#include <devmand/channels/cli/TimeoutTrackingCli.h>
#include <magma_logging.h>

namespace devmand::channels::cli {

using namespace std;
using namespace folly;
using devmand::channels::cli::Command;

TimeoutTrackingCli::TimeoutTrackingCli(
        shared_ptr<Cli> _cli,
        shared_ptr<folly::ThreadWheelTimekeeper> _timekeeper,
        shared_ptr<folly::Executor> _executor,
        std::chrono::milliseconds _timeoutInterval) :
        cli(_cli),
        timekeeper(_timekeeper),
        executor(_executor),
        timeoutInterval(_timeoutInterval) {
  shutdown = false;
}

TimeoutTrackingCli::~TimeoutTrackingCli() {
  MLOG(MDEBUG) << "~TimeoutTrackingCli";
  shutdown = true;
}

Future<string> TimeoutTrackingCli::executeAndRead(const Command &cmd) {
  return executeSomething(cmd, "TTCli.executeAndRead",
                          [=]() { return cli->executeAndRead(cmd); });
}

Future<string> TimeoutTrackingCli::execute(const Command &cmd) {
  return executeSomething(cmd, "TTCli.execute",
                          [=]() { return cli->execute(cmd); });
}

Future<string> TimeoutTrackingCli::executeSomething(const Command &cmd, const string &loggingPrefix,
                                                    const function<Future<string>()>& innerFunc) {
  MLOG(MDEBUG) << loggingPrefix << "('" << cmd << "') called";
  Future <string> inner = innerFunc(); // we expect that this method does not block
  if (shutdown) throw runtime_error("TTCli Shutting down 1");
  return move(inner).via(executor.get()).onTimeout(timeoutInterval, [=](...) -> Future<string> {
    MLOG(MDEBUG) << loggingPrefix << "('" << cmd << "') timing out";
    throw FutureTimeout();
  }, timekeeper.get());
}

}
