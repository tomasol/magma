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
    : cli(_cli),
    ready(true),
    hi_limit(_hi_limit),
    lo_limit(_lo_limit),
    quit(false) {}

QueuedCli::~QueuedCli() {
  quit = true;
  MLOG(MDEBUG) << this << ": QCli: destructor begin (queue size " << outstandingCmds.size() << ")\n";
  while (!outstandingCmds.empty()) {
    MLOG(MDEBUG) << this << ": Qli: removing residues (" << outstandingCmds.front().isFulfilled() << ")\n";
    if (!outstandingCmds.front().isFulfilled()) {
      outstandingCmds.front().setException(std::runtime_error("CANCELLED"));
    }
    outstandingCmds.pop();
  }
  MLOG(MDEBUG) << this << ": QCli: destructor end\n";
}

folly::Future<string> QueuedCli::executeAndRead(const Command& cmd) {
  bool empty = false;
  MLOG(MDEBUG) << this << ": QCli: executeAndRead: '" << cmd.toString() << "' ready ("
            << ready << ")\n";

  if (outstandingCmds.size() >= hi_limit) {
    MLOG(MDEBUG) << this << ": QCli: queue size (" << outstandingCmds.size()
              << ") reached HI-limit -> WAIT\n";
    wait();
  }

  MLOG(MDEBUG) << this << ": QCli: Enqueued '" << cmd
            << "' (queue size: " << outstandingCmds.size() << ")...\n";

  folly::Promise<std::string> p;
  p.setInterruptHandler([&](const folly::exception_wrapper& e) {
    MLOG(MDEBUG) << this << ": QCli: '" << cmd << "' setInterruptHandler " << e << "\n";
  });
  auto future_exec = p.getFuture();
  folly::Future<std::string> f =
      std::move(future_exec)
          .thenValue([=](...) { return cli->executeAndRead(cmd); })
          .thenValue(
              [=](std::string result) {
                  MLOG(MERROR) << ": QCli: on Value '" << result << "'\n";
                  return returnAndExecNext(result);
              })
          .thenError(folly::tag_t<std::runtime_error>{}, /*[&](auto const&)*/ [this](std::exception const& e) {
              MLOG(MERROR) << ": QCli: on ERROR '" << e.what() << "'\n";
              returnAndExecNext(e.what());
              return folly::Future<std::string>(std::runtime_error(e.what()));
          })
              ;

  { // synchronized insert block
    std::lock_guard<std::mutex> lock(mutex);
    empty = outstandingCmds.empty();
    outstandingCmds.push(std::move(p));
  }

  if (empty) {
    MLOG(MDEBUG) << this << ": QCli: Executing...\n";
    outstandingCmds.front().setValue("GOGOGO");
  } else {
    MLOG(MDEBUG) << this << ": QCli: Enqueued (queue size: " << outstandingCmds.size()
              << ")...\n";
  }

  return f;
}

// THREAD SAFETY: there should only one command being processed at time, so
// there should be no race on returnAndExecNext
folly::Future<string> QueuedCli::returnAndExecNext(std::string result) {
  MLOG(MDEBUG) << this << ": QCli: returnAndExecNext '" << result << "'\n";
  if (quit) {
    MLOG(MDEBUG) << this << ": QCli: EXPIRED\n";
//    return folly::Future<std::string>(result);
    return folly::Future<std::string>(std::runtime_error("EXPIRED"));
  }

  outstandingCmds.pop();
  if (!ready && outstandingCmds.size() <= lo_limit) {
    MLOG(MDEBUG) << this << ": QCli: queue size reached LO-limit(" << lo_limit
              << ") -> RELEASE\n";
    notify();
  }

  if (outstandingCmds.empty()) {
    MLOG(MDEBUG) << this << ": QCli: Queue empty\n";
  } else {
    MLOG(MDEBUG) << this << ": QCli: Executing Next...\n";
    outstandingCmds.front().setValue("GOGOGO");
  }
  return folly::Future<std::string>(result);
}
} // namespace cli
} // namespace channels
}