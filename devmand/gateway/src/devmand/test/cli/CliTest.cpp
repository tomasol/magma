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
#include <devmand/channels/cli/SshSessionAsync.h>
#include <devmand/channels/cli/PromptAwareCli.h>
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



TEST_F(CliTest, api) {
  std::string foo("foo");
  Command cmd = Command::makeReadCommand(foo);
  EXPECT_EQ("foo", cmd.toString());
  foo.clear();
  EXPECT_EQ("foo", cmd.toString());
  cmd.toString().clear();
  EXPECT_EQ("foo", cmd.toString());

  const EchoCli& mockCli = EchoCli();
  folly::Future<std::string> future = mockCli.executeAndRead(cmd);
  EXPECT_EQ("foo", std::move(future).get());

  const Channel cliChannel(std::make_shared<EchoCli>());
  folly::Future<std::string> futureFromChannel = cliChannel.executeAndRead(cmd);
  EXPECT_EQ("foo", std::move(futureFromChannel).get());
}

} // namespace cli
} // namespace test
} // namespace devmand
