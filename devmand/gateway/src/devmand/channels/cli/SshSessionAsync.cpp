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
#include <mutex>
#include <chrono>
#include <condition_variable>
#include <ErrorHandler.h>
#include <boost/lockfree/spsc_queue.hpp>

namespace devmand {
namespace channels {
namespace cli {
namespace sshsession {

using devmand::channels::cli::sshsession::SshSessionAsync;
using devmand::channels::cli::sshsession::SshSession;
using devmand::ErrorHandler;
using std::lock_guard;
using std::unique_lock;
using boost::lockfree::spsc_queue;

SshSessionAsync::SshSessionAsync(shared_ptr<IOThreadPoolExecutor> _executor)
    : executor(_executor), quit(false) {}

SshSessionAsync::~SshSessionAsync() {
  quit = true;
  executor->join();

  if (this->sessionEvent != nullptr && event_get_base(this->sessionEvent) != nullptr) {
      event_free(this->sessionEvent);
  }
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
    ((SshSessionAsync *) ptr)->readToBuffer();
}

socket_t SshSessionAsync::getSshFd() {
    return this->session.getSshFd();
}

void SshSessionAsync::readToBuffer() {
    {
        std::lock_guard<mutex> guard(mutex1);
        ErrorHandler::executeWithCatch([this]() {
            const string &output = this->session.read();
            readQueue.push(output);
        });
    }
    condition.notify_one();
}

 string SshSessionAsync::readUntilOutputBlocking(string lastOutput) {
     string result;
     while (!quit) {
         unique_lock<mutex> lck(mutex1);
         condition.wait_for(lck, std::chrono::seconds(2), [this]{ return this->readQueue.read_available(); });
         //TODO check if readQueue is being filled, if I always wake up on timeout something
         //is wrong (session gone bad?) and I probably have to throw an exception so I terminate

         string output;
         if (!readQueue.pop(output) || output.empty()) {  //sometimes we get the string "" or " " back, we can ignore that ...
             continue;
         }
         result.append(output);
         std::size_t found = result.find(lastOutput);
         if (found != std::string::npos) {
             // TODO check for any additional output after lastOutput
             return result.substr(0, found);
         }
     }
     return "FINISHED";
 }

} // namespace sshsession
} // namespace cli
} // namespace channels
}