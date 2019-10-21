// Copyright (c) 2019-present, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant
// of patent rights can be found in the PATENTS file in the same directory.

#pragma once

#include <devmand/channels/cli/Cli.h>
#include <folly/futures/Future.h>
#include <mutex>
#include <queue>

namespace devmand {
namespace channels {
namespace cli {

class QueuedCli : public Cli { // TODO: check thread safety
 private:
  std::shared_ptr<Cli> cli;
  std::queue<folly::Promise<std::string>>
      outstandingCmds; // TODO: check q size limit
  std::mutex mutex;

 public:
  QueuedCli(
      std::shared_ptr<Cli> cli,
      unsigned int hi_limit = 1000,
      unsigned int lo_limit = 900);

  folly::Future<std::string> executeAndRead(const Command& cmd) override;

  folly::Future<std::string> executeAndSwitchPrompt(
      const Command& cmd) override {
    LOG(ERROR) << "[" << this << "] "
               << "Not Implemented\n";
    return folly::Future<std::string>(cmd.toString());
  }

  folly::Future<std::string> returnAndExecNext(std::string result);

 private:
  void wait() {
    ready = false;
    std::unique_lock<std::mutex> lk(m);
    cv.wait(lk, [this] { return ready; });
  }

  void notify() {
    {
      std::lock_guard<std::mutex> lk(m);
      ready = true;
    }
    cv.notify_one();
  }

 private:
  std::mutex m;
  std::condition_variable cv;
  bool ready{false};
  unsigned int hi_limit, lo_limit;
};

} // namespace cli
} // namespace channels
} // namespace devmand
