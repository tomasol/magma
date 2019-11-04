// Copyright (c) 2019-present, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant
// of patent rights can be found in the PATENTS file in the same directory.

#include <boost/algorithm/string/trim.hpp>
#include <devmand/channels/cli/Channel.h>
#include <devmand/channels/cli/Cli.h>
#include <devmand/channels/cli/PromptAwareCli.h>
#include <devmand/channels/cli/QueuedCli.h>
#include <devmand/channels/cli/KeepaliveCli.h>
#include <devmand/channels/cli/SshSessionAsync.h>
#include <folly/executors/IOThreadPoolExecutor.h>
#include <folly/executors/CPUThreadPoolExecutor.h>
#include <folly/executors/ThreadedExecutor.h>
#include <gtest/gtest.h>
#include <devmand/cartography/DeviceConfig.h>
#include <devmand/devices/cli/PlaintextCliDevice.h>
#include <devmand/devices/Device.h>
#include <devmand/devices/State.h>
#include <devmand/Application.h>
#include <chrono>
#include <magma_logging.h>

namespace devmand {
namespace test {
namespace cli {

using namespace devmand::channels::cli;
using devmand::cartography::DeviceConfig;
using devmand::cartography::ChannelConfig;
using devmand::devices::Device;
using devmand::devices::State;
using devmand::devices::cli::PlaintextCliDevice;
using devmand::Application;
using devmand::channels::cli::sshsession::SshSessionAsync;

class CliTest : public ::testing::Test {
 public:
  CliTest() = default;
  virtual ~CliTest() = default;
  CliTest(const CliTest&) = delete;
  CliTest& operator=(const CliTest&) = delete;
  CliTest(CliTest&&) = delete;
  CliTest& operator=(CliTest&&) = delete;
};

TEST_F(CliTest, queuedCli) {
  std::vector<unsigned int> durations = {2};
  QueuedCli cli(std::make_shared<AsyncCli>(std::make_shared<EchoCli>(), durations), 3, 1);

  std::vector<std::string> results;
  results.push_back("one");
  results.push_back("two");
  results.push_back("three");
  results.push_back("four");
  results.push_back("five");
  results.push_back("six");
  results.push_back("seven");

  // create requests
  std::vector<Command> cmds;
  for (const auto& str : results) {
    cmds.push_back(Command::makeReadCommand(str));
  }

  // send requests
  std::vector<folly::Future<std::string>> futures;
  for (const auto& cmd : cmds) {
    MLOG(MDEBUG) << "test exec '" << cmd << "'\n";
    futures.push_back(cli.executeAndRead(cmd));
  }

  // collect values
  const std::vector<folly::Try<std::string>>& values =
      collectAll(futures.begin(), futures.end()).get();

  // check values
  EXPECT_EQ(values.size(), results.size());
  for (unsigned int i = 0; i < values.size(); ++i) {
    EXPECT_EQ(boost::algorithm::trim_copy(values[i].value()), results[i]);
  }
}

TEST_F(CliTest, queuedCliMT) {
  const int loopcount = 10;
  std::vector<unsigned int> durations = {1};
  folly::CPUThreadPoolExecutor executor(8);

  QueuedCli cli(std::make_shared<AsyncCli>(std::make_shared<EchoCli>(), durations));

  // create requests
  std::vector<folly::Future<std::string>> futures;
  Command cmd = Command::makeReadCommand("hello");
  for (int i = 0; i < loopcount; ++i) {
    MLOG(MDEBUG) << "test exec '" << cmd << "'\n";
    futures.push_back(folly::via(&executor, [&]() {
      return cli.executeAndRead(cmd);
    }));
  }

  // collect values
  const std::vector<folly::Try<std::string>>& values =
    collectAll(futures.begin(), futures.end()).get();

  // check values
  EXPECT_EQ(values.size(), loopcount);
  for (auto v : values) {
    EXPECT_EQ(boost::algorithm::trim_copy(v.value()), "hello");
  }
}

TEST_F(CliTest, queuedCliMTLimit) {
    const int loopcount = 10;
    std::vector<unsigned int> durations = {1};
    folly::CPUThreadPoolExecutor executor(8);

    QueuedCli cli(std::make_shared<AsyncCli>(std::make_shared<EchoCli>(), durations), 3, 1);

    // create requests
    std::vector<folly::Future<std::string>> futures;
    Command cmd = Command::makeReadCommand("hello");
    for (int i = 0; i < loopcount; ++i) {
//        std::ostringstream ss;
//        ss << "hello #" << i;
//        Command cmd = Command::makeReadCommand(ss.str());
        MLOG(MDEBUG) << "test exec '" << cmd << "'\n";
        futures.push_back(folly::via(&executor, [&]() { return cli.executeAndRead(cmd); }));
    }

    // collect values
    const std::vector<folly::Try<std::string>>& values =
            collectAll(futures.begin(), futures.end()).get();

    // check values
    EXPECT_EQ(values.size(), loopcount);
    for (auto v : values) {
        EXPECT_EQ(boost::algorithm::trim_copy(v.value()), "hello");
    }
}

// TODO: KA test to pass through standard commands   (EchoCli)
TEST_F(CliTest, keepaliveCliPass) {
    const int loopcount = 10;
    std::vector<unsigned int> durations = {1};

    KeepaliveCli cli(
            [&] {
                auto ret = std::make_shared<AsyncCli>(std::make_shared<EchoCli>(), durations);
                MLOG(MDEBUG) << ": KACli: created new CLi stack (" << ret.get() << ")\n";
                return ret;
            },
            5,
            10);

    // create requests
    Command cmd = Command::makeReadCommand("hello");
    std::vector<folly::Future<std::string>> futures;
    for (int i = 0; i < loopcount; ++i) {
        MLOG(MDEBUG) << "test exec '" << cmd << "'\n";
        futures.push_back(cli.executeAndRead(cmd));
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    // collect values
    const std::vector<folly::Try<std::string>>& values =
            collectAll(futures.begin(), futures.end()).get();

    // check values
    EXPECT_EQ(values.size(), loopcount);
    int res_value = 0;
    int res_failed = 0;
    int res_cancelled = 0;
    int res_other = 0;
    for (auto v : values) {
        if (v.hasException()) {
            std::cout << "main: exception: " << v.exception() << "\n";
            if(std::strcmp(v.exception().get_exception()->what(), "EXPIRED") == 0) {
                res_failed++;
            } else if(std::strcmp(v.exception().get_exception()->what(), "CANCELLED") == 0) {
                res_cancelled++;
            } else {
                res_other++;
            }
        } else {
            std::cout << "main: value " << v.value() << "\n";
            res_value++;
        }
    }
    EXPECT_EQ(res_value, loopcount);
    EXPECT_EQ(res_failed, 0);
    EXPECT_EQ(res_cancelled, 0);
    EXPECT_EQ(res_other, 0);
}

// TODO: KA to send periodical requests with timeout -> restart cli stack
TEST_F(CliTest, keepaliveCliTimeout) {
    const int loopcount = 10;
    std::vector<unsigned int> durations = {4};

    KeepaliveCli cli(
            [&] {
                auto ret = std::make_shared<AsyncCli>(std::make_shared<EchoCli>(), durations);
                MLOG(MDEBUG) << ": KACli: created new CLi stack (" << ret.get() << ")\n";
                return ret;
            },
            5,
            5);

    // create requests
    Command cmd = Command::makeReadCommand("hello");
    std::vector<folly::Future<std::string>> futures;
    for (int i = 0; i < loopcount; ++i) {
        MLOG(MDEBUG) << "test exec '" << cmd << "'\n";
        futures.push_back(cli.executeAndRead(cmd));
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }

    // collect values
    const std::vector<folly::Try<std::string>>& values =
            collectAll(futures.begin(), futures.end()).get();

    // check values
    EXPECT_EQ(values.size(), loopcount);
    int res_value = 0;
    int res_failed = 0;
    for (auto v : values) {
        if (v.hasException()) {
            std::cout << "main: exception: " << v.exception() << "\n";
            res_failed++;
        } else {
            std::cout << "main: value " << v.value() << "\n";
            res_value++;
        }
    }
    EXPECT_EQ(res_value > 3, true);
    EXPECT_EQ(res_failed > 3, true);
}

// TODO: KA to send periodical requests with error -> restart cli stack
TEST_F(CliTest, keepaliveCliErr) {
    const int loopcount = 10;
    std::vector<unsigned int> durations = {1};

    KeepaliveCli cli(
            [&] {
                auto ret = std::make_shared<AsyncCli>(std::make_shared<ErrCli>(), durations);
                MLOG(MDEBUG) << ": KACli: created new CLi stack (" << ret.get() << ")\n";
                return ret;
            },
            5,
            5);

    // create requests
    Command cmd = Command::makeReadCommand("hello");
    std::vector<folly::Future<std::string>> futures;
    for (int i = 0; i < loopcount; ++i) {
        MLOG(MDEBUG) << "test exec '" << cmd << "'\n";
        futures.push_back(cli.executeAndRead(cmd));
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    // collect values
    const std::vector<folly::Try<std::string>>& values =
            collectAll(futures.begin(), futures.end()).get();

    // check values
    EXPECT_EQ(values.size(), loopcount);
    int res_value = 0;
    int res_failed = 0;
    int res_other = 0;
    for (auto v : values) {
        if (v.hasException()) {
            std::cout << "main: exception: " << v.exception() << "\n";
            if(std::strcmp(v.exception().get_exception()->what(), "hello") == 0) {
                res_failed++;
            } else {
                res_other++;
            }
        } else {
            std::cout << "main: value " << v.value() << "\n";
            res_value++;
        }
    }
    EXPECT_EQ(res_value, 0);
    EXPECT_EQ(res_failed, loopcount);
    EXPECT_EQ(res_other, 0);
}

TEST_F(CliTest, api) {
  std::string foo("foo");
  Command cmd = Command::makeReadCommand(foo);
  EXPECT_EQ("foo", cmd.toString());
  foo.clear();
  EXPECT_EQ("foo", cmd.toString());
  cmd.toString().clear();
  EXPECT_EQ("foo", cmd.toString());

  const auto mockCli = std::make_shared<EchoCli>();
  folly::Future<std::string> future = mockCli->executeAndRead(cmd);
  EXPECT_EQ("foo", std::move(future).get());

  Channel cliChannel(std::make_shared<EchoCli>());
  folly::Future<std::string> futureFromChannel = cliChannel.executeAndRead(cmd);
  EXPECT_EQ("foo", std::move(futureFromChannel).get());
}

} // namespace cli
} // namespace test
} // namespace devmand
