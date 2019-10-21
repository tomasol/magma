// Copyright (c) 2019-present, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant
// of patent rights can be found in the PATENTS file in the same directory.

#include <devmand/channels/cli/Channel.h>
#include <devmand/channels/cli/Cli.h>
#include <gtest/gtest.h>
#include <folly/executors/IOThreadPoolExecutor.h>
#include <folly/executors/ThreadedExecutor.h>
#include <devmand/channels/cli/SshSessionAsync.h>
#include <devmand/channels/cli/PromptAwareCli.h>
#include <devmand/channels/cli/QueuedCli.h>
#include <boost/algorithm/string/trim.hpp>

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
    const std::string expectedOutput = "inet 172.8.0.85/16 brd 172.8.255.255 scope global eth0";
    std::vector<folly::Future<std::string>> futures;
    std::vector<PromptAwareCli*> clis;
    int iterations = 10;
    std::shared_ptr<folly::IOThreadPoolExecutor> executor = std::make_shared<folly::IOThreadPoolExecutor>(10);
    CliFlavour cliFlavour;
    const Command &cmd = Command::makeReadCommand("ip addr | grep inet | grep 85");
    for (int i = 0; i < iterations; i++) {
        const std::shared_ptr<SshSessionAsync> &session = std::make_shared<SshSessionAsync>(executor);
        clis.push_back(new PromptAwareCli(session, cliFlavour));
    }
    for (const auto &cli : clis) {
        cli->init("localhost", 22, "root", "root");
        cli->resolvePrompt();
    }
    for (const auto &cli : clis) {
        futures.push_back(cli->executeAndRead(cmd));
    }
    const std::vector<folly::Try<std::string>> &values = collectAll(futures.begin(), futures.end()).get();

    EXPECT_EQ(values.size(), iterations);
    for (auto v : values) {
        string cliOutput = boost::algorithm::trim_copy(v.value());
        EXPECT_EQ(cliOutput, expectedOutput);
    }
}

TEST_F(CliTest, queuedCli) {
    std::shared_ptr<folly::IOThreadPoolExecutor> executor = std::make_shared<folly::IOThreadPoolExecutor>(10);
    const std::shared_ptr<SshSessionAsync> &session = std::make_shared<SshSessionAsync>(executor);
    CliFlavour cliFlavour;
    const std::shared_ptr<PromptAwareCli> &cli = std::make_shared<PromptAwareCli>(session, cliFlavour);
    cli->init("localhost", 22, "root", "root");
    cli->resolvePrompt();
    QueuedCli qcli(cli, 3, 1);

    std::vector<std::string> results;
    results.push_back("root");
    results.push_back("inet 172.8.0.85/16 brd 172.8.255.255 scope global eth0");
    results.push_back("4.15.0-65-generic");
    results.push_back("");
    results.push_back("/root");
    results.push_back("hello");
    results.push_back("-bash: foo: command not found");

    // create requests
    std::vector<Command> cmds;
    cmds.push_back(Command::makeReadCommand("whoami"));
    cmds.push_back(Command::makeReadCommand("ip addr | grep inet | grep 85"));
    cmds.push_back(Command::makeReadCommand("uname -r"));
    cmds.push_back(Command::makeReadCommand("sleep 1"));
    cmds.push_back(Command::makeReadCommand("pwd"));
    cmds.push_back(Command::makeReadCommand("echo \"hello\""));
    cmds.push_back(Command::makeReadCommand("foo "));

    // send requests
    std::vector<folly::Future<std::string>> futures;
    for (const auto &cmd : cmds) {
        futures.push_back(qcli.test(cmd));
//        futures.push_back(qcli.executeAndRead(cmd));
    }

    // collect values
    const std::vector<folly::Try<std::string>> &values = collectAll(futures.begin(), futures.end()).get();

    // check values
    EXPECT_EQ(values.size(), results.size());
    for (unsigned int i = 0; i < values.size(); ++i) {
//    for (auto v : values) {
//        std::cout << "main: value " << v.value() << "\n";
        EXPECT_EQ(boost::algorithm::trim_copy(values[i].value()), results[i]);
    }
}

TEST_F(CliTest, queuedCliMT) {
    const int loopcount = 10;

    std::shared_ptr<folly::IOThreadPoolExecutor> io_executor = std::make_shared<folly::IOThreadPoolExecutor>(10);
    folly::ThreadedExecutor executor;
    const std::shared_ptr<SshSessionAsync> &session = std::make_shared<SshSessionAsync>(io_executor);
    CliFlavour cliFlavour;
    const std::shared_ptr<PromptAwareCli> &cli = std::make_shared<PromptAwareCli>(session, cliFlavour);
    cli->init("localhost", 22, "root", "root");
    cli->resolvePrompt();
    QueuedCli qcli(cli);

    // create requests
    Command cmd = Command::makeReadCommand("whoami");
    std::vector<folly::Future<std::string>> futures;
    for (int i = 0; i < loopcount; ++i) {
        futures.push_back(qcli.test(cmd).via(&executor));
    }

    // collect values
    const std::vector<folly::Try<std::string>> &values = collectAll(futures.begin(), futures.end()).get();

    // check values
    EXPECT_EQ(values.size(), loopcount);
    for (auto v : values) {
//            std::cout << "main: value " << v.value() << "\n";
        EXPECT_EQ(boost::algorithm::trim_copy(v.value()), "root");
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
