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
#include <devmand/channels/cli/SshSessionAsync.h>
#include <folly/executors/IOThreadPoolExecutor.h>
#include <folly/executors/CPUThreadPoolExecutor.h>
#include <folly/executors/ThreadedExecutor.h>
#include <gtest/gtest.h>

namespace devmand {
namespace test {
namespace cli {

using namespace devmand::channels::cli;
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

TEST_F(CliTest, promptAwareCli) {
  const std::string expectedOutput =
      "inet 172.8.0.85/16 brd 172.8.255.255 scope global eth0";
  std::vector<folly::Future<std::string>> futures;
  std::vector<PromptAwareCli*> clis;
  int iterations = 10;
  std::shared_ptr<folly::IOThreadPoolExecutor> executor =
      std::make_shared<folly::IOThreadPoolExecutor>(10);
  CliFlavour cliFlavour;
  const Command& cmd =
      Command::makeReadCommand("ip addr | grep inet | grep 85");
  for (int i = 0; i < iterations; i++) {
    const std::shared_ptr<SshSessionAsync>& session =
        std::make_shared<SshSessionAsync>(executor);
    clis.push_back(new PromptAwareCli(session, cliFlavour));
  }
  for (const auto& cli : clis) {
    cli->init("localhost", 22, "root", "root");
    cli->resolvePrompt();
  }
  for (const auto& cli : clis) {
    futures.push_back(cli->executeAndRead(cmd));
  }
  const std::vector<folly::Try<std::string>>& values =
      collectAll(futures.begin(), futures.end()).get();

  EXPECT_EQ(values.size(), iterations);
  for (auto v : values) {
    string cliOutput = boost::algorithm::trim_copy(v.value());
    EXPECT_EQ(cliOutput, expectedOutput);
  }
}

TEST_F(CliTest, queuedCli) {
  QueuedCli qcli(std::make_shared<AsyncEchoCli>(
    std::make_shared<folly::ThreadedExecutor>()),3,1);

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
  cmds.push_back(Command::makeReadCommand("one"));
  cmds.push_back(Command::makeReadCommand("two"));
  cmds.push_back(Command::makeReadCommand("three"));
  cmds.push_back(Command::makeReadCommand("four"));
  cmds.push_back(Command::makeReadCommand("five"));
  cmds.push_back(Command::makeReadCommand("six"));
  cmds.push_back(Command::makeReadCommand("seven"));

  // send requests
  std::vector<folly::Future<std::string>> futures;
  for (const auto& cmd : cmds) {
    DLOG(INFO) << "test exec '" << cmd << "'\n";
    futures.push_back(qcli.executeAndRead(cmd));
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
  folly::CPUThreadPoolExecutor executor(8);

  QueuedCli qcli(std::make_shared<AsyncEchoCli>(
    std::make_shared<folly::ThreadedExecutor>()));

  // create requests
  Command cmd = Command::makeReadCommand("hello");
  std::vector<folly::Future<std::string>> futures;
  for (int i = 0; i < loopcount; ++i) {
    futures.push_back(folly::via(&executor, [&, i]() { return qcli.executeAndRead(cmd); }));
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
