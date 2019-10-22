// Copyright (c) 2019-present, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant
// of patent rights can be found in the PATENTS file in the same directory.

#include <devmand/channels/cli/Command.h>
#include <devmand/channels/cli/QueuedCli.h>
#include <folly/executors/IOThreadPoolExecutor.h>
#include <folly/futures/Promise.h>

#include <iostream>
#include <regex>

using devmand::channels::cli::Command;
using devmand::channels::cli::QueuedCli;
using std::string;

namespace devmand {
namespace channels {
namespace cli {

QueuedCli::QueuedCli(
    std::shared_ptr<Cli> _cli,
    unsigned int _hi_limit,
    unsigned int _lo_limit)
    : cli(_cli), ready(true), hi_limit(_hi_limit), lo_limit(_lo_limit) {}

folly::Future<string> QueuedCli::executeAndRead(const Command& cmd) {
  bool empty = false;
  DLOG(INFO) << this << ": QCli received: '" << cmd.toString() << "' ready ("
            << ready << ")\n";

  if (outstandingCmds.size() >= hi_limit) {
    DLOG(INFO) << this << ": queue size (" << outstandingCmds.size()
              << ") reached HI-limit -> WAIT\n";
    wait();
  }

  DLOG(INFO) << this << ": Enqueued '" << cmd
            << "' (queue size: " << outstandingCmds.size() << ")...\n";

  folly::Promise<std::string> p;
  auto future_exec = p.getFuture();
  folly::Future<std::string> f =
      std::move(future_exec)
          .thenValue([=](...) { return cli->executeAndRead(cmd); })
          .thenValue(
              [=](std::string result) { return returnAndExecNext(result); });

  { // synchronized insert block
    std::lock_guard<std::mutex> lock(mutex);
    empty = outstandingCmds.empty();
    outstandingCmds.push(std::move(p));
  }

  if (empty) {
    DLOG(INFO) << this << ": Executing...\n";
    outstandingCmds.front().setValue("GOGOGO");
  } else {
    DLOG(INFO) << this << ": Enqueued (queue size: " << outstandingCmds.size()
              << ")...\n";
  }

  return f;
}

// THREAD SAFETY: there should only one command beinbg processed at time, so
// there should be no race on returnAndExecNext
folly::Future<string> QueuedCli::returnAndExecNext(std::string result) {
  DLOG(INFO) << this << ": returnAndExecNext '" << result << "'\n";

  outstandingCmds.pop();
  if (!ready && outstandingCmds.size() <= lo_limit) {
    DLOG(INFO) << this << ": queue size reached LO-limit(" << lo_limit
              << ") -> RELEASE\n";
    notify();
  }

  if (outstandingCmds.empty()) {
    DLOG(INFO) << this << ": Queue empty\n";
  } else {
    DLOG(INFO) << this << ": Executing Next...\n";
    outstandingCmds.front().setValue("GOGOGO");
  }
  return folly::Future<std::string>(result);
}
} // namespace cli
} // namespace channels
}