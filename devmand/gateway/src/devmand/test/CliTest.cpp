// Copyright (c) 2019-present, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant
// of patent rights can be found in the PATENTS file in the same directory.

#include <devmand/channels/cli/Channel.h>
#include <devmand/channels/cli/Cli.h>
#include <gtest/gtest.h>

namespace devmand {
namespace test {

using namespace devmand::channels::cli;

class CliTest : public ::testing::Test {
 public:
  CliTest() = default;
  virtual ~CliTest() = default;
  CliTest(const CliTest&) = delete;
  CliTest& operator=(const CliTest&) = delete;
  CliTest(CliTest&&) = delete;
  CliTest& operator=(CliTest&&) = delete;
};

TEST_F(CliTest, api) {
  std::string foo("foo");
  Command cmd = Command::createReadCommand(foo);
  EXPECT_EQ(std::string("foo"), cmd.toString());
  foo.clear();
  EXPECT_EQ(std::string("foo"), cmd.toString());
  cmd.toString().clear();
  EXPECT_EQ(std::string("foo"), cmd.toString());

  const EchoCli& mockCli = EchoCli();
  folly::Future<std::string> future = mockCli.executeAndRead(cmd);
  EXPECT_EQ(std::string("foo"), std::move(future).get());
  const Channel cliChannel(mockCli);
  folly::Future<std::string> futureFromChannel = cliChannel.executeAndRead(cmd);
  EXPECT_EQ(std::string("foo"), std::move(futureFromChannel).get());
}

} // namespace test
} // namespace devmand
