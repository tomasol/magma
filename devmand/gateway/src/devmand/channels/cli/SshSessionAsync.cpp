// Copyright (c) 2019-present, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant
// of patent rights can be found in the PATENTS file in the same directory.

#include <devmand/channels/cli/SshSessionAsync.h>
#include <devmand/channels/cli/SshSession.h>
#include <folly/executors/IOThreadPoolExecutor.h>
#include <folly/futures/Future.h>

namespace devmand {
namespace channels {
namespace cli {
namespace sshsession {

using devmand::channels::cli::sshsession::SshSessionAsync;
using devmand::channels::cli::sshsession::SshSession;

SshSessionAsync::SshSessionAsync(shared_ptr<IOThreadPoolExecutor> _executor)
    : executor(_executor) {}

SshSessionAsync::~SshSessionAsync() {
  event_free(this->sessionEvent);
  session.close();
}

Future<string> SshSessionAsync::read(int timeoutMillis) {
  return via(executor.get(), [this, timeoutMillis] {
    return session.read(timeoutMillis);
  });
}

Future<Unit> SshSessionAsync::openShell(
    const string& ip,
    int port,
    const string& username,
    const string& password) {
  return via(executor.get(), [this, ip, port, username, password] {
    session.openShell(ip, port, username, password);
  });
}

Future<Unit> SshSessionAsync::write(const string& command) {
  return via(executor.get(), [this, command] { session.write(command); });
}

Future<Unit> SshSessionAsync::close() {
  return via(executor.get(), [this] { session.close(); });
}

Future<string> SshSessionAsync::readUntilOutput(const string& lastOutput) {
  return via(executor.get(), [this, lastOutput] {
    return this->readUntilOutputBlocking(lastOutput);
  });
}

void SshSessionAsync::setEvent(event * event) {
    this->sessionEvent = event;
}

void readCallback(evutil_socket_t fd, short what, void *ptr)
{
    (void) fd;
    (void) what;
    ((SshSessionAsync *)ptr)->read();
}

socket_t SshSessionAsync::getSshFd() {
    return this->session.getSshFd();
}

void SshSessionAsync::read() {
    const string &output = this->session.read();
    readQueue.push(output);
}

 string SshSessionAsync::readUntilOutputBlocking(string lastOutput) {
     string result;
     while (true) { //TODO this wastes CPU if output from SSH is delayed
         string output;
         if (!readQueue.pop(output) || output.empty()) {
             continue;
         }
         result.append(output);
         std::size_t found = result.find(lastOutput);
         if (found != std::string::npos) {
             // TODO check for any additional output after lastOutput
             return result.substr(0, found);
         }
     }
 }


} // namespace sshsession
} // namespace cli
} // namespace channels
}