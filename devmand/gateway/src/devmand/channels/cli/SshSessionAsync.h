// Copyright (c) 2019-present, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant
// of patent rights can be found in the PATENTS file in the same directory.

#pragma once

#include <devmand/channels/cli/Command.h>
#include <devmand/channels/cli/SshSession.h>
#include <folly/executors/IOThreadPoolExecutor.h>
#include <folly/futures/Future.h>
#include <event2/event.h>

namespace devmand {
namespace channels {
namespace cli {
namespace sshsession {

using devmand::channels::cli::sshsession::SshSession;
using folly::Future;
using folly::IOThreadPoolExecutor;
using folly::makeFuture;
using folly::Unit;
using folly::via;
using std::shared_ptr;
using std::string;

void readCallback(evutil_socket_t fd, short , void *ptr);

class SshSessionAsync {
 private:
  shared_ptr<IOThreadPoolExecutor> executor;
  SshSession session;
  event * sessionEvent;
public:
  explicit SshSessionAsync(shared_ptr<IOThreadPoolExecutor> _executor);
  Future<Unit> openShell(
      const string& ip,
      int port,
      const string& username,
      const string& password);
  Future<Unit> write(const string& command);
  Future<string> read(int timeoutMillis); //for clearing ssh channel and prompt resolving
  Future<string> readUntilOutput(const string& lastOutput);
  Future<Unit> close();
  void setEvent(event *);
  void read();
  socket_t getSshFd();
  ~SshSessionAsync();
};

} // namespace sshsession
} // namespace cli
} // namespace channels
} // namespace devmand
