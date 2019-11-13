// Copyright (c) 2019-present, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant
// of patent rights can be found in the PATENTS file in the same directory.

#include <devmand/channels/cli/Command.h>
#include <devmand/channels/cli/KeepaliveCli.h>
#include <magma_logging.h>

#include <iostream>
#include <regex>
#include <chrono>

using devmand::channels::cli::Command;
using devmand::channels::cli::KeepaliveCli;
using std::string;

namespace devmand {
namespace channels {
namespace cli {

KeepaliveCli::KeepaliveCli(
        std::function<std::shared_ptr<Cli>()> _func,
        unsigned int _delay,
        unsigned int _timeout) :
        func(_func),
        delay(_delay),
        timeout(_timeout),
        quit(false)
{
  executor = std::make_shared<folly::CPUThreadPoolExecutor>(1);
  timekeeper = std::make_shared<folly::ThreadWheelTimekeeper>();

  // initialeze cli stack for the first time
  cli = func();
  ready = true;
  MLOG(MDEBUG) << this << ": KACli: new cli " << cli.get() << ")\n";

  // start keepalive loop
  executor->add([&]() { return keepalive(); });
}

KeepaliveCli::~KeepaliveCli() {
  // stop keepalive loop
  MLOG(MDEBUG) << this << ": KACli: destructor begin (tasks " << executor->getPendingTaskCount() << ")\n";
  quit = true;
  executor->stop();
  executor->join();
  MLOG(MDEBUG) << this << ": thread joined\n";
}

folly::Future<string> KeepaliveCli::executeAndRead(const Command& cmd) {
  waitUntilCliReady();
  MLOG(MDEBUG) << this << ": KACli executeAndRead: (cli " << cli.get() << ") '" << cmd << "'\n";

  return cli->executeAndRead(cmd);
}

folly::Future<string> KeepaliveCli::execute(const Command& cmd) {
  waitUntilCliReady();
  MLOG(MDEBUG) << this << ": KACli execute: (cli " << cli.get() << ") '" << cmd << "\n";

  return cli->execute(cmd);
}

void KeepaliveCli::keepalive() {
  int cnt = 0;
  bool skip;

  // infinite keepaly loop (sending keepalive message every "delay" seconds)
  while (!quit) {
    waitUntilCliReady();
    skip = false;
//        std::ostringstream ss;
//        ss << "KA #" << ++cnt << std::endl;
//        Command cmd = Command::makeReadCommand(ss.str());
    Command cmd = Command::makeReadCommand("\n");
    MLOG(MDEBUG) << this << ": KACli: sending Keepalive (cli " << cli.get() << ") #" << ++cnt << " \n";

    // create future chain (ExecuteAndRead(<ENTER>) + onTimeout(timeout)
    outstandingKas.push(folly::makeFuture("GOGOGO")
      .thenValue([=](...) {
        return cli->executeAndRead(cmd);
      })
      .onTimeout(std::chrono::seconds(timeout), [&] {
        MLOG(MERROR) << this << ": KACli: TIMEOUT " << cnt << " (outstanding KAs " << outstandingKas.size() << ")\n";
        // cli stack is being reinitialized, temprorary disable all send operations
        block();

        // make before break (store old cli into tmp, create new cli and then destroy tmp)
        std::shared_ptr<Cli> tmp = std::move(cli);
        if (!quit) {    // if destructor called meantime, do not bother with new stack
          MLOG(MDEBUG) << this << ": KACli: create new cli\n";
          cli = func();   // create new stack
          MLOG(MDEBUG) << this << ": KACli: NEW cli is " << cli.get() << "\n";
          skip = true;
        } else {
          MLOG(MDEBUG) << this << ": KACli: shuting down\n";
        }

        // remove all pending futures on old stack
        while (!outstandingKas.empty()) {
          MLOG(MDEBUG) << this << ": KACli: removing residues (ready " << outstandingKas.front().isReady() << ")\n";
          if (!outstandingKas.front().isReady()) {
            outstandingKas.front().cancel();
          }
          outstandingKas.pop();
        }

        // enable send operations on new stack
        notify();

        // destroy old stack
        MLOG(MDEBUG) << this << ": KACli: delete old cli " << tmp.get() << "\n";
        tmp.reset();
        MLOG(MDEBUG) << this << ": KACli: delete old cli DONE\n";

        return folly::Future<std::string>(std::runtime_error("KA-TIMEOUT"));
      }, timekeeper.get())
//            .thenError(folly::tag_t<std::runtime_error>{}, [&](auto const&) {   // in case something went wrong
//                return folly::Future<std::string>("KA-TIMEOUT");
//            })
    );

    // skip sleep when new stack has been created (send first keepalive as soon as possible)
    if (!skip) {
      MLOG(MDEBUG) << this << ": KACli: sleep\n";
      std::this_thread::sleep_for(std::chrono::seconds(delay));
    }

    // after sleep : check whether keepalive response(s) returned (future.isReady() == true)
    // if so, remove from pending list
    while (!outstandingKas.empty() && outstandingKas.front().isReady()) {
      MLOG(MDEBUG) << this << ": KACli: received Keepalive\n";
      outstandingKas.pop();
    }
  }
}

} // namespace cli
} // namespace channels
}