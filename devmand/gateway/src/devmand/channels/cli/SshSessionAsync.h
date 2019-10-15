// Copyright (c) 2019-present, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant
// of patent rights can be found in the PATENTS file in the same directory.

#pragma once

#include <folly/futures/Future.h>
#include <folly/executors/IOThreadPoolExecutor.h>
#include <devmand/channels/cli/SshSession.h>
#include <devmand/channels/cli/Command.h>

namespace devmand {
namespace channels {
namespace cli {
namespace sshsession {

using folly::makeFuture;
using folly::via;
using folly::Future;
using std::string;
using folly::IOThreadPoolExecutor;
using folly::Unit;
using devmand::channels::cli::sshsession::SshSession;
using std::shared_ptr;

class SshSessionAsync {
private:
    shared_ptr<IOThreadPoolExecutor> executor;
    SshSession session;
public:
    explicit SshSessionAsync(shared_ptr<IOThreadPoolExecutor> _executor);
    Future<Unit> openShell(const string& ip, int port, const string& username, const string& password);
    Future<Unit> write(const string &command);
    Future<string> read(int timeoutMillis);
    Future<string> readUntilOutput(string lastOutput);
    Future<Unit> close();
    ~SshSessionAsync();
};


} // namespace sshsession
} // namespace cli
} // namespace channels
} // namespace devmand
