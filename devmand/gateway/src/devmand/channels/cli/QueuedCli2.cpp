// Copyright (c) 2019-present, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant
// of patent rights can be found in the PATENTS file in the same directory.

#include <devmand/channels/cli/QueuedCli2.h>

namespace devmand::channels::cli {
using namespace std;
using namespace folly;

QueuedCli2::QueuedCli2(shared_ptr<Cli> _cli, const shared_ptr<Executor> &_parentExecutor) :
        cli(_cli),
        serialExecutorKeepAlive(
                SerialExecutor::create(Executor::getKeepAliveToken(_parentExecutor.get()))) {
}

Future<string> QueuedCli2::executeAndRead(const Command &cmd) {
  return executeSomething(cmd, "QCli.executeAndRead",
                          [=]() { return cli->executeAndRead(cmd); });
}

Future<string> QueuedCli2::executeAndSwitchPrompt(const Command &cmd) {
  return executeSomething(cmd, "QCli.executeAndSwitchPrompt",
                          [=]() { return cli->executeAndSwitchPrompt(cmd); });
}

Future<string> QueuedCli2::executeSomething(
        const Command &cmd,
        const string &prefix,
        function<Future<string>()> innerFunc) {

  MLOG(MDEBUG) << "[" << this_thread::get_id() << "] " << prefix << "('" << cmd << "') called";
  shared_ptr<Promise<string>> promise = std::make_shared<Promise<string>>();
  QueueEntry queueEntry;
  queueEntry.obtainFutureFromCli = move(innerFunc);
  queueEntry.promise = promise;
  queueEntry.command = cmd.toString();
  queueEntry.loggingPrefix = prefix;
  queue.enqueue(move(queueEntry));
  triggerDequeue();
  return promise->getFuture(); // TODO: check lifetime
}

/*
 * Start queue reading on consumer thread if queue contains new items.
 * It is safe to call this method anytime, it is thread safe.
 */
void QueuedCli2::triggerDequeue() {
  // switch to consumer thread
  via(serialExecutorKeepAlive, [=]() {
    MLOG(MDEBUG) << "[" << this_thread::get_id() << "] " << "isProcessing:" << this->isProcessing;
    // do nothing if still waiting for remote device to respond
    if (!this->isProcessing) {
      QueueEntry queueEntry;
      if (queue.try_dequeue(queueEntry)) {
        isProcessing = true;
        Future<string> cliFuture = queueEntry.obtainFutureFromCli();
        MLOG(MDEBUG) << "[" << this_thread::get_id() << "] " << queueEntry.loggingPrefix
                     << "('" << queueEntry.command << "') dequeued and cli future obtained";
        move(cliFuture).then(
                serialExecutorKeepAlive,
                [this, queueEntry](std::string result) {
                  // after cliFuture completes, finish processing on consumer thread
                  MLOG(MDEBUG) << "[" << this_thread::get_id() << "] " << queueEntry.loggingPrefix << "('"
                               << queueEntry.command << "') finished with result '" << result << "'";
                  queueEntry.promise->setValue(result);
                  isProcessing = false;
                  triggerDequeue();
                });
      }
    }
  });
}
}
