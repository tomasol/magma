// Copyright (c) 2019-present, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant
// of patent rights can be found in the PATENTS file in the same directory.

#pragma once

#include <devmand/channels/cli/Cli.h>
#include <folly/executors/CPUThreadPoolExecutor.h>
#include <folly/futures/Future.h>

namespace devmand {
namespace channels {
namespace cli {

class KeepaliveCli : public Cli {
 private:
  std::shared_ptr<Cli> cli;                     // underlaying cli layer
  std::function<std::shared_ptr<Cli>()> func;   // underlaying initialozator
  unsigned int delay;                           // how often to send keepalives
  unsigned int timeout;                         // how long does it take to expire a keepalalive
  bool quit;                                    // shut the infinite keepalive loop down, when cli gets destroyed
  bool ready;                                   // prohibit cli operations when underlaying stack is being reinitalized
  std::shared_ptr<folly::CPUThreadPoolExecutor> executor;   // runs keepalive loop in separate thread
  std::queue<folly::Future<std::string>> outstandingKas;    // list of sent keepalives

 public:
  KeepaliveCli(
      std::function<std::shared_ptr<Cli>()> func,
      unsigned int delay = 1,
      unsigned int timeout = 60);

  ~KeepaliveCli();

  folly::Future<std::string> executeAndRead(const Command& cmd) override;

  folly::Future<std::string> executeAndSwitchPrompt(const Command& cmd) override;

  void keepalive();
};

} // namespace cli
} // namespace channels
} // namespace devmand
