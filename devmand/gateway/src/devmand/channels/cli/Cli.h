// Copyright (c) 2019-present, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant
// of patent rights can be found in the PATENTS file in the same directory.

#pragma once

#include <devmand/channels/cli/Command.h>
#include <folly/futures/Future.h>
#include <folly/futures/Promise.h>
#include <folly/executors/ThreadedExecutor.h>

#include <chrono>
#include <thread>

namespace devmand {
namespace channels {
namespace cli {
class Cli {
 public:
  Cli() = default;
  virtual ~Cli() = default;
  Cli(const Cli&) = delete;
  Cli& operator=(const Cli&) = delete;
  Cli(Cli&&) = delete;
  Cli& operator=(Cli&&) = delete;

 public:
  virtual folly::Future<std::string> executeAndRead(const Command& cmd) = 0;

  virtual folly::Future<std::string> executeAndSwitchPrompt(
      const Command& cmd) = 0;
};

// TODO remove once not necessary (when the real CLI stack is implemented)

class EchoCli : public Cli {
 public:
  folly::Future<std::string> executeAndRead(const Command& cmd) override {
    return folly::Future<std::string>(cmd.toString());
  }

  folly::Future<std::string> executeAndSwitchPrompt(
      const Command& cmd) override {
    return folly::Future<std::string>(cmd.toString());
  }
};

// TODO remove once not necessary (when the real CLI stack is implemented)

    class AsyncEchoCli : public Cli {
    public:
        AsyncEchoCli(std::shared_ptr<folly::ThreadedExecutor> _executor) : executor(_executor) {}

        folly::Future<std::string> executeAndRead(const Command& cmd) override {
            DLOG(INFO) << ": AsyncEchoCli:executeAndRead '" << cmd << "'\n";

            folly::Promise<std::string> p;
            folly::Future<std::string> f = p.getFuture()
                    .via(executor.get())
                    .thenValue([=](...) { return delay(cmd); });
            p.setValue("GOGOGO");
            return f;
        }

        folly::Future<std::string> executeAndSwitchPrompt(
                const Command& cmd) override {
            return folly::Future<std::string>(cmd.toString());
        }

    private:
        std::shared_ptr<folly::ThreadedExecutor> executor;

        folly::Future<std::string> delay(const Command& cmd) {
            DLOG(INFO) << ": AsyncEchoCli:delay [" << std::this_thread::get_id() << "] '" << cmd << "' before\n";
            std::this_thread::sleep_for(std::chrono::seconds(1));
            DLOG(INFO) << ": AsyncEchoCli:delay [" << std::this_thread::get_id() << "] '" << cmd << "' after\n";
            return folly::Future<std::string>(cmd.toString());
        }
    };

} // namespace cli
} // namespace channels
} // namespace devmand