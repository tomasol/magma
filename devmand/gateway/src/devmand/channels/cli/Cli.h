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
#include <folly/executors/CPUThreadPoolExecutor.h>
#include <magma_logging.h>

#include <chrono>
#include <thread>
#include <vector>

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

class ErrCli : public Cli {
public:
    folly::Future<std::string> executeAndRead(const Command& cmd) override {
        throw std::runtime_error(cmd.toString());
        return folly::Future<std::string>(std::runtime_error(cmd.toString()));
    }

    folly::Future<std::string> executeAndSwitchPrompt(
            const Command& cmd) override {
        throw std::runtime_error(cmd.toString());
        return folly::Future<std::string>(std::runtime_error(cmd.toString()));
    }
};

class AsyncCli : public Cli {
public:
    AsyncCli(
            std::shared_ptr<Cli> _cli,
            std::vector<unsigned int> _durations) :
            cli(_cli),
            durations(_durations),
            index(0),
            quit(false)
    {
        executor = std::make_shared<folly::CPUThreadPoolExecutor>(1);
    }

    ~AsyncCli() {
        MLOG(MDEBUG) << this << ": AsyncCli: desctructor begin (tasks " << executor->getPendingTaskCount() << ")\n";
        quit = true;
        executor->stop();
        MLOG(MDEBUG) << this << ": AsyncCli: desctructor after stop (tasks " << executor->getPendingTaskCount() << ")\n";
        executor->join();
        MLOG(MDEBUG) << this << ": AsyncCli: desctructor after join (tasks " << executor->getPendingTaskCount() << ")\n";
    }

    folly::Future<std::string> executeAndRead(const Command& cmd) override {
        MLOG(MDEBUG) << this << ": AsyncCli:executeAndRead '" << cmd << "'\n";

        folly::Promise<std::string> p;
        folly::Future<std::string> f = p.getFuture()
                .via(executor.get())
                .thenValue([=](...) {
                    unsigned int tis = durations[(index++)%durations.size()];
                    if (quit) {
                        MLOG(MDEBUG) << this << ": AsyncCli: '" << cmd << "' ABORTED\n";
                        return folly::Future<std::string>(std::runtime_error(cmd.toString()));
                    }
                    MLOG(MDEBUG) << this << ": AsyncCli: '" << cmd << "' busy for " << tis << " second(s) (tid " << std::this_thread::get_id() << ")\n";
                    std::this_thread::sleep_for(std::chrono::seconds(tis));

                    MLOG(MDEBUG) << this << ": AsyncCli: '" << cmd << "' done\n";
                    return cli->executeAndRead(cmd);
                });
        p.setValue("GOGOGO");
        return f;
    }

    folly::Future<std::string> executeAndSwitchPrompt(
            const Command& cmd) override {
        MLOG(MDEBUG) << this << ": AsyncCli:executeAndSwitchPrompt '" << cmd << "'\n";

        folly::Promise<std::string> p;
        folly::Future<std::string> f = p.getFuture()
                .via(executor.get())
                .thenValue([=](...) {
                    unsigned int tis = durations[(index++)%durations.size()];
                    if (quit) {
                        MLOG(MDEBUG) << this << ": AsyncCli: '" << cmd << "' ABORTED\n";
                        return folly::Future<std::string>(std::runtime_error(cmd.toString()));
                    }
                    MLOG(MDEBUG) << this << ": AsyncCli: '" << cmd << "' busy for " << tis << " second(s) (tid " << std::this_thread::get_id() << ")\n";
                    std::this_thread::sleep_for(std::chrono::seconds(tis));

                    MLOG(MDEBUG) << this << ": AsyncCli: '" << cmd << "' done\n";
                    return cli->executeAndSwitchPrompt(cmd);
                });
        p.setValue("GOGOGO");
        return f;
    }

protected:
    std::shared_ptr<Cli> cli;                 // underlying cli layers
    std::shared_ptr<folly::CPUThreadPoolExecutor> executor;
    std::vector<unsigned int> durations;
    unsigned int index;
    bool quit;
};

// TODO remove once not necessary (when the real CLI stack is implemented)

} // namespace cli
} // namespace channels
} // namespace devmand