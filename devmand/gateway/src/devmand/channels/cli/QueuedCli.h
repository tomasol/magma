// Copyright (c) 2019-present, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant
// of patent rights can be found in the PATENTS file in the same directory.

#pragma once

#include <devmand/channels/cli/Cli.h>
#include <folly/futures/Future.h>
#include <queue>
#include <mutex>

namespace devmand {
    namespace channels {
        namespace cli {

            class QueuedCli : public Cli {  // TODO: check thread safety
            private:
                std::shared_ptr<Cli> cli;
                std::queue<folly::Promise<std::string>> outstandingCmds;    // TODO: check q size limit
                std::mutex mutex;

            public:
                QueuedCli(std::shared_ptr<Cli> cli);

                folly::Future<std::string> executeAndRead(const Command& cmd) const override;

                folly::Future<std::string> executeAndSwitchPrompt(const Command& cmd) const override {
                    LOG(ERROR) << "[" << this << "] " << "Not Implemented\n";
                    return folly::Future<std::string>(cmd.toString());
                }

                folly::Future<std::string> test(const Command &cmd);
                folly::Future<std::string> returnAndExecNext(std::string result);
            };


        } // namespace cli
    } // namespace channels
} // namespace devmand
