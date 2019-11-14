// Copyright (c) 2019-present, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant
// of patent rights can be found in the PATENTS file in the same directory.

#include <devmand/channels/cli/ReconnectingCli.h>
#include <magma_logging.h>

namespace devmand::channels::cli {

using namespace std;
using namespace folly;

ReconnectingCli::ReconnectingCli(
        std::function<std::shared_ptr<Cli>()> &&_createCliStack) :
        createCliStack(_createCliStack) {
  cli = createCliStack();
}

Future<string> ReconnectingCli::executeAndRead(const Command &cmd) {
  return executeSomething(cmd, "RCli.executeAndRead", [=]() { return cli->executeAndRead(cmd); });
}

Future<string> ReconnectingCli::execute(const Command &cmd) {
  return executeSomething(cmd, "RCli.execute", [=]() { return cli->execute(cmd); });
}

Future<string> ReconnectingCli::executeSomething(const Command &cmd, const string &loggingPrefix,
                                                 const function<Future<string>()> &innerFunc) {
  return innerFunc().thenError(
          folly::tag_t<std::exception>{},
          [&](std::exception const &e) -> Future<string> {
            MLOG(MDEBUG) << loggingPrefix << " got error, recreating cli stack: " << e.what()
                         << " while executing '" << cmd << "'";
            cli = createCliStack();
            throw e;
          });
}
}
