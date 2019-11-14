// Copyright (c) 2019-present, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant
// of patent rights can be found in the PATENTS file in the same directory.

#define LOG_WITH_GLOG
#include <magma_logging.h>

#include <boost/algorithm/string/trim.hpp>
#include <devmand/cartography/DeviceConfig.h>
#include <devmand/channels/cli/Cli.h>
#include <devmand/channels/cli/KeepaliveCli.h>
#include <devmand/devices/State.h>
#include <devmand/test/cli/utils/Log.h>
#include <devmand/test/cli/utils/MockCli.h>
#include <folly/executors/CPUThreadPoolExecutor.h>
#include <folly/executors/ThreadedExecutor.h>
#include <gtest/gtest.h>
#include <chrono>

namespace devmand {
namespace test {
namespace cli {

using namespace devmand::channels::cli;
using namespace devmand::test::utils::cli;
using namespace std;

shared_ptr<folly::CPUThreadPoolExecutor> testExecutor =
    make_shared<folly::CPUThreadPoolExecutor>(8);

class DISABLED_KeepaliveCliTest : public ::testing::Test {
 protected:
  void SetUp() override {
    devmand::test::utils::log::initLog();
  }
};

// TODO: KA test to pass through standard commands   (EchoCli)
TEST_F(DISABLED_KeepaliveCliTest, keepaliveCliPass) {
  const int loopcount = 10;
  vector<unsigned int> durations = {1};

  KeepaliveCli cli(
      [&] {
        auto ret = make_shared<AsyncCli>(
            make_shared<EchoCli>(), testExecutor, durations);
        MLOG(MDEBUG) << ": KACli: created new CLi stack (" << ret.get()
                     << ")\n";
        return ret;
      },
      5,
      10);

  // create requests
  Command cmd = Command::makeReadCommand("hello");
  vector<folly::Future<string>> futures;
  for (int i = 0; i < loopcount; ++i) {
    MLOG(MDEBUG) << "test exec '" << cmd << "'\n";
    futures.push_back(cli.executeAndRead(cmd));
    this_thread::sleep_for(chrono::seconds(1));
  }

  // collect values
  const vector<folly::Try<string>>& values =
      collectAll(futures.begin(), futures.end()).get();

  // check values
  EXPECT_EQ(values.size(), loopcount);
  int res_value = 0;
  int res_failed = 0;
  int res_cancelled = 0;
  int res_other = 0;
  for (auto v : values) {
    if (v.hasException()) {
      cout << "main: exception: " << v.exception() << "\n";
      if (strcmp(v.exception().get_exception()->what(), "EXPIRED") == 0) {
        res_failed++;
      } else if (
          strcmp(v.exception().get_exception()->what(), "CANCELLED") == 0) {
        res_cancelled++;
      } else {
        res_other++;
      }
    } else {
      cout << "main: value " << v.value() << "\n";
      res_value++;
    }
  }
  EXPECT_EQ(res_value, loopcount);
  EXPECT_EQ(res_failed, 0);
  EXPECT_EQ(res_cancelled, 0);
  EXPECT_EQ(res_other, 0);
}

// TODO: KA to send periodical requests with timeout -> restart cli stack
TEST_F(DISABLED_KeepaliveCliTest, keepaliveCliTimeout) {
  const int loopcount = 10;
  vector<unsigned int> durations = {4};

  KeepaliveCli cli(
      [&] {
        auto ret = make_shared<AsyncCli>(
            make_shared<EchoCli>(), testExecutor, durations);
        MLOG(MDEBUG) << ": KACli: created new CLi stack (" << ret.get()
                     << ")\n";
        return ret;
      },
      5,
      5);

  // create requests
  Command cmd = Command::makeReadCommand("hello");
  vector<folly::Future<string>> futures;
  for (int i = 0; i < loopcount; ++i) {
    MLOG(MDEBUG) << "test exec '" << cmd << "'\n";
    futures.push_back(cli.executeAndRead(cmd));
    this_thread::sleep_for(chrono::seconds(2));
  }

  // collect values
  const vector<folly::Try<string>>& values =
      collectAll(futures.begin(), futures.end()).get();

  // check values
  EXPECT_EQ(values.size(), loopcount);
  int res_value = 0;
  int res_failed = 0;
  for (auto v : values) {
    if (v.hasException()) {
      cout << "main: exception: " << v.exception() << "\n";
      res_failed++;
    } else {
      cout << "main: value " << v.value() << "\n";
      res_value++;
    }
  }
  EXPECT_EQ(res_value > 3, true);
  EXPECT_EQ(res_failed > 3, true);
}

// TODO: KA to send periodical requests with error -> restart cli stack
TEST_F(DISABLED_KeepaliveCliTest, keepaliveCliErr) {
  const int loopcount = 10;
  vector<unsigned int> durations = {1};

  KeepaliveCli cli(
      [&] {
        auto ret = make_shared<AsyncCli>(
            make_shared<ErrCli>(), testExecutor, durations);
        MLOG(MDEBUG) << ": KACli: created new CLi stack (" << ret.get()
                     << ")\n";
        return ret;
      },
      5,
      5);

  // create requests
  Command cmd = Command::makeReadCommand("hello");
  vector<folly::Future<string>> futures;
  for (int i = 0; i < loopcount; ++i) {
    MLOG(MDEBUG) << "test exec '" << cmd << "'\n";
    futures.push_back(cli.executeAndRead(cmd));
    this_thread::sleep_for(chrono::seconds(1));
  }

  // collect values
  const vector<folly::Try<string>>& values =
      collectAll(futures.begin(), futures.end()).get();

  // check values
  EXPECT_EQ(values.size(), loopcount);
  int res_value = 0;
  int res_failed = 0;
  int res_other = 0;
  for (auto v : values) {
    if (v.hasException()) {
      cout << "main: exception: " << v.exception() << "\n";
      if (strcmp(v.exception().get_exception()->what(), "hello") == 0) {
        res_failed++;
      } else {
        res_other++;
      }
    } else {
      cout << "main: value " << v.value() << "\n";
      res_value++;
    }
  }
  EXPECT_EQ(res_value, 0);
  EXPECT_EQ(res_failed, loopcount);
  EXPECT_EQ(res_other, 0);
}

} // namespace cli
} // namespace test
} // namespace devmand
