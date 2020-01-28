// Copyright (c) 2019-present, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant
// of patent rights can be found in the PATENTS file in the same directory.

#define LOG_WITH_GLOG
#include <magma_logging.h>

#include <devmand/channels/cli/CliHttpServer.h>
#include <devmand/channels/cli/Cli.h>
#include <devmand/test/cli/utils/Log.h>
#include <devmand/test/cli/utils/MockCli.h>
#include <gtest/gtest.h>
#include <thread>
#include <folly/json.h>

namespace devmand {
namespace test {
namespace cli {

using namespace devmand::test::utils::cli;
using namespace httplib;
using namespace std;
using namespace folly;

class HttpServerTest : public ::testing::Test {
 public:
  shared_ptr<CliHttpServer> server;
  thread t;

 protected:
  void SetUp() override {
    devmand::test::utils::log::initLog();
  }

  void TearDown() override {
    MLOG(MDEBUG) << "Stopping";
    server->stop();
    MLOG(MDEBUG) << "Joining";
    t.join();
    MLOG(MDEBUG) << "Joined";
  }
};

static const auto host = "localhost";
static const int port = 1234;

static const CliResolver dummyCliResolver = [](const string token) -> shared_ptr<Cli> {
  if (token == "secret") {
    return make_shared<EchoCli>();
  }
  throw runtime_error("Wrong token");
};

static const TxResolver dummyTxResolver = []
    (const string token, /*DatastoreType type*/bool configDS, bool readCurrentTx, Path path) -> dynamic {
  if (token == "secret") {
    dynamic result = dynamic::object;
    result["path"] = path.str();
    result["readCurrentTx"] = readCurrentTx;
    result["isConfig"] = configDS/*type == DatastoreType::config*/;
    result["isOperational"] = !configDS/*type == DatastoreType::operational*/;
    return result;
  }
  throw runtime_error("Wrong token");
};

static thread startServer(shared_ptr<CliHttpServer> server) {
  thread t([server]() {
    server->listen();
  });
  while (not server->is_running()) {
    this_thread::sleep_for(chrono::milliseconds(10));
  }
  return t;
}

static shared_ptr<CliHttpServer> makeCliServer(CliResolver resolver) {
  return make_shared<CliHttpServer>(host, port, resolver, dummyTxResolver);
}

TEST_F(HttpServerTest, startAndStopServer) {
  server = makeCliServer(dummyCliResolver);
  t = startServer(server);
}

static shared_ptr<Response> postCliCommand(string deviceToken) {
  Client client(host, port);
  auto body = "{\"cmd\":\"show running-config\"}";
  return client.Post(("/executeCliCommand/" + deviceToken).c_str(), body, "application/json");
}

TEST_F(HttpServerTest, echoCli) {
  server = makeCliServer(dummyCliResolver);
  t = startServer(server);
  // HTTP client
  shared_ptr<Response> res = postCliCommand("secret");
  EXPECT_EQ(res->status, 200);
  auto expected = "show running-config";
  EXPECT_EQ(res->body, expected);
  // TODO check Content-Type: text/plain
}

TEST_F(HttpServerTest, wrongSecret) {
  server = makeCliServer(dummyCliResolver);
  t = startServer(server);
  // HTTP client
  shared_ptr<Response> res = postCliCommand("secret1");
  EXPECT_EQ(res->status, 500);
  auto expected = parseJson("{\"error\":{\"message\":\"Wrong token\",\"code\":1}}");
  EXPECT_EQ(parseJson(res->body), expected);
}

TEST_F(HttpServerTest, errCli) {
  CliResolver resolver = [](const string cliToken) -> shared_ptr<Cli> {
    if (cliToken == "secret") {
      return make_shared<ErrCli>();
    }
    throw runtime_error("Wrong token");
  };
  server = makeCliServer(resolver);
  t = startServer(server);
  // HTTP client
  Client client(host, port);
  auto body = "{\"cmd\":\"show running-config\"}";
  auto res = client.Post("/executeCliCommand/secret", body, "application/json");
  EXPECT_EQ(res->status, 500);
  auto expected = parseJson("{\"error\":{\"message\":\"show running-config\",\"code\":1}}");
  EXPECT_EQ(parseJson(res->body), expected);
}

static shared_ptr<CliHttpServer> makeDSServer(TxResolver resolver) {
  return make_shared<CliHttpServer>(host, port, dummyCliResolver, resolver);
}

static shared_ptr<Response> postTxRequest(string deviceToken) {
  Client client(host, port);
  auto body = "{\"path\":\"/somepath\", \"readFromConfigDS\":true,"
              "\"readCurrentTx\":false }";
  return client.Post(("/executeDSRead/" + deviceToken).c_str(), body, "application/json");
}

TEST_F(HttpServerTest, getOperData) {
  server = makeDSServer(dummyTxResolver);
  t = startServer(server);
  // HTTP client
  shared_ptr<Response> res = postTxRequest("secret");
  EXPECT_EQ(res->status, 200);
  auto expected = parseJson("{\"isOperational\":false,\"isConfig\":true,"
                            "\"readCurrentTx\":false,\"path\":\"/somepath\"}");
  EXPECT_EQ(parseJson(res->body), expected);
}

TEST_F(HttpServerTest, txWrongSecret) {
  server = makeDSServer(dummyTxResolver);
  t = startServer(server);
  // HTTP client
  shared_ptr<Response> res = postTxRequest("secret2");
  EXPECT_EQ(res->status, 500);
  auto expected = parseJson("{\"error\":{\"message\":\"Wrong token\",\"code\":1}}");
  EXPECT_EQ(parseJson(res->body), expected);
}

}
}
}
