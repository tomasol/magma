// Copyright (c) 2019-present, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant
// of patent rights can be found in the PATENTS file in the same directory.

#pragma once

#include <devmand/channels/Channel.h>
#include <devmand/channels/cli/Cli.h>
#include <libssh/libssh.h>
#include <boost/lockfree/spsc_queue.hpp>
#include <boost/lockfree/policies.hpp>

namespace devmand {
namespace channels {
namespace cli {
namespace sshsession {

using std::runtime_error;
using std::string;
using boost::lockfree::spsc_queue;
using boost::lockfree::capacity;

class SshSession {
 private:
  struct SshSessionState {
    string ip;
    int port;
    string username;
    string password;
    ssh_channel channel = nullptr;
    ssh_session session = nullptr;
  } sessionState;
  int verbosity;
  bool checkSuccess(int return_code, int OK_RETURN_CODE);
  void terminate();

 public:
  spsc_queue<string, capacity<200>> readQueue;

  explicit SshSession(int verbosity);
  socket_t getSshFd();
  SshSession();
  ~SshSession();

  void openShell(
      const string& ip,
      int port,
      const string& username,
      const string& password);
  void close();
  void write(const string& command);
  string read(int timeoutMillis);
  string read();
  string readUntilOutput(string lastOutput);
};

} // namespace sshsession
} // namespace cli
} // namespace channels
} // namespace devmand
