// Copyright (c) 2019-present, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant
// of patent rights can be found in the PATENTS file in the same directory.

#include <devmand/channels/cli/PromptAwareCli.h>
#include <devmand/channels/cli/SshSessionAsync.h>
#include <devmand/channels/cli/Command.h>
#include <folly/executors/IOThreadPoolExecutor.h>

using devmand::channels::cli::PromptAwareCli;
using devmand::channels::cli::PromptResolver;
using devmand::channels::cli::CliInitializer;
using devmand::channels::cli::SshSessionAsync;
using devmand::channels::cli::Command;
using std::string;

namespace devmand {
    namespace channels {
        namespace cli {

            void PromptAwareCli::resolvePrompt(PromptResolver resolver) {
                this->prompt = resolver.resolvePrompt(session, cliFlavour.newline);
            }

            void PromptAwareCli::initializeCli(CliInitializer initializer) {
                initializer.initialize(session);
            }

            folly::Future<string> PromptAwareCli::executeAndRead(const Command &cmd) const {
                const string &command = cmd.toString();
                return session->write(command)
                        .thenValue([=](...) {
                            return session->readUntilOutput(command);
                        }).thenValue([=](...) {
                            return session->write(cliFlavour.newline);
                        }).thenValue([=](...) {
                            return session->readUntilOutput(prompt);;
                        });
            }

            PromptAwareCli::PromptAwareCli(shared_ptr<SshSessionAsync> _session, CliFlavour _cliFlavour)
                    : session(_session), cliFlavour(_cliFlavour) {}

            void PromptAwareCli::init(const string hostname, const int port, const string username, const string password) {
                session->openShell(hostname, port, username, password).get();
            }
        }
    }
}