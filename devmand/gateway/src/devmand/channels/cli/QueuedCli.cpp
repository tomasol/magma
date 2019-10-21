// Copyright (c) 2019-present, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant
// of patent rights can be found in the PATENTS file in the same directory.

#include <devmand/channels/cli/QueuedCli.h>
#include <devmand/channels/cli/Command.h>
#include <folly/executors/IOThreadPoolExecutor.h>
#include <folly/futures/Promise.h>

#include <iostream>
#include <regex>


using devmand::channels::cli::QueuedCli;
using devmand::channels::cli::Command;
using std::string;

namespace devmand {
    namespace channels {
        namespace cli {

            QueuedCli::QueuedCli(std::shared_ptr<Cli> _cli, unsigned int _hi_limit, unsigned int _lo_limit) :
            cli(_cli),
            ready(true),
            hi_limit(_hi_limit),
            lo_limit(_lo_limit)
            {}

            folly::Future<string> QueuedCli::executeAndRead(const Command &cmd) {
//                std::cout << this << ": QCli received: '" << cmd << "'\n";
//
//                if (!busy) {
////                    busy = true;
//                    std::cout << this << ": Executing: '" << cmd << "' using cli " << cli << "\n";
//                    return cli->executeAndRead(cmd).thenValue([=](std::string passthrough) { return returnAndExecNext(passthrough); });
//                } else {
//                    std::cout << this << ": Postponing: '" << cmd << "', current queue size: " << outstandingCmds.size() << "\n";
//                    folly::Promise<std::string> p;
//                    const folly::Future<std::string> future_exec = p.getFuture();
//                    const folly::Future<std::string> f1 = std::move(future_exec).thenValue([=](...) { return cli->executeAndRead(cmd); });
//                    const folly::Future<std::string> f2 = std::move(f1).thenValue([=](std::string passthrough) { return returnAndExecNext(passthrough); });
//
//                    std::cout << this << ": Q size before: '" << outstandingCmds.size() << "'\n";
//                    outstandingCmds.push(std::move(p));
//                    std::cout << this << ": Q size after: '" << outstandingCmds.size() << "'\n";
//
//                    return f2;
//                }
                return folly::Future<std::string>(cmd.toString());
            }

            folly::Future<string> QueuedCli::test(const Command &cmd) {
                bool empty = false;
                LOG(INFO) << this << ": QCli received: '" << cmd.toString() << " ready (" << ready << ")'\n";

                if (outstandingCmds.size() >= hi_limit) {
                    LOG(INFO) << this << ": queue size (" << outstandingCmds.size() << ") reached HI-limit -> WAIT\n";
                    wait();
                }

                LOG(INFO) << this << ": Enqueued '" << cmd << "' (queue size: " << outstandingCmds.size() << ")...\n";

                folly::Promise<std::string> p;
                auto future_exec = p.getFuture();
                folly::Future<std::string> f2 = std::move(future_exec)
                        .thenValue([=](...) { return cli->executeAndRead(cmd); })
                        .thenValue([=](std::string result) { return returnAndExecNext(result); });

                {   // synchronized insert block
                    std::lock_guard<std::mutex> lock(mutex);
                    empty = outstandingCmds.empty();
                    outstandingCmds.push(std::move(p));
                }

                if (empty) {
                    LOG(INFO) << this << ": Executing...\n";
                    outstandingCmds.front().setValue("GOGOGO");
                } else {
                    LOG(INFO) << this << ": Enqueued (queue size: " << outstandingCmds.size() << ")...\n";
                }

                return f2;
            }

            // THREAD SAFETY: there should only one command beinbg processed at time, so
            // there should be no race on returnAndExecNext
            folly::Future<string> QueuedCli::returnAndExecNext(std::string result) {
                LOG(INFO) << this << ": returnAndExecNext '" << result << "'\n";

                outstandingCmds.pop();
                if (!ready && outstandingCmds.size() <= lo_limit) {
                    LOG(INFO) << this << ": queue size reached LO-limit(" << lo_limit << ") -> RELEASE\n";
                    notify();
                }

                if (outstandingCmds.empty()) {
                    LOG(INFO) << this << ": Queue empty\n";
                } else {
                    LOG(INFO) << this << ": Executing Next...\n";
                    outstandingCmds.front().setValue("GOGOGO");
                }
                return folly::Future<std::string>(result);
            }
        }
    }
}