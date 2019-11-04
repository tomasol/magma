// Copyright (c) 2019-present, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant
// of patent rights can be found in the PATENTS file in the same directory.

#pragma once

#include <devmand/channels/cli/Cli.h>
#include <folly/futures/Future.h>
#include <magma_logging.h>
#include <mutex>
#include <queue>

namespace devmand {
namespace channels {
namespace cli {

class QueuedCli : public Cli {
 private:
  std::shared_ptr<Cli> cli;                 // underlying cli layers
  std::queue<folly::Promise<std::string>>
      outstandingCmds;                      // queue of commands waiting for processing
  std::mutex mutex;                         // blocking lock when queue size limit reached

 public:
  QueuedCli(
      std::shared_ptr<Cli> cli,
      unsigned int hi_limit = 1000,         // default queue block size limit
      unsigned int lo_limit = 900);         // default queue release block size

  ~QueuedCli();

  folly::Future<std::string> executeAndRead(const Command& cmd) override;

  folly::Future<std::string> executeAndSwitchPrompt(
      const Command& cmd) override {
    MLOG(MERROR) << "[" << this << "] "
               << "Not Implemented\n";
    return folly::Future<std::string>(cmd.toString());
  }

  folly::Future<std::string> returnAndExecNext(std::string result);     // get next from queue when pending finished

 private:
  void wait() {     // blocking wait on conditional variable when queue full
    ready = false;
    std::unique_lock<std::mutex> lk(m);
    cv.wait(lk, [this] { return ready; });
  }

  void notify() {   // unblock threads waiting on conditional variable
    {
      std::lock_guard<std::mutex> lk(m);
      ready = true;
    }
    cv.notify_all();
  }

 private:
  std::mutex m;
  std::condition_variable cv;
  bool ready{false};
  unsigned int hi_limit, lo_limit;
  bool quit;       // do not process next when
};

} // namespace cli
} // namespace channels
} // namespace devmand
