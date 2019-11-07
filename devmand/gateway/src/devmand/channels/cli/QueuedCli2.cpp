// Copyright (c) 2019-present, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant
// of patent rights can be found in the PATENTS file in the same directory.

#include <devmand/channels/cli/QueuedCli2.h>

namespace devmand::channels::cli {
using namespace std;
using namespace folly;

QueuedCli2::QueuedCli2(shared_ptr<Cli> _cli, const shared_ptr<Executor>& _parentExecutor) :
        cli(_cli),
        serialExecutorKeepAlive(
                SerialExecutor::create(Executor::getKeepAliveToken(_parentExecutor.get()))) {
}

Future<string> QueuedCli2::executeSomething(const string &prefix, const Command &cmd,
                                            const function<Future<string>()> &innerFunc) {
  MLOG(MDEBUG) << prefix << "called with cmd : '" << cmd << "'";
  auto func = [=] {
    MLOG(MDEBUG) << prefix << " started : '" << cmd << "'";
    Future<string> result = innerFunc();
    MLOG(MDEBUG) << prefix << " future obtained : '" << cmd << "'";
    return result;
  };
  return via(serialExecutorKeepAlive, func).thenValue([=](std::string result) {
    MLOG(MDEBUG) << prefix << " finished: '" << cmd << "' with result '" << result << "'";
    return result;
  });
}

Future<string> QueuedCli2::executeAndRead(const Command &cmd) {
  string prefix = "QCli.executeAndRead";
  return executeSomething(prefix, cmd, [=](...) { return cli->executeAndRead(cmd); });
}

Future<string> QueuedCli2::executeAndSwitchPrompt(const Command &cmd) {
  string prefix = "QCli.executeAndSwitchPrompt";
  return executeSomething(prefix, cmd, [=](...) { return cli->executeAndSwitchPrompt(cmd); });
}

}
