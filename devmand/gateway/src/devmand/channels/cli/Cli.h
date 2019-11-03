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

// TODO remove once not necessary (when the real CLI stack is implemented)

class AsyncEchoCli : public Cli {
public:
    AsyncEchoCli(std::shared_ptr<folly::CPUThreadPoolExecutor> _executor,
                 const unsigned int _timeout_in_seconds = 1) :
                 executor(_executor),
                 index(0)
    {
        durations = {_timeout_in_seconds};
    }

    AsyncEchoCli(std::vector<unsigned int> _durations) :
            durations(_durations),
            index(0),
            quit(false)
    {
        executor = std::make_shared<folly::CPUThreadPoolExecutor>(1);
    }

    ~AsyncEchoCli() {
        DLOG(INFO) << this << ": AsyncEchoCli: desctructor begin (tasks " << executor->getPendingTaskCount() << ")\n";
        quit = true;
        executor->stop();
        DLOG(INFO) << this << ": AsyncEchoCli: desctructor after stop (tasks " << executor->getPendingTaskCount() << ")\n";
        executor->join();
        DLOG(INFO) << this << ": AsyncEchoCli: desctructor after join (tasks " << executor->getPendingTaskCount() << ")\n";
    }

    folly::Future<std::string> executeAndRead(const Command& cmd) override {
        DLOG(INFO) << this << ": AsyncEchoCli:executeAndRead '" << cmd << "'\n";

        folly::Promise<std::string> p;
        folly::Future<std::string> f = p.getFuture()
                .via(executor.get())
                .thenValue([=](...) { return delay(cmd, durations[(index++)%durations.size()]); });
        p.setValue("GOGOGO");
        return f;
    }

    folly::Future<std::string> executeAndSwitchPrompt(
            const Command& cmd) override {
        DLOG(INFO) << this << ": AsyncEchoCli:executeAndSwitchPrompt '" << cmd << "'\n";

        folly::Promise<std::string> p;
        folly::Future<std::string> f = p.getFuture()
                .via(executor.get())
                .thenValue([=](...) { return delay(cmd, durations[(index++)%durations.size()]); });
        p.setValue("GOGOGO");
        return f;
    }

protected:
    std::shared_ptr<folly::CPUThreadPoolExecutor> executor;
    std::vector<unsigned int> durations;
    unsigned int index;
    bool quit;

    folly::Future<std::string> delay(const Command& cmd, unsigned int tis) {
        if (quit) {
            DLOG(INFO) << this << ": AsyncEchoCli: '" << cmd << "' ABORTED\n";
            return folly::Future<std::string>(std::runtime_error(cmd.toString()));
        }
        DLOG(INFO) << this << ": AsyncEchoCli: '" << cmd << "' busy for " << tis << " second(s) (tid " << std::this_thread::get_id() << ")\n";
        std::this_thread::sleep_for(std::chrono::seconds(tis));

        DLOG(INFO) << this << ": AsyncEchoCli: '" << cmd << "' done\n";
        return folly::Future<std::string>(cmd.toString());
    }
};

// TODO remove once not necessary (when the real CLI stack is implemented)

class ErrCli : public Cli {
public:
    folly::Future<std::string> executeAndRead(const Command& cmd) override {
        throw std::runtime_error("FAAAAAIL");
        return folly::Future<std::string>(cmd.toString());
    }

    folly::Future<std::string> executeAndSwitchPrompt(
            const Command& cmd) override {
        throw std::runtime_error("FAAAAAIL");
        return folly::Future<std::string>(cmd.toString());
    }
};

class AsyncErrCli : public Cli {
public:
    AsyncErrCli(std::shared_ptr<folly::CPUThreadPoolExecutor> _executor,
                const unsigned int _timeout_in_seconds = 1) :
            executor(_executor),
            index(0)
    {
        durations = {_timeout_in_seconds};
    }

    AsyncErrCli(std::vector<unsigned int> _durations) :
            durations(_durations),
            index(0),
            quit(false)
    {
        executor = std::make_shared<folly::CPUThreadPoolExecutor>(1);
    }

    ~AsyncErrCli() {
        DLOG(INFO) << this << ": AsyncErrCli: desctructor begin\n";
        quit = true;
        executor->join();
        DLOG(INFO) << this << ": AsyncErrCli: desctructor end\n";
    }

    folly::Future<std::string> executeAndRead(const Command& cmd) override {
        DLOG(INFO) << this << ": AsyncErrCli:executeAndRead '" << cmd << "'\n";

        folly::Promise<std::string> p;
        folly::Future<std::string> f = p.getFuture()
                .via(executor.get())
                .thenValue([=](...) { return delay(cmd, durations[(index++)%durations.size()]); });
        p.setValue("GOGOGO");
        return f;
    }

    folly::Future<std::string> executeAndSwitchPrompt(
            const Command& cmd) override {
        DLOG(INFO) << this << ": AsyncErrCli:executeAndSwitchPrompt '" << cmd << "'\n";

        folly::Promise<std::string> p;
        folly::Future<std::string> f = p.getFuture()
                .via(executor.get())
                .thenValue([=](...) { return delay(cmd, durations[(index++)%durations.size()]); });
        p.setValue("GOGOGO");
        return f;
    }

protected:
    std::shared_ptr<folly::CPUThreadPoolExecutor> executor;
    std::vector<unsigned int> durations;
    unsigned int index;
    bool quit;

    folly::Future<std::string> delay(const Command& cmd, unsigned int tis) {
        if (quit) {
            DLOG(INFO) << this << ": AsyncErrCli: '" << cmd << "' ABORTED\n";
            return folly::Future<std::string>(std::runtime_error(cmd.toString()));
        }
        DLOG(INFO) << this << ": AsyncErrCli: '" << cmd << "' busy for " << tis << " second(s) (tid " << std::this_thread::get_id() << ")\n";
        std::this_thread::sleep_for(std::chrono::seconds(tis));

        DLOG(INFO) << this << ": AsyncErrCli: '" << cmd << "' done\n";
        throw std::runtime_error(cmd.toString());
        return folly::Future<std::string>(std::runtime_error(cmd.toString()));
    }
};

} // namespace cli
} // namespace channels
} // namespace devmand